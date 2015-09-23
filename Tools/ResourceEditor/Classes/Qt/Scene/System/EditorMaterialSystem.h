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


#ifndef __EDITOR_MATERIAL_SYSTEM_H__
#define __EDITOR_MATERIAL_SYSTEM_H__

#include "DAVAEngine.h"
#include "Base/Introspection.h"

class Command2;
class EditorMaterialSystem : public DAVA::SceneSystem
{
	friend class SceneEditor2;

public:
    enum MaterialLightViewMode
    {
        LIGHTVIEW_NOTHING     = 0x0,

        LIGHTVIEW_ALBEDO      = 0x1,
        LIGHTVIEW_AMBIENT     = 0x2,
        LIGHTVIEW_DIFFUSE     = 0x4,
        LIGHTVIEW_SPECULAR    = 0x8,

        LIGHTVIEW_ALL         = (LIGHTVIEW_ALBEDO | LIGHTVIEW_AMBIENT | LIGHTVIEW_DIFFUSE | LIGHTVIEW_SPECULAR)
    };

	EditorMaterialSystem(DAVA::Scene * scene);
    ~EditorMaterialSystem();

    const DAVA::Set<DAVA::NMaterial *>& GetTopParents() const;

	DAVA::Entity* GetEntity(DAVA::NMaterial*) const;
	const DAVA::RenderBatch *GetRenderBatch(DAVA::NMaterial*) const;

    void SetLightViewMode(EditorMaterialSystem::MaterialLightViewMode viewMode, bool set);
    bool GetLightViewMode(EditorMaterialSystem::MaterialLightViewMode viewMode) const;

    void SetLightViewMode(int fullViewMode);
    int GetLightViewMode();

    void SetLightmapCanvasVisible(bool enable);
    bool IsLightmapCanvasVisible() const;

private:
    struct MaterialMapping
    {
        enum class Mode : DAVA::uint32
        {
            RetainedRenderBatch,
            RenderBatchIndexInRenderObject
        };

        DAVA::Entity* entity = nullptr;
        DAVA::RenderBatch* renderBatch = nullptr;
        DAVA::uint32 renderBatchIndexInRenderObject = static_cast<DAVA::uint32>(-1);
        Mode mode = Mode::RenderBatchIndexInRenderObject;
    };
    using MaterialToObjectsMap = DAVA::Map<DAVA::NMaterial*, MaterialMapping>;

    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;

    void ProcessCommand(const Command2 *command, bool redo);

    void AddMaterialsFromEntity(DAVA::Entity* entity);
    void AddMaterialFromRenderBatchWithEntity(DAVA::RenderBatch* renderBatch, DAVA::Entity* entity);
    void AddMaterial(DAVA::NMaterial*, const MaterialMapping& mapping);

    void RemoveMaterial(DAVA::NMaterial *material);

    void ApplyViewMode();
    void ApplyViewMode(DAVA::NMaterial *material);

    bool IsEditable(DAVA::NMaterial *material) const;

private:
    MaterialToObjectsMap materialToObjectsMap;
    DAVA::Set<DAVA::NMaterial*> ownedParents;
    DAVA::uint32 curViewMode = LIGHTVIEW_ALL;
    bool showLightmapCanvas = false;
};

#endif // __EDITOR_MATERIAL_SYSTEM_H__
