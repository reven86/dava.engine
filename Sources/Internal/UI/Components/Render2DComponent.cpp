#include "Render2DComponent.h"

#if 0

namespace DAVA
{

DAVA::Render2DComponent::Render2DComponent()
{
    visible = true;
    visibleForUIEditor = true;
    background = new UIControlBackground();
    clipContents = false;

    debugDrawEnabled = false;
    debugDrawColor = Color(1.0f, 0.0f, 0.0f, 1.0f);

    drawPivotPointMode = DRAW_NEVER;
}

Render2DComponent::~Render2DComponent()
{
    SafeRelease(background);
}

DAVA::Component* DAVA::Render2DComponent::Clone(UIControl* toControl)
{
}

void DAVA::Render2DComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
}

void DAVA::Render2DComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
}

}

#endif //0

