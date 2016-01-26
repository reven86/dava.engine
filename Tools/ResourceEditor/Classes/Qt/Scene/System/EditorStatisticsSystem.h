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


#ifndef __EDITOR_STATISTICS_SYSTEM_V2_H__
#define __EDITOR_STATISTICS_SYSTEM_V2_H__

#include "Entity/SceneSystem.h"
#include "Scene/SceneTypes.h"

namespace DAVA
{
    class Entity;
    class RenderComponent;
}

class EditorStatisticsSystemUIDelegate;
struct TrianglesData;

class EditorStatisticsSystem : public DAVA::SceneSystem
{
    enum eStatisticsSystemFlag : DAVA::uint32
    {
        FLAG_TRIANGLES = 1 << 0,

        FLAG_NONE = 0
    };

public:

    static const DAVA::int32 INDEX_OF_ALL_LODS_TRIANGLES = 0;
    static const DAVA::int32 INDEX_OF_FIRST_LOD_TRIANGLES = 1;

    EditorStatisticsSystem(DAVA::Scene * scene);

    void AddEntity(DAVA::Entity * entity) override;
    void RemoveEntity(DAVA::Entity * entity) override;
    void AddComponent(DAVA::Entity * entity, DAVA::Component * component);
    void RemoveComponent(DAVA::Entity * entity, DAVA::Component * component);

    void Process(DAVA::float32 timeElapsed) override;

    const DAVA::Vector<DAVA::uint32> &GetTriangles(eEditorMode mode, bool allTriangles) const;

    void AddDelegate(EditorStatisticsSystemUIDelegate *uiDelegate);
    void RemoveDelegate(EditorStatisticsSystemUIDelegate *uiDelegate);

private:

    void CalculateTriangles();


    //signals
    void EmitInvalidateUI(DAVA::uint32 flags);
    void DispatchSignals();
    //signals

private:

    DAVA::Vector<TrianglesData> triangles;

    DAVA::Vector<EditorStatisticsSystemUIDelegate *> uiDelegates;
    DAVA::uint32 invalidateUIflag = FLAG_NONE;
};

class EditorStatisticsSystemUIDelegate
{
public:

    virtual ~EditorStatisticsSystemUIDelegate() = default;

    virtual void UpdateTrianglesUI(EditorStatisticsSystem *forSystem){};
};



#endif // __SCENE_LOD_SYSTEM_V2_H__

