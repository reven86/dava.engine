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


#ifndef __RESOURCEEDITORQT__CUSTOMCOLORSCOMMANDS2__
#define __RESOURCEEDITORQT__CUSTOMCOLORSCOMMANDS2__

#include "Commands2/Command2.h"
#include "CommandAction.h"
#include "DAVAEngine.h"
#include "Main/Request.h"

class CustomColorsProxy;
class SceneEditor2;

using namespace DAVA;

class ActionEnableCustomColors: public CommandAction
{
public:
	ActionEnableCustomColors(SceneEditor2* forSceneEditor);
	
protected:
	SceneEditor2* sceneEditor;
	
	virtual void Redo();
};

class ActionDisableCustomColors: public CommandAction
{
public:
	ActionDisableCustomColors(SceneEditor2* forSceneEditor,  bool textureSavingNeeded);
	
protected:
	SceneEditor2*	sceneEditor;
	bool			textureSavingNeeded;
	virtual void Redo();
};

class ModifyCustomColorsCommand: public Command2
{
public:
    ModifyCustomColorsCommand(Image* originalImage, Image* currentImage,
                              CustomColorsProxy* customColorsProxy,
                              const Rect& updatedRect);
    ~ModifyCustomColorsCommand();

    virtual void Undo();
	virtual void Redo();
	virtual Entity* GetEntity() const;

protected:
	CustomColorsProxy* customColorsProxy;
	Image* undoImage;
	Image* redoImage;
	Rect updatedRect;

    Texture* texture;

    void ApplyImage(Image* image);
};

#endif /* defined(__RESOURCEEDITORQT__CUSTOMCOLORSCOMMANDS2__) */
