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
#include "Render/RenderHelper.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "Utils/Utils.h"

TextDrawSystem::TextDrawSystem(DAVA::Scene * scene, SceneCameraSystem *_cameraSystem)
	: DAVA::SceneSystem(scene)
	, cameraSystem(_cameraSystem)
	, font(NULL)
{
	DAVA::FilePath defPath = DAVA::FilePath("~res:/Fonts/terminus.def");
	DAVA::FilePath txtPath = DAVA::FilePath("~res:/Gfx/Fonts/terminus.txt");

	font = DAVA::GraphicsFont::Create(defPath, txtPath);
}

TextDrawSystem::~TextDrawSystem()
{
	SafeRelease(font);
}

void TextDrawSystem::Process(float timeElapsed)
{ }

DAVA::Vector2 TextDrawSystem::ToPos2d(const DAVA::Vector3 &pos3d) const
{
	DAVA::Vector3 pos2ddepth = cameraSystem->GetScreenPosAndDepth(pos3d);
	if(pos2ddepth.z >= 0)
	{
		return DAVA::Vector2(pos2ddepth.x, pos2ddepth.y);
	}

	return DAVA::Vector2(-1, -1);
}

void TextDrawSystem::Draw()
{
#if RHI_COMPLETE_EDITOR
	if(listToDraw.size() > 0)
	{

		if(NULL != font)
		{
            DAVA::RenderSystem2D::Instance()->Setup2DMatrices();
            DAVA::RenderManager::Instance()->SetRenderState(DAVA::RenderState::RENDERSTATE_2D_BLEND);
            DAVA::RenderManager::Instance()->FlushState();
            
			DAVA::List<TextToDraw>::iterator i  = listToDraw.begin();
			DAVA::List<TextToDraw>::iterator end  = listToDraw.end();

			for (; i != end; ++i)
			{
				DAVA::WideString wStr = DAVA::StringToWString(i->text);
				DAVA::Size2i sSize = font->GetStringSize(wStr);

				DAVA::float32 x = i->pos.x;
				DAVA::float32 y = i->pos.y;

				DAVA::RenderManager::Instance()->SetColor(i->color);

				switch(i->align)
				{
				case TopLeft:
					break;
				case TopCenter:
					x -= (sSize.dx/2);
					break;
				case TopRight:
					x -= sSize.dx;
					break;
				case Left:
					y -= (sSize.dy/2);
					break;
				case Center:
					x -= (sSize.dx/2);
					y -= (sSize.dy/2);
					break;
				case Right:
					x -= sSize.dx;
					y -= (sSize.dy/2);
					break;
				case BottomLeft:
					y -= sSize.dy;
					break;
				case BottomCenter:
					x -= (sSize.dx/2);
					y -= sSize.dy;
					break;
				case BottomRight:
					x -= sSize.dx;
					y -= sSize.dy;
					break;
				}

				font->DrawString(x, y, wStr);
			}
		}

		listToDraw.clear();
	}
#endif RHI_COMPLETE_EDITOR
}

void TextDrawSystem::DrawText(int x, int y, const DAVA::String &text, const DAVA::Color &color, Align align)
{
	DrawText(DAVA::Vector2((DAVA::float32)x, (DAVA::float32)y), text, color);
}

void TextDrawSystem::DrawText(DAVA::Vector2 pos2d, const DAVA::String &text, const DAVA::Color &color, Align align)
{
	if(pos2d.x >= 0 && pos2d.y >= 0)
	{
		TextToDraw ttd(pos2d, text, color, align);
		listToDraw.push_back(ttd);
	}
}
