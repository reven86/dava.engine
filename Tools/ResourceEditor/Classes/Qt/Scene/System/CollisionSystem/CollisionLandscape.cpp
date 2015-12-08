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


#include "Base/AlignedAllocator.h"
#include "Qt/Scene/System/CollisionSystem/CollisionLandscape.h"
#include "Render/Highlevel/Heightmap.h"

const DAVA::int32 targetChunkSize = 64;

class CollisionLandscape::CollisionLandscapePrivate
{
public:
    struct LandscapeChunk
    {
        DAVA::Vector<DAVA::float32> data;
        btHeightfieldTerrainShape* shape = nullptr;
        btCollisionObject* object = nullptr;
    };

    void buildCollisionObject(const DAVA::Rect2i& bounds, DAVA::Heightmap* hm);

public:
    CollisionLandscape* owner = nullptr;
    DAVA::Vector<LandscapeChunk*> chunks;
};

CollisionLandscape::CollisionLandscape(DAVA::Entity* entity, btCollisionWorld* world, DAVA::Landscape* landscape)
    : CollisionBaseObject(entity, world)
{
    static_assert(sizeof(implData) >= sizeof(CollisionLandscapePrivate), "Invalid configuration, increase implData size");
    impl = new (implData) CollisionLandscapePrivate();
    impl->owner = this;

    if ((nullptr == world) || (nullptr == landscape))
        return;

    DAVA::Heightmap* heightmap = landscape->GetHeightmap();
    if ((nullptr == heightmap) || (heightmap->Size() <= 0))
        return;

    boundingBox = landscape->GetBoundingBox();

    DAVA::Vector<DAVA::Rect2i> subdivisions;

    auto hmSize = heightmap->Size();

    DAVA::int32 landcapeSubdivisionsX = hmSize / targetChunkSize;
    DAVA::int32 landcapeSubdivisionsY = hmSize / targetChunkSize;

    auto hmPartSizeX = hmSize / landcapeSubdivisionsX;
    auto hmPartSizeY = hmSize / landcapeSubdivisionsY;

    auto generateSubdivisions = [&subdivisions, &hmSize, &hmPartSizeX, &landcapeSubdivisionsX](DAVA::int32 y0, DAVA::int32 h) {
        DAVA::int32 u = 0;
        for (; u + 1 < landcapeSubdivisionsX; ++u)
        {
            subdivisions.emplace_back(u * hmPartSizeX, y0, hmPartSizeX, h);
        }
        DAVA::int32 remainingSize = hmSize - u * hmPartSizeX;
        subdivisions.emplace_back(u * hmPartSizeX, y0, remainingSize, h);
    };

    DAVA::int32 v = 0;
    for (; v + 1 < landcapeSubdivisionsY; ++v)
    {
        generateSubdivisions(v * hmPartSizeY, hmPartSizeY);
    }
    generateSubdivisions(v * hmPartSizeY, hmSize - v * hmPartSizeY);

    for (const auto& b : subdivisions)
    {
        impl->buildCollisionObject(b, heightmap);
    }
}

CollisionLandscape::~CollisionLandscape()
{
    for (auto& c : impl->chunks)
    {
        btWord->removeCollisionObject(c->object);
        DAVA::SafeDelete(c->object);
        DAVA::DestroyObjectAligned(c->shape);
        delete c;
    }
    impl->~CollisionLandscapePrivate();
}

void CollisionLandscape::CollisionLandscapePrivate::buildCollisionObject(const DAVA::Rect2i& bounds, DAVA::Heightmap* heightmap)
{
    LandscapeChunk* chunk = new LandscapeChunk();
    chunks.push_back(chunk);

    DAVA::float32 invHmSize = 1.0f / static_cast<DAVA::float32>(heightmap->Size());

    DAVA::Vector3 landSize = owner->boundingBox.max - owner->boundingBox.min;
    DAVA::float32 landScaleX = (landSize.x + 0.5f) * invHmSize;
    DAVA::float32 landScaleY = (landSize.y + 0.5f) * invHmSize;
    DAVA::float32 landScaleZ = landSize.z / static_cast<DAVA::float32>(DAVA::Heightmap::MAX_VALUE);

    DAVA::uint16* heightData = heightmap->Data();
    chunk->data.resize(bounds.dx * bounds.dy);

    DAVA::int32 k = 0;
    for (DAVA::int32 y = bounds.y; y < bounds.y + bounds.dy; ++y)
    {
        for (DAVA::int32 x = bounds.x; x < bounds.x + bounds.dx; ++x)
        {
            chunk->data[k++] = float(heightData[x + y * heightmap->Size()]) * landScaleZ;
        }
    }

    chunk->shape = DAVA::CreateObjectAligned<btHeightfieldTerrainShape, 16>(bounds.dx, bounds.dy,
                                                                            chunk->data.data(), landScaleZ, 0.0f, landSize.z, 2, PHY_FLOAT, true);
    chunk->shape->setLocalScaling(btVector3(landScaleX, landScaleY, 1.0f));

    DAVA::float32 xr = static_cast<DAVA::float32>(bounds.x) * invHmSize;
    DAVA::float32 yr = static_cast<DAVA::float32>(bounds.y) * invHmSize;
    DAVA::float32 bx = static_cast<DAVA::float32>(bounds.dx) * invHmSize;
    DAVA::float32 by = static_cast<DAVA::float32>(bounds.dy) * invHmSize;
    DAVA::float32 dx = (xr + 0.5f * bx - 0.5f) * landSize.x;
    DAVA::float32 dy = (yr + 0.5f * by - 0.5f) * landSize.y;
    DAVA::float32 dz = 0.5f * landSize.z;

    btTransform landTransform = btTransform::getIdentity();
    landTransform.setOrigin(btVector3(dx, dy, dz));

    chunk->object = new btCollisionObject();
    chunk->object->setWorldTransform(landTransform);
    chunk->object->setCollisionShape(chunk->shape);
    owner->btWord->addCollisionObject(chunk->object);
}
