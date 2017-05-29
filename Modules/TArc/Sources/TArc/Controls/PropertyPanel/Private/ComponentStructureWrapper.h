#pragma once

#include <Reflection/Reflection.h>

namespace DAVA
{
class StructureWrapperClass;

namespace TArc
{
class ComponentStructureWrapper : public StructureWrapper
{
public:
    ComponentStructureWrapper(const Type* type);
    ~ComponentStructureWrapper();

    bool HasFields(const ReflectedObject& object, const ValueWrapper* vw) const override;
    Reflection GetField(const ReflectedObject& obj, const ValueWrapper* vw, const Any& key) const override;
    size_t GetFieldsCount(const ReflectedObject& object, const ValueWrapper* vw) const override;
    Vector<Reflection::Field> GetFields(const ReflectedObject& obj, const ValueWrapper* vw) const override;
    Vector<Reflection::Field> GetFields(const ReflectedObject& obj, const ValueWrapper* vw, size_t, size_t) const override;
    bool HasMethods(const ReflectedObject& object, const ValueWrapper* vw) const override;
    AnyFn GetMethod(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const override;
    Vector<Reflection::Method> GetMethods(const ReflectedObject& object, const ValueWrapper* vw) const override;
    const Reflection::FieldCaps& GetFieldsCaps(const ReflectedObject& object, const ValueWrapper* vw) const override;
    AnyFn GetFieldCreator(const ReflectedObject& object, const ValueWrapper* vw) const override;
    bool AddField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key, const Any& value) const override;
    bool InsertField(const ReflectedObject& object, const ValueWrapper* vw, const Any& beforeKey, const Any& key, const Any& value) const override;
    bool RemoveField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const override;
    void Update() override;

private:
    std::unique_ptr<StructureWrapperClass> classWrapper;
};

template <typename T>
std::unique_ptr<StructureWrapper> CreateComponentStructureWrapper()
{
    return std::make_unique<ComponentStructureWrapper>(Type::Instance<T>());
}

} // namespace TArc
} // namespace DAVA