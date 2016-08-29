#pragma once
#include "Base/BaseTypes.h"
#include "Reflection/Reflection.h"
#include "Reflection/Public/Wrappers.h"
#include "Reflection/Public/ReflectedMeta.h"
#include "Reflection/Public/ReflectedType.h"
#include "Reflection/Private/StructureWrapperDefault.h"

namespace DAVA
{
class StructureWrapperClass : public StructureWrapper
{
public:
    struct ClassBase
    {
        const Type* type;
        const ReflectedType* refType;

        Type::InheritanceCastOP castToBaseOP;
        ReflectedObject GetBaseObject(const ReflectedObject& obj) const;
    };

    struct ClassField
    {
        DAVA_DEPRECATED(ClassField()) = default; // visual studio 2013 require this
        DAVA_DEPRECATED(ClassField(ClassField&& cf)) // visual studio 2013 require this
        : vw(std::move(cf.vw)), meta(std::move(cf.meta)), type(std::move(cf.type))
        {
        }

        const ReflectedType* type;

        std::unique_ptr<ValueWrapper> vw;
        std::unique_ptr<ReflectedMeta> meta;
    };

    StructureWrapperClass(const Type*);

    bool HasFields(const ReflectedObject& object, const ValueWrapper* vw) const override;
    Reflection::Field GetField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const override;
    Vector<Reflection::Field> GetFields(const ReflectedObject& object, const ValueWrapper* vw) const override;

    bool HasMethods(const ReflectedObject& object, const ValueWrapper* vw) const override;
    Reflection::Method GetMethod(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const override;
    Vector<Reflection::Method> GetMethods(const ReflectedObject& object, const ValueWrapper* vw) const override;

    template <typename T>
    void AddField(const char* fieldName, std::unique_ptr<ValueWrapper>&& vw)
    {
        ClassField clField;
        clField.type = ReflectedType::Get<T>();
        clField.vw = std::move(vw);
        fields.emplace_back(std::make_pair(fieldName, std::move(clField)));
    }

    template <typename T>
    void AddFieldFn(const char* fieldName, std::unique_ptr<ValueWrapper>&& vw)
    {
        ClassField clField;
        clField.type = FnRetTypeToFieldType<T>::Get();
        clField.vw = std::move(vw);
        fields.emplace_back(std::make_pair(fieldName, std::move(clField)));
    }

    template <typename F>
    void AddMethod(const char* methodName, const F& fn)
    {
        methods.emplace_back(std::make_pair(methodName, AnyFn(fn)));
    }

    void AddMeta(ReflectedMeta&& meta)
    {
        if (fields.size() > 0)
        {
            ClassField& clsFiled = fields.back().second;
            clsFiled.meta = std::make_unique<ReflectedMeta>(std::move(meta));
        }
    }

private:
    template <typename T>
    struct FnRetTypeToFieldType
    {
        static inline const ReflectedType* Get()
        {
            return ReflectedType::Get<std::nullptr_t>();
        }
    };

    template <typename T>
    struct FnRetTypeToFieldType<T*>
    {
        static inline const ReflectedType* Get()
        {
            return ReflectedType::Get<T>();
        }
    };

    template <typename T>
    struct FnRetTypeToFieldType<T&>
    {
        static inline const ReflectedType* Get()
        {
            return ReflectedType::Get<T>();
        }
    };

    const Type* thisType;

    Vector<std::pair<String, ClassField>> fields;
    Vector<std::pair<String, AnyFn>> methods;

    mutable bool basesInitialized = false;
    mutable Vector<ClassBase> bases;

    void InitBaseClasses() const;
};

} // namespace DAVA
