#pragma once

#include "Scene3D/Entity.h"
#include <type_traits>

class Selectable
{
public:
    using Object = DAVA::InspBase;

    enum class TransformPivot : DAVA::uint32
    {
        ObjectCenter,
        CommonCenter
    };

    enum class TransformType : DAVA::uint32
    {
        Disabled,
        Translation,
        Rotation,
        Scale
    };

    class TransformProxy
    {
    public:
        virtual ~TransformProxy() = default;
        virtual const DAVA::Matrix4& GetWorldTransform(Object* object) = 0;
        virtual const DAVA::Matrix4& GetLocalTransform(Object* object) = 0;
        virtual void SetLocalTransform(Object* object, const DAVA::Matrix4& matrix) = 0;
        virtual bool SupportsTransformType(Object* object, TransformType transformType) const = 0;
        virtual bool TransformDependsFromObject(Object* dependant, Object* dependsOn) const = 0;
    };

    template <typename CLASS, typename PROXY>
    static void AddTransformProxyForClass();
    static void RemoveAllTransformProxies();

public:
    Selectable() = default;
    explicit Selectable(Object* baseObject);
    Selectable(const Selectable& other);
    Selectable(Selectable&& other);

    Selectable& operator=(const Selectable& other);
    Selectable& operator=(Selectable&& other);

    bool operator==(const Selectable& other) const;
    bool operator!=(const Selectable& other) const;

    // comparing only pointers here, and not using bounding box
    // added for compatibility with sorted containers
    bool operator<(const Selectable& other) const;

    template <typename T>
    bool CanBeCastedTo() const;

    template <typename T>
    T* Cast() const;

    Object* GetContainedObject() const;
    DAVA::Entity* AsEntity() const;

    const DAVA::AABBox3& GetBoundingBox() const;
    void SetBoundingBox(const DAVA::AABBox3& box);

    bool SupportsTransformType(TransformType) const;
    const DAVA::Matrix4& GetLocalTransform() const;
    const DAVA::Matrix4& GetWorldTransform() const;
    void SetLocalTransform(const DAVA::Matrix4& transform);

    bool TransformDependsOn(const Selectable&) const;

    bool ContainsObject() const;

private:
    static void AddConcreteProxy(DAVA::MetaInfo* classInfo, TransformProxy* proxy);
    static TransformProxy* GetTransformProxyForClass(const DAVA::MetaInfo* classInfo);

private:
    Object* object = nullptr;
    DAVA::AABBox3 boundingBox;
};

template <typename T>
bool Selectable::CanBeCastedTo() const
{
    DVASSERT(object != nullptr);
    return object->GetTypeInfo()->Type() == DAVA::MetaInfo::Instance<T>();
}

template <typename T>
inline T* Selectable::Cast() const
{
    DVASSERT(object != nullptr);
    if (CanBeCastedTo<T>())
    {
        return static_cast<T*>(object);
    }
    return nullptr;
}

inline Selectable::Object* Selectable::GetContainedObject() const
{
    return object;
}

inline const DAVA::AABBox3& Selectable::GetBoundingBox() const
{
    return boundingBox;
}

inline DAVA::Entity* Selectable::AsEntity() const
{
    return Cast<DAVA::Entity>();
}

template <typename CLASS, typename PROXY>
inline void Selectable::AddTransformProxyForClass()
{
    static_assert(std::is_base_of<Selectable::TransformProxy, PROXY>::value,
                  "Transform proxy should be derived from Selectable::TransformProxy");
    AddConcreteProxy(DAVA::MetaInfo::Instance<CLASS>(), new PROXY());
}

inline bool Selectable::ContainsObject() const
{
    return object != nullptr;
}
