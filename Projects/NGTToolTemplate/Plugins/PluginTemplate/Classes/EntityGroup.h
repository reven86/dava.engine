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


#ifndef __ENTITY_GROUP_H__
#define __ENTITY_GROUP_H__

#include "Scene3D/Entity.h"

struct EntityGroupItem 
{
	EntityGroupItem() : entity(NULL)
	{ }

	EntityGroupItem(DAVA::Entity *_entity, DAVA::AABBox3 _bbox) 
		: entity(_entity), bbox(_bbox)
	{ }

	DAVA::Entity *entity;
	DAVA::AABBox3 bbox;
};

class EntityGroup
{
public:
	EntityGroup();
	EntityGroup(const EntityGroup &ss);
	~EntityGroup();

	void Add(DAVA::Entity *entity, DAVA::AABBox3 entityBbox = DAVA::AABBox3());
	void Add(const EntityGroupItem &groupItem);
	void Rem(DAVA::Entity *entity);
	void Clear();

	size_t Size() const;
	DAVA::Entity* GetEntity(size_t i) const;

	EntityGroupItem* GetItem(size_t i) const;

	DAVA::AABBox3 GetBbox(size_t i) const;
	void SetBbox(size_t i, const DAVA::AABBox3 &entityBbox);

    DAVA::AABBox3 GetCommonBbox() const;

	DAVA::Vector3 GetZeroPos(size_t i) const;
	DAVA::Vector3 GetCommonZeroPos() const;

	bool ContainsEntity(DAVA::Entity *entity) const;
	bool Index(DAVA::Entity *entity, size_t &index) const;

	DAVA::Entity* IntersectedEntity(const EntityGroup *group) const;

	EntityGroup& operator=(const EntityGroup &ss);
	bool operator==(const EntityGroup &ss) const;
    bool operator!=(const EntityGroup &ss) const;

protected:
	DAVA::Vector<EntityGroupItem> entities;
	DAVA::AABBox3 entitiesBbox;
};

#endif // __ENTITY_GROUP_H__
