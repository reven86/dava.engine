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


#include "EntityGroup.h"


EntityGroup::EntityGroup()
{ }

EntityGroup::EntityGroup(const EntityGroup &ss)
{
	entities = ss.entities;
}

EntityGroup::~EntityGroup()
{ }

void EntityGroup::Add(DAVA::Entity *entity, DAVA::AABBox3 entityBbox /* = DAVA::AABBox3() */)
{
	size_t i;
	if(!Index(entity, i))
	{
		EntityGroupItem item;
		item.entity = entity;
		item.bbox = entityBbox;

		entities.push_back(item);
		entitiesBbox.AddAABBox(entityBbox);
	}
}

void EntityGroup::Add(const EntityGroupItem &groupItem)
{
	Add(groupItem.entity, groupItem.bbox);
}

void EntityGroup::Rem(DAVA::Entity *entity)
{
	for(size_t i = 0; i < entities.size(); ++i)
	{
		if(entities[i].entity == entity)
		{
			DAVA::Vector<EntityGroupItem>::iterator it = entities.begin();
			entities.erase(it + i);

			// recalc common ab
			entitiesBbox.Empty();
			for(size_t j = 0; j < entities.size(); ++j)
			{
				entitiesBbox.AddAABBox(entities[j].bbox);
			}

			break;
		}
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

DAVA::Entity* EntityGroup::GetEntity(size_t i) const
{
	DAVA::Entity *ret = NULL;

	if(i < entities.size())
	{
		ret = entities[i].entity;
	}

	return ret;
}

EntityGroupItem* EntityGroup::GetItem(size_t i) const
{
	EntityGroupItem* ret = NULL;

	if(i < entities.size())
	{
		ret = (EntityGroupItem *) &entities[i];
	}

	return ret;
}

DAVA::AABBox3 EntityGroup::GetBbox(size_t i) const
{
	DAVA::AABBox3 ret;

	if(i < entities.size())
	{
		entities[i].bbox.GetTransformedBox(entities[i].entity->GetWorldTransform(), ret);
	}

	return ret;
}

void EntityGroup::SetBbox(size_t i, const DAVA::AABBox3 &entityBbox)
{
	if(i < entities.size())
	{
        entities[i].bbox = entitiesBbox;
    }
}

DAVA::AABBox3 EntityGroup::GetCommonBbox() const
{
	DAVA::AABBox3 ret;

	for(size_t i = 0; i <entities.size(); ++i)
	{
		ret.AddAABBox(GetBbox(i));
	}

	return ret;
}

DAVA::Vector3 EntityGroup::GetZeroPos(size_t i) const
{
	DAVA::Vector3 ret;

	if(i < entities.size())
	{
		ret = entities[i].entity->GetWorldTransform().GetTranslationVector();
	}

	return ret;
}

DAVA::Vector3 EntityGroup::GetCommonZeroPos() const
{
	DAVA::Vector3 ret;

	if(entities.size() == 1)
	{
		ret = GetZeroPos(0);
	}
	else if(entities.size() > 0)
	{
		DAVA::AABBox3 tmp;

		for(size_t i = 0; i < entities.size(); ++i)
		{
			tmp.AddPoint(entities[i].entity->GetWorldTransform().GetTranslationVector());
		}

		ret = tmp.GetCenter();
	}

	return ret;
}

bool EntityGroup::ContainsEntity(DAVA::Entity *entity) const
{
	size_t i;
	return Index(entity, i);
}

EntityGroup& EntityGroup::operator=( const EntityGroup &ss )
{
	entities = ss.entities;
	return *this;
}

bool EntityGroup::operator==( const EntityGroup &ss ) const
{
	bool ret = false;

	if(entities.size() == ss.entities.size())
	{
		ret = true;

		for(size_t i = 0; i < entities.size(); ++i)
		{
			if(!ss.ContainsEntity(entities[i].entity))
			{
				ret = false;
				break;
			}
		}
	}

	return ret;
}

bool EntityGroup::operator!=( const EntityGroup &ss ) const
{
    return ((*this == ss) == false);
}


bool EntityGroup::Index(DAVA::Entity *entity, size_t &index) const
{
	for(size_t i = 0; i < entities.size(); ++i)
	{
		if(entities[i].entity == entity)
		{
			index = i;
			return true;
		}
	}

	return false;
}

DAVA::Entity* EntityGroup::IntersectedEntity(const EntityGroup *group) const
{
	DAVA::Entity* ret = NULL;

	for(size_t i = 0; i < entities.size(); ++i)
	{
		if(group->ContainsEntity(entities[i].entity))
		{
			ret = entities[i].entity;
			break;
		}
	}

	return ret;
}

