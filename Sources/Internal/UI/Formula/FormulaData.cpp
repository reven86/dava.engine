 #include "FormulaData.h"

#include "UI/Formula/FormulaError.h"
#include "UI/Formula/FormulaDataStructureWrapper.h"
#include "Debug/DVAssert.h"

#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
using std::shared_ptr;
using std::make_shared;

    
FormulaDataVector::FormulaDataVector()
{
    
}

FormulaDataVector::~FormulaDataVector()
{
    
}

bool FormulaDataVector::IsEmpty() const
{
    return vector.empty();
}

void FormulaDataVector::Add(const Any &value)
{
    vector.push_back(value);
}

Any &FormulaDataVector::Get(size_t index)
{
    if (index >= vector.size())
    {
        throw FormulaCalculationError("Array out of bound");
    }
    return vector.at(index);
}

size_t FormulaDataVector::GetCount() const
{
    return vector.size();
}


FormulaDataMap::FormulaDataMap()
{
    
}

FormulaDataMap::~FormulaDataMap()
{
    
}

bool FormulaDataMap::IsEmpty() const
{
    return map.empty();
}

Any &FormulaDataMap::Find(const String &key)
{
    auto it = map.find(key);
    if (it != map.end())
    {
        return it->second;
    }
    return empty;
}

void FormulaDataMap::Add(const String &key, const Any &value)
{
    map[key] = value;
    orderedKeys.push_back(key);
}

const Vector<String> &FormulaDataMap::GetOrderedKeys() const
{
    return orderedKeys;
}

DAVA_VIRTUAL_REFLECTION_IMPL(FormulaDataVector)
{
    ReflectionRegistrator<FormulaDataVector>::Begin(std::make_unique<FormulaDataVectorStructureWrapper>())
    .End();
}

DAVA_VIRTUAL_REFLECTION_IMPL(FormulaDataMap)
{
    ReflectionRegistrator<FormulaDataMap>::Begin(std::make_unique<FormulaDataMapStructureWrapper>())
    .End();
}

}
