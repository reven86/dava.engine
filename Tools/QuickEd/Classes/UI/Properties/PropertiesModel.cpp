/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "PropertiesModel.h"

#include <QPoint>
#include <QColor>
#include <QFont>
#include <QTimer>
#include <QVector2D>
#include <QVector4D>
#include "Document.h"
#include "Ui/QtModelPackageCommandExecutor.h"

#include "Model/ControlProperties/AbstractProperty.h"
#include "Model/ControlProperties/RootProperty.h"
#include "Model/ControlProperties/SectionProperty.h"
#include "Model/ControlProperties/StyleSheetRootProperty.h"
#include "Model/ControlProperties/StyleSheetProperty.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/StyleSheetNode.h"
#include "Utils/QtDavaConvertion.h"
#include "UI/Commands/ChangePropertyValueCommand.h"
#include "UI/QtModelPackageCommandExecutor.h"

#include "UI/UIControl.h"

#include <chrono>

using namespace std::chrono;
using namespace DAVA;

PropertiesModel::PropertiesModel(ControlNode* _controlNode, QtModelPackageCommandExecutor* _commandExecutor, QObject* parent)
    : QAbstractItemModel(parent)
    , commandExecutor(SafeRetain(_commandExecutor))
{
    controlNode = SafeRetain(_controlNode);
    controlNode->GetRootProperty()->AddListener(this);
    rootProperty = SafeRetain(controlNode->GetRootProperty());
    Init();
}

PropertiesModel::PropertiesModel(StyleSheetNode *aStyleSheet, QtModelPackageCommandExecutor *_commandExecutor, QObject *parent)
    : QAbstractItemModel(parent)
    , commandExecutor(SafeRetain(_commandExecutor))
{
    styleSheet = SafeRetain(aStyleSheet);
    styleSheet->GetRootProperty()->AddListener(this);
    rootProperty = SafeRetain(styleSheet->GetRootProperty());
    Init();
}

PropertiesModel::~PropertiesModel()
{
    if (controlNode)
        controlNode->GetRootProperty()->RemoveListener(this);
    
    if (styleSheet)
        styleSheet->GetRootProperty()->RemoveListener(this);
    
    SafeRelease(commandExecutor);
    SafeRelease(controlNode);
    SafeRelease(rootProperty);
    SafeRelease(styleSheet);
}

void PropertiesModel::Init()
{
    updatePropertyTimer = new QTimer(this);
    updatePropertyTimer->setSingleShot(true);
    updatePropertyTimer->setInterval(30);
    connect(updatePropertyTimer, &QTimer::timeout, this, &PropertiesModel::UpdateAllChangedProperties, Qt::QueuedConnection);
}

QModelIndex PropertiesModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();
    
    if (!parent.isValid())
        return createIndex(row, column, rootProperty->GetProperty(row));
    
    AbstractProperty *property = static_cast<AbstractProperty*>(parent.internalPointer());
    return createIndex(row, column, property->GetProperty(row));
}

QModelIndex PropertiesModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();
    
    AbstractProperty *property = static_cast<AbstractProperty*>(child.internalPointer());
    AbstractProperty *parent = property->GetParent();
    
    if (parent == nullptr || parent == rootProperty)
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
        return rootProperty ? rootProperty->GetCount() : 0;
    
    return static_cast<AbstractProperty*>(parent.internalPointer())->GetCount();
}

int PropertiesModel::columnCount(const QModelIndex &parent) const
{
    return 2;
}

QVariant PropertiesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    
    AbstractProperty *property = static_cast<AbstractProperty*>(index.internalPointer());
    uint32 flags = property->GetFlags();
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
                else if (index.column() == 1)
                {
                    QString res = makeQVariant(property);
                    
                    StyleSheetProperty *p = dynamic_cast<StyleSheetProperty*>(property);
                    if (p && p->HasTransition())
                    {
                        const char *interp = GlobalEnumMap<Interpolation::FuncType>::Instance()->ToString(p->GetTransitionFunction());
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

        case Qt::BackgroundRole:
            return property->GetType() == AbstractProperty::TYPE_HEADER ? QColor(Qt::lightGray) : QColor(Qt::white);
            
        case Qt::FontRole:
            {
                if (property->IsOverriddenLocally() || property->IsReadOnly())
                {
                    QFont myFont;
                    myFont.setBold(property->IsOverriddenLocally());
                    myFont.setItalic(property->IsReadOnly());
                    return myFont;
                }
            }
            break;
            
        case Qt::TextColorRole:
        {
            if (controlNode)
            {
                int32 propertyIndex = property->GetStylePropertyIndex();
                if (propertyIndex != -1)
                {
                    bool setByStyle = controlNode->GetControl()->GetStyledPropertySet().test(propertyIndex);
                    if (setByStyle)
                        return QColor(Qt::darkGreen);
                }
            }
            return (flags & AbstractProperty::EF_INHERITED) != 0 ? QColor(Qt::blue) : QColor(Qt::black);
        }

    }

    return QVariant();
}

bool PropertiesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    AbstractProperty *property = static_cast<AbstractProperty*>(index.internalPointer());
    if (property->IsReadOnly())
        return false;
    
    switch (role)
    {
    case Qt::CheckStateRole:
        {
            if (property->GetValue().GetType() == VariantType::TYPE_BOOLEAN)
            {
                VariantType newVal(value != Qt::Unchecked);
                ChangeProperty(property, newVal);
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

            ChangeProperty(property, newVal);
            return true;
        }
        break;

    case DAVA::ResetRole:
        {
            ResetProperty(property);
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
    
    AbstractProperty* prop = static_cast<AbstractProperty*>(index.internalPointer());
    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled;
    if (!prop->IsReadOnly() && (prop->GetType() == AbstractProperty::TYPE_ENUM || prop->GetType() == AbstractProperty::TYPE_FLAGS || prop->GetType() == AbstractProperty::TYPE_VARIANT))
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

void PropertiesModel::UpdateAllChangedProperties()
{
    for (auto pair : changedIndexes)
    {
        emit dataChanged(pair.first, pair.second, QVector<int>() << Qt::DisplayRole);
    }
}

void PropertiesModel::PropertyChanged(AbstractProperty *property)
{
    QModelIndex nameIndex = indexByProperty(property, 0);
    QModelIndex valueIndex = nameIndex.sibling(nameIndex.row(), 1);
    changedIndexes.insert(qMakePair(nameIndex, valueIndex));
    updatePropertyTimer->start();
}

void PropertiesModel::ComponentPropertiesWillBeAdded(RootProperty *root, ComponentPropertiesSection *section, int index)
{
    QModelIndex parentIndex = indexByProperty(root, 0);
    beginInsertRows(parentIndex, index, index);
}

void PropertiesModel::ComponentPropertiesWasAdded(RootProperty *root, ComponentPropertiesSection *section, int index)
{
    endInsertRows();
}

void PropertiesModel::ComponentPropertiesWillBeRemoved(RootProperty *root, ComponentPropertiesSection *section, int index)
{
    QModelIndex parentIndex = indexByProperty(root, 0);
    beginRemoveRows(parentIndex, index, index);
}

void PropertiesModel::ComponentPropertiesWasRemoved(RootProperty *root, ComponentPropertiesSection *section, int index)
{
    endRemoveRows();
}

void PropertiesModel::StylePropertyWillBeAdded(StyleSheetPropertiesSection *section, StyleSheetProperty *property, int index)
{
    QModelIndex parentIndex = indexByProperty(section, 0);
    beginInsertRows(parentIndex, index, index);
}

void PropertiesModel::StylePropertyWasAdded(StyleSheetPropertiesSection *section, StyleSheetProperty *property, int index)
{
    endInsertRows();
}

void PropertiesModel::StylePropertyWillBeRemoved(StyleSheetPropertiesSection *section, StyleSheetProperty *property, int index)
{
    QModelIndex parentIndex = indexByProperty(section, 0);
    beginRemoveRows(parentIndex, index, index);
}

void PropertiesModel::StylePropertyWasRemoved(StyleSheetPropertiesSection *section, StyleSheetProperty *property, int index)
{
    endRemoveRows();
}

void PropertiesModel::StyleSelectorWillBeAdded(StyleSheetSelectorsSection *section, StyleSheetSelectorProperty *property, int index)
{
    QModelIndex parentIndex = indexByProperty(section, 0);
    beginInsertRows(parentIndex, index, index);

}

void PropertiesModel::StyleSelectorWasAdded(StyleSheetSelectorsSection *section, StyleSheetSelectorProperty *property, int index)
{
    endInsertRows();
}

void PropertiesModel::StyleSelectorWillBeRemoved(StyleSheetSelectorsSection *section, StyleSheetSelectorProperty *property, int index)
{
    QModelIndex parentIndex = indexByProperty(section, 0);
    beginRemoveRows(parentIndex, index, index);
}

void PropertiesModel::StyleSelectorWasRemoved(StyleSheetSelectorsSection *section, StyleSheetSelectorProperty *property, int index)
{
    endRemoveRows();
}

void PropertiesModel::ChangeProperty(AbstractProperty *property, const DAVA::VariantType &value)
{
    if (controlNode)
    {
        microseconds us = duration_cast<microseconds>(system_clock::now().time_since_epoch());
        size_t usCount = static_cast<size_t>(us.count());
        commandExecutor->ChangeProperty(controlNode, property, value, usCount);
    }
    else if (styleSheet)
    {
        commandExecutor->ChangeProperty(styleSheet, property, value);
    }
    else
    {
        DVASSERT(false);
    }
}

void PropertiesModel::ResetProperty(AbstractProperty *property)
{
    if (controlNode)
    {
        commandExecutor->ResetProperty(controlNode, property);
    }
    else
    {
        DVASSERT(false);
    }
}

QModelIndex PropertiesModel::indexByProperty(AbstractProperty *property, int column)
{
    AbstractProperty *parent = property->GetParent();
    if (parent == nullptr)
        return QModelIndex();
    
    return createIndex(parent->GetIndex(property), column, property);
}

QString PropertiesModel::makeQVariant(const AbstractProperty *property) const
{
    const VariantType &val = property->GetValue();
    switch (val.GetType())
    {
        case VariantType::TYPE_NONE:
            return QString();
            
        case VariantType::TYPE_BOOLEAN:
            return QString();

        case VariantType::TYPE_INT32:
            if (property->GetType() == AbstractProperty::TYPE_ENUM)
            {
                int32 e = val.AsInt32();
                return QString::fromStdString(property->GetEnumMap()->ToString(e));
            }
            else if (property->GetType() == AbstractProperty::TYPE_FLAGS)
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
            return StringToQString(Format("%g; %g", val.AsVector2().x, val.AsVector2().y));
            
        case VariantType::TYPE_COLOR:
            return QColorToHex(ColorToQColor(val.AsColor()));

        case VariantType::TYPE_VECTOR4:
            return StringToQString(Format("%g; %g; %g; %g", val.AsVector4().x, val.AsVector4().y, val.AsVector4().z, val.AsVector4().w));
            
        case VariantType::TYPE_FILEPATH:
            return StringToQString(val.AsFilePath().GetStringValue());
            
        case VariantType::TYPE_BYTE_ARRAY:
        case VariantType::TYPE_KEYED_ARCHIVE:
        case VariantType::TYPE_VECTOR3:
            
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
        case VariantType::TYPE_VECTOR4:
        {
            QVector4D vector = val.value<QVector4D>();
            var.SetVector4(DAVA::Vector4(vector.x(), vector.y(), vector.z(), vector.w()));
        }
            break;

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
