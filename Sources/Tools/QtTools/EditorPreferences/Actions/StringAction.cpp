#include "Base/Introspection.h"
#include "QtTools/EditorPreferences/Actions/StringAction.h"
#include "Preferences/PreferencesStorage.h"
#include <QInputDialog>
#include <QApplication>

StringAction::StringAction(const DAVA::InspMember* member_, QObject* parent)
    : AbstractAction(member_, parent)
{
}

void StringAction::OnTriggered(bool /*triggered*/)
{
    QString currentValue = data().toString();
    QString newVal = QInputDialog::getText(qApp->activeWindow(), QString("enter new value"), QString("enter new value of %s").arg(QString(member->Desc().text)), QLineEdit::Normal, currentValue);
    DAVA::VariantType newValueVar;
    switch (type)
    {
    case DAVA::VariantType::TYPE_STRING:
        newValueVar.SetString(newVal.toStdString());
        break;
    case DAVA::VariantType::TYPE_WIDE_STRING:
        newValueVar.SetWideString(newVal.toStdWString());
        break;
    case DAVA::VariantType::TYPE_FASTNAME:
        newValueVar.SetFastName(DAVA::FastName(newVal.toStdString()));
        break;
    case DAVA::VariantType::TYPE_FILEPATH:
        newValueVar.SetFilePath(DAVA::FilePath(newVal.toStdString()));
        break;
    default:
        DVASSERT(false && "bad type passed to factory");
        return;
    }
    PreferencesStorage::Instance()->SetValue(member, newValueVar);
}

void StringAction::OnValueChanged(const DAVA::VariantType& value)
{
    QString newVal;
    switch (value.type)
    {
    case DAVA::VariantType::TYPE_STRING:
        newVal = QString::fromStdString(value.AsString());
        break;
    case DAVA::VariantType::TYPE_WIDE_STRING:
        newVal = QString::fromStdWString(value.AsWideString());
        break;
    case DAVA::VariantType::TYPE_FASTNAME:
        newVal = QString(value.AsFastName().c_str());
        break;
    case DAVA::VariantType::TYPE_FILEPATH:
        newVal = QString::fromStdString(value.AsFilePath().GetStringValue());
        break;
    default:
        DVASSERT(false && "unknown type passed to IntAction");
        return;
    }
    setText(member->Desc().text + QString(" : %1").arg(newVal));
    setData(QVariant(newVal));
}
