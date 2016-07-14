#pragma once

#include "core_ui_framework/i_component_provider.hpp"

namespace wgt
{
class IDefinitionManager;
}

namespace NGTLayer
{
class ComponentProvider : public wgt::IComponentProvider
{
public:
    ComponentProvider(wgt::IDefinitionManager& defMng);
    const char* componentId(const wgt::TypeId& typeId, std::function<bool(size_t)>& predicate) const override;

private:
    wgt::IDefinitionManager& definitionManager;
};
}