#include "Base/Introspection.h"
#include "QtTools/EditorPreferences/Actions/IntAction.h"
#include "Preferences/PreferencesStorage.h"
#include <QInputDialog>
#include <QApplication>

IntAction::IntAction(const DAVA::InspMember* member_, QObject* parent)
    : AbstractAction(member_, parent)
{
}

void IntAction::OnTriggered(bool /*triggered*/)
{
    int currentValue = data().toInt();
    int newVal = QInputDialog::getInt(qApp->activeWindow(), QString("enter new value"), QString("enter new value of %s").arg(QString(member->Desc().text)), currentValue);
    DAVA::VariantType newValueVar;
    switch (type)
    {
    case DAVA::VariantType::TYPE_INT8:
        newValueVar.SetInt8(newVal);
        break;
    case DAVA::VariantType::TYPE_UINT8:
        newValueVar.SetUInt8(newVal);
        break;
    case DAVA::VariantType::TYPE_INT16:
        newValueVar.SetInt16(newVal);
        break;
    case DAVA::VariantType::TYPE_UINT16:
        newValueVar.SetUInt16(newVal);
        break;
    case DAVA::VariantType::TYPE_INT32:
        newValueVar.SetInt32(newVal);
        break;
    case DAVA::VariantType::TYPE_UINT32:
        newValueVar.SetUInt32(newVal);
        break;
    case DAVA::VariantType::TYPE_INT64:
        newValueVar.SetInt64(newVal);
        break;
    case DAVA::VariantType::TYPE_UINT64:
        newValueVar.SetUInt64(newVal);
        break;
    default:
        DVASSERT(false && "bad type passed to factory");
        return;
    }
    PreferencesStorage::Instance()->SetValue(member, newValueVar);
}

void IntAction::OnValueChanged(const DAVA::VariantType& value)
{
    int newVal = 0;
    switch (value.type)
    {
    case DAVA::VariantType::TYPE_INT8:
        newVal = value.AsInt8();
        break;
    case DAVA::VariantType::TYPE_UINT8:
        newVal = value.AsUInt8();
        break;
    case DAVA::VariantType::TYPE_INT16:
        newVal = value.AsInt16();
        break;
    case DAVA::VariantType::TYPE_UINT16:
        newVal = value.AsUInt16();
        break;
    case DAVA::VariantType::TYPE_INT32:
        newVal = value.AsInt32();
        break;
    case DAVA::VariantType::TYPE_UINT32:
        newVal = value.AsUInt32();
        break;
    case DAVA::VariantType::TYPE_INT64:
        newVal = value.AsInt64();
        break;
    case DAVA::VariantType::TYPE_UINT64:
        newVal = value.AsInt64();
        break;
    default:
        DVASSERT(false && "unknown type passed to IntAction");
        return;
    }
    setText(member->Desc().text + QString(" : %1").arg(newVal));
    setData(QVariant(newVal));
}
