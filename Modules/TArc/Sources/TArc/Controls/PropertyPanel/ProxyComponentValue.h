#pragma once

#include "TArc/Controls/PropertyPanel/BaseComponentValue.h"
#include "TArc/Controls/PropertyPanel/StaticEditorDrawer.h"
#include "TArc/Controls/PropertyPanel/ValueCompositor.h"

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
namespace TArc
{
template <typename TStaticEditor, typename TValueCompositor>
class ProxyComponentValue : public BaseComponentValue
{
public:
    ProxyComponentValue()
    {
        static_assert(std::is_base_of<StaticEditorDrawer, TStaticEditor>::value, "TStaticEditor should be derived from StaticEditorDrawer");
        static_assert(std::is_base_of<ValueCompositor, TValueCompositor>::value, "TValueCompositor should be derived from ValueCompositor");
    }

protected:
    Any GetValue() const override
    {
        return valueCompositor.Compose(nodes);
    }

    bool IsValidValueToSet(const Any& value) const override
    {
        return valueCompositor.IsValidValue(value);
    }

    const StaticEditorDrawer* GetStaticEditorDrawer() const override
    {
        return &staticEditor;
    }

private:
    TStaticEditor staticEditor;
    TValueCompositor valueCompositor;

    using TThis = ProxyComponentValue<TStaticEditor, TValueCompositor>;

    DAVA_VIRTUAL_REFLECTION_INPLACE(TThis, BaseComponentValue)
    {
        DAVA::ReflectionRegistrator<TThis>::Begin()
        .End();
    }
};

} // namespace TArc
} // namespace DAVA