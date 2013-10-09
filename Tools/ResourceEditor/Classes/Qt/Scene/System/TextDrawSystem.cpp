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



#include "TextDrawSystem.h"

// framework
#include "Render/RenderManager.h"
#include "Render/RenderHelper.h"
#include "Utils/Utils.h"

TextDrawSystem::TextDrawSystem(DAVA::Scene * scene, SceneCameraSystem *_cameraSystem)
	: DAVA::SceneSystem(scene)
	, cameraSystem(_cameraSystem)
	, font(NULL)
{
	font = DAVA::GraphicsFont::Create("d:/Projects/dava.framework/Tools/ResourceEditor/Data/Fonts/terminus.def", "d:/Projects/dava.framework/Tools/ResourceEditor/Data/Gfx/Fonts/terminus.txt");
	//font = DAVA::GraphicsFont::Create("/Users/smile4u/Projects/dava.framework/Tools/ResourceEditor/Data/Fonts/terminus.def", "/Users/smile4u/Projects/dava.framework/Tools/ResourceEditor/Data/Gfx/Fonts/terminus.txt");
	//font = DAVA::GraphicsFont::Create("d:/Projects/dava.framework/Tools/ResourceEditor/Data/Fonts/arial_mono_11.def", "d:/Projects/dava.framework/Tools/ResourceEditor/Data/Gfx/Fonts/arial_mono_11.txt");
}

TextDrawSystem::~TextDrawSystem()
{
	SafeRelease(font);
}

void TextDrawSystem::Update(float timeElapsed)
{ }

DAVA::Vector2 TextDrawSystem::ToPos2d(const DAVA::Vector3 &pos3d) const
{
	return cameraSystem->GetScreenPos(pos3d);
}

void TextDrawSystem::Draw()
{
	if(listToDraw.size() > 0)
	{
		if(NULL != font)
		{
			DAVA::RenderManager::Instance()->SetRenderOrientation(DAVA::Core::SCREEN_ORIENTATION_PORTRAIT);
			DAVA::RenderManager::Instance()->SetRenderEffect(NULL);
			DAVA::RenderManager::Instance()->SetColor(255, 0, 0, 255);

			DAVA::List<TextToDraw>::iterator i  = listToDraw.begin();
			DAVA::List<TextToDraw>::iterator end  = listToDraw.end();

			for (; i != end; ++i)
			{
				DAVA::WideString wStr = DAVA::StringToWString(i->text);
				DAVA::Size2i sSize = font->GetStringSize(wStr);

//				font->SetColor(i->color);
				font->DrawString(i->pos.x - sSize.dx / 2, i->pos.y - sSize.dy, wStr);
			}

		}

		listToDraw.clear();
	}
}

void TextDrawSystem::DrawText(int x, int y, const DAVA::String &text, const DAVA::Color &color)
{
	DrawText(DAVA::Vector2((DAVA::float32)x, (DAVA::float32)y), text, color);
}

void TextDrawSystem::DrawText(DAVA::Vector2 pos2d, const DAVA::String &text, const DAVA::Color &color)
{
	TextToDraw ttd(pos2d, text, color);
	listToDraw.push_back(ttd);
}

void TextDrawSystem::DrawText(DAVA::Vector3 pos3d, const DAVA::String &text, const DAVA::Color &color)
{
	if(NULL != cameraSystem)
	{
		DrawText(ToPos2d(pos3d), text, color);
	}
}
