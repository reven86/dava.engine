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

#ifndef __VISIBILITYCHECKSYSTEM_H__
#define __VISIBILITYCHECKSYSTEM_H__

#include "VisibilityCheckRenderer.h"
#include "Entity/SceneSystem.h"

namespace DAVA
{
class Landscape;
}

class Command2;
class VisibilityCheckSystem : public DAVA::SceneSystem, VisibilityCheckRendererDelegate
{
public:
    static void ReleaseCubemapRenderTargets();

public:
    VisibilityCheckSystem(DAVA::Scene* scene);
    ~VisibilityCheckSystem();

    void RegisterEntity(DAVA::Entity* entity) override;
    void UnregisterEntity(DAVA::Entity* entity) override;
    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;

    void Recalculate();
    void Process(DAVA::float32 timeElapsed);
    void Draw();

    void InvalidateMaterials();

    void FixCurrentFrame();
    void ReleaseFixedFrame();

private:
    using EntityMap = DAVA::Map<DAVA::Entity*, DAVA::Vector<DAVA::Vector3>>;

    void BuildPointSetForEntity(EntityMap::value_type& item);
    void BuildIndexSet();
    DAVA::Color GetNormalizedColorForEntity(const EntityMap::value_type& item) const;

    void UpdatePointSet();
    void Prerender();
    void CreateRenderTarget();

    bool CacheIsValid();
    void BuildCache();

    bool ShouldDrawRenderObject(DAVA::RenderObject*) override;

    struct StateCache
    {
        DAVA::Size2i viewportSize;
        DAVA::Matrix4 viewprojMatrix;
        DAVA::Camera* camera = nullptr;
    };

private:
    EntityMap entitiesWithVisibilityComponent;
    DAVA::Landscape* landscape = nullptr;
    DAVA::Vector<VisibilityCheckRenderer::VisbilityPoint> controlPoints;
    DAVA::Vector<DAVA::uint32> controlPointIndices;
    DAVA::Map<DAVA::RenderObject*, DAVA::Entity*> renderObjectToEntity;
    VisibilityCheckRenderer renderer;
    StateCache stateCache;
    size_t currentPointIndex = 0;
    bool shouldPrerender = true;
    bool forceRebuildPoints = true;
    bool shouldFixFrame = false;
};

#endif
