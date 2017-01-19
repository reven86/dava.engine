#pragma once

#include "Base/BaseTypes.h"
#include "Base/Any.h"
#include "Functional/Function.h"

#include "TArc/Core/FieldBinder.h"
#include "TArc/DataProcessing/Common.h"

#include <QAction>

namespace DAVA
{
namespace TArc
{
class QtAction : public QAction
{
public:
    QtAction(ContextAccessor* accessor, QObject* parent = nullptr);
    QtAction(ContextAccessor* accessor, const QString& text, QObject* parent = nullptr);
    QtAction(ContextAccessor* accessor, const QIcon& icon, const QString& text, QObject* parent = nullptr);

    enum eActionState
    {
        Enabled, // call back should return Any that can be casted to bool
        Checked,
        Text, // DAVA::String
        Icon // DAVA::String
    };

    void SetStateUpdationFunction(eActionState state, const FieldDescriptor& fieldDescr, const Function<Any(const Any&)>& fn);

private:
    void OnFieldValueChanged(const Any& value, eActionState state);

private:
    FieldBinder fieldBinder;
    UnorderedMap<eActionState, Function<Any(const Any&)>> functorsMap;
};
} // namespace TArc
} // namespace DAVA