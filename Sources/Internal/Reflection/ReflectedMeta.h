#pragma once
#include "Base/Type.h"

#include <map>

namespace DAVA
{
class ReflectedMeta final
{
public:
    template <typename M>
    bool HasMeta() const
    {
        // TODO:
        // ...
        return false;
    }

    template <typename M>
    M* GetMeta() const
    {
        // TODO:
        // ...
        return nullptr;
    }

    template <typename M>
    void AddMeta()
    {
        // TODO:
        // ...
    }

protected:
    std::map<const Type*, void*> metaContainer;
};

} // namespace DAVA
