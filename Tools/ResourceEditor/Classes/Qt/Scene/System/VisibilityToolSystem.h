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



#ifndef __RESOURCEEDITORQT__VISIBILITYTOOLSYSTEM__
#define __RESOURCEEDITORQT__VISIBILITYTOOLSYSTEM__

#include "DAVAEngine.h"
#include "LandscapeEditorDrawSystem.h"

class SceneCollisionSystem;
class SceneSelectionSystem;
class EntityModificationSystem;

using namespace DAVA;

class VisibilityToolSystem: public DAVA::SceneSystem
{
public:
	enum eVisibilityToolState
	{
		VT_STATE_NORMAL = 0,
		VT_STATE_SET_POINT,
		VT_STATE_SET_AREA,

		VT_STATES_COUNT
	};

	VisibilityToolSystem(Scene* scene);
	virtual ~VisibilityToolSystem();

	LandscapeEditorDrawSystem::eErrorType EnableLandscapeEditing();
	bool DisableLandscapeEdititing();
	bool IsLandscapeEditingEnabled() const;

	void Update(DAVA::float32 timeElapsed);
	void ProcessUIEvent(DAVA::UIEvent *event);

	void SetBrushSize(int32 brushSize);
	void SetColor(int32 colorIndex);

	void SetVisibilityPoint();
	void SetVisibilityArea();
	eVisibilityToolState GetState();
	int32 GetBrushSize();

	void SaveTexture(const FilePath& filePath);

protected:
	static const uint32 CROSS_TEXTURE_SIZE = 64;

	bool enabled;

	SceneCollisionSystem* collisionSystem;
	SceneSelectionSystem* selectionSystem;
	EntityModificationSystem* modifSystem;
	LandscapeEditorDrawSystem* drawSystem;

	Texture* cursorTexture;
	Texture* crossTexture;
	uint32 cursorSize;
	uint32 curToolSize;

	int32 landscapeSize;
	bool isIntersectsLandscape;
	Vector2 cursorPosition;
	Vector2 prevCursorPos;

	Rect updatedRectAccumulator;

	bool editingIsEnabled;

	Image* originalImage;

	eVisibilityToolState state;

	float32 pointsDensity;
	float32 visibilityPointHeight;
	Vector<float32> areaPointHeights;
	Vector<Color> areaPointColors;

	Vector2 visibilityPoint;

	Landscape::eTextureLevel textureLevel;

	void UpdateCursorPosition(int32 landscapeSize);

	void AddRectToAccumulator(const Rect& rect);
	void ResetAccumulatorRect();
	Rect GetUpdatedRect();

	void StoreOriginalState();
	void CreateUndoPoint();

	void PrepareConfig();
	void SetState(eVisibilityToolState newState);

	void SetVisibilityPointInternal(const Vector2& point);
	void SetVisibilityAreaInternal();

	
	void PerformHeightTest(Vector3 spectatorCoords,
						   Vector2 circleCenter,
						   float32 circleRadius,
						   float32 density,
						   const Vector<float32>& heightValues,
						   Vector<Vector3>* colorizedPoints);
	bool IsCircleContainsPoint(const Vector2& circleCenter,
							   float32 circleRadius,
							   const Vector2& point);
	void DrawVisibilityAreaPoints(const Vector<DAVA::Vector3> &points);

	LandscapeEditorDrawSystem::eErrorType IsCanBeEnabled();
};

#endif /* defined(__RESOURCEEDITORQT__VISIBILITYTOOLSYSTEM__) */
