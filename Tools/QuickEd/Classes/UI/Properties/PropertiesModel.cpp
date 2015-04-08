
#include "PropertiesModel.h"

#include <QPoint>
#include <QColor>
#include <QFont>
#include <QVector2D>
#include <QUndoStack>

#include "Model/ControlProperties/BaseProperty.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Utils/QtDavaConvertion.h"
#include "UI/Commands/ChangePropertyValueCommand.h"
#include "UI/Document.h"
#include "UI/QtModelPackageCommandExecutor.h"
#include "UI/PropertiesContext.h"

using namespace DAVA;

PropertiesModel::PropertiesModel(ControlNode *_controlNode, QtModelPackageCommandExecutor *_commandExecutor, QObject *parent)
    : QAbstractItemModel(parent)
    , controlNode(nullptr)
    , commandExecutor(SafeRetain(_commandExecutor))
{
    controlNode = SafeRetain(_controlNode);
}

PropertiesModel::~PropertiesModel()
{
    SafeRelease(commandExecutor);
    SafeRelease(controlNode);
}

void PropertiesModel::emitPropertyChanged(BaseProperty *property)
{
    QModelIndex nameIndex = indexByProperty(property, 0);
    QModelIndex valueIndex = nameIndex.sibling(nameIndex.row(), 1);
    emit dataChanged(nameIndex, valueIndex);
}

QModelIndex PropertiesModel::indexByProperty(BaseProperty *property, int column)
{
    BaseProperty *parent = property->GetParent();
    if (parent == NULL)
        return QModelIndex();
    
    if (parent)
        return createIndex(parent->GetIndex(property), column, property);
    else
        return createIndex(0, column, parent);
}

QModelIndex PropertiesModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();
    
    if (!parent.isValid())
        return createIndex(row, column, controlNode->GetPropertiesRoot()->GetProperty(row));
    
    BaseProperty *property = static_cast<BaseProperty*>(parent.internalPointer());
    return createIndex(row, column, property->GetProperty(row));
}

QModelIndex PropertiesModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();
    
    BaseProperty *property = static_cast<BaseProperty*>(child.internalPointer());
    BaseProperty *parent = property->GetParent();
    
    if (parent == NULL || parent == controlNode->GetPropertiesRoot())
        return QModelIndex();

    if (parent->GetParent())
        return createIndex(parent->GetParent()->GetIndex(parent), 0, parent);
    else
        return createIndex(0, 0, parent);
}

int PropertiesModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;
    
    if (!parent.isValid())
        return controlNode->GetPropertiesRoot() ? controlNode->GetPropertiesRoot()->GetCount() : 0;
    
    return static_cast<BaseProperty*>(parent.internalPointer())->GetCount();
}

int PropertiesModel::columnCount(const QModelIndex &parent) const
{
    if (!parent.isValid() || parent.internalPointer() == controlNode->GetPropertiesRoot())
        return 2;
    
    return 2;
}

QVariant PropertiesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    
    BaseProperty *property = static_cast<BaseProperty*>(index.internalPointer());
    switch (role)
    {
        case Qt::CheckStateRole:
            {
                if (property->GetValue().GetType() == VariantType::TYPE_BOOLEAN && index.column() == 1)
                    return property->GetValue().AsBool() ? Qt::Checked : Qt::Unchecked;
            }
            break;
            
        case Qt::DisplayRole:
            {
                if (index.column() == 0)
                    return QVariant(property->GetName().c_str());

                return makeQVariant(property);
            }
            break;

        case Qt::ToolTipRole:
            {
                if (index.column() == 0)
                    return QVariant(property->GetName().c_str());

                return makeQVariant(property);
            }
            break;

        case Qt::EditRole:
            {
                QVariant var;
                if (index.column() != 0)
                {
                    var.setValue<DAVA::VariantType>(property->GetValue());
                }
                return var;
            }
            break;
        case Qt::DecorationRole:
            {
                // TODO: fix
                return QVariant();
//                if (property->GetType() == VariantType::TYPE_COLOR)
//                    return ColorToQColor(property->GetValue().AsColor());
            }
            break;

        case Qt::BackgroundRole:
            return property->GetType() == BaseProperty::TYPE_HEADER ? QColor(Qt::lightGray) : QColor(Qt::white);
            
        case Qt::FontRole:
            {
                if (property->IsReplaced() || property->IsReadOnly())
                {
                    QFont myFont;
                    myFont.setBold(property->IsReplaced());
                    myFont.setItalic(property->IsReadOnly());
                    return myFont;
                }
            }
            break;
    }

    return QVariant();
}

bool PropertiesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    BaseProperty *property = static_cast<BaseProperty*>(index.internalPointer());
    if (property->IsReadOnly())
        return false;
    
    switch (role)
    {
    case Qt::CheckStateRole:
        {
            if (property->GetValue().GetType() == VariantType::TYPE_BOOLEAN)
            {
                VariantType newVal(value != Qt::Unchecked);
                commandExecutor->ChangeProperty(controlNode, property, newVal);
                return true;
            }
        }
        break;
    case Qt::EditRole:
        {
            VariantType newVal;

            if (value.userType() == QMetaTypeId<VariantType>::qt_metatype_id())
            {
                newVal = value.value<VariantType>();
            }
            else
            {
                newVal = property->GetValue();
                initVariantType(newVal, value);
            }

            commandExecutor->ChangeProperty(controlNode, property, newVal);
            return true;
        }
        break;

    case DAVA::ResetRole:
        {
            commandExecutor->ResetProperty(controlNode, property);
            return true;
        }
        break;
    }
    return false;
}

Qt::ItemFlags PropertiesModel::flags(const QModelIndex &index) const
{
    if (index.column() != 1)
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    
    BaseProperty* prop = static_cast<BaseProperty*>(index.internalPointer());
    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled;
    if (!prop->IsReadOnly() && (prop->GetType() == BaseProperty::TYPE_ENUM || prop->GetType() == BaseProperty::TYPE_FLAGS || prop->GetType() == BaseProperty::TYPE_VARIANT))
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

void PropertiesModel::AddComponentSection(ComponentPropertiesSection *section)
{
    int32 sectionRow = controlNode->GetPropertiesRoot()->GetIndexOfCompoentPropertiesSection(section);
    QModelIndex parentIndex = indexByProperty(controlNode->GetPropertiesRoot(), 0);
    beginInsertRows(parentIndex, sectionRow, sectionRow);
    controlNode->GetPropertiesRoot()->AddComponentPropertiesSection(section);
    endInsertRows();
}

void PropertiesModel::RemoveComponentSection(ComponentPropertiesSection *section)
{
    int32 sectionRow = controlNode->GetPropertiesRoot()->GetIndexOfCompoentPropertiesSection(section);
    QModelIndex parentIndex = indexByProperty(controlNode->GetPropertiesRoot(), 0);
    beginRemoveRows(parentIndex, sectionRow, sectionRow);
    controlNode->GetPropertiesRoot()->RemoveComponentPropertiesSection(section);
    endRemoveRows();
}

QVariant PropertiesModel::makeQVariant(const BaseProperty *property) const
{
    const VariantType &val = property->GetValue();
    switch (val.GetType())
    {
        case VariantType::TYPE_NONE:
            return QString();
            
        case VariantType::TYPE_BOOLEAN:
            return QString();

        case VariantType::TYPE_INT32:
            if (property->GetType() == BaseProperty::TYPE_ENUM)
            {
                int32 e = val.AsInt32();
                return QString::fromStdString(property->GetEnumMap()->ToString(e));
            }
            else if (property->GetType() == BaseProperty::TYPE_FLAGS)
            {
                int32 e = val.AsInt32();
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
            else
            {
                return QVariant(val.AsInt32()).toString();
            }

        case VariantType::TYPE_UINT32:
            return QVariant(val.AsUInt32()).toString();

        case VariantType::TYPE_FLOAT:
            return QVariant(val.AsFloat()).toString();
            
        case VariantType::TYPE_STRING:
            return StringToQString(val.AsString());
            
        case VariantType::TYPE_WIDE_STRING:
            return WideStringToQString(val.AsWideString());
            
//        case VariantType::TYPE_UINT32:
//            return val.AsUInt32();
//            
//        case VariantType::TYPE_INT64:
//            return val.AsInt64();
//            
//        case VariantType::TYPE_UINT64:
//            return val.AsUInt64();
            
        case VariantType::TYPE_VECTOR2:
            return StringToQString(Format("[%g, %g]", val.AsVector2().x, val.AsVector2().y));
            
        case VariantType::TYPE_COLOR:
            return QColorToHex(ColorToQColor(val.AsColor()));

        case VariantType::TYPE_FILEPATH:
            return StringToQString(val.AsFilePath().GetStringValue());
            
        case VariantType::TYPE_BYTE_ARRAY:
        case VariantType::TYPE_KEYED_ARCHIVE:
        case VariantType::TYPE_VECTOR3:
        case VariantType::TYPE_VECTOR4:
        case VariantType::TYPE_MATRIX2:
        case VariantType::TYPE_MATRIX3:
        case VariantType::TYPE_MATRIX4:
        case VariantType::TYPE_FASTNAME:
        case VariantType::TYPE_AABBOX3:
        default:
            DVASSERT(false);
            break;
    }
    return QString();
}

void PropertiesModel::initVariantType(DAVA::VariantType &var, const QVariant &val) const
{
    switch (var.GetType())
    {
        case VariantType::TYPE_NONE:
            break;
            
        case VariantType::TYPE_BOOLEAN:
            var.SetBool(val.toBool());
            break;
            
        case VariantType::TYPE_INT32:
            var.SetInt32(val.toInt());
            break;
            
        case VariantType::TYPE_FLOAT:
            var.SetFloat(val.toFloat());
            break;
            
        case VariantType::TYPE_STRING:
            var.SetString(val.toString().toStdString());
            break;
            
        case VariantType::TYPE_WIDE_STRING:
            var.SetWideString(QStringToWideString(val.toString()));
            break;
            
//        case VariantType::TYPE_UINT32:
//            return val.AsUInt32();
//            
//        case VariantType::TYPE_INT64:
//            return val.AsInt64();
//            
//        case VariantType::TYPE_UINT64:
//            return val.AsUInt64();
            
        case VariantType::TYPE_VECTOR2:
        {
            QVector2D vector = val.value<QVector2D>();
            var.SetVector2(DAVA::Vector2(vector.x(), vector.y()));
        }
            break;
            
        case VariantType::TYPE_COLOR:
            //return QString::fromStdString(Format("%.3f, %.3f, %.3f, %.3f", val.AsColor().a, val.AsColor().r, val.AsColor().g, val.AsColor().b));
            break;
            
//        case VariantType::TYPE_BYTE_ARRAY:
//        case VariantType::TYPE_KEYED_ARCHIVE:
//        case VariantType::TYPE_VECTOR3:
//        case VariantType::TYPE_VECTOR4:
//        case VariantType::TYPE_MATRIX2:
//        case VariantType::TYPE_MATRIX3:
//        case VariantType::TYPE_MATRIX4:
//        case VariantType::TYPE_FASTNAME:
//        case VariantType::TYPE_AABBOX3:
//        case VariantType::TYPE_FILEPATH:
        default:
            DVASSERT(false);
            break;
    }
}
