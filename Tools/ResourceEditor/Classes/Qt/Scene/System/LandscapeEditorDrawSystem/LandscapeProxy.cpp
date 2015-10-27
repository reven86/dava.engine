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


#include "Qt/Main/QtUtils.h"
#include "LandscapeProxy.h"

const FastName LandscapeProxy::LANDSCAPE_TEXTURE_TOOL("toolTexture");
const FastName LandscapeProxy::LANDSCAPE_TEXTURE_CURSOR("cursorTexture");
const FastName LandscapeProxy::LANSDCAPE_FLAG_CURSOR("LANDSCAPE_CURSOR");
const FastName LandscapeProxy::LANSDCAPE_FLAG_TOOL("LANDSCAPE_TOOL");
const FastName LandscapeProxy::LANSDCAPE_FLAG_TOOL_MIX("LANDSCAPE_TOOL_MIX");
const FastName LandscapeProxy::LANDSCAPE_PARAM_CURSOR_COORD_SIZE("cursorCoordSize");

LandscapeProxy::LandscapeProxy(Landscape* landscape, Entity* node)
:   tilemaskImageCopy(NULL)
,	tilemaskWasChanged(0)
,	mode(MODE_ORIGINAL_LANDSCAPE)
,	cursorTexture(NULL)
{
	DVASSERT(landscape != NULL);

    tilemaskDrawTextures[TILEMASK_TEXTURE_SOURCE] = NULL;
    tilemaskDrawTextures[TILEMASK_TEXTURE_DESTINATION] = NULL;

    baseLandscape = SafeRetain(landscape);

    sourceTilemaskPath = GetPathForSourceTexture();

    landscapeEditorMaterial = new NMaterial();
    landscapeEditorMaterial->SetMaterialName(FastName("Landscape.Tool.Material"));
    landscapeEditorMaterial->SetFXName(FastName("~res:/Materials/Landscape.Tool.material"));
    landscapeEditorMaterial->AddFlag(LANSDCAPE_FLAG_TOOL, 0);
    landscapeEditorMaterial->AddFlag(LANSDCAPE_FLAG_CURSOR, 0);
    landscapeEditorMaterial->AddFlag(LANSDCAPE_FLAG_TOOL_MIX, 0);
    landscapeEditorMaterial->AddProperty(LANDSCAPE_PARAM_CURSOR_COORD_SIZE, cursorCoordSize.data, rhi::ShaderProp::TYPE_FLOAT4);

    ScopedPtr<Texture> texture(Texture::CreateFromFile("~res:/LandscapeEditor/Tools/cursor/cursor.tex"));
    texture->SetWrapMode(rhi::TEXADDR_CLAMP, rhi::TEXADDR_CLAMP);
    texture->SetMinMagFilter(rhi::TEXFILTER_LINEAR, rhi::TEXFILTER_LINEAR, rhi::TEXMIPFILTER_NONE);
    landscapeEditorMaterial->AddTexture(LANDSCAPE_TEXTURE_CURSOR, texture);
}

LandscapeProxy::~LandscapeProxy()
{
    SafeRelease(landscapeEditorMaterial);

    SafeRelease(baseLandscape);
	SafeRelease(tilemaskImageCopy);
    SafeRelease(tilemaskDrawTextures[TILEMASK_TEXTURE_SOURCE]);
    SafeRelease(tilemaskDrawTextures[TILEMASK_TEXTURE_DESTINATION]);

    SafeRelease(cursorTexture);
}

void LandscapeProxy::SetMode(LandscapeProxy::eLandscapeMode _mode)
{
    if (mode != _mode)
    {
        mode = _mode;
        if (mode == LandscapeProxy::MODE_CUSTOM_LANDSCAPE)
        {
            landscapeEditorMaterial->SetParent(baseLandscape->GetMaterial());
            baseLandscape->SetMaterial(landscapeEditorMaterial);
            baseLandscape->SetForceFirstLod(true);
        }
        else
        {
            baseLandscape->SetMaterial(landscapeEditorMaterial->GetParent());
            landscapeEditorMaterial->SetParent(nullptr);
            baseLandscape->SetForceFirstLod(false);
        }
    }
}

const AABBox3 & LandscapeProxy::GetLandscapeBoundingBox()
{
	return baseLandscape->GetBoundingBox();
}

Texture* LandscapeProxy::GetLandscapeTexture(const FastName& level)
{
    return baseLandscape->GetMaterial()->GetEffectiveTexture(level);
}

Color LandscapeProxy::GetLandscapeTileColor(const FastName& level)
{
    const float32* prop = baseLandscape->GetMaterial()->GetEffectivePropValue(level);
    if (prop)
        return Color(prop[0], prop[1], prop[2], 1.f);
    else
        return Color::White;
}

void LandscapeProxy::SetLandscapeTileColor(const FastName& level, const Color& color)
{
    NMaterial* landscapeMaterial = baseLandscape->GetMaterial();
    while (landscapeMaterial)
    {
        if (landscapeMaterial->HasLocalProperty(level))
            break;

        landscapeMaterial = landscapeMaterial->GetParent();
    }

    if (landscapeMaterial)
    {
        landscapeMaterial->SetPropertyValue(level, color.color);
    }
}

void LandscapeProxy::SetToolTexture(Texture* texture, bool mixColors)
{
    if (texture)
    {
        landscapeEditorMaterial->AddTexture(LANDSCAPE_TEXTURE_TOOL, texture);
        landscapeEditorMaterial->SetFlag(LANSDCAPE_FLAG_TOOL, 1);
        landscapeEditorMaterial->SetFlag(LANSDCAPE_FLAG_TOOL_MIX, (mixColors) ? 1 : 0);
    }
    else
    {
        landscapeEditorMaterial->RemoveTexture(LANDSCAPE_TEXTURE_TOOL);
        landscapeEditorMaterial->SetFlag(LANSDCAPE_FLAG_TOOL, 0);
        landscapeEditorMaterial->SetFlag(LANSDCAPE_FLAG_TOOL_MIX, 0);
    }
}

void LandscapeProxy::SetHeightmap(DAVA::Heightmap *heightmap)
{
    baseLandscape->SetHeightmap(heightmap);
}

void LandscapeProxy::CursorEnable()
{
    landscapeEditorMaterial->SetFlag(LANSDCAPE_FLAG_CURSOR, 1);
}

void LandscapeProxy::CursorDisable()
{
    landscapeEditorMaterial->SetFlag(LANSDCAPE_FLAG_CURSOR, 0);
}

void LandscapeProxy::SetCursorTexture(Texture* texture)
{
    if(cursorTexture != texture)
    {
        SafeRelease(cursorTexture);
        cursorTexture = SafeRetain(texture);
    }

    landscapeEditorMaterial->SetTexture(LANDSCAPE_TEXTURE_CURSOR, texture);
}

void LandscapeProxy::SetCursorSize(float32 size)
{
    cursorCoordSize.z = size;
    cursorCoordSize.w = size;

    landscapeEditorMaterial->SetPropertyValue(LANDSCAPE_PARAM_CURSOR_COORD_SIZE, cursorCoordSize.data);
}

void LandscapeProxy::SetCursorPosition(const Vector2& position)
{
    cursorCoordSize.x = position.x;
    cursorCoordSize.y = position.y;

    landscapeEditorMaterial->SetPropertyValue(LANDSCAPE_PARAM_CURSOR_COORD_SIZE, cursorCoordSize.data);
}

Vector3 LandscapeProxy::PlacePoint(const Vector3& point)
{
	Vector3 landscapePoint;
    baseLandscape->PlacePoint(point, landscapePoint);

    return landscapePoint;
}

bool LandscapeProxy::IsTilemaskChanged()
{
	return (tilemaskWasChanged != 0);
}

void LandscapeProxy::ResetTilemaskChanged()
{
	tilemaskWasChanged = 0;
}

void LandscapeProxy::IncreaseTilemaskChanges()
{
	++tilemaskWasChanged;
}

void LandscapeProxy::DecreaseTilemaskChanges()
{
	--tilemaskWasChanged;
}

void LandscapeProxy::InitTilemaskImageCopy()
{
	SafeRelease(tilemaskImageCopy);

    Vector<Image*> imgs;
    ImageSystem::Instance()->Load(sourceTilemaskPath, imgs);

    DVASSERT(imgs.size() == 1);
    tilemaskImageCopy = imgs[0];
}

DAVA::FilePath LandscapeProxy::GetPathForSourceTexture() const
{
    NMaterial* material = baseLandscape->GetMaterial();
    if (nullptr != material)
    {
        Texture* tiletexture = material->GetEffectiveTexture(Landscape::TEXTURE_TILEMASK);
        if (nullptr != tiletexture)
        {
            return tiletexture->GetDescriptor()->GetSourceTexturePathname();
        }
    }

    return FilePath();
}

Image* LandscapeProxy::GetTilemaskImageCopy()
{
	return tilemaskImageCopy;
}

void LandscapeProxy::InitTilemaskDrawTextures()
{
    if (tilemaskDrawTextures[TILEMASK_TEXTURE_SOURCE] == NULL || tilemaskDrawTextures[TILEMASK_TEXTURE_DESTINATION] == NULL)
    {
        SafeRelease(tilemaskDrawTextures[TILEMASK_TEXTURE_SOURCE]);
        SafeRelease(tilemaskDrawTextures[TILEMASK_TEXTURE_DESTINATION]);

        uint32 texSize = (uint32)GetLandscapeTexture(Landscape::TEXTURE_TILEMASK)->GetWidth();
        tilemaskDrawTextures[TILEMASK_TEXTURE_SOURCE] = Texture::CreateFBO(texSize, texSize, FORMAT_RGBA8888, rhi::TEXTURE_TYPE_2D);
        tilemaskDrawTextures[TILEMASK_TEXTURE_DESTINATION] = Texture::CreateFBO(texSize, texSize, FORMAT_RGBA8888, rhi::TEXTURE_TYPE_2D);
    }
}

Texture* LandscapeProxy::GetTilemaskDrawTexture(int32 number)
{
    if (number >= 0 && number < TILEMASK_TEXTURE_COUNT)
    {
        return tilemaskDrawTextures[number];
    }

	return NULL;
}

void LandscapeProxy::SwapTilemaskDrawTextures()
{
    Texture* temp = tilemaskDrawTextures[TILEMASK_TEXTURE_SOURCE];
    tilemaskDrawTextures[TILEMASK_TEXTURE_SOURCE] = tilemaskDrawTextures[TILEMASK_TEXTURE_DESTINATION];
    tilemaskDrawTextures[TILEMASK_TEXTURE_DESTINATION] = temp;
}

void LandscapeProxy::UpdateTileMaskPathname()
{
    if (sourceTilemaskPath.IsEmpty())
    {
        sourceTilemaskPath = GetPathForSourceTexture();
        DVASSERT(sourceTilemaskPath.IsEmpty() == false);
    }
}
