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


#include "Base/Introspection.h"
#include "PreferencesModel.h"
#include "QtTools/EditorPreferences/PreferencesStorage.h"
#include "QtTools/Utils/Themes/Themes.h"
#include "Utils/QtDavaConvertion.h"

PreferencesModel::PreferencesModel(QObject* parent)
    : QStandardItemModel(parent)
{
    BuildTree();
}

QVariant PreferencesModel::data(const QModelIndex& index, int role) const
{
    QStandardItem* item = itemFromIndex(index);
    if (role == ROLE_TYPE)
    {
        return item->data(ROLE_TYPE);
    }
    auto parent = item->parent();
    switch (role)
    {
    case Qt::BackgroundRole:
        return parent == nullptr ? Themes::GetViewLineAlternateColor() : QVariant();
    default:
        return item->data(role);
    }
}

bool PreferencesModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    QStandardItem* item = itemFromIndex(index);
    item->setData(value, role);
    return true;
}

void PreferencesModel::BuildTree()
{
    setHorizontalHeaderLabels(QStringList() << tr("Property") << tr("Value"));
    const auto& registeredInsp = PreferencesStorage::GetRegisteredInsp();
    for (const DAVA::InspInfo* info : registeredInsp)
    {
        QList<QStandardItem*> items;
        QStandardItem* itemInspName = new QStandardItem(info->Name().c_str());
        itemInspName->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        itemInspName->setData(TYPE_INSP, ROLE_TYPE);
        itemInspName->setData(QVariant::fromValue(info), ROLE_POINTER);
        items << itemInspName;

        for (int i = 0, count = info->MembersCount(); i < count; ++i)
        {
            QList<QStandardItem*> memberItems;
            const DAVA::InspMember* member = info->Member(i);
            if ((member->Flags() & DAVA::I_VIEW) == 0)
            {
                continue;
            }
            bool isEditable = member->Flags() & DAVA::I_EDIT;
            QStandardItem* itemMemberName = new QStandardItem(member->Desc().text);
            itemInspName->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            itemMemberName->setData(TYPE_MEMBER, ROLE_TYPE);
            itemMemberName->setData(QVariant::fromValue(member), ROLE_POINTER);
            memberItems << itemMemberName;

            QString text = MakeDataFromVariant(member);
            QStandardItem* itemMemberValue = new QStandardItem(text);
            Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
            if (isEditable)
            {
                flags |= Qt::ItemIsEditable;
            }
            itemInspName->setFlags(flags);
            itemMemberValue->setData(TYPE_MEMBER, ROLE_TYPE);
            itemMemberValue->setData(QVariant::fromValue(member), ROLE_POINTER);
            memberItems << itemMemberValue;
            itemInspName->appendRow(memberItems);
        }

        QStandardItem* itemInspValue = new QStandardItem();
        itemInspValue->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        itemInspValue->setData(TYPE_INSP, ROLE_TYPE);
        itemInspValue->setData(QVariant::fromValue(info), ROLE_POINTER);
        items << itemInspValue;

        invisibleRootItem()->appendRow(items);
    }
}

QString PreferencesModel::MakeDataFromVariant(const DAVA::InspMember* member)
{
    DAVA::VariantType value = PreferencesStorage::GetPreferencesValue(member);
    const DAVA::InspDesc& desc = member->Desc();
    switch (value.GetType())
    {
    case DAVA::VariantType::TYPE_NONE:
        return QString();

    case DAVA::VariantType::TYPE_BOOLEAN:
        return QString();

    case DAVA::VariantType::TYPE_INT32:
        if (desc.type == DAVA::InspDesc::T_ENUM)
        {
            DAVA::int32 e = value.AsInt32();
            return QString::fromStdString(desc.enumMap->ToString(e));
        }
        else if (desc.type == DAVA::InspDesc::T_FLAGS)
        {
            DAVA::int32 e = value.AsInt32();
            QString res = "";
            int p = 0;
            while (e)
            {
                if ((e & 0x01) != 0)
                {
                    if (!res.isEmpty())
                        res += " | ";
                    res += QString::fromStdString(desc.enumMap->ToString(1 << p));
                }
                p++;
                e >>= 1;
            }
            return res;
        }
        else
        {
            return QVariant(value.AsInt32()).toString();
        }

    case DAVA::VariantType::TYPE_UINT32:
        return QVariant(value.AsUInt32()).toString();

    case DAVA::VariantType::TYPE_INT64:
        return QVariant(value.AsInt64()).toString();

    case DAVA::VariantType::TYPE_UINT64:
        return QVariant(value.AsUInt64()).toString();

    case DAVA::VariantType::TYPE_FLOAT:
        return QVariant(value.AsFloat()).toString();

    case DAVA::VariantType::TYPE_STRING:
        return QString::fromStdString(value.AsString());

    case DAVA::VariantType::TYPE_WIDE_STRING:
        return QString::fromStdWString(value.AsWideString());

    case DAVA::VariantType::TYPE_VECTOR2:
        return QString::fromStdString(DAVA::Format("%g; %g", value.AsVector2().x, value.AsVector2().y));

    case DAVA::VariantType::TYPE_COLOR:
        return QColorToHex(ColorToQColor(value.AsColor()));

    case DAVA::VariantType::TYPE_VECTOR4:
        return StringToQString(DAVA::Format("%g; %g; %g; %g", value.AsVector4().x, value.AsVector4().y, value.AsVector4().z, value.AsVector4().w));

    case DAVA::VariantType::TYPE_FILEPATH:
        return StringToQString(value.AsFilePath().GetStringValue());

    case DAVA::VariantType::TYPE_BYTE_ARRAY:
    case DAVA::VariantType::TYPE_KEYED_ARCHIVE:
    case DAVA::VariantType::TYPE_VECTOR3:

    case DAVA::VariantType::TYPE_MATRIX2:
    case DAVA::VariantType::TYPE_MATRIX3:
    case DAVA::VariantType::TYPE_MATRIX4:
    case DAVA::VariantType::TYPE_FASTNAME:
    case DAVA::VariantType::TYPE_AABBOX3:
    default:
        DVASSERT(false);
        break;
    }
    return QString();
}
