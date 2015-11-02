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


#include "LandscapeEditorSystem.h"
#include "LandscapeEditorDrawSystem/LandscapeProxy.h"
#include "LandscapeEditorDrawSystem/HeightmapProxy.h"

#include "Scene/SceneEditor2.h"

using namespace DAVA;

LandscapeEditorSystem::LandscapeEditorSystem(Scene* scene, const DAVA::FilePath & cursorPathname)
    : SceneSystem(scene)
    , cursorPosition(-100.f, -100.f)
    , prevCursorPos(-1.f, -1.f)
{
    cursorTexture = Texture::CreateFromFile( cursorPathname );
    cursorTexture->SetWrapMode(rhi::TEXADDR_CLAMP, rhi::TEXADDR_CLAMP);

    collisionSystem = ((SceneEditor2 *) GetScene())->collisionSystem;
	selectionSystem = ((SceneEditor2 *) GetScene())->selectionSystem;
	modifSystem = ((SceneEditor2 *) GetScene())->modifSystem;
	drawSystem = ((SceneEditor2 *) GetScene())->landscapeEditorDrawSystem;
}

LandscapeEditorSystem::~LandscapeEditorSystem()
{
	SafeRelease(cursorTexture);
    
    collisionSystem = nullptr;
    selectionSystem = nullptr;
    modifSystem = nullptr;
    drawSystem = nullptr;
}

LandscapeEditorDrawSystem::eErrorType LandscapeEditorSystem::IsCanBeEnabled() const
{
	return drawSystem->VerifyLandscape();
}

bool LandscapeEditorSystem::IsLandscapeEditingEnabled() const
{
    return enabled;
}

void LandscapeEditorSystem::UpdateCursorPosition()
{
	Vector3 landPos;
	isIntersectsLandscape = collisionSystem->LandRayTestFromCamera(landPos);
	if (isIntersectsLandscape)
	{
		landPos.x = (float32)((int32)landPos.x);
		landPos.y = (float32)((int32)landPos.y);
		
		const AABBox3 & box = drawSystem->GetLandscapeProxy()->GetLandscapeBoundingBox();

        cursorPosition.x = (landPos.x - box.min.x) / (box.max.x - box.min.x);
        cursorPosition.y = (landPos.y - box.min.y) / (box.max.y - box.min.y);
        cursorPosition.x = cursorPosition.x;
        cursorPosition.y = 1.f - cursorPosition.y;

        drawSystem->SetCursorPosition(cursorPosition);
    }
    else
	{
		// hide cursor
		drawSystem->SetCursorPosition(DAVA::Vector2(-100.f, -100.f));
	}
}




