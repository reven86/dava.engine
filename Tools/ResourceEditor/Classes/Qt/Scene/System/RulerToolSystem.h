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


#ifndef __RESOURCEEDITORQT__RULERTOOLSYSTEM__
#define __RESOURCEEDITORQT__RULERTOOLSYSTEM__

#include "LandscapeEditorSystem.h"
#include "LandscapeEditorDrawSystem.h"

using namespace DAVA;

class RulerToolSystem: public LandscapeEditorSystem
{
	static const DAVA::int32 APPROXIMATION_COUNT = 10;

public:
	RulerToolSystem(Scene* scene);
	virtual ~RulerToolSystem();

	LandscapeEditorDrawSystem::eErrorType EnableLandscapeEditing();
	bool DisableLandscapeEdititing();

	virtual void Process(DAVA::float32 timeElapsed);
	virtual void Input(DAVA::UIEvent *event);

	float32 GetLength();
	float32 GetPreviewLength();

protected:
    Vector2 MirrorPoint(const Vector2& point) const;

    uint32 curToolSize;
    Texture* toolImageTexture;

    List<Vector2> linePoints;
    List<float32> lengths;
    Vector2 previewPoint;
    float32 previewLength;
    bool previewEnabled;

    void SetStartPoint(const Vector2& point);
    void AddPoint(const Vector2& point);
    void RemoveLastPoint();
    void CalcPreviewPoint(const Vector2& point, bool force = false);
    float32 GetLength(const Vector2& startPoint, const Vector2& endPoint);
    void DrawPoints();
    void DisablePreview();
    void SendUpdatedLength();

	void Clear();
};

#endif /* defined(__RESOURCEEDITORQT__RULERTOOLSYSTEM__) */
