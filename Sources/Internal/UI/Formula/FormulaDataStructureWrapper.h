#pragma once

#include "Base/Any.h"
#include "Base/BaseTypes.h"
#include "Reflection/Private/Wrappers/StructureWrapperDefault.h"

namespace DAVA
{
class FormulaDataStructureWrapper : public StructureWrapperDefault
    {
    public:
        FormulaDataStructureWrapper();
        ~FormulaDataStructureWrapper() override;
        
    protected:
        Reflection CreateReflection(Any &val) const;
    };
    
    class FormulaDataVectorStructureWrapper : public FormulaDataStructureWrapper
    {
    public:
        FormulaDataVectorStructureWrapper();
        ~FormulaDataVectorStructureWrapper() override;
        
        bool HasFields(const ReflectedObject& object, const ValueWrapper* vw) const override;
        Reflection GetField(const ReflectedObject& obj, const ValueWrapper* vw, const Any& key) const override;
        Vector<Reflection::Field> GetFields(const ReflectedObject& obj, const ValueWrapper* vw) const override;
    private:
        Reflection::FieldCaps caps;
    };
    
    class FormulaDataMapStructureWrapper : public FormulaDataStructureWrapper
    {
    public:
        FormulaDataMapStructureWrapper();
        ~FormulaDataMapStructureWrapper() override;
        
        bool HasFields(const ReflectedObject& object, const ValueWrapper* vw) const override;
        Reflection GetField(const ReflectedObject& obj, const ValueWrapper* vw, const Any& key) const override;
        Vector<Reflection::Field> GetFields(const ReflectedObject& obj, const ValueWrapper* vw) const override;
    private:
        Reflection::FieldCaps caps;
    };
    
}
