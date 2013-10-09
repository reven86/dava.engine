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



#include <QFileDialog>
#include "VisibilityCheckToolCommands.h"
#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../Qt/Main/QtUtils.h"
#include "../SceneEditor/EditorBodyControl.h"
#include "../Qt/Scene/System/LandscapeEditorDrawSystem/VisibilityToolProxy.h"

CommandSaveTextureVisibilityTool::CommandSaveTextureVisibilityTool()
:	Command(COMMAND_WITHOUT_UNDO_EFFECT, CommandList::ID_COMMAND_SAVE_TEXTURE_VISIBILITY_TOOL)
{
}

void CommandSaveTextureVisibilityTool::Execute()
{
    FilePath currentPath = FileSystem::Instance()->GetUserDocumentsPath();
	QString filePath = QFileDialog::getSaveFileName(NULL,
													QString("Save texture"),
													QString(currentPath.GetAbsolutePathname().c_str()),
													QString("PNG image (*.png)"));

	FilePath selectedPathname = PathnameToDAVAStyle(filePath);

	if(!selectedPathname.IsEmpty())
	{
		SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
		if(screen)
		{
			screen->VisibilityToolSaveTexture(selectedPathname);
		}
	}
}

CommandPlacePointVisibilityTool::CommandPlacePointVisibilityTool(const Vector2& newVisibilityPoint, const Vector2& oldVisibilityPoint, bool oldPointIsSet, Image* oldImage)
:	Command(COMMAND_WITHOUT_UNDO_EFFECT, CommandList::ID_COMMAND_PLACE_POINT_VISIBILITY_TOOL)
,	point(newVisibilityPoint)
,	oldPoint(oldVisibilityPoint)
,	oldPointIsSet(oldPointIsSet)
{
	commandName = "Place Visibility Point";

	this->oldImage = SafeRetain(oldImage);
}

CommandPlacePointVisibilityTool::~CommandPlacePointVisibilityTool()
{
	SafeRelease(oldImage);
}

void CommandPlacePointVisibilityTool::Execute()
{
	LandscapeEditorVisibilityCheckTool* editor = GetEditor();
	if (editor)
		editor->SetVisibilityPoint(point);
	else
		SetState(STATE_INVALID);
}

void CommandPlacePointVisibilityTool::Cancel()
{
	LandscapeEditorVisibilityCheckTool* editor = GetEditor();
	if (editor)
		editor->RestorePointState(oldPoint, oldPointIsSet, oldImage);
}

LandscapeEditorVisibilityCheckTool* CommandPlacePointVisibilityTool::GetEditor()
{
	LandscapeEditorVisibilityCheckTool* editor = NULL;

	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if(screen)
	{
		editor = dynamic_cast<LandscapeEditorVisibilityCheckTool*>(screen->FindCurrentBody()->bodyControl->GetLandscapeEditor(SceneEditorScreenMain::ELEMID_VISIBILITY_CHECK_TOOL));
	}

	return editor;
}

CommandPlaceAreaVisibilityTool::CommandPlaceAreaVisibilityTool(const Vector2& areaPoint, uint32 areaSize, Image* oldImage)
:	Command(COMMAND_WITHOUT_UNDO_EFFECT, CommandList::ID_COMMAND_PLACE_AREA_VISIBILITY_TOOL),
	point(areaPoint),
	size(areaSize),
	redoImage(NULL)
{
	commandName = "Place Visibility Area";

	this->oldImage = SafeRetain(oldImage);
}

CommandPlaceAreaVisibilityTool::~CommandPlaceAreaVisibilityTool()
{
	SafeRelease(oldImage);
	SafeRelease(redoImage);
}

void CommandPlaceAreaVisibilityTool::Execute()
{
	LandscapeEditorVisibilityCheckTool* editor = GetEditor();
	if (editor)
	{
		if (!redoImage)
		{
			editor->SetVisibilityArea(point, size);
			redoImage = editor->StoreAreaState();
		}
		else
			editor->RestoreAreaState(redoImage);
	}
	else
		SetState(STATE_INVALID);
}

void CommandPlaceAreaVisibilityTool::Cancel()
{
	LandscapeEditorVisibilityCheckTool* editor = GetEditor();
	if (editor)
		editor->RestoreAreaState(oldImage);
}

LandscapeEditorVisibilityCheckTool* CommandPlaceAreaVisibilityTool::GetEditor()
{
	LandscapeEditorVisibilityCheckTool* editor = NULL;

	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if(screen)
	{
		editor = dynamic_cast<LandscapeEditorVisibilityCheckTool*>(screen->FindCurrentBody()->bodyControl->GetLandscapeEditor(SceneEditorScreenMain::ELEMID_VISIBILITY_CHECK_TOOL));
	}

	return editor;
}


CommandSetVisibilityPoint::CommandSetVisibilityPoint(Image* originalImage,
													 Sprite* cursorSprite,
													 VisibilityToolProxy* visibilityToolProxy,
													 const Vector2& visibilityPoint)
:	Command(COMMAND_WITHOUT_UNDO_EFFECT, CommandList::ID_COMMAND_PLACE_POINT_VISIBILITY_TOOL)
{
	commandName = "Place Visibility Point";

	this->undoImage = SafeRetain(originalImage);
	this->cursorSprite = SafeRetain(cursorSprite);
	this->visibilityToolProxy = SafeRetain(visibilityToolProxy);
	this->undoVisibilityPoint = visibilityToolProxy->GetVisibilityPoint();
	this->redoVisibilityPoint = visibilityPoint;
	this->undoVisibilityPointSet = visibilityToolProxy->IsVisibilityPointSet();
}

CommandSetVisibilityPoint::~CommandSetVisibilityPoint()
{
	SafeRelease(undoImage);
	SafeRelease(cursorSprite);
	SafeRelease(visibilityToolProxy);
}

void CommandSetVisibilityPoint::Execute()
{
	Sprite* visibilityToolSprite = visibilityToolProxy->GetSprite();
	RenderManager::Instance()->SetRenderTarget(visibilityToolSprite);
	RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 0.f);

	cursorSprite->SetPosition(redoVisibilityPoint - cursorSprite->GetSize() / 2.f);
	cursorSprite->Draw();

	RenderManager::Instance()->RestoreRenderTarget();

	visibilityToolProxy->UpdateVisibilityPointSet(true);
	visibilityToolProxy->UpdateRect(Rect(0.f, 0.f, undoImage->GetWidth(), undoImage->GetHeight()));
	visibilityToolProxy->SetVisibilityPoint(redoVisibilityPoint);
}

void CommandSetVisibilityPoint::Cancel()
{
	Sprite* visibilityToolSprite = visibilityToolProxy->GetSprite();
	RenderManager::Instance()->SetRenderTarget(visibilityToolSprite);
	RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 0.f);

	if (undoImage)
	{
		Texture* drawTexture = Texture::CreateFromData(undoImage->GetPixelFormat(),
													   undoImage->GetData(),
													   undoImage->GetWidth(),
													   undoImage->GetHeight(),
													   false);
		Sprite* drawSprite = Sprite::CreateFromTexture(drawTexture, 0, 0, undoImage->GetWidth(), undoImage->GetHeight());

		drawSprite->SetPosition(0.f, 0.f);
		drawSprite->Draw();

		SafeRelease(drawSprite);
		SafeRelease(drawTexture);

		visibilityToolProxy->UpdateRect(Rect(0.f, 0.f, undoImage->GetWidth(), undoImage->GetHeight()));
	}
	
	RenderManager::Instance()->RestoreRenderTarget();

	visibilityToolProxy->SetVisibilityPoint(undoVisibilityPoint);
	visibilityToolProxy->UpdateVisibilityPointSet(undoVisibilityPointSet);
}


CommandSetVisibilityArea::CommandSetVisibilityArea(Image* originalImage,
												   VisibilityToolProxy* visibilityToolProxy,
												   const Rect& updatedRect)
:	Command(COMMAND_WITHOUT_UNDO_EFFECT, CommandList::ID_COMMAND_PLACE_AREA_VISIBILITY_TOOL)
{
	commandName = "Place Visibility Area";

	Image* currentImage = visibilityToolProxy->GetSprite()->GetTexture()->CreateImageFromMemory();

	undoImage = Image::CopyImageRegion(originalImage, updatedRect);
	redoImage = Image::CopyImageRegion(currentImage, updatedRect);

	SafeRelease(currentImage);

	this->visibilityToolProxy = SafeRetain(visibilityToolProxy);
	this->updatedRect = updatedRect;
}

CommandSetVisibilityArea::~CommandSetVisibilityArea()
{
	SafeRelease(undoImage);
	SafeRelease(redoImage);
	SafeRelease(visibilityToolProxy);
}

void CommandSetVisibilityArea::Execute()
{
	ApplyImage(redoImage);
}

void CommandSetVisibilityArea::Cancel()
{
	ApplyImage(undoImage);
}

void CommandSetVisibilityArea::ApplyImage(DAVA::Image *image)
{
	Sprite* visibilityToolSprite = visibilityToolProxy->GetSprite();

	Texture* texture = Texture::CreateFromData(image->GetPixelFormat(), image->GetData(),
											   image->GetWidth(), image->GetHeight(), false);
	texture->GeneratePixelesation();
	Sprite* sprite = Sprite::CreateFromTexture(texture, 0, 0, image->GetWidth(), image->GetHeight());

	RenderManager::Instance()->SetRenderTarget(visibilityToolSprite);
	RenderManager::Instance()->ClipPush();
	RenderManager::Instance()->ClipRect(updatedRect);

	RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 0.f);
	sprite->SetPosition(updatedRect.x, updatedRect.y);
	sprite->Draw();

	RenderManager::Instance()->ClipPop();
	RenderManager::Instance()->RestoreRenderTarget();

	visibilityToolProxy->UpdateRect(updatedRect);

	SafeRelease(texture);
	SafeRelease(sprite);
}
