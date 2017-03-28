#pragma once

#include "Base/Any.h"
#include "Base/BaseTypes.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
    class FormulaDataStructureWrapper : public StructureWrapper
    {
    public:
        FormulaDataStructureWrapper();
        ~FormulaDataStructureWrapper() override;
        
        bool HasMethods(const ReflectedObject& object, const ValueWrapper* vw) const override;
        AnyFn GetMethod(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const override;
        Vector<Reflection::Method> GetMethods(const ReflectedObject& object, const ValueWrapper* vw) const override;
        const Reflection::FieldCaps& GetFieldsCaps(const ReflectedObject& object, const ValueWrapper* vw) const override;
        AnyFn GetFieldCreator(const ReflectedObject& object, const ValueWrapper* vw) const override;
        bool AddField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key, const Any& value) const override;
        bool InsertField(const ReflectedObject& object, const ValueWrapper* vw, const Any& beforeKey, const Any& key, const Any& value) const override;
        bool RemoveField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const override;
        void Update() override;
        
    protected:
        Reflection CreateReflection(Any &val) const;
        
    private:
        Reflection::FieldCaps caps;
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
