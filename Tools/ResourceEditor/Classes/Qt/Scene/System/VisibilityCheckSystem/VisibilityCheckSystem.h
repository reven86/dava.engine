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

#ifndef __VisibilityCheckSystem_h__
#define __VisibilityCheckSystem_h__

#include "VisibilityCheckRenderPass.h"
#include "Entity/SceneSystem.h"

class VisibilityCheckSystem : public DAVA::SceneSystem
{
public:
    VisibilityCheckSystem(DAVA::Scene* scene);
    ~VisibilityCheckSystem();

    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;
    void Process(DAVA::float32 timeElapsed) override;

    void Draw();

private:
    void UpdatePointSet();
    void Prerender();
    void CreateRenderTarget();

    bool CacheIsValid();
    void BuildCache();

    struct StateCache
    {
        DAVA::Size2i viewportSize;
        DAVA::Matrix4 viewprojMatrix;
        DAVA::Camera* camera = nullptr;
    };

    struct RenderPoint
    {
        DAVA::Vector3 point;
        DAVA::Color color;

        RenderPoint(const DAVA::Vector3& p, const DAVA::Color& c)
            : point(p)
            , color(c)
        {
        }
    };

    static const DAVA::uint32 CubemapsCount = 1;

private:
    DAVA::Vector<DAVA::Entity*> entities;
    DAVA::Texture* cubemapTarget[CubemapsCount];
    DAVA::Texture* renderTarget = nullptr;
    DAVA::Vector<RenderPoint> controlPoints;
    VisibilityCheckRenderPass renderPass;
    StateCache stateCache;
    size_t currentPointIndex = 0;
    bool shouldPrerender = true;
};

#endif
