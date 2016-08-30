#if 0
#include "ComponentProvider.h"

NGTLayer::ComponentProvider::ComponentProvider(wgt::IDefinitionManager& defMng)
    : definitionManager(defMng)
{
}

const char* NGTLayer::ComponentProvider::componentId(const wgt::TypeId& typeId, std::function<bool(size_t)>& predicate) const
{
    if (typeId.isPointer())
    {
        return "string";
    }

    return nullptr;
}
#endif //0
