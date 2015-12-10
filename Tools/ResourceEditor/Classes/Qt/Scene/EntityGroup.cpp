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


EntityGroup::EntityGroup()
{
}

EntityGroup::EntityGroup(const EntityGroup &ss)
{
	entities = ss.entities;
}

EntityGroup::~EntityGroup()
{
}

void EntityGroup::Add(DAVA::Entity *entity, DAVA::AABBox3 entityBbox /* = DAVA::AABBox3() */)
{
    if (entities.count(entity) == 0)
    {
        entities.insert({ entity, entityBbox });
        entitiesBbox.AddAABBox(entityBbox);
	}
}

void EntityGroup::Remove(DAVA::Entity* entity)
{
    entities.erase(entity);

    entitiesBbox.Empty();
    for (const auto& item : entities)
    {
        entitiesBbox.AddAABBox(item.second);
    }
}

void EntityGroup::Clear()
{
	entities.clear();
	entitiesBbox.Empty();
}

size_t EntityGroup::Size() const
{
	return entities.size();
}

DAVA::Entity* EntityGroup::GetFirstEntity() const
{
    return entities.empty() ? nullptr : entities.begin()->first;
}

DAVA::Entity* EntityGroup::GetEntitySlow(size_t i) const
{
    size_t index = 0;
    for (const auto& item : entities)
    {
        if (index == i)
        {
            return item.first;
        }
        ++index;
    }
    return nullptr;
}

DAVA::AABBox3 EntityGroup::GetBboxSlow(size_t i) const
{
    size_t index = 0;
    for (const auto& item : entities)
    {
        if (index == i)
        {
            return TransformItemBoundingBox(item);
        }
        ++index;
    }
    return DAVA::AABBox3();
}

DAVA::AABBox3 EntityGroup::GetCommonBbox() const
{
	DAVA::AABBox3 ret;
    for (const auto& item : entities)
    {
        ret.AddAABBox(item.second);
    }
	return ret;
}

DAVA::Vector3 EntityGroup::GetFirstZeroPos() const
{
    return entities.empty() ? DAVA::Vector3(0.0f, 0.0f, 0.0f) :
                              entities.begin()->first->GetWorldTransform().GetTranslationVector();
}

DAVA::Vector3 EntityGroup::GetCommonZeroPos() const
{
	DAVA::Vector3 ret;

	if(entities.size() == 1)
	{
        ret = GetFirstZeroPos();
    }
	else if(entities.size() > 0)
	{
		DAVA::AABBox3 tmp;
        for (const auto& item : entities)
        {
            tmp.AddPoint(item.first->GetWorldTransform().GetTranslationVector());
        }

		ret = tmp.GetCenter();
	}

	return ret;
}

bool EntityGroup::ContainsEntity(DAVA::Entity *entity) const
{
    return entities.count(entity) > 0;
}

EntityGroup& EntityGroup::operator=( const EntityGroup &ss )
{
	entities = ss.entities;
	return *this;
}

bool EntityGroup::operator==( const EntityGroup &ss ) const
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

bool EntityGroup::operator!=( const EntityGroup &ss ) const
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

EntityGroup::EntityMap& EntityGroup::GetContent()
{
    return entities;
}

const EntityGroup::EntityMap& EntityGroup::GetContent() const
{
    return entities;
}

DAVA::AABBox3 EntityGroup::TransformItemBoundingBox(const EntityMap::value_type& item)
{
    DAVA::AABBox3 ret;
    item.second.GetTransformedBox(item.first->GetWorldTransform(), ret);
    return ret;
}
