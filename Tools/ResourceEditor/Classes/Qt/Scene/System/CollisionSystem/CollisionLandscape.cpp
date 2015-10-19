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

using namespace DAVA;

const int targetChunkSize = 64;

class CollisionLandscape::CollisionLandscapePrivate
{
public:
    struct LandscapeChunk
    {
        Vector<float> data;
        btHeightfieldTerrainShape* shape = nullptr;
        btCollisionObject* object = nullptr;
    };

    void buildCollisionObject(const Rect2i& bounds, Heightmap* hm);

public:
    CollisionLandscape* owner = nullptr;
    Vector<LandscapeChunk*> chunks;
};

CollisionLandscape::CollisionLandscape(Entity* entity, btCollisionWorld* world, Landscape* landscape)
    : CollisionBaseObject(entity, world)
{
    static_assert(sizeof(implData) >= sizeof(CollisionLandscapePrivate), "Invalid configuration, increase implData size");
    impl = new (implData) CollisionLandscapePrivate();
    impl->owner = this;

    if ((nullptr == world) || (nullptr == landscape))
        return;

    Heightmap* heightmap = landscape->GetHeightmap();
    if ((nullptr == heightmap) || (heightmap->Size() <= 0))
        return;

    boundingBox = landscape->GetBoundingBox();

    Vector<Rect2i> subdivisions;

    auto hmSize = heightmap->Size();

    int landcapeSubdivisionsX = hmSize / targetChunkSize;
    int landcapeSubdivisionsY = hmSize / targetChunkSize;

    auto hmPartSizeX = hmSize / landcapeSubdivisionsX;
    auto hmPartSizeY = hmSize / landcapeSubdivisionsY;

    auto generateSubdivisions = [&subdivisions, &hmSize, &hmPartSizeX, &landcapeSubdivisionsX](int32 y0, int32 h) {
        int32 u = 0;
        for (; u + 1 < landcapeSubdivisionsX; ++u)
        {
            subdivisions.emplace_back(u * hmPartSizeX, y0, hmPartSizeX, h);
        }
        int32 remainingSize = hmSize - u * hmPartSizeX;
        subdivisions.emplace_back(u * hmPartSizeX, y0, remainingSize, h);
    };

    int32 v = 0;
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
        SafeDelete(c->object);
        DestroyObjectAligned(c->shape);
        delete c;
    }
}

void CollisionLandscape::CollisionLandscapePrivate::buildCollisionObject(const Rect2i& bounds, Heightmap* heightmap)
{
    LandscapeChunk* chunk = new LandscapeChunk();
    chunks.push_back(chunk);

    float invHmSize = 1.0f / static_cast<float>(heightmap->Size());

    Vector3 landSize = owner->boundingBox.max - owner->boundingBox.min;
    float32 landScaleX = landSize.x * invHmSize;
    float32 landScaleY = landSize.y * invHmSize;
    float32 landScaleZ = landSize.z / static_cast<float>(Heightmap::MAX_VALUE);

    uint16* heightData = heightmap->Data();
    chunk->data.resize(bounds.dx * bounds.dy);

    int32 k = 0;
    for (int32 y = bounds.y; y < bounds.y + bounds.dy; ++y)
    {
        for (int32 x = bounds.x; x < bounds.x + bounds.dx; ++x)
        {
            chunk->data[k++] = float(heightData[x + y * heightmap->Size()]) * landScaleZ;
        }
    }

    chunk->shape = CreateObjectAligned<btHeightfieldTerrainShape, 16>(bounds.dx, bounds.dy,
                                                                      chunk->data.data(), landScaleZ, 0.0f, landSize.z, 2, PHY_FLOAT, true);
    chunk->shape->setLocalScaling(btVector3(landScaleX, landScaleY, 1.0f));

    float xr = static_cast<float>(bounds.x) * invHmSize;
    float yr = static_cast<float>(bounds.y) * invHmSize;
    float bx = static_cast<float>(bounds.dx) * invHmSize;
    float by = static_cast<float>(bounds.dy) * invHmSize;
    float dx = (xr + 0.5f * bx - 0.5f) * landSize.x;
    float dy = (yr + 0.5f * by - 0.5f) * landSize.y;
    float dz = 0.5f * landSize.z;

    btTransform landTransform = btTransform::getIdentity();
    landTransform.setOrigin(btVector3(dx, dy, dz));

    chunk->object = new btCollisionObject();
    chunk->object->setWorldTransform(landTransform);
    chunk->object->setCollisionShape(chunk->shape);
    owner->btWord->addCollisionObject(chunk->object);
}
