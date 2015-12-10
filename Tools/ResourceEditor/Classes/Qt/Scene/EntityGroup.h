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


class EntityGroup
{
public:
    using EntityMap = DAVA::Map<DAVA::Entity*, DAVA::AABBox3>;

public:
    EntityGroup();
	EntityGroup(const EntityGroup &ss);
	~EntityGroup();

	void Add(DAVA::Entity *entity, DAVA::AABBox3 entityBbox = DAVA::AABBox3());
    void Remove(DAVA::Entity* entity);
    void Clear();

    EntityMap& GetContent();
    const EntityMap& GetContent() const;

    DAVA::AABBox3 GetCommonBbox() const;

    DAVA::Vector3 GetFirstZeroPos() const;
    DAVA::Vector3 GetCommonZeroPos() const;

	bool ContainsEntity(DAVA::Entity *entity) const;
    bool IndexOfEntity(DAVA::Entity* entity, size_t& index) const;

    DAVA::Entity* IntersectedEntity(const EntityGroup *group) const;

	EntityGroup& operator=(const EntityGroup &ss);
	bool operator==(const EntityGroup &ss) const;
    bool operator!=(const EntityGroup &ss) const;

    size_t Size() const;
    DAVA::Entity* GetFirstEntity() const;
    DAVA::Entity* GetEntitySlow(size_t i) const;
    DAVA::AABBox3 GetBboxSlow(size_t i) const;

    static DAVA::AABBox3 TransformItemBoundingBox(const EntityMap::value_type& item);

private:
    EntityMap entities;
    DAVA::AABBox3 entitiesBbox;
};

#endif // __ENTITY_GROUP_H__
