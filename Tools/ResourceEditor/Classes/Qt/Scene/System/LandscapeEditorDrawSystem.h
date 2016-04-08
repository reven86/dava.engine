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


#ifndef __RESOURCEEDITORQT__LANDSCAPEEDITORDRAWSYSTEM__
#define __RESOURCEEDITORQT__LANDSCAPEEDITORDRAWSYSTEM__

#include "Entity/SceneSystem.h"
#include "Scene3D/Scene.h"
#include "Render/Highlevel/Landscape.h"
#include "Render/UniqueStateSet.h"

class LandscapeProxy;
class HeightmapProxy;
class NotPassableTerrainProxy;
class CustomColorsProxy;
class RulerToolProxy;
class Command2;

class LandscapeEditorDrawSystem : public DAVA::SceneSystem
{
public:
    enum eErrorType
    {
        LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS = 0,
        LANDSCAPE_EDITOR_SYSTEM_LANDSCAPE_ENTITY_ABSENT,
        LANDSCAPE_EDITOR_SYSTEM_TILEMASK_TEXTURE_ABSENT,
        LANDSCAPE_EDITOR_SYSTEM_FULLTILED_TEXTURE_ABSENT,
        LANDSCAPE_EDITOR_SYSTEM_TILE_TEXTURE_ABSENT,
        LANDSCAPE_EDITOR_SYSTEM_COLOR_TEXTURE_ABSENT,
        LANDSCAPE_EDITOR_SYSTEM_HEIGHTMAP_ABSENT,
        LANDSCAPE_EDITOR_SYSTEM_CUSTOMCOLORS_ABSENT,
    };

    LandscapeEditorDrawSystem(DAVA::Scene* scene);
    virtual ~LandscapeEditorDrawSystem();

    LandscapeProxy* GetLandscapeProxy();
    HeightmapProxy* GetHeightmapProxy();
    CustomColorsProxy* GetCustomColorsProxy();
    RulerToolProxy* GetRulerToolProxy();

    eErrorType EnableCustomDraw();
    void DisableCustomDraw();

    eErrorType EnableTilemaskEditing();
    void DisableTilemaskEditing();

    bool IsNotPassableTerrainEnabled();
    eErrorType EnableNotPassableTerrain();
    bool DisableNotPassableTerrain();

    void EnableCursor();
    void DisableCursor();
    void SetCursorTexture(DAVA::Texture* cursorTexture);
    void SetCursorSize(DAVA::float32 cursorSize);
    void SetCursorPosition(const DAVA::Vector2& cursorPos);

    void Process(DAVA::float32 timeElapsed) override;

    void ProcessCommand(const Command2* command, bool redo);

    DAVA::float32 GetTextureSize(const DAVA::FastName& level);
    DAVA::Vector3 GetLandscapeSize();
    DAVA::float32 GetLandscapeMaxHeight();
    DAVA::float32 GetHeightAtHeightmapPoint(const DAVA::Vector2& point);
    DAVA::float32 GetHeightAtTexturePoint(const DAVA::FastName& level, const DAVA::Vector2& point);
    DAVA::KeyedArchive* GetLandscapeCustomProperties();

    DAVA::Vector2 HeightmapPointToTexturePoint(const DAVA::FastName& level, const DAVA::Vector2& point);
    DAVA::Vector2 TexturePointToHeightmapPoint(const DAVA::FastName& level, const DAVA::Vector2& point);
    DAVA::Vector2 TexturePointToLandscapePoint(const DAVA::FastName& level, const DAVA::Vector2& point);
    DAVA::Vector2 LandscapePointToTexturePoint(const DAVA::FastName& level, const DAVA::Vector2& point);
    DAVA::Vector2 TranslatePoint(const DAVA::Vector2& point, const DAVA::Rect& fromRect, const DAVA::Rect& toRect);

    void ClampToTexture(const DAVA::FastName& level, DAVA::Rect& rect);
    void ClampToHeightmap(DAVA::Rect& rect);

    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;

    DAVA::Rect GetTextureRect(const DAVA::FastName& level);
    DAVA::Rect GetHeightmapRect();
    DAVA::Rect GetLandscapeRect();

    bool SaveTileMaskTexture();
    void ResetTileMaskTexture();
    DAVA::Texture* GetTileMaskTexture();
    void SetTileMaskTexture(DAVA::Texture* texture);

    eErrorType VerifyLandscape() const;

    DAVA::Landscape* GetBaseLandscape() const;

    static DAVA::String GetDescriptionByError(eErrorType error);

private:
    void UpdateBaseLandscapeHeightmap();
    eErrorType Init();

    eErrorType InitLandscape(DAVA::Entity* landscapeEntity, DAVA::Landscape* landscape);
    void DeinitLandscape();

    eErrorType IsNotPassableTerrainCanBeEnabled();

    bool UpdateTilemaskPathname();

private:
    DAVA::Entity* landscapeNode = nullptr;
    DAVA::Landscape* baseLandscape = nullptr;
    LandscapeProxy* landscapeProxy = nullptr;
    HeightmapProxy* heightmapProxy = nullptr;
    NotPassableTerrainProxy* notPassableTerrainProxy = nullptr;
    CustomColorsProxy* customColorsProxy = nullptr;
    RulerToolProxy* rulerToolProxy = nullptr;
    DAVA::uint32 customDrawRequestCount = 0;
    DAVA::FilePath sourceTilemaskPath;
};

#endif /* defined(__RESOURCEEDITORQT__LANDSCAPEEDITORDRAWSYSTEM__) */
