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

#include "LandscapeEditorSystem.h"
#include "LandscapeEditorDrawSystem.h"

using namespace DAVA;

class EntityGroup;
class VisibilityToolSystem: public LandscapeEditorSystem
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

	virtual void Process(DAVA::float32 timeElapsed);
	virtual void Input(DAVA::UIEvent *event);

	void SetBrushSize(int32 brushSize);
	void SetColor(int32 colorIndex);

	void SetVisibilityPoint();
	void SetVisibilityArea();
	eVisibilityToolState GetState();
	int32 GetBrushSize();

	void SaveTexture(const FilePath& filePath);

protected:
    static const float32 CROSS_TEXTURE_SIZE;

    Texture* crossTexture;
    uint32 curToolSize;

    bool editingIsEnabled;

    eVisibilityToolState state;

	float32 pointsDensity;
	float32 visibilityPointHeight;
	Vector<float32> areaPointHeights;
	Vector<Color> areaPointColors;

	Vector2 visibilityPoint;

    const FastName& textureLevel;

    void PrepareConfig();
    void SetState(eVisibilityToolState newState);

    void SetVisibilityPointInternal();
	void SetVisibilityAreaInternal();

    void PerformHeightTest(const Vector3& spectatorCoords,
                           const Vector2& circleCenter,
                           float32 circleRadius,
                           float32 density,
                           const Vector<float32>& heightValues,
                           Vector<Vector3>& colorizedPoints);

    void DrawVisibilityAreaPoints(const Vector<DAVA::Vector3>& points);
    void DrawVisibilityPoint();
    void RenderVisibilityPoint(bool clearTarget);

    void ExcludeEntities(EntityGroup *entities) const;
    
};

#endif /* defined(__RESOURCEEDITORQT__VISIBILITYTOOLSYSTEM__) */
