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
#include "EditorScene.h"

class LandscapeProxy;
class HeightmapProxy;
class NotPassableTerrainProxy;
class CustomColorsProxy;
class VisibilityToolProxy;
class RulerToolProxy;

class LandscapeEditorDrawSystem: public DAVA::SceneSystem
{
public:
	LandscapeEditorDrawSystem(Scene* scene);
	virtual ~LandscapeEditorDrawSystem();
	
	LandscapeProxy* GetLandscapeProxy();
	HeightmapProxy* GetHeightmapProxy();
	CustomColorsProxy* GetCustomColorsProxy();
	VisibilityToolProxy* GetVisibilityToolProxy();
	RulerToolProxy* GetRulerToolProxy();

	bool EnableCustomDraw();
	void DisableCustomDraw();

	bool EnableTilemaskEditing();
	void DisableTilemaskEditing();

	bool IsNotPassableTerrainEnabled();
	bool EnableNotPassableTerrain();
	void DisableNotPassableTerrain();
	
	void EnableCursor(int32 landscapeSize);
	void DisableCursor();
	void SetCursorTexture(Texture* cursorTexture);
	void SetCursorSize(uint32 cursorSize);
	void SetCursorPosition(const Vector2& cursorPos);
	void UpdateCursorPosition();
	
	void Update(DAVA::float32 timeElapsed);

	float32 GetTextureSize();
	Vector3 GetLandscapeSize();
	float32 GetLandscapeMaxHeight();
	float32 GetHeightAtPoint(const Vector2& point);
	float32 GetHeightAtTexturePoint(const Vector2& point);
	KeyedArchive* GetLandscapeCustomProperties();

	Vector2 HeightmapPointToTexturePoint(const Vector2& point);
	Vector2 TexturePointToHeightmapPoint(const Vector2& point);
	Vector2 TexturePointToLandscapePoint(const Vector2& point);
	Vector2 LandscapePointToTexturePoint(const Vector2& point);
	Vector2 TranslatePoint(const Vector2& point, const Rect& fromRect, const Rect& toRect);

private:
	Entity* landscapeNode;
	Landscape* baseLandscape;
	LandscapeProxy* landscapeProxy;
	HeightmapProxy* heightmapProxy;
	NotPassableTerrainProxy* notPassableTerrainProxy;
	CustomColorsProxy* customColorsProxy;
	VisibilityToolProxy* visibilityToolProxy;
	RulerToolProxy* rulerToolProxy;

	uint32 customDrawRequestCount;
	
	Texture* cursorTexture;
	uint32 cursorSize;
	Vector2 cursorPosition;
	
	void UpdateBaseLandscapeHeightmap();
	bool Init();

	bool IsNotPassableTerrainCanBeEnabled();
};

#endif /* defined(__RESOURCEEDITORQT__LANDSCAPEEDITORDRAWSYSTEM__) */
