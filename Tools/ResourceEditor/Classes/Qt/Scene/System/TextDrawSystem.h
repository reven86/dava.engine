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


#ifndef __TEXT_DRAW_SYSTEM_H__
#define __TEXT_DRAW_SYSTEM_H__

#include "CameraSystem.h"

// framework
#include "Entity/SceneSystem.h"
#include "Render/2D/GraphicFont.h"

class TextDrawSystem : public DAVA::SceneSystem
{
public:
    enum class Align : DAVA::uint8
    {
        TopLeft,
        TopCenter,
        TopRight,
        Left,
        Center,
        Right,
        BottomLeft,
        BottomCenter,
        BottomRight
    };

public:
    TextDrawSystem(DAVA::Scene* scene, SceneCameraSystem* cameraSystem);
    ~TextDrawSystem();

    DAVA::Vector2 ToPos2d(const DAVA::Vector3& pos3d) const;

    void DrawText(DAVA::int32 x, DAVA::int32 y, const DAVA::String& text, const DAVA::Color& color, Align align = Align::TopLeft);
    void DrawText(const DAVA::Vector2& pos2d, const DAVA::String& text, const DAVA::Color& color, Align align = Align::TopLeft);

    DAVA::GraphicFont* GetFont() const;

protected:
	struct TextToDraw
	{
        TextToDraw(DAVA::Vector2 _pos, const DAVA::String& _text, const DAVA::Color& _color, Align _align)
            : pos(_pos)
            , text(_text)
            , color(_color)
            , align(_align)
        {
        }

        DAVA::Vector2 pos;
        DAVA::String text;
        DAVA::Color color;
		Align align;
	};

    using GraphicFontVertexVector = DAVA::Vector<DAVA::GraphicFont::GraphicFontVertex>;

protected:
    void Draw();

    void AdjustPositionBasedOnAlign(DAVA::float32& x, DAVA::float32& y, const DAVA::Size2i& size, Align align);
    void PushNextBatch(const DAVA::Color& color);

private:
    friend class SceneEditor2;

    SceneCameraSystem* cameraSystem = nullptr;
    DAVA::GraphicFont* font = nullptr;
    DAVA::NMaterial* fontMaterial = nullptr;
    DAVA::Vector<TextToDraw> textToDraw;
    GraphicFontVertexVector vertices;
};

inline DAVA::GraphicFont* TextDrawSystem::GetFont() const
{
    return font;
}

#endif // GUARD
