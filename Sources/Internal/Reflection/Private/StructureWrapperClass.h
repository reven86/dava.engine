#pragma once
#include "../Reflection.h"
#include "../ReflectionWrappers.h"

#include <vector>

namespace DAVA
{
class StructureWrapperClass : public StructureWrapper
{
public:
    StructureWrapperClass() = default;

    bool IsDynamic() const override
    {
        return false;
    }

    bool CanAdd() const override
    {
        return false;
    }
    bool CanInsert() const override
    {
        return false;
    }
    bool CanRemove() const override
    {
        return false;
    }

    bool AddField(const ReflectedObject& object, const Any& key, const Any& value) const override
    {
        return false;
    }
    bool InsertField(const ReflectedObject& object, const Any& key, const Any& beforeKey, const Any& value) const override
    {
        return false;
    }
    bool RemoveField(const ReflectedObject& object, const Any& key) const override
    {
        return false;
    }

    Ref::Field GetField(const ReflectedObject& object, const Any& key) const override;
    Ref::FieldsList GetFields(const ReflectedObject& object) const override;

    template <typename T>
    void AddField(const char* fieldName, std::unique_ptr<ValueWrapper>&& vw)
    {
        ClassField field;
        field.name = fieldName;
        field.vw = std::move(vw);
        field.db = ReflectionDB::GetGlobalDB<T>();
        fields.push_back(std::move(field));
    }

    template <typename T>
    void AddFieldFn(const char* fieldName, std::unique_ptr<ValueWrapper>&& vw)
    {
        ClassField field;
        field.name = fieldName;
        field.vw = std::move(vw);
        field.db = FieldFnDBGetter<T>::Get();
        fields.push_back(std::move(field));
    }

    template <typename C, typename B>
    void AddBase(const ReflectionDB* baseDb)
    {
        static_assert(std::is_base_of<B, C>::value, "C should be derived from B");

        BaseClass base;
        base.db = baseDb;
        base.castOP = [](const ReflectedObject& obj) -> ReflectedObject
        {
            C* classPtr = obj.GetPtr<C>();
            B* basePtr = static_cast<B*>(classPtr);
            return basePtr;
        };

        bases.emplace_back(std::move(base));
    }

protected:
    struct BaseClass
    {
        const ReflectionDB* db;
        ReflectedObject (*castOP)(const ReflectedObject& obj);
    };

    struct ClassField
    {
        ClassField() = default;
        ClassField(const ClassField&) = delete;

        ClassField(ClassField&& cf)
            : name(std::move(cf.name))
            , vw(std::move(cf.vw))
            , db(cf.db)
        {
        }

        std::string name;
        std::unique_ptr<ValueWrapper> vw;
        const ReflectionDB* db;
    };

    template <typename T>
    struct FieldFnDBGetter
    {
        static inline const ReflectionDB* Get()
        {
            return ReflectionDB::GetGlobalDB<std::nullptr_t>();
        }
    };

    template <typename T>
    struct FieldFnDBGetter<T*>
    {
        static inline const ReflectionDB* Get()
        {
            return ReflectionDB::GetGlobalDB<T>();
        }
    };

    template <typename T>
    struct FieldFnDBGetter<T&>
    {
        static inline const ReflectionDB* Get()
        {
            return ReflectionDB::GetGlobalDB<T>();
        }
    };

    std::vector<BaseClass> bases;
    std::vector<ClassField> fields;
};

} // namespace DAVA
