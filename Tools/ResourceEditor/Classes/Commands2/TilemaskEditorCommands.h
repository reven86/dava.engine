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


#ifndef __RESOURCEEDITORQT__TILEMASKEDITORCOMMANDS__
#define __RESOURCEEDITORQT__TILEMASKEDITORCOMMANDS__

#include "Commands2/Command2.h"
#include "Commands2/CommandAction.h"
#include "DAVAEngine.h"

#include "Render/UniqueStateSet.h"

using namespace DAVA;

class LandscapeProxy;
class SceneEditor2;

class ActionEnableTilemaskEditor: public CommandAction
{
public:
	ActionEnableTilemaskEditor(SceneEditor2* forSceneEditor);
	
protected:
	SceneEditor2* sceneEditor;
	
	virtual void Redo();
};

class ActionDisableTilemaskEditor: public CommandAction
{
public:
	ActionDisableTilemaskEditor(SceneEditor2* forSceneEditor);
	
protected:
	SceneEditor2* sceneEditor;
	
	virtual void Redo();
};


class ModifyTilemaskCommand: public Command2
{
public:
	ModifyTilemaskCommand(LandscapeProxy* landscapeProxy,
						  const Rect& updatedRect);
	~ModifyTilemaskCommand();

	virtual void Undo();
	virtual void Redo();
	virtual Entity* GetEntity() const;

protected:
	Image* undoImageMask;
	Image* redoImageMask;
	LandscapeProxy* landscapeProxy;
	Rect updatedRect;

    void ApplyImageToTexture(Image* image, Texture* dstTex, int32 internalHandle);

    Texture* texture[2];
};

class SetTileColorCommand: public Command2
{
public:
    SetTileColorCommand(LandscapeProxy* landscapeProxy,
                        const FastName& level,
                        const Color& color);
    ~SetTileColorCommand();

	virtual void Undo();
	virtual void Redo();
	virtual Entity* GetEntity() const;

protected:
    const FastName& level;
    Color redoColor;
	Color undoColor;
	LandscapeProxy* landscapeProxy;
};

#endif /* defined(__RESOURCEEDITORQT__TILEMASKEDITORCOMMANDS__) */
