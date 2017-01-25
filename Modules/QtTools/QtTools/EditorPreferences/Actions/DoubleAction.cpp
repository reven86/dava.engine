#include "Base/Introspection.h"
#include "QtTools/EditorPreferences/Actions/DoubleAction.h"
#include "Preferences/PreferencesStorage.h"
#include <QInputDialog>
#include <QApplication>

DoubleAction::DoubleAction(const DAVA::InspMember* member_, QObject* parent)
    : AbstractAction(member_, parent)
{
}

void DoubleAction::OnTriggered(bool /*triggered*/)
{
    double currentValue = data().toDouble();
    double newVal = QInputDialog::getDouble(qApp->activeWindow(), QString("enter new value"), QString("enter new value of %s").arg(QString(member->Desc().text)), currentValue);
    DAVA::VariantType newValueVar;
    switch (type)
    {
    case DAVA::VariantType::TYPE_FLOAT:
        newValueVar.SetFloat(static_cast<DAVA::float32>(newVal));
        break;
    case DAVA::VariantType::TYPE_FLOAT64:
        newValueVar.SetFloat64(newVal);
        break;
    default:
        DVASSERT(false && "bad type passed to factory");
        return;
    }
    PreferencesStorage::Instance()->SetValue(member, newValueVar);
}

void DoubleAction::OnValueChanged(const DAVA::VariantType& value)
{
    double newVal = 0.0f;
    switch (value.type)
    {
    case DAVA::VariantType::TYPE_FLOAT:
        newVal = value.AsFloat();
        break;
    case DAVA::VariantType::TYPE_FLOAT64:
        newVal = value.AsFloat64();
        break;
    default:
        DVASSERT(false && "unknown type passed to IntAction");
        return;
    }
    setText(member->Desc().text + QString(" : %1").arg(newVal));
    setData(QVariant(newVal));
}
