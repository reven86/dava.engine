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


#ifndef __LANDSCAPE_EDITOR_SYSTEM__
#define __LANDSCAPE_EDITOR_SYSTEM__

#include "DAVAEngine.h"
#include "LandscapeEditorDrawSystem.h"

class SceneCollisionSystem;
class SceneSelectionSystem;
class EntityModificationSystem;


class LandscapeEditorSystem: public DAVA::SceneSystem
{
public:
	LandscapeEditorSystem(DAVA::Scene* scene, const DAVA::FilePath & cursorPathname);
	virtual ~LandscapeEditorSystem();
	
    bool IsLandscapeEditingEnabled() const;

protected:
    LandscapeEditorDrawSystem::eErrorType IsCanBeEnabled() const;

    void UpdateCursorPosition();

protected:
	SceneCollisionSystem *collisionSystem;
	SceneSelectionSystem *selectionSystem;
	EntityModificationSystem *modifSystem;
    LandscapeEditorDrawSystem *drawSystem;
	Vector2 cursorPosition;
    Vector2 prevCursorPos;
    Texture* cursorTexture = nullptr;
    float32 cursorSize = 0.0f;
    float32 landscapeSize = 0.0f;
    bool isIntersectsLandscape = false;
    bool enabled = false;
};

#endif //__LANDSCAPE_EDITOR_SYSTEM__
