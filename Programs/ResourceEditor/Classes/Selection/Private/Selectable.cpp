#include "Classes/Selection/Selectable.h"

Selectable::Selectable(Object* baseObject)
    : object(baseObject)
{
    DVASSERT(object != nullptr);
}

Selectable::Selectable(const Selectable& other)
    : object(other.object)
    , boundingBox(other.boundingBox)
{
}

Selectable::Selectable(Selectable&& other)
    : boundingBox(other.boundingBox)
{
    std::swap(object, other.object);
}

Selectable& Selectable::operator=(const Selectable& other)
{
    if (this != &other)
    {
        object = other.object;
        boundingBox = other.boundingBox;
    }
    return *this;
}

Selectable& Selectable::operator=(Selectable&& other)
{
    if (this != &other)
    {
        object = other.object;
        boundingBox = other.boundingBox;
        other.object = nullptr;
    }
    return *this;
}

bool Selectable::operator==(const Selectable& other) const
{
    return object == other.object;
}

bool Selectable::operator!=(const Selectable& other) const
{
    return object != other.object;
}

bool Selectable::operator<(const Selectable& other) const
{
    return object < other.object;
}

void Selectable::SetBoundingBox(const DAVA::AABBox3& box)
{
    boundingBox = box;
}

const DAVA::Matrix4& Selectable::GetLocalTransform() const
{
    auto proxy = GetTransformProxyForClass(object->GetTypeInfo()->Type());
    return (proxy == nullptr) ? DAVA::Matrix4::IDENTITY : proxy->GetLocalTransform(object);
}

const DAVA::Matrix4& Selectable::GetWorldTransform() const
{
    auto proxy = GetTransformProxyForClass(object->GetTypeInfo()->Type());
    return (proxy == nullptr) ? DAVA::Matrix4::IDENTITY : proxy->GetWorldTransform(object);
}

void Selectable::SetLocalTransform(const DAVA::Matrix4& transform)
{
    auto proxy = GetTransformProxyForClass(object->GetTypeInfo()->Type());
    if (proxy != nullptr)
    {
        proxy->SetLocalTransform(object, transform);
    }
}

bool Selectable::SupportsTransformType(TransformType transformType) const
{
    auto proxyClass = GetTransformProxyForClass(object->GetTypeInfo()->Type());
    return (proxyClass != nullptr) && proxyClass->SupportsTransformType(object, transformType);
}

bool Selectable::TransformDependsOn(const Selectable& other) const
{
    auto proxyClass = GetTransformProxyForClass(object->GetTypeInfo()->Type());
    return (proxyClass != nullptr) && proxyClass->TransformDependsFromObject(object, other.GetContainedObject());
}

/*
 * Transform proxy stuff
 */
static DAVA::Map<const DAVA::MetaInfo*, Selectable::TransformProxy*> transformProxies;

void Selectable::AddConcreteProxy(DAVA::MetaInfo* classInfo, Selectable::TransformProxy* proxy)
{
    DVASSERT(transformProxies.count(classInfo) == 0);
    transformProxies.emplace(classInfo, proxy);
}

Selectable::TransformProxy* Selectable::GetTransformProxyForClass(const DAVA::MetaInfo* classInfo)
{
    auto i = transformProxies.find(classInfo);
    return (i == transformProxies.end()) ? nullptr : i->second;
}

void Selectable::RemoveAllTransformProxies()
{
    for (auto& tp : transformProxies)
    {
        delete tp.second;
    }
    transformProxies.clear();
}
