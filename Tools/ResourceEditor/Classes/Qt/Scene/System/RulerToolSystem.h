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

class RulerToolSystem : public LandscapeEditorSystem
{
    static const DAVA::int32 APPROXIMATION_COUNT = 10;

public:
    RulerToolSystem(DAVA::Scene* scene);
    virtual ~RulerToolSystem();

    LandscapeEditorDrawSystem::eErrorType EnableLandscapeEditing();
    bool DisableLandscapeEdititing();

    virtual void Process(DAVA::float32 timeElapsed);
    virtual void Input(DAVA::UIEvent* event);

    DAVA::float32 GetLength();
    DAVA::float32 GetPreviewLength();

protected:
    DAVA::Vector2 MirrorPoint(const DAVA::Vector2& point) const;

    DAVA::uint32 curToolSize;
    DAVA::Texture* toolImageTexture;

    DAVA::List<DAVA::Vector2> linePoints;
    DAVA::List<DAVA::float32> lengths;
    DAVA::Vector2 previewPoint;
    DAVA::float32 previewLength;
    bool previewEnabled;

    void SetStartPoint(const DAVA::Vector2& point);
    void AddPoint(const DAVA::Vector2& point);
    void RemoveLastPoint();
    void CalcPreviewPoint(const DAVA::Vector2& point, bool force = false);
    DAVA::float32 GetLength(const DAVA::Vector2& startPoint, const DAVA::Vector2& endPoint);
    void DrawPoints();
    void DisablePreview();
    void SendUpdatedLength();

    void Clear();
};

#endif /* defined(__RESOURCEEDITORQT__RULERTOOLSYSTEM__) */
