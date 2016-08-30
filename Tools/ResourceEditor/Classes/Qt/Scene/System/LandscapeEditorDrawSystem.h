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

class RECommandNotificationObject;
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

    void ProcessCommand(const RECommandNotificationObject& commandNotification);

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

    bool UpdateTilemaskPathname();
    bool InitTilemaskImageCopy();

    void EnableSystem();
    void DisableSystem();

private:
    void UpdateBaseLandscapeHeightmap();
    eErrorType Init();

    eErrorType InitLandscape(DAVA::Entity* landscapeEntity, DAVA::Landscape* landscape);
    void DeinitLandscape();

    eErrorType IsNotPassableTerrainCanBeEnabled();

    DAVA::Entity* landscapeNode = nullptr;
    DAVA::Landscape* baseLandscape = nullptr;
    LandscapeProxy* landscapeProxy = nullptr;
    HeightmapProxy* heightmapProxy = nullptr;
    NotPassableTerrainProxy* notPassableTerrainProxy = nullptr;
    CustomColorsProxy* customColorsProxy = nullptr;
    RulerToolProxy* rulerToolProxy = nullptr;
    DAVA::uint32 customDrawRequestCount = 0;
    DAVA::FilePath sourceTilemaskPath;

    bool systemEnabled = false;
};

#endif /* defined(__RESOURCEEDITORQT__LANDSCAPEEDITORDRAWSYSTEM__) */
