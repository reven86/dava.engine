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
            if (descr.second)
                defaultControls.push_back(ControlNode::CreateFromControlWithChildren(control));
            else
                defaultControls.push_back(ControlNode::CreateFromControl(control));

            auto prop = defaultControls.back()->GetRootProperty()->FindPropertyByName("Size");

            prop->SetValue(VariantType(Vector2(32.0f, 32.0f)));
        }
        else
        {
            DVASSERT(false);
        }
    }

    defaultControlsRootItem = new QStandardItem(tr("Default controls"));
    invisibleRootItem()->appendRow(defaultControlsRootItem);
    for (ControlNode* defaultControl : defaultControls)
    {
        QString className = QString::fromStdString(defaultControl->GetControl()->GetClassName());
        auto item = new QStandardItem(QIcon(IconHelper::GetIconPathForClassName(className)), className);
        item->setData(QVariant::fromValue(static_cast<void*>(defaultControl)), POINTER_DATA);
        item->setData(className, INNER_NAME_DATA);
        item->setData(false, PROTOTYPE);
        defaultControlsRootItem->appendRow(item);
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

void LibraryModel::SetLibraryPackages(const DAVA::Vector<DAVA::FilePath>& libraryPackagePaths)
{
    for (PackageNode* package : libraryPackages)
    {
        package->Release();
    }
    libraryPackages.clear();

    for (const FilePath& path : libraryPackagePaths)
    {
        QuickEdPackageBuilder builder;
        if (UIPackageLoader().LoadPackage(path, &builder))
        {
            RefPtr<PackageNode> libraryPackage = builder.BuildPackage();
            libraryPackages.push_back(SafeRetain(libraryPackage.Get()));
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
                    resultControl = RefPtr<ControlNode>(ControlNode::CreateFromPrototype(control));
                else
                    resultControl = control;

                controls.push_back(resultControl.Get());

                YamlPackageSerializer serializer;

                serializer.SerializePackageNodes(package, controls, styles);
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
    for (QStandardItem* item : libraryRootItems)
    {
        removeRow(item->row());
    }
    libraryRootItems.clear();

    if (nullptr != controlsRootItem)
    {
        removeRow(controlsRootItem->row());
        controlsRootItem = nullptr;
    }
    if (nullptr != importedPackageRootItem)
    {
        removeRow(importedPackageRootItem->row());
        importedPackageRootItem = nullptr;
    }
    if (package != nullptr)
    {
        package->RemoveListener(this);
    }
    package = package_;
    if (package != nullptr)
    {
        package->AddListener(this);
        BuildModel();
    }
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

void LibraryModel::BuildModel()
{
    for (QStandardItem* item : libraryRootItems)
    {
        removeRow(item->row());
    }
    libraryRootItems.clear();

    int index = 0;
    for (PackageNode* package : libraryPackages)
    {
        QStandardItem* libraryRootItem = new QStandardItem(tr(package->GetName().c_str()));
        libraryRootItems.push_back(libraryRootItem);
        libraryRootItem->setData(QVariant::fromValue(static_cast<void*>(package->GetPackageControlsNode())), POINTER_DATA);
        invisibleRootItem()->insertRow(index, libraryRootItem);

        const PackageControlsNode* packageControls = package->GetPackageControlsNode();
        if (packageControls->GetCount())
        {
            for (int j = 0; j < packageControls->GetCount(); ++j)
            {
                ControlNode* controlNode = packageControls->Get(j);
                QString controlName = QString::fromStdString(controlNode->GetName());
                /*
                 auto item = new QStandardItem(QIcon(IconHelper::GetIconPathForClassName(className)), className);
                 item->setData(QVariant::fromValue(static_cast<void*>(defaultControl)), POINTER_DATA);
                 */
                QString className = QString::fromStdString(controlNode->GetControl()->GetClassName());
                auto item = new QStandardItem(QIcon(IconHelper::GetIconPathForClassName(className)), controlName);
                item->setData(QVariant::fromValue(static_cast<void*>(controlNode)), POINTER_DATA);
                item->setData(controlName, INNER_NAME_DATA);
                item->setData(false, PROTOTYPE);
                libraryRootItem->appendRow(item);
            }
        }

        index++;
    }

    CreateControlsRootItem(index);
    CreateImportPackagesRootItem(index + 1);

    DVASSERT(nullptr != package);
    if (nullptr != package)
    {
        const auto packageControls = package->GetPackageControlsNode();
        if (packageControls->GetCount())
        {
            for (int i = 0; i < packageControls->GetCount(); i++)
            {
                AddControl(packageControls->Get(i));
            }
        }
        const auto importedPackagesNode = package->GetImportedPackagesNode();
        if (importedPackagesNode->GetCount())
        {
            for (int i = 0; i < importedPackagesNode->GetCount(); ++i)
            {
                AddImportedControl(importedPackagesNode->GetImportedPackage(i));
            }
        }
    }
}

void LibraryModel::AddControl(ControlNode* node)
{
    DVASSERT(nullptr != node);
    DVASSERT(nullptr != controlsRootItem);
    QString name = QString::fromStdString(node->GetName());
    auto item = new QStandardItem(
    QIcon(IconHelper::GetCustomIconPath()),
    name
    );
    item->setData(QVariant::fromValue(static_cast<void*>(node)), POINTER_DATA);
    item->setData(name, INNER_NAME_DATA);
    item->setData(true, PROTOTYPE);
    controlsRootItem->appendRow(item);
}

void LibraryModel::AddImportedControl(PackageNode* node)
{
    DVASSERT(nullptr != node);
    DVASSERT(nullptr != importedPackageRootItem);
    QString packageName = QString::fromStdString(node->GetName());
    auto importedPackageItem = new QStandardItem(packageName);
    importedPackageItem->setData(QVariant::fromValue(static_cast<void*>(node)), POINTER_DATA);
    importedPackageItem->setData(packageName, INNER_NAME_DATA);
    importedPackageRootItem->appendRow(importedPackageItem);
    const auto controls = node->GetPackageControlsNode();
    for (int j = 0; j < controls->GetCount(); ++j)
    {
        const auto subNode = controls->Get(j);
        QString subPackageName = QString::fromStdString(subNode->GetName());
        auto item = new QStandardItem(
        QIcon(IconHelper::GetCustomIconPath()),
        subPackageName
        );
        item->setData(QVariant::fromValue(static_cast<void*>(subNode)), POINTER_DATA);
        item->setData(packageName + "/" + subPackageName, INNER_NAME_DATA);
        importedPackageItem->appendRow(item);
    }
}

void LibraryModel::CreateControlsRootItem(int row)
{
    DVASSERT(nullptr == controlsRootItem);
    DVASSERT(nullptr != package);
    if (nullptr != package)
    {
        controlsRootItem = new QStandardItem(tr("Package controls"));
        controlsRootItem->setData(QVariant::fromValue(static_cast<void*>(package->GetPackageControlsNode())), POINTER_DATA);
        invisibleRootItem()->insertRow(row, controlsRootItem);
    }
}

void LibraryModel::CreateImportPackagesRootItem(int row)
{
    DVASSERT(nullptr == importedPackageRootItem);
    DVASSERT(nullptr != package);
    if (nullptr != package)
    {
        importedPackageRootItem = new QStandardItem(tr("Importred controls"));
        importedPackageRootItem->setData(QVariant::fromValue(static_cast<void*>(package->GetImportedPackagesNode())), POINTER_DATA);
        invisibleRootItem()->insertRow(row, importedPackageRootItem);
    }
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
                auto text = QString::fromStdString(property->GetValue().AsString());
                item->setText(text);
                const auto itemParent = item->parent();
                if (itemParent == controlsRootItem || itemParent == importedPackageRootItem)
                {
                    item->setData(text, INNER_NAME_DATA);
                }
                else if (itemParent != nullptr) //control of imported package
                {
                    item->setData(itemParent->text() + "/" + text, INNER_NAME_DATA);
                }
            }
        }
    }
}

void LibraryModel::ControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int row)
{
    Q_UNUSED(destination);
    Q_UNUSED(row);
    DVASSERT(nullptr != node);

    if (controlsRootItem->data().value<void*>() == node->GetParent())
    {
        const QModelIndex destIndex = indexByNode(node, controlsRootItem); //check that we already do not have this item
        if (!destIndex.isValid())
        {
            AddControl(node);
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
    if (importedPackageRootItem->data().value<void*>() == node->GetParent())
    {
        const QModelIndex destIndex = indexByNode(node, importedPackageRootItem); //check that we already do not have this item
        if (!destIndex.isValid())
        {
            AddImportedControl(node);
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
