#include "PackageModel.h"

#include "Modules/DocumentsModule/DocumentData.h"

#include "UI/QtModelPackageCommandExecutor.h"
#include "UI/IconHelper.h"
#include "Utils/QtDavaConvertion.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/PackageHierarchy/ImportedPackagesNode.h"
#include "Model/PackageHierarchy/ControlsContainerNode.h"
#include "Model/PackageHierarchy/StyleSheetNode.h"
#include "Model/PackageHierarchy/StyleSheetsNode.h"
#include "Model/ControlProperties/RootProperty.h"
#include "Model/ControlProperties/NameProperty.h"
#include "Model/ControlProperties/ClassProperty.h"
#include "Model/ControlProperties/CustomClassProperty.h"
#include "Model/ControlProperties/PrototypeNameProperty.h"
#include "Model/ControlProperties/VisibleValueProperty.h"
#include "Model/YamlPackageSerializer.h"

#include "QECommands/ChangePropertyValueCommand.h"

#include "PackageMimeData.h"

#include <TArc/Core/ContextAccessor.h>
#include <TArc/DataProcessing/DataContext.h>
#include <TArc/WindowSubSystem/UI.h>

#include <QtTools/Utils/Themes/Themes.h>

#include <Base/ObjectFactory.h>
#include <UI/UIControl.h>

#include <QIcon>
#include <QAction>
#include <QUrl>

using namespace DAVA;

namespace PackageModel_local
{
void SetAbsoulutePosToControlNode(PackageNode* package, ControlNode* node, ControlNode* dstNode, const DAVA::Vector2& pos)
{
    DVASSERT(nullptr != node);
    DVASSERT(nullptr != node->GetControl());
    DVASSERT(nullptr != dstNode);
    DVASSERT(nullptr != dstNode->GetControl());
    UIControl* parent = dstNode->GetControl();
    Vector2 sizeOffset = parent->GetSize() * parent->GetPivot();
    float32 angle = parent->GetAngle();
    UIGeometricData gd = parent->GetGeometricData();
    const Vector2& nodeSize = node->GetControl()->GetSize();
    sizeOffset -= nodeSize / 2;
    sizeOffset *= gd.scale;
    Vector2 controlPos = gd.position - ::Rotate(sizeOffset, angle); //top left corner of dest control
    Vector2 relativePos = pos - controlPos; //new abs pos

    //now calculate new relative pos

    Vector2 scale = gd.scale;
    if (scale.x == 0.0f || scale.y == 0.0f)
    {
        relativePos.SetZero();
    }
    else
    {
        relativePos /= scale;
    }
    relativePos = ::Rotate(relativePos, -angle);
    RootProperty* rootProperty = node->GetRootProperty();
    AbstractProperty* positionProperty = rootProperty->FindPropertyByName("position");
    DVASSERT(nullptr != positionProperty);
    Vector2 clampedRelativePos(std::floor(relativePos.x), std::floor(relativePos.y));
    package->SetControlProperty(node, positionProperty, clampedRelativePos);
}
} //PackageModel_local

PackageModel::PackageModel(QObject* parent)
    : QAbstractItemModel(parent)
{
}

PackageModel::~PackageModel() = default;

void PackageModel::SetAccessor(DAVA::TArc::ContextAccessor* accessor_)
{
    accessor = accessor_;
}

void PackageModel::SetUI(DAVA::TArc::UI* ui_)
{
    ui = ui_;
}

void PackageModel::Reset(PackageNode* package_)
{
    beginResetModel();
    if (nullptr != package)
    {
        package->RemoveListener(this);
    }
    package = package_;
    if (nullptr != package)
    {
        package->AddListener(this);
    }
    endResetModel();
}

QModelIndex PackageModel::indexByNode(PackageBaseNode* node) const
{
    PackageBaseNode* parent = node->GetParent();
    if (parent == nullptr)
    {
        return QModelIndex();
    }
    return createIndex(parent->GetIndex(node), 0, node);
}

QModelIndex PackageModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
    {
        return QModelIndex();
    }
    if (!parent.isValid())
    {
        DVASSERT(nullptr != package);
        if (nullptr != package)
        {
            return createIndex(row, column, package->Get(row));
        }
        return QModelIndex();
    }
    PackageBaseNode* node = static_cast<PackageBaseNode*>(parent.internalPointer());
    return createIndex(row, column, node->Get(row));
}

QModelIndex PackageModel::parent(const QModelIndex& child) const
{
    if (!child.isValid())
    {
        DVASSERT(false && "invalid child passed to parent function");
        return QModelIndex();
    }

    PackageBaseNode* node = static_cast<PackageBaseNode*>(child.internalPointer());
    PackageBaseNode* parent = node->GetParent();
    DVASSERT(package != nullptr);
    if (package != nullptr)
    {
        DVASSERT(nullptr != parent);
        if (nullptr == parent || parent == package.Get())
        {
            return QModelIndex();
        }
        DVASSERT(nullptr != parent->GetParent());
        return createIndex(parent->GetParent()->GetIndex(parent), 0, parent);
    }
    return QModelIndex();
}

int PackageModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid())
    {
        if (package != nullptr)
        {
            return package->GetCount();
        }
        return 0;
    }

    return static_cast<PackageBaseNode*>(parent.internalPointer())->GetCount();
}

int PackageModel::columnCount(const QModelIndex& /*parent*/) const
{
    return 1;
}

QVariant PackageModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        DVASSERT(false && "invalid index passed to data function");
        return QVariant();
    }
    PackageBaseNode* node = static_cast<PackageBaseNode*>(index.internalPointer());
    ControlNode* controlNode = dynamic_cast<ControlNode*>(node);

    if (controlNode)
    {
        switch (role)
        {
        case Qt::EditRole:
        case Qt::DisplayRole:
            return StringToQString(node->GetName());

        case Qt::DecorationRole:
            if (controlNode->GetRootProperty()->GetCustomClassProperty()->IsOverridden())
            {
                return QIcon(IconHelper::GetCustomIconPath());
            }
            else
            {
                const String& className = controlNode->GetRootProperty()->GetClassProperty()->GetClassName();
                return QIcon(IconHelper::GetIconPathForClassName(QString::fromStdString(className)));
            }

        case Qt::CheckStateRole:
        {
            auto prop = controlNode->GetRootProperty()->GetVisibleProperty();
            return prop->GetVisibleInEditor() ? Qt::Checked : Qt::Unchecked;
        }

        case Qt::ToolTipRole:
        {
            const String& prototype = controlNode->GetRootProperty()->GetPrototypeProperty()->GetPrototypeName();
            const String& className = controlNode->GetRootProperty()->GetClassProperty()->GetClassName();
            const String& customClassName = controlNode->GetRootProperty()->GetCustomClassProperty()->GetCustomClassName();

            QString toolTip;

            if (controlNode->HasErrors())
            {
                toolTip = QString::fromStdString(controlNode->GetResults().GetResultMessages());
            }
            else
            {
                toolTip = QString("class: ") + className.c_str();
                if (!customClassName.empty())
                {
                    toolTip += QString("\ncustom class: ") + customClassName.c_str();
                }

                if (controlNode->GetPrototype())
                {
                    toolTip += QString("\nprototype: ") + prototype.c_str();
                }
            }
            return toolTip;
        }

        case Qt::TextColorRole:
            if (controlNode->HasErrors())
            {
                return Themes::GetErrorColor();
            }
            else if (controlNode->GetPrototype() != nullptr)
            {
                return Themes::GetPrototypeColor();
            }
            return QVariant();

        case Qt::FontRole:
        {
            QFont myFont;
            if (controlNode->GetCreationType() == ControlNode::CREATED_FROM_PROTOTYPE)
                myFont.setBold(true);

            if (node->IsReadOnly())
                myFont.setItalic(true);

            return myFont;
        }
        }
    }
    else
    {
        StyleSheetNode* styleSheet = dynamic_cast<StyleSheetNode*>(node);
        if (styleSheet)
        {
            switch (role)
            {
            case Qt::DisplayRole:
                return StringToQString(node->GetName());

            case Qt::TextColorRole:
            {
                return Themes::GetStyleSheetNodeColor();
            }

            case Qt::FontRole:
            {
                QFont myFont;
                if (node->IsReadOnly())
                    myFont.setItalic(true);

                return myFont;
            }
            }
        }
        else
        {
            switch (role)
            {
            case Qt::DisplayRole:
                return StringToQString(node->GetName());

            case Qt::BackgroundRole:
                return Themes::GetViewLineAlternateColor();

            case Qt::TextColorRole:
                if (node->HasErrors())
                {
                    return Themes::GetErrorColor();
                }
                return QVariant();

            case Qt::ToolTipRole:
            {
                if (node->HasErrors())
                {
                    return QString::fromStdString(node->GetResults().GetResultMessages());
                }
                return QVariant();
            }
            case Qt::FontRole:
            {
                QFont myFont;
                myFont.setBold(true);

                if (node->IsReadOnly())
                    myFont.setItalic(true);

                return myFont;
            }
            }
        }
    }

    return QVariant();
}

bool PackageModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid())
    {
        DVASSERT(false && "invalid index passed to setData");
        return false;
    }
    PackageBaseNode* node = static_cast<PackageBaseNode*>(index.internalPointer());
    auto control = node->GetControl();
    if (nullptr == control)
    {
        return false;
    }
    ControlNode* controlNode = dynamic_cast<ControlNode*>(node);
    DVASSERT(controlNode);

    if (role == Qt::CheckStateRole)
    {
        auto prop = controlNode->GetRootProperty()->GetVisibleProperty();
        prop->SetVisibleInEditor(value.toBool());
        return true;
    }
    if (role == Qt::EditRole)
    {
        auto prop = controlNode->GetRootProperty()->GetNameProperty();
        const auto& newName = value.toString().toStdString();
        if (newName != node->GetName())
        {
            DAVA::TArc::DataContext* activeContext = accessor->GetActiveContext();
            DVASSERT(activeContext != nullptr);
            DocumentData* documentData = activeContext->GetData<DocumentData>();
            documentData->ExecCommand<ChangePropertyValueCommand>(controlNode, prop, Any(newName));
            return true;
        }
    }
    return false;
}

Qt::ItemFlags PackageModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }
    Qt::ItemFlags flags = QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;

    const PackageBaseNode* node = static_cast<PackageBaseNode*>(index.internalPointer());
    if (node->CanCopy())
    {
        flags |= Qt::ItemIsDragEnabled;
    }
    if (node->IsInsertingControlsSupported() || node->IsInsertingPackagesSupported() || node->IsInsertingStylesSupported())
    {
        flags |= Qt::ItemIsDropEnabled;
    }
    if (node->IsEditingSupported())
    {
        flags |= Qt::ItemIsEditable;
    }

    return flags;
}

Qt::DropActions PackageModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
}

QStringList PackageModel::mimeTypes() const
{
    QStringList types;
    types << PackageMimeData::MIME_TYPE;
    types << "text/plain";
    types << "text/uri-list";
    return types;
}

QMimeData* PackageModel::mimeData(const QModelIndexList& indices) const
{
    DVASSERT(nullptr != package);
    if (nullptr == package)
    {
        return nullptr;
    }

    PackageMimeData* mimeData = new PackageMimeData();

    SortedPackageBaseNodeSet controlNodesForCopy(CompareByLCA);
    SortedPackageBaseNodeSet styleSheetNodesForCopy(CompareByLCA);

    for (const QModelIndex& index : indices)
    {
        if (index.isValid())
        {
            PackageBaseNode* node = static_cast<PackageBaseNode*>(index.internalPointer());
            if (node->CanCopy())
            {
                if (dynamic_cast<ControlNode*>(node) != nullptr)
                {
                    controlNodesForCopy.insert(node);
                }
                else if (dynamic_cast<StyleSheetNode*>(node) != nullptr)
                {
                    styleSheetNodesForCopy.insert(node);
                }
            }
        }
    }

    for (PackageBaseNode* controlNode : controlNodesForCopy)
    {
        PackageBaseNode* parent = controlNode->GetParent();
        while (parent != nullptr && controlNodesForCopy.find(parent) == controlNodesForCopy.end())
        {
            parent = parent->GetParent();
        }
        if (parent == nullptr)
        {
            mimeData->AddControl(static_cast<ControlNode*>(controlNode));
        }
    }

    for (PackageBaseNode* styleSheetNode : controlNodesForCopy)
    {
        mimeData->AddStyle(static_cast<StyleSheetNode*>(styleSheetNode));
    }

    YamlPackageSerializer serializer;
    serializer.SerializePackageNodes(package.Get(), mimeData->GetControls(), mimeData->GetStyles());
    String str = serializer.WriteToString();
    mimeData->setText(QString::fromStdString(str));

    return mimeData;
}

int PackageModel::GetRowIndex(int row, const QModelIndex& parent) const
{
    if (row != -1)
    {
        return row;
    }
    return rowCount(parent);
}

bool PackageModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int /*column*/, const QModelIndex& parent)
{
    int rowIndex = GetRowIndex(row, parent);
    DVASSERT(rowIndex >= 0);
    PackageBaseNode* destNode = static_cast<PackageBaseNode*>(parent.internalPointer());
    OnDropMimeData(data, action, destNode, rowIndex, nullptr);
    return true; //if we can drop - we must drop. Otherwise CanDropMimeData must return false;
}

void PackageModel::OnDropMimeData(const QMimeData* data, Qt::DropAction action, PackageBaseNode* destNode, uint32 destIndex, const DAVA::Vector2* pos)
{
    DVASSERT(nullptr != package);

    ControlsContainerNode* destControlContainer = dynamic_cast<ControlsContainerNode*>(destNode);
    StyleSheetsNode* destStylesContainer = dynamic_cast<StyleSheetsNode*>(destNode);

    QtModelPackageCommandExecutor executor(accessor, ui);

    if (destControlContainer && data->hasFormat(PackageMimeData::MIME_TYPE))
    {
        const PackageMimeData* controlMimeData = DynamicTypeCheck<const PackageMimeData*>(data);

        const Vector<ControlNode*>& srcControls = controlMimeData->GetControls();
        DVASSERT(!srcControls.empty());
        Vector<ControlNode*> nodes;
        emit BeforeProcessNodes(SelectedNodes(srcControls.begin(), srcControls.end()));
        switch (action)
        {
        case Qt::CopyAction:
            nodes = executor.CopyControls(srcControls, destControlContainer, destIndex);
            break;
        case Qt::MoveAction:
            nodes = executor.MoveControls(srcControls, destControlContainer, destIndex);
            break;
        case Qt::LinkAction:
            nodes = executor.InsertInstances(srcControls, destControlContainer, destIndex);
            break;
        default:
            DVASSERT(false && "unrecognised action!");
        }
        if (pos != nullptr && destNode != package->GetPackageControlsNode())
        {
            auto destControl = dynamic_cast<ControlNode*>(destNode);
            if (destControl != nullptr)
            {
                for (const auto& node : nodes)
                {
                    PackageModel_local::SetAbsoulutePosToControlNode(package.Get(), node, destControl, *pos);
                }
            }
        }
        emit AfterProcessNodes(SelectedNodes(nodes.begin(), nodes.end()));
    }
    else if (destStylesContainer && data->hasFormat(PackageMimeData::MIME_TYPE))
    {
        const PackageMimeData* mimeData = DynamicTypeCheck<const PackageMimeData*>(data);

        const Vector<StyleSheetNode*>& srcStyles = mimeData->GetStyles();
        DVASSERT(!srcStyles.empty());

        switch (action)
        {
        case Qt::CopyAction:
            executor.CopyStyles(srcStyles, destStylesContainer, destIndex);
            break;
        case Qt::MoveAction:
            executor.MoveStyles(srcStyles, destStylesContainer, destIndex);
            break;
        default:
            DVASSERT(false && "unrecognised action!");
        }
    }
    else if (data->hasFormat("text/uri-list") && data->hasText())
    {
        QStringList list = data->text().split("\n");
        Vector<FilePath> packages;
        for (const QString& str : list)
        {
            QUrl url(str);
            if (url.isLocalFile())
            {
                packages.push_back(FilePath(url.toLocalFile().toStdString()));
            }
        }
        if (!packages.empty())
        {
            executor.AddImportedPackagesIntoPackage(packages, package.Get());
        }
    }
    else if (destNode && data->hasFormat("text/plain") && data->hasText())
    {
        String string = data->text().toStdString();
        auto nodes = executor.Paste(package.Get(), destNode, destIndex, string);
        if (pos != nullptr && destNode != package->GetPackageControlsNode())
        {
            auto destControl = dynamic_cast<ControlNode*>(destNode);
            if (destControl != nullptr)
            {
                for (const auto& node : nodes)
                {
                    auto control = dynamic_cast<ControlNode*>(node);
                    if (control != nullptr)
                    {
                        PackageModel_local::SetAbsoulutePosToControlNode(package.Get(), control, destControl, *pos);
                    }
                }
            }
        }
        emit AfterProcessNodes(SelectedNodes(nodes.begin(), nodes.end()));
    }
}

void PackageModel::ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property)
{
    if (property->GetName() == "Name")
    {
        QModelIndex index = indexByNode(node);
        emit dataChanged(index, index, QVector<int>() << Qt::DisplayRole);
    }

    else if (property->GetName() == "Custom Class")
    {
        QModelIndex index = indexByNode(node);
        emit dataChanged(index, index, QVector<int>() << Qt::DecorationRole);
    }
}

void PackageModel::StylePropertyWasChanged(StyleSheetNode* node, AbstractProperty* property)
{
    const auto& name = property->GetName();
    if (name == "Name" || name == "Selector")
    {
        QModelIndex index = indexByNode(node);
        emit dataChanged(index, index, QVector<int>() << Qt::DisplayRole);
    }
}

void PackageModel::ControlWillBeAdded(ControlNode* /*node*/, ControlsContainerNode* destination, int row)
{
    QModelIndex destIndex = indexByNode(destination);
    beginInsertRows(destIndex, row, row);
}

void PackageModel::ControlWasAdded(ControlNode* /*node*/, ControlsContainerNode* /*destination*/, int /*row*/)
{
    endInsertRows();
}

void PackageModel::ControlWillBeRemoved(ControlNode* node, ControlsContainerNode* from)
{
    QModelIndex parentIndex = indexByNode(from);
    int index = from->GetIndex(node);
    beginRemoveRows(parentIndex, index, index);
}

void PackageModel::ControlWasRemoved(ControlNode* /*node*/, ControlsContainerNode* /*from*/)
{
    endRemoveRows();
}

void PackageModel::StyleWillBeAdded(StyleSheetNode* /*node*/, StyleSheetsNode* destination, int index)
{
    QModelIndex destIndex = indexByNode(destination);
    beginInsertRows(destIndex, index, index);
}

void PackageModel::StyleWasAdded(StyleSheetNode* /*node*/, StyleSheetsNode* /*destination*/, int /*index*/)
{
    endInsertRows();
}

void PackageModel::StyleWillBeRemoved(StyleSheetNode* node, StyleSheetsNode* from)
{
    QModelIndex parentIndex = indexByNode(from);
    int index = from->GetIndex(node);
    beginRemoveRows(parentIndex, index, index);
}

void PackageModel::StyleWasRemoved(StyleSheetNode* /*node*/, StyleSheetsNode* /*from*/)
{
    endRemoveRows();
}

void PackageModel::ImportedPackageWillBeAdded(PackageNode* /*node*/, ImportedPackagesNode* to, int index)
{
    QModelIndex destIndex = indexByNode(to);
    beginInsertRows(destIndex, index, index);
}

void PackageModel::ImportedPackageWasAdded(PackageNode* /*node*/, ImportedPackagesNode* /*to*/, int /*index*/)
{
    endInsertRows();
}

void PackageModel::ImportedPackageWillBeRemoved(PackageNode* node, ImportedPackagesNode* from)
{
    QModelIndex parentIndex = indexByNode(from);
    int index = from->GetIndex(node);
    beginRemoveRows(parentIndex, index, index);
}

void PackageModel::ImportedPackageWasRemoved(PackageNode* /*node*/, ImportedPackagesNode* /*from*/)
{
    endRemoveRows();
}
