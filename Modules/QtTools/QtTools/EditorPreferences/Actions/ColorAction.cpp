#include "Base/Introspection.h"
#include "QtTools/EditorPreferences/Actions/ColorAction.h"
#include "Preferences/PreferencesStorage.h"
#include "QtTools/Utils/Utils.h"
#include <QColorDialog>
#include <QApplication>

ColorAction::ColorAction(const DAVA::InspMember* member_, QObject* parent)
    : AbstractAction(member_, parent)
{
}

void ColorAction::OnTriggered(bool /*triggered*/)
{
    if (type != DAVA::VariantType::TYPE_COLOR)
    {
        DVASSERT(false && "bad type passed to factory");
        return;
    }
    QColor currentValue = data().value<QColor>();

    QColor color = QColorDialog::getColor(currentValue, qApp->activeWindow(), "Select color", QColorDialog::ShowAlphaChannel | QColorDialog::DontUseNativeDialog);
    if (!color.isValid())
    {
        return;
    }
    DAVA::VariantType newValueVar(QColorToColor(color));
    PreferencesStorage::Instance()->SetValue(member, newValueVar);
}

void ColorAction::OnValueChanged(const DAVA::VariantType& value)
{
    if (value.type != DAVA::VariantType::TYPE_COLOR)
    {
        DVASSERT(false && "unknown type passed to IntAction");
        return;
    }

    QColor color = ColorToQColor(value.AsColor());
    setIcon(CreateIconFromColor(color));
    setData(color);
}
