#include "PropertiesModel.h"

#include "Modules/DocumentsModule/DocumentData.h"

#include "Model/ControlProperties/AbstractProperty.h"
#include "Model/ControlProperties/RootProperty.h"
#include "Model/ControlProperties/SectionProperty.h"
#include "Model/ControlProperties/StyleSheetRootProperty.h"
#include "Model/ControlProperties/StyleSheetProperty.h"
#include "Model/ControlProperties/SubValueProperty.h"
#include "Model/ControlProperties/ValueProperty.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/StyleSheetNode.h"
#include "Utils/QtDavaConvertion.h"
#include "Utils/StringFormat.h"
#include "QECommands/ChangePropertyValueCommand.h"
#include "QECommands/ChangeStylePropertyCommand.h"

#include <TArc/Core/ContextAccessor.h>
#include <TArc/Core/FieldBinder.h>
#include <TArc/DataProcessing/DataContext.h>

#include <QtTools/Utils/Themes/Themes.h>
#include <QtTools/Utils/Utils.h>

#include <Reflection/ReflectedTypeDB.h>
#include <UI/UIControl.h>
#include <UI/UIControlSystem.h>

#include <QFont>
#include <QVector2D>
#include <QVector4D>

using namespace DAVA;

PropertiesModel::PropertiesModel(QObject* parent)
    : QAbstractItemModel(parent)
    , propertiesUpdater(500)
    , nodeUpdater(300)
{
    propertiesUpdater.SetUpdater(MakeFunction(this, &PropertiesModel::UpdateAllChangedProperties));
    nodeUpdater.SetUpdater(MakeFunction(this, &PropertiesModel::ResetInternal));
    nodeUpdater.SetStopper([this]() { return nodeToReset == nullptr; });

    UIControlSystem::Instance()->GetStyleSheetSystem()->SetListener(this);
}

PropertiesModel::~PropertiesModel()
{
    UIControlSystem::Instance()->GetStyleSheetSystem()->SetListener(nullptr);

    CleanUp();
    propertiesUpdater.Abort();
    nodeUpdater.Abort();
}

void PropertiesModel::SetAccessor(DAVA::TArc::ContextAccessor* accessor_)
{
    accessor = accessor_;
    BindFields();
}

void PropertiesModel::Reset(PackageBaseNode* node_)
{
    nodeToReset = node_;

    nodeUpdater.Update();
}

void PropertiesModel::ResetInternal()
{
    propertiesUpdater.Abort();
    beginResetModel();
    CleanUp();
    controlNode = dynamic_cast<ControlNode*>(nodeToReset);
    if (nullptr != controlNode)
    {
        controlNode->GetRootProperty()->AddListener(this);
        rootProperty = controlNode->GetRootProperty();
    }

    styleSheet = dynamic_cast<StyleSheetNode*>(nodeToReset);
    if (nullptr != styleSheet)
    {
        styleSheet->GetRootProperty()->AddListener(this);
        rootProperty = styleSheet->GetRootProperty();
    }
    endResetModel();
}

QModelIndex PropertiesModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    if (!parent.isValid())
        return createIndex(row, column, rootProperty->GetProperty(row));

    AbstractProperty* property = static_cast<AbstractProperty*>(parent.internalPointer());
    return createIndex(row, column, property->GetProperty(row));
}

QModelIndex PropertiesModel::parent(const QModelIndex& child) const
{
    if (!child.isValid())
        return QModelIndex();

    AbstractProperty* property = static_cast<AbstractProperty*>(child.internalPointer());
    AbstractProperty* parent = property->GetParent();

    if (parent == nullptr || parent == rootProperty)
        return QModelIndex();

    if (parent->GetParent())
        return createIndex(parent->GetParent()->GetIndex(parent), 0, parent);
    else
        return createIndex(0, 0, parent);
}

int PropertiesModel::rowCount(const QModelIndex& parent) const
{
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        return rootProperty ? rootProperty->GetCount() : 0;

    return static_cast<AbstractProperty*>(parent.internalPointer())->GetCount();
}

int PropertiesModel::columnCount(const QModelIndex&) const
{
    return 2;
}

QVariant PropertiesModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    AbstractProperty* property = static_cast<AbstractProperty*>(index.internalPointer());
    DAVA::Any value = property->GetValue();
    uint32 flags = property->GetFlags();
    switch (role)
    {
    case Qt::CheckStateRole:
    {
        if (value.CanGet<bool>() && index.column() == 1)
            return value.Get<bool>() ? Qt::Checked : Qt::Unchecked;
    }
    break;

    case Qt::DisplayRole:
    {
        if (index.column() == 0)
            return QVariant(property->GetName().c_str());
        else if (index.column() == 1)
        {
            QString res = makeQVariant(property);

            StyleSheetProperty* p = dynamic_cast<StyleSheetProperty*>(property);
            if (p && p->HasTransition())
            {
                const char* interp = GlobalEnumMap<Interpolation::FuncType>::Instance()->ToString(p->GetTransitionFunction());
                res += QString(" (") + QVariant(p->GetTransitionTime()).toString() + " sec., " + interp + ")";
            }
            return res;
        }
    }
    break;

    case Qt::ToolTipRole:
    {
        if (index.column() == 0)
            return QVariant(property->GetName().c_str());

        return makeQVariant(property);
    }

    case Qt::EditRole:
    {
        QVariant var;
        if (index.column() != 0)
        {
            var.setValue<Any>(value);
        }
        return var;
    }

    case Qt::BackgroundRole:
        if (property->GetType() == AbstractProperty::TYPE_HEADER)
        {
            return Themes::GetViewLineAlternateColor();
        }
        break;

    case Qt::FontRole:
    {
        if (property->IsOverriddenLocally() || property->IsReadOnly())
        {
            QFont myFont;
            // We should set font family manually, to set familyResolved flag in font.
            // If we don't do this, Qt will get resolve family almost randomly
            myFont.setFamily(myFont.family());
            myFont.setBold(property->IsOverriddenLocally());
            myFont.setItalic(property->IsReadOnly());
            return myFont;
        }
    }
    break;

    case Qt::TextColorRole:
    {
        if (property->IsOverriddenLocally() || property->IsReadOnly())
        {
            return Themes::GetChangedPropertyColor();
        }
        if (controlNode)
        {
            int32 propertyIndex = property->GetStylePropertyIndex();
            if (propertyIndex != -1)
            {
                bool setByStyle = controlNode->GetControl()->GetStyledPropertySet().test(propertyIndex);
                if (setByStyle)
                {
                    return Themes::GetStyleSheetNodeColor();
                }
            }
        }
        if (flags & AbstractProperty::EF_INHERITED)
        {
            return Themes::GetPrototypeColor();
        }
    }
    }

    return QVariant();
}

bool PropertiesModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid())
        return false;

    AbstractProperty* property = static_cast<AbstractProperty*>(index.internalPointer());
    if (property->IsReadOnly())
        return false;

    switch (role)
    {
    case Qt::CheckStateRole:
    {
        if (property->GetValueType() == Type::Instance<bool>())
        {
            Any newVal(value != Qt::Unchecked);
            ChangeProperty(property, newVal);
            UpdateProperty(property);
            return true;
        }
    }
    break;
    case Qt::EditRole:
    {
        Any newVal;

        if (value.canConvert<Any>())
        {
            newVal = value.value<Any>();
        }
        else
        {
            newVal = property->GetValue();
            initAny(newVal, value);
        }

        ChangeProperty(property, newVal);
        UpdateProperty(property);
        return true;
    }

    case ResetRole:
    {
        ResetProperty(property);
        UpdateProperty(property);
        return true;
    }
    }
    return false;
}

Qt::ItemFlags PropertiesModel::flags(const QModelIndex& index) const
{
    if (index.column() != 1)
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    AbstractProperty* prop = static_cast<AbstractProperty*>(index.internalPointer());
    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled;
    AbstractProperty::ePropertyType propType = prop->GetType();
    if (!prop->IsReadOnly() && (propType == AbstractProperty::TYPE_ENUM || propType == AbstractProperty::TYPE_FLAGS || propType == AbstractProperty::TYPE_VARIANT))
        flags |= Qt::ItemIsEditable;
    return flags;
}

QVariant PropertiesModel::headerData(int section, Qt::Orientation /*orientation*/, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (section == 0)
            return "Property";
        else
            return "Value";
    }
    return QVariant();
}

const AbstractProperty* PropertiesModel::GetRootProperty() const
{
    return rootProperty;
}

void PropertiesModel::UpdateAllChangedProperties()
{
    for (auto property : changedProperties)
    {
        UpdateProperty(property.Get());
    }
    changedProperties.clear();
}

void PropertiesModel::PropertyChanged(AbstractProperty* property)
{
    changedProperties.insert(RefPtr<AbstractProperty>::ConstructWithRetain(property));
    propertiesUpdater.Update();
}

void PropertiesModel::UpdateProperty(AbstractProperty* property)
{
    QPersistentModelIndex nameIndex = indexByProperty(property, 0);
    QPersistentModelIndex valueIndex = nameIndex.sibling(nameIndex.row(), 1);
    if (nameIndex.isValid() && valueIndex.isValid())
        emit dataChanged(nameIndex, valueIndex, QVector<int>() << Qt::DisplayRole);
}

void PropertiesModel::ComponentPropertiesWillBeAdded(RootProperty* root, ComponentPropertiesSection* section, int index)
{
    QModelIndex parentIndex = indexByProperty(root, 0);
    beginInsertRows(parentIndex, index, index);
}

void PropertiesModel::ComponentPropertiesWasAdded(RootProperty* root, ComponentPropertiesSection* section, int index)
{
    endInsertRows();
    QModelIndex modelIndex = indexByProperty(root->GetProperty(index), 0);
    emit ComponentAdded(modelIndex);
}

void PropertiesModel::ComponentPropertiesWillBeRemoved(RootProperty* root, ComponentPropertiesSection* section, int index)
{
    QModelIndex parentIndex = indexByProperty(root, 0);
    beginRemoveRows(parentIndex, index, index);
}

void PropertiesModel::ComponentPropertiesWasRemoved(RootProperty* root, ComponentPropertiesSection* section, int index)
{
    endRemoveRows();
}

void PropertiesModel::StylePropertyWillBeAdded(StyleSheetPropertiesSection* section, StyleSheetProperty* property, int index)
{
    QModelIndex parentIndex = indexByProperty(section, 0);
    beginInsertRows(parentIndex, index, index);
}

void PropertiesModel::StylePropertyWasAdded(StyleSheetPropertiesSection* section, StyleSheetProperty* property, int index)
{
    endInsertRows();
}

void PropertiesModel::StylePropertyWillBeRemoved(StyleSheetPropertiesSection* section, StyleSheetProperty* property, int index)
{
    QModelIndex parentIndex = indexByProperty(section, 0);
    beginRemoveRows(parentIndex, index, index);
}

void PropertiesModel::StylePropertyWasRemoved(StyleSheetPropertiesSection* section, StyleSheetProperty* property, int index)
{
    endRemoveRows();
}

void PropertiesModel::StyleSelectorWillBeAdded(StyleSheetSelectorsSection* section, StyleSheetSelectorProperty* property, int index)
{
    QModelIndex parentIndex = indexByProperty(section, 0);
    beginInsertRows(parentIndex, index, index);
}

void PropertiesModel::StyleSelectorWasAdded(StyleSheetSelectorsSection* section, StyleSheetSelectorProperty* property, int index)
{
    endInsertRows();
}

void PropertiesModel::StyleSelectorWillBeRemoved(StyleSheetSelectorsSection* section, StyleSheetSelectorProperty* property, int index)
{
    QModelIndex parentIndex = indexByProperty(section, 0);
    beginRemoveRows(parentIndex, index, index);
}

void PropertiesModel::StyleSelectorWasRemoved(StyleSheetSelectorsSection* section, StyleSheetSelectorProperty* property, int index)
{
    endRemoveRows();
}

void PropertiesModel::OnStylePropertyChanged(DAVA::UIControl* control, DAVA::UIComponent* component, uint32 propertyIndex)
{
    if (controlNode != nullptr && rootProperty != nullptr && controlNode->GetControl() == control)
    {
        AbstractProperty* changedProperty = rootProperty->FindPropertyByStyleIndex(static_cast<int32>(propertyIndex));
        if (changedProperty != nullptr)
        {
            PropertyChanged(changedProperty);
        }
    }
}

void PropertiesModel::ChangeProperty(AbstractProperty* property, const Any& value)
{
    DAVA::TArc::DataContext* activeContext = accessor->GetActiveContext();
    DVASSERT(activeContext != nullptr);
    DocumentData* documentData = activeContext->GetData<DocumentData>();
    DVASSERT(documentData != nullptr);

    if (nullptr != controlNode)
    {
        SubValueProperty* subValueProperty = dynamic_cast<SubValueProperty*>(property);
        if (subValueProperty)
        {
            ValueProperty* valueProperty = subValueProperty->GetValueProperty();
            Any newValue = valueProperty->ChangeValueComponent(valueProperty->GetValue(), value, subValueProperty->GetIndex());
            documentData->ExecCommand<ChangePropertyValueCommand>(controlNode, valueProperty, newValue);
        }
        else
        {
            documentData->ExecCommand<ChangePropertyValueCommand>(controlNode, property, value);
        }
    }
    else if (styleSheet)
    {
        documentData->ExecCommand<ChangeStylePropertyCommand>(styleSheet, property, value);
    }
    else
    {
        DVASSERT(false);
    }
}

void PropertiesModel::ResetProperty(AbstractProperty* property)
{
    DAVA::TArc::DataContext* activeContext = accessor->GetActiveContext();
    DVASSERT(activeContext != nullptr);
    DocumentData* documentData = activeContext->GetData<DocumentData>();
    DVASSERT(documentData != nullptr);

    if (nullptr != controlNode)
    {
        documentData->ExecCommand<ChangePropertyValueCommand>(controlNode, property, Any());
    }
    else
    {
        DVASSERT(false);
    }
}

QModelIndex PropertiesModel::indexByProperty(const AbstractProperty* property, int column)
{
    AbstractProperty* parent = property->GetParent();
    if (parent == nullptr)
        return QModelIndex();

    return createIndex(parent->GetIndex(property), column, static_cast<void*>(const_cast<AbstractProperty*>(property)));
}

QString PropertiesModel::makeQVariant(const AbstractProperty* property) const
{
    Any val = property->GetValue();

    if (property->GetType() == AbstractProperty::TYPE_ENUM)
    {
        return QString::fromStdString(property->GetEnumMap()->ToString(val.Cast<int32>()));
    }

    if (property->GetType() == AbstractProperty::TYPE_FLAGS)
    {
        int32 e = val.Get<int32>();
        QString res = "";
        int p = 0;
        while (e)
        {
            if ((e & 0x01) != 0)
            {
                if (!res.isEmpty())
                    res += " | ";
                res += QString::fromStdString(property->GetEnumMap()->ToString(1 << p));
            }
            p++;
            e >>= 1;
        }
        return res;
    }

    if (val.IsEmpty())
    {
        return QString();
    }

    if (val.CanGet<bool>())
    {
        return QString();
    }

    if (val.CanGet<int8>())
    {
        return QVariant(val.Get<int8>()).toString();
    }

    if (val.CanGet<uint8>())
    {
        return QVariant(val.Get<uint8>()).toString();
    }

    if (val.CanGet<int16>())
    {
        return QVariant(val.Get<int16>()).toString();
    }

    if (val.CanGet<uint16>())
    {
        return QVariant(val.Get<uint16>()).toString();
    }

    if (val.CanGet<int32>())
    {
        return QVariant(val.Get<int32>()).toString();
    }

    if (val.CanGet<uint32>())
    {
        return QVariant(val.Get<uint32>()).toString();
    }

    if (val.CanGet<int64>())
    {
        return QVariant(val.Get<int64>()).toString();
    }

    if (val.CanGet<uint64>())
    {
        return QVariant(val.Get<uint64>()).toString();
    }

    if (val.CanGet<float32>())
    {
        return QVariant(val.Get<float32>()).toString();
    }

    if (val.CanGet<float64>())
    {
        return QVariant(val.Get<float64>()).toString();
    }

    if (val.CanGet<String>())
    {
        return UnescapeString(StringToQString(val.Get<String>()));
    }

    if (val.CanGet<WideString>())
    {
        DVASSERT(false);
        return UnescapeString(WideStringToQString(val.Get<WideString>()));
    }

    if (val.CanGet<FastName>())
    {
        const FastName& fastName = val.Get<FastName>();
        if (fastName.IsValid())
        {
            return StringToQString(fastName.c_str());
        }
        else
        {
            return QString();
        }
    }

    if (val.CanGet<Vector2>())
    {
        Vector2 vec = val.Get<Vector2>();
        return StringToQString(Format("%g; %g", vec.x, vec.y));
    }

    if (val.CanGet<Color>())
    {
        return QColorToHex(ColorToQColor(val.Get<Color>()));
    }

    if (val.CanGet<Vector4>())
    {
        Vector4 vec = val.Get<Vector4>();
        return StringToQString(Format("%g; %g; %g; %g", vec.x, vec.y, vec.z, vec.w));
    }

    if (val.CanGet<FilePath>())
    {
        return StringToQString(val.Get<FilePath>().GetStringValue());
    }

    DVASSERT(false);
    return QString();
}

void PropertiesModel::initAny(Any& var, const QVariant& val) const
{
    if (var.IsEmpty())
    {
        // do nothing;
    }
    else if (var.CanGet<bool>())
    {
        var = val.toBool();
    }
    else if (var.CanGet<int32>())
    {
        var = val.toInt();
    }
    else if (var.CanGet<float32>())
    {
        var = val.toFloat();
    }
    else if (var.CanGet<String>())
    {
        var = val.toString().toStdString();
    }
    else if (var.CanGet<WideString>())
    {
        DVASSERT(false);
        var = QStringToWideString(val.toString());
    }
    else if (var.CanGet<FastName>())
    {
        var = FastName(val.toString().toStdString());
    }
    else if (var.CanGet<Vector2>())
    {
        QVector2D vector = val.value<QVector2D>();
        var = Vector2(vector.x(), vector.y());
    }
    else if (var.CanGet<Vector4>())
    {
        QVector4D vector = val.value<QVector4D>();
        var = Vector4(vector.x(), vector.y(), vector.z(), vector.w());
    }
    else
    {
        DVASSERT(false);
    }
}

void PropertiesModel::CleanUp()
{
    if (nullptr != controlNode)
    {
        controlNode->GetRootProperty()->RemoveListener(this);
    }
    if (nullptr != styleSheet)
    {
        styleSheet->GetRootProperty()->RemoveListener(this);
    }
    controlNode = nullptr;
    styleSheet = nullptr;
    rootProperty = nullptr;
}

void PropertiesModel::OnPackageChanged(const DAVA::Any& /*package*/)
{
    nodeUpdater.Abort();
}

void PropertiesModel::BindFields()
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    fieldBinder.reset(new FieldBinder(accessor));
    {
        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescr.fieldName = FastName(DocumentData::packagePropertyName);
        fieldBinder->BindField(fieldDescr, MakeFunction(this, &PropertiesModel::OnPackageChanged));
    }
}
