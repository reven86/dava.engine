#include "LibraryModel.h"

#include <QIcon>
#include <QMimeData>

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/ImportedPackagesNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/ControlProperties/RootProperty.h"
#include "Model/ControlProperties/ClassProperty.h"
#include "Model/ControlProperties/CustomClassProperty.h"
#include "Model/YamlPackageSerializer.h"
#include "Model/QuickEdPackageBuilder.h"

#include "Base/ObjectFactory.h"
#include "UI/UIControl.h"
#include "UI/UIPackageLoader.h"

#include "Utils/QtDavaConvertion.h"
#include "UI/IconHelper.h"
#include "QtTools/Utils/Themes/Themes.h"

#include <TArc/WindowSubSystem/UI.h>

using namespace DAVA;

LibraryModel::LibraryModel(QObject* parent)
    : QStandardItemModel(parent)
    , defaultControlsRootItem(nullptr)
    , controlsRootItem(nullptr)
    , importedPackageRootItem(nullptr)
{
    Vector<std::pair<String, bool>> controlDescrs =
    {
      { "UIControl", false },
      { "UIStaticText", false },
      { "UITextField", false },
      { "UISlider", true },
      { "UIList", false },
      { "UIListCell", false },
      { "UIScrollBar", true },
      { "UIScrollView", true },
      { "UISpinner", true },
      { "UISwitch", true },
      { "UIParticles", false },
      { "UIWebView", false },
      { "UIMovieView", false },
      { "UI3DView", false },
      { "UIJoypad", true }
    };

    for (std::pair<String, bool>& descr : controlDescrs)
    {
        ScopedPtr<UIControl> control(ObjectFactory::Instance()->New<UIControl>(descr.first));
        if (control)
        {
            control->SetName(descr.first);
            if (descr.second)
                defaultControls.push_back(ControlNode::CreateFromControlWithChildren(control));
            else
                defaultControls.push_back(ControlNode::CreateFromControl(control));

            AbstractProperty* prop = defaultControls.back()->GetRootProperty()->FindPropertyByName("size");

            prop->SetValue(Vector2(32.0f, 32.0f));
        }
        else
        {
            DVASSERT(false);
        }
    }

    controlsRootItem = new QStandardItem(tr("Prototypes"));
    controlsRootItem->setData(QVariant::fromValue(static_cast<void*>(nullptr)));
    invisibleRootItem()->appendRow(controlsRootItem);

    importedPackageRootItem = new QStandardItem(tr("Importred prototypes"));
    importedPackageRootItem->setData(QVariant::fromValue(static_cast<void*>(nullptr)));
    invisibleRootItem()->appendRow(importedPackageRootItem);

    defaultControlsRootItem = new QStandardItem(tr("Default controls"));
    defaultControlsRootItem->setData(QVariant::fromValue(static_cast<void*>(nullptr)));
    invisibleRootItem()->appendRow(defaultControlsRootItem);

    for (ControlNode* defaultControl : defaultControls)
    {
        AddControl(defaultControl, defaultControlsRootItem, false);
    }
}

LibraryModel::~LibraryModel()
{
    for (ControlNode* control : defaultControls)
    {
        control->Release();
    }
    defaultControls.clear();

    for (PackageNode* package : libraryPackages)
    {
        package->Release();
    }
    libraryPackages.clear();
}

void LibraryModel::SetUI(DAVA::TArc::UI* ui_)
{
    ui = ui_;
}

void LibraryModel::SetProjectLibraries(const DAVA::Map<DAVA::String, DAVA::Set<DAVA::FastName>>& prototypes_, const DAVA::Vector<DAVA::FilePath>& libraryPackages_)
{
    prototypes = prototypes_;
    libraryPackagePaths = libraryPackages_;

    for (QStandardItem* item : libraryRootItems)
    {
        removeRow(item->row());
    }
    libraryRootItems.clear();

    for (PackageNode* package : libraryPackages)
    {
        package->Release();
    }
    libraryPackages.clear();

    const EngineContext* engineContext = GetEngineContext();
    int32 index = 0;
    for (const FilePath& path : libraryPackagePaths)
    {
        QuickEdPackageBuilder builder(engineContext);
        PackageNode* package = nullptr;
        if (UIPackageLoader(prototypes).LoadPackage(path, &builder))
        {
            RefPtr<PackageNode> libraryPackage = builder.BuildPackage();
            if (builder.GetResults().HasErrors())
            {
                using namespace TArc;
                NotificationParams params;
                params.title = "Can't load library package";
                params.message.type = Result::RESULT_ERROR;
                params.message.message = Format("Package '%s' has problems...", path.GetFilename().c_str());
                ui->ShowNotification(DAVA::TArc::mainWindowKey, params);
            }
            else
            {
                package = SafeRetain(libraryPackage.Get());
                libraryPackages.push_back(package);
            }
        }

        if (package)
        {
            QStandardItem* libraryRootItem = CreatePackageControlsItem(package, false);
            libraryRootItems.push_back(libraryRootItem);
            invisibleRootItem()->insertRow(index, libraryRootItem);
            index++;
        }
    }
}

Qt::ItemFlags LibraryModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }

    QStandardItem* item = itemFromIndex(index);

    Qt::ItemFlags result = QAbstractItemModel::flags(index);
    Vector<ControlNode*> controls;
    Vector<StyleSheetNode*> styles;
    PackageBaseNode* node = static_cast<PackageBaseNode*>(item->data(POINTER_DATA).value<void*>());
    if (nullptr != node && node->GetControl() != nullptr)
    {
        result |= Qt::ItemIsDragEnabled;
    }
    return result;
}

QStringList LibraryModel::mimeTypes() const
{
    return QStringList() << "text/plain";
}

QMimeData* LibraryModel::mimeData(const QModelIndexList& indexes) const
{
    DVASSERT(nullptr != package);
    if (nullptr == package)
    {
        return nullptr;
    }

    for (const auto& index : indexes)
    {
        if (index.isValid())
        {
            QMimeData* data = new QMimeData();
            auto item = itemFromIndex(index);

            PackageBaseNode* node = static_cast<PackageBaseNode*>(item->data(POINTER_DATA).value<void*>());
            ControlNode* control = node ? dynamic_cast<ControlNode*>(node) : nullptr;
            bool makePrototype = item->data(PROTOTYPE).value<bool>();
            if (control)
            {
                Vector<ControlNode*> controls;
                Vector<StyleSheetNode*> styles;

                RefPtr<ControlNode> resultControl;
                if (makePrototype)
                {
                    resultControl = RefPtr<ControlNode>(ControlNode::CreateFromPrototype(control));
                }
                else
                {
                    resultControl = control;
                }

                controls.push_back(resultControl.Get());

                YamlPackageSerializer serializer;

                serializer.SerializePackageNodes(package.Get(), controls, styles);
                String str = serializer.WriteToString();
                data->setText(QString::fromStdString(str));

                return data;
            }
        }
    }
    return nullptr;
}

void LibraryModel::SetPackageNode(PackageNode* package_)
{
    controlsRootItem->removeRows(0, controlsRootItem->rowCount());
    importedPackageRootItem->removeRows(0, importedPackageRootItem->rowCount());

    if (package != nullptr)
    {
        package->RemoveListener(this);
    }
    package = package_;
    if (package != nullptr)
    {
        package->AddListener(this);

        ImportedPackagesNode* importedPackagesNode = package->GetImportedPackagesNode();
        for (PackageNode* package : *importedPackagesNode)
        {
            importedPackageRootItem->appendRow(CreatePackageControlsItem(package, true));
        }

        AddPackageControls(package->GetPrototypes(), controlsRootItem, true);
    }
}

QModelIndex LibraryModel::GetDefaultControlsModelIndex() const
{
    return defaultControlsRootItem->index();
}

QVariant LibraryModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::BackgroundRole)
    {
        QStandardItem* item = itemFromIndex(index);
        if (item->parent() == nullptr)
        {
            return Themes::GetViewLineAlternateColor();
        }
    }
    return QStandardItemModel::data(index, role);
}

QModelIndex LibraryModel::indexByNode(const void* node, const QStandardItem* item) const
{
    DVASSERT(nullptr != node);
    DVASSERT(nullptr != item);
    void* ptr = item->data().value<void*>();
    if (ptr == node)
    {
        return item->index();
    }
    for (auto row = item->rowCount() - 1; row >= 0; --row)
    {
        QModelIndex index = indexByNode(node, item->child(row));
        if (index.isValid())
        {
            return index;
        }
    }
    return QModelIndex();
}

void LibraryModel::AddControl(ControlNode* node, QStandardItem* rootItem, bool makePrototype)
{
    QString name = QString::fromStdString(node->GetName());
    QString iconName;
    if (makePrototype || node->GetPrototype() != nullptr)
    {
        iconName = IconHelper::GetCustomIconPath();
    }
    else
    {
        QString className = QString::fromStdString(node->GetControl()->GetClassName());
        iconName = IconHelper::GetIconPathForClassName(className);
    }
    QStandardItem* item = new QStandardItem(QIcon(iconName), name);
    item->setData(QVariant::fromValue(static_cast<void*>(node)), POINTER_DATA);
    item->setData(makePrototype, PROTOTYPE);
    rootItem->appendRow(item);
}

void LibraryModel::AddPackageControls(PackageControlsNode* packageControls, QStandardItem* rootItem, bool makePrototype)
{
    if (packageControls->GetCount())
    {
        for (ControlNode* node : *packageControls)
        {
            AddControl(node, rootItem, makePrototype);
        }
    }
}

QStandardItem* LibraryModel::CreatePackageControlsItem(PackageNode* package, bool makePrototype)
{
    QStandardItem* rootItem = new QStandardItem(QString::fromStdString(package->GetName()));
    rootItem->setData(QVariant::fromValue(static_cast<void*>(package)), POINTER_DATA);
    AddPackageControls(makePrototype ? package->GetPrototypes() : package->GetPackageControlsNode(), rootItem, makePrototype);
    return rootItem;
}

void LibraryModel::ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property)
{
    if (property->GetName() == "Name")
    {
        QModelIndex index = indexByNode(node, invisibleRootItem());
        if (index.isValid())
        {
            auto item = itemFromIndex(index);
            if (nullptr != item)
            {
                auto text = QString::fromStdString(property->GetValue().Get<String>());
                item->setText(text);
            }
        }
    }
}

void LibraryModel::ControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int row)
{
    Q_UNUSED(destination);
    Q_UNUSED(row);
    DVASSERT(nullptr != node);

    if (package->GetPrototypes() == node->GetParent())
    {
        const QModelIndex destIndex = indexByNode(node, controlsRootItem); //check that we already do not have this item
        if (!destIndex.isValid())
        {
            AddControl(node, controlsRootItem, true);
        }
    }
}

void LibraryModel::ControlWillBeRemoved(ControlNode* node, ControlsContainerNode* from)
{
    Q_UNUSED(from);
    DVASSERT(nullptr != node);
    DVASSERT(nullptr != controlsRootItem);

    QModelIndex index = indexByNode(node, controlsRootItem);
    if (index.isValid())
    {
        removeRow(index.row(), index.parent());
    }
}

void LibraryModel::ImportedPackageWasAdded(PackageNode* node, ImportedPackagesNode* to, int index)
{
    Q_UNUSED(to);
    Q_UNUSED(index);
    DVASSERT(nullptr != node);
    if (package->GetImportedPackagesNode() == node->GetParent())
    {
        const QModelIndex destIndex = indexByNode(node, importedPackageRootItem); //check that we already do not have this item
        if (!destIndex.isValid())
        {
            importedPackageRootItem->appendRow(CreatePackageControlsItem(node, true));
        }
    }
}

void LibraryModel::ImportedPackageWillBeRemoved(PackageNode* node, ImportedPackagesNode* from)
{
    Q_UNUSED(from);
    DVASSERT(nullptr != node);
    DVASSERT(nullptr != importedPackageRootItem);

    QModelIndex parentIndex = indexByNode(node, importedPackageRootItem);
    if (parentIndex.isValid())
    {
        removeRow(parentIndex.row(), parentIndex.parent());
    }
}
