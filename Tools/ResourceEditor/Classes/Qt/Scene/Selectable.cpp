/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "Scene/Selectable.h"

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
