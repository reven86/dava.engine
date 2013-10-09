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



#ifndef __LANDSCAPES_CONTROLLER_H__
#define __LANDSCAPES_CONTROLLER_H__

#include "DAVAEngine.h"

class LandscapeRenderer;
class NotPassableTerrain;
class EditorHeightmap;
class EditorLandscape;
class RulerToolLandscape;
class LandscapesController: public DAVA::BaseObject
{
public:

    LandscapesController();
	virtual ~LandscapesController();

    void SetScene(DAVA::Scene *scene);
    void SaveLandscape(DAVA::Landscape *landscape);
    
    void ToggleNotPassableLandscape();

    bool EditorLandscapeIsActive();
    
	EditorLandscape *CreateEditorLandscape();
	void ReleaseEditorLandscape();

    RulerToolLandscape *CreateRulerToolLandscape();
    void ReleaseRulerToolLandscape();

    
    
    DAVA::Landscape *GetCurrentLandscape();
    DAVA::Heightmap *GetCurrentHeightmap();
    
    
    void HeghtWasChanged(const DAVA::Rect &changedRect);

    void CursorEnable();
    void CursorDisable();
    
    
protected:

    bool ShowEditorLandscape(EditorLandscape *displayingLandscape);
    bool HideEditorLandscape(EditorLandscape *hiddingLandscape);
    
    bool NeedToKillRenderer(DAVA::Landscape *landscapeForDetection);
    
    void ReleaseScene();
    
    void ReleaseLandscape(EditorLandscape *landscapeNode);
    
    
    DAVA::Scene *scene;
    
    EditorHeightmap *renderedHeightmap;
    NotPassableTerrain *notPassableTerrain;
    LandscapeRenderer *landscapeRenderer;
    EditorLandscape *editorLandscape;
    RulerToolLandscape *rulerToolLandscape;
    
    DAVA::Landscape *currentLandscape;
    
    
    DAVA::Landscape *savedLandscape;
    DAVA::Heightmap *savedHeightmap;
};


#endif //__LANDSCAPES_CONTROLLER_H__
