#include "TArc/WindowSubSystem/QtAction.h"

namespace DAVA
{
namespace TArc
{
QtAction::QtAction(ContextAccessor* accessor, QObject* parent)
    : QAction(parent)
    , fieldBinder(accessor)
{
}

QtAction::QtAction(ContextAccessor* accessor, const QString& text, QObject* parent)
    : QAction(text, parent)
    , fieldBinder(accessor)

{
}

QtAction::QtAction(ContextAccessor* accessor, const QIcon& icon, const QString& text, QObject* parent)
    : QAction(icon, text, parent)
    , fieldBinder(accessor)
{
}

void QtAction::SetStateUpdationFunction(eActionState state, const FieldDescriptor& fieldDescr, const Function<Any(const Any&)>& fn)
{
    DVASSERT(functorsMap.count(state) == 0);
    functorsMap.emplace(state, fn);
    fieldBinder.BindField(fieldDescr, Bind(&QtAction::OnFieldValueChanged, this, _1, state));
}

void QtAction::OnFieldValueChanged(const Any& value, eActionState state)
{
    const auto iter = functorsMap.find(state);
    DVASSERT(iter != functorsMap.end());
    Any stateResult = iter->second(value);
    switch (state)
    {
    case Enabled:
    {
        DVASSERT(stateResult.CanCast<bool>());
        bool stateEnabled = stateResult.Cast<bool>();
        if (stateEnabled != isEnabled())
        {
            setEnabled(stateEnabled);
        }
    }
    break;
    case Checked:
    {
        DVASSERT(stateResult.CanCast<bool>());
        if (isCheckable() == false)
        {
            setCheckable(true);
        }

        bool stateChecked = stateResult.Cast<bool>();
        if (stateChecked != isChecked())
        {
            setChecked(stateChecked);
        }
    }
    break;
    case Text:
        DVASSERT(stateResult.CanCast<String>());
        setText(QString::fromStdString(stateResult.Cast<String>()));
        break;
    case Icon:
        DVASSERT(stateResult.CanCast<String>());
        setIcon(QIcon(QString::fromStdString(stateResult.Cast<String>())));
    default:
        DVASSERT(false);
        break;
    }
}

} // namespace TArc
} // namespace DAVA
