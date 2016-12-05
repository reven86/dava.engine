#pragma once

#include "CameraSystem.h"
#include "Classes/Qt/Scene/System/EditorSceneSystem.h"

// framework
#include "Entity/SceneSystem.h"
#include "Render/2D/GraphicFont.h"

namespace DAVA
{
class NMaterial;
}

class TextDrawSystem : public DAVA::SceneSystem, public EditorSceneSystem
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
    void Draw() override;

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

    void AdjustPositionBasedOnAlign(DAVA::float32& x, DAVA::float32& y, const DAVA::Size2i& size, Align align);
    void PushNextBatch(const DAVA::Color& color);

private:
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
