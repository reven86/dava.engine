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
    case DAVA::TArc::QtAction::Enabling:
        DVASSERT(stateResult.CanCast<bool>());
        setEnabled(stateResult.Cast<bool>());
        break;
    default:
        DVASSERT(false);
        break;
    }
}

} // namespace TArc
} // namespace DAVA
