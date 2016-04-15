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


#ifndef __RESOURCEEDITORQT__LANDSCAPEPROXY__
#define __RESOURCEEDITORQT__LANDSCAPEPROXY__

#include "DAVAEngine.h"

#include "Render/UniqueStateSet.h"

namespace DAVA
{
class Image;
}

class LandscapeProxy : public DAVA::BaseObject
{
public:
    enum eTilemaskTextures
    {
        TILEMASK_TEXTURE_SOURCE = 0,
        TILEMASK_TEXTURE_DESTINATION,

        TILEMASK_TEXTURE_COUNT
    };

    enum eLandscapeMode
    {
        MODE_CUSTOM_LANDSCAPE = 0,
        MODE_ORIGINAL_LANDSCAPE,

        MODES_COUNT
    };

    static const DAVA::FastName LANDSCAPE_TEXTURE_TOOL;
    static const DAVA::FastName LANDSCAPE_TEXTURE_CURSOR; //should use clamp wrap mode
    static const DAVA::FastName LANSDCAPE_FLAG_CURSOR;
    static const DAVA::FastName LANSDCAPE_FLAG_TOOL;
    static const DAVA::FastName LANSDCAPE_FLAG_TOOL_MIX;
    static const DAVA::FastName LANDSCAPE_PARAM_CURSOR_COORD_SIZE; //x,y - cursor position [0...1] (in landscape space); z,w - cursor size [0...1] (fraction of landscape)

protected:
    virtual ~LandscapeProxy();

public:
    LandscapeProxy(DAVA::Landscape* landscape, DAVA::Entity* node);

    void SetMode(LandscapeProxy::eLandscapeMode mode);

    const DAVA::AABBox3& GetLandscapeBoundingBox();
    DAVA::Texture* GetLandscapeTexture(const DAVA::FastName& level);
    DAVA::Color GetLandscapeTileColor(const DAVA::FastName& level);
    void SetLandscapeTileColor(const DAVA::FastName& level, const DAVA::Color& color);

    void SetToolTexture(DAVA::Texture* texture, bool mixColors);

    DAVA::RenderObject* GetRenderObject();
    void SetHeightmap(DAVA::Heightmap* heightmap);

    void CursorEnable();
    void CursorDisable();
    void SetCursorTexture(DAVA::Texture* texture);
    void SetCursorSize(DAVA::float32 size);
    void SetCursorPosition(const DAVA::Vector2& position);

    DAVA::Vector3 PlacePoint(const DAVA::Vector3& point);

    bool IsTilemaskChanged();
    void ResetTilemaskChanged();
    void IncreaseTilemaskChanges();
    void DecreaseTilemaskChanges();

    void InitTilemaskImageCopy();
    DAVA::Image* GetTilemaskImageCopy();

    void InitTilemaskDrawTextures();
    DAVA::Texture* GetTilemaskDrawTexture(DAVA::int32 number);
    void SwapTilemaskDrawTextures();

    void UpdateTileMaskPathname();

protected:
    DAVA::FilePath GetPathForSourceTexture() const;

protected:
    enum eToolTextureType
    {
        TEXTURE_TYPE_NOT_PASSABLE = 0,
        TEXTURE_TYPE_CUSTOM_COLORS,
        TEXTURE_TYPE_VISIBILITY_CHECK_TOOL,
        TEXTURE_TYPE_RULER_TOOL,

        TEXTURE_TYPES_COUNT
    };

    DAVA::Image* tilemaskImageCopy = nullptr;
    DAVA::Array<DAVA::Texture*, TILEMASK_TEXTURE_COUNT> tilemaskDrawTextures;

    DAVA::int32 tilemaskWasChanged = 0;

    DAVA::FilePath sourceTilemaskPath;

    DAVA::Landscape* baseLandscape = nullptr;
    DAVA::NMaterial* landscapeEditorMaterial = nullptr;
    DAVA::Vector4 cursorCoordSize;

    eLandscapeMode mode = MODE_ORIGINAL_LANDSCAPE;

    void UpdateDisplayedTexture();

    DAVA::Texture* cursorTexture = nullptr;
};

#endif /* defined(__RESOURCEEDITORQT__LANDSCAPEPROXY__) */
