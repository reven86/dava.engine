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


#include "Scene/EntityGroup.h"

EntityGroup::EntityGroup(DAVA::Entity* entity, const DAVA::AABBox3& entityBbox)
{
    entities.insert({ entity, entityBbox });
    entitiesBbox.AddAABBox(entityBbox);
}

EntityGroup::EntityGroup(EntityGroup&& other)
{
    entities = other.entities;
    other.entities.clear();

    entitiesBbox = other.entitiesBbox;
    other.entitiesBbox.Empty();
}

EntityGroup::EntityGroup(const EntityVector& ss)
{
    entities.insert(ss.begin(), ss.end());
}

void EntityGroup::Add(DAVA::Entity* entity, const DAVA::AABBox3& entityBbox)
{
    if (entities.insert({ entity, entityBbox }).second)
    {
        entitiesBbox.AddAABBox(entityBbox);
    }
}

void EntityGroup::Remove(DAVA::Entity* entity)
{
    entities.erase(entity);
    RebuildBoundingBox();
}

void EntityGroup::Clear()
{
    entities.clear();
    entitiesBbox.Empty();
}

DAVA::Entity* EntityGroup::GetFirstEntity() const
{
    return entities.empty() ? nullptr : entities.begin()->first;
}

DAVA::Vector3 EntityGroup::GetAnyEntityTranslationVector() const
{
    return entities.empty() ? DAVA::Vector3(0.0f, 0.0f, 0.0f) :
                              entities.begin()->first->GetWorldTransform().GetTranslationVector();
}

DAVA::Vector3 EntityGroup::GetCommonTranslationVector() const
{
    DAVA::AABBox3 tmp;
    for (const auto& item : entities)
    {
        tmp.AddPoint(item.first->GetWorldTransform().GetTranslationVector());
    }
    return tmp.GetCenter();
}

EntityGroup& EntityGroup::operator=(const EntityGroup& ss)
{
    entities = ss.entities;
    return *this;
}

bool EntityGroup::operator==(const EntityGroup& ss) const
{
    if (entities.size() != ss.entities.size())
        return false;

    for (const auto& item : entities)
    {
        if (!ss.ContainsEntity(item.first))
        {
            return false;
        }
    }

    return true;
}

bool EntityGroup::operator!=(const EntityGroup& ss) const
{
    return ((*this == ss) == false);
}

DAVA::Entity* EntityGroup::IntersectedEntity(const EntityGroup* group) const
{
    for (const auto& item : entities)
    {
        if (group->ContainsEntity(item.first))
        {
            return item.first;
        }
    }
    return nullptr;
}

DAVA::Entity* EntityGroup::IntersectedEntity(const EntityGroup::EntityVector& group) const
{
    for (const auto& item : entities)
    {
        if (std::find_if(group.begin(), group.end(),
                         [&item](const EntityVector::value_type& v) { return v.first == item.first; }) != group.end())
        {
            return item.first;
        }
    }
    return nullptr;
}

void EntityGroup::Join(const EntityGroup& group)
{
    for (const auto& item : group.GetContent())
    {
        Add(item.first, item.second);
    }
}

void EntityGroup::Exclude(const EntityGroup& group)
{
    for (const auto& item : group.GetContent())
    {
        entities.erase(item.first);
    }
    RebuildBoundingBox();
}

void EntityGroup::RebuildBoundingBox()
{
    entitiesBbox.Empty();
    for (const auto& item : entities)
    {
        entitiesBbox.AddAABBox(item.second);
    }
}

DAVA::AABBox3 EntityGroup::TransformItemBoundingBox(const EntityGroup::EntityWithBbox& item)
{
    DAVA::AABBox3 ret;
    item.second.GetTransformedBox(item.first->GetWorldTransform(), ret);
    return ret;
}

const DAVA::AABBox3& EntityGroup::GetBoundingBoxForEntity(DAVA::Entity* entity) const
{
    DVASSERT(ContainsEntity(entity));
    return entities.at(entity);
}

EntityGroup::EntityVector EntityGroup::CopyContentToVector() const
{
    EntityGroup::EntityVector result;
    result.reserve(entities.size());
    for (const auto& item : entities)
    {
        result.push_back(item);
    }
    return result;
}
