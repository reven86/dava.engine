#include "FormulaDataStructureWrapper.h"

#include "Debug/DVAssert.h"

#include "UI/Formula/FormulaData.h"

#include "Reflection/Reflection.h"
#include "Reflection/ReflectedStructure.h"
#include "Reflection/ReflectedTypeDB.h"
#include "Reflection/Private/Wrappers/StructureWrapperClass.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
    using std::shared_ptr;
    using std::make_shared;
    
    FormulaDataStructureWrapper::FormulaDataStructureWrapper()
    {
    }
    
    FormulaDataStructureWrapper::~FormulaDataStructureWrapper() = default;
    
    bool FormulaDataStructureWrapper::HasMethods(const ReflectedObject& object, const ValueWrapper* vw) const
    {
        return false;
    }
    
    AnyFn FormulaDataStructureWrapper::GetMethod(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const
    {
        return AnyFn();
    }
    
    Vector<Reflection::Method> FormulaDataStructureWrapper::GetMethods(const ReflectedObject& object, const ValueWrapper* vw) const
    {
        return Vector<Reflection::Method>();
    }
    
    const Reflection::FieldCaps& FormulaDataStructureWrapper::GetFieldsCaps(const ReflectedObject& object, const ValueWrapper* vw) const
    {
        return caps;
    }
    
    AnyFn FormulaDataStructureWrapper::GetFieldCreator(const ReflectedObject& object, const ValueWrapper* vw) const
    {
        return AnyFn();
    }
    
    bool FormulaDataStructureWrapper::AddField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key, const Any& value) const
    {
        return false;
    }
    
    bool FormulaDataStructureWrapper::InsertField(const ReflectedObject& object, const ValueWrapper* vw, const Any& beforeKey, const Any& key, const Any& value) const
    {
        return false;
    }
    
    bool FormulaDataStructureWrapper::RemoveField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const
    {
        return false;
    }
    
    Reflection FormulaDataStructureWrapper::CreateReflection(Any &val) const
    {
        if (val.IsEmpty())
        {
            return Reflection();
        }
        else if (val.CanGet<shared_ptr<FormulaDataMap>>())
        {
            return Reflection::Create(ReflectedObject(val.Get<shared_ptr<FormulaDataMap>>().get()));
        }
        else if (val.CanGet<shared_ptr<FormulaDataVector>>())
        {
            return Reflection::Create(ReflectedObject(val.Get<shared_ptr<FormulaDataVector>>().get()));
        }
        
        return Reflection::Create(&val);
    }
    
    void FormulaDataStructureWrapper::Update()
    {
        // do nothing
    }
    
    FormulaDataVectorStructureWrapper::FormulaDataVectorStructureWrapper()
    {
    }
    
    FormulaDataVectorStructureWrapper::~FormulaDataVectorStructureWrapper()
    {
    }
    
    bool FormulaDataVectorStructureWrapper::HasFields(const ReflectedObject& object, const ValueWrapper* vw) const
    {
        FormulaDataVector *data = vw->GetValueObject(object).GetPtr<FormulaDataVector>();
        return !data->IsEmpty();
    }
    
    Reflection FormulaDataVectorStructureWrapper::GetField(const ReflectedObject& obj, const ValueWrapper* vw, const Any& key) const
    {
        FormulaDataVector *data = vw->GetValueObject(obj).GetPtr<FormulaDataVector>();
        return CreateReflection(data->Get(key.Cast<size_t>()));
    }
    
    Vector<Reflection::Field> FormulaDataVectorStructureWrapper::GetFields(const ReflectedObject& obj, const ValueWrapper* vw) const
    {
        Vector<Reflection::Field> fields;
        FormulaDataVector *data = vw->GetValueObject(obj).GetPtr<FormulaDataVector>();
        
        for (size_t index = 0; index < data->GetCount(); index++)
        {
            fields.emplace_back(index, CreateReflection(data->Get(index)), nullptr);
        }
        
        return fields;
    }
    
    FormulaDataMapStructureWrapper::FormulaDataMapStructureWrapper()
    {
    }
    
    FormulaDataMapStructureWrapper::~FormulaDataMapStructureWrapper()
    {
    }
    
    bool FormulaDataMapStructureWrapper::HasFields(const ReflectedObject& object, const ValueWrapper* vw) const
    {
        FormulaDataMap *data = vw->GetValueObject(object).GetPtr<FormulaDataMap>();
        return !data->IsEmpty();
    }
    
    Reflection FormulaDataMapStructureWrapper::GetField(const ReflectedObject& obj, const ValueWrapper* vw, const Any& key) const
    {
        FormulaDataMap *data = vw->GetValueObject(obj).GetPtr<FormulaDataMap>();
        return CreateReflection(data->Find(key.Cast<String>()));
    }
    
    Vector<Reflection::Field> FormulaDataMapStructureWrapper::GetFields(const ReflectedObject& obj, const ValueWrapper* vw) const
    {
        Vector<Reflection::Field> fields;
        FormulaDataMap *data = vw->GetValueObject(obj).GetPtr<FormulaDataMap>();
        
        for (const String &key : data->GetOrderedKeys())
        {
            fields.emplace_back(key, CreateReflection(data->Find(key)), nullptr);
        }
        
        return fields;
    }
    
}
