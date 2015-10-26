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
    enum class State : uint32
    {
        NotActive,
        AddingPoint,
        ComputingVisibility
    };

public:
    VisibilityToolSystem(Scene* scene);
	virtual ~VisibilityToolSystem();

	LandscapeEditorDrawSystem::eErrorType EnableLandscapeEditing();
	bool DisableLandscapeEdititing();

    void Process(DAVA::float32 timeElapsed);
    void Input(DAVA::UIEvent* event);

    void Draw();

    uint32 StartAddingVisibilityPoint();
    void CancelAddingCheckPoint();

    void ComputeVisibilityArea();
    void CancelComputing();

    State GetState();
    int32 GetProgress();

    void SaveTexture(const FilePath& filePath);

private:
    struct CheckPoint
    {
        Vector2 relativePosition;
        Vector3 worldPosition;
        Color color = Color(1.0f, 0.5f, 0.25f, 1.0f);
        float angleUp = PI / 6.0f;
        float angleDown = PI / 6.0f;
        float radius = 0.0f; // point for now
        Vector<std::pair<Point2i, Color>> result;
        uint32 numPoints = 0;
    };
    using CheckPoints = Vector<CheckPoint>;
    using VisibilityTests = Vector<std::pair<uint32, Vector2>>;

    void SetState(State newState);

    void CommitLatestCheckPoint();
    void SetVisibilityAreaInternal();

    void DrawResults();
    void DrawVisibilityAreaPoints();
    void DrawVisibilityAreaMark(const CheckPoint& point);

    void ExcludeEntities(EntityGroup *entities) const;

    void PerformVisibilityTest(const VisibilityTests::value_type& test);
    void ProcessNextVisibilityTests();

private:
    FastName textureLevel;
    Texture* crossTexture = nullptr;
    CheckPoints checkPoints;
    VisibilityTests remainingVisibilityTests;
    State state = State::NotActive;
    uint32 textureStepSizeX = 4;
    uint32 textureStepSizeY = 4;
    uint32 textureSize = 0;
    uint32 pointsRowSize = 0;
    uint32 totalRowsInPoints = 0;
    uint32 totalVisibilityTests = 0;
    bool editingIsEnabled = false;
};

#endif /* defined(__RESOURCEEDITORQT__VISIBILITYTOOLSYSTEM__) */
