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

DAVA_DEPRECATED(class EntityGroup)
{
public:
    using EntityWithBbox = std::pair<DAVA::Entity*, DAVA::AABBox3>;

    using EntityMap = DAVA::Map<EntityWithBbox::first_type, EntityWithBbox::second_type>;
    using EntityVector = DAVA::Vector<EntityWithBbox>;

    static DAVA::AABBox3 TransformItemBoundingBox(const EntityWithBbox& item);

public:
    EntityGroup() = default;
    EntityGroup(const EntityGroup&) = default;

    EntityGroup(const EntityVector& ss);
    EntityGroup(EntityGroup&&);
    EntityGroup(DAVA::Entity* entity, const DAVA::AABBox3& entityBbox);

    void Add(DAVA::Entity* entity, const DAVA::AABBox3& entityBbox);
    void Remove(DAVA::Entity* entity);
    void Clear();

    EntityMap& GetMutableContent();
    const EntityMap& GetContent() const;

    const DAVA::AABBox3& GetCommonBbox() const;
    const DAVA::AABBox3& GetBoundingBoxForEntity(DAVA::Entity*) const;

    DAVA::Vector3 GetAnyEntityTranslationVector() const;
    DAVA::Vector3 GetCommonTranslationVector() const;

    bool IsEmpty() const;
    bool ContainsEntity(DAVA::Entity* entity) const;

    size_t Size() const;

    DAVA::Entity* IntersectedEntity(const EntityGroup* group) const;
    DAVA::Entity* IntersectedEntity(const EntityVector& group) const;

    EntityGroup& operator=(const EntityGroup&);
    bool operator==(const EntityGroup& ss) const;
    bool operator!=(const EntityGroup& ss) const;

    DAVA::Entity* GetFirstEntity() const;
    EntityVector CopyContentToVector() const;

    void Join(const EntityGroup&);
    void Exclude(const EntityGroup&);
    void RebuildBoundingBox();

    void FilterChildrenComponents();

    template <typename Predicate>
    inline void RemoveIf(Predicate predicate);

private:
    EntityMap entities;
    DAVA::AABBox3 entitiesBbox;
};

inline const DAVA::AABBox3& EntityGroup::GetCommonBbox() const
{
    return entitiesBbox;
}

inline bool EntityGroup::ContainsEntity(DAVA::Entity* entity) const
{
    return entities.count(entity) > 0;
}

inline EntityGroup::EntityMap& EntityGroup::GetMutableContent()
{
    return entities;
}

inline const EntityGroup::EntityMap& EntityGroup::GetContent() const
{
    return entities;
}

inline bool EntityGroup::IsEmpty() const
{
    return entities.empty();
}

inline size_t EntityGroup::Size() const
{
    return entities.size();
}

template <typename Predicate>
inline void EntityGroup::RemoveIf(Predicate predicate)
{
    auto i = entities.begin();
    while (i != entities.end())
    {
        if (predicate(i->first))
        {
            i = entities.erase(i);
        }
        else
        {
            ++i;
        }
    }
    RebuildBoundingBox();
}

#endif // __ENTITY_GROUP_H__
