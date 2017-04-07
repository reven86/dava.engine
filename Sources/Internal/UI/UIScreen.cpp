#include "UI/UIScreen.h"
#include "UI/UIControlSystem.h"
#include "Render/RenderHelper.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "Render/RHI/Common/PreProcess.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Time/SystemTimer.h"

namespace DAVA
{
List<UIScreen*> UIScreen::appScreens;
int32 UIScreen::groupIdCounter = -1;

DAVA_VIRTUAL_REFLECTION_IMPL(UIScreen)
{
    ReflectionRegistrator<UIScreen>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UIScreen* o) { o->Release(); })
    .End();
}

UIScreen::UIScreen(const Rect& rect)
    : UIControl(rect)
    , groupId(groupIdCounter)
{
    // add screen to list
    appScreens.push_back(this);
    groupIdCounter--;
    isLoaded = false;
    fillBorderOrder = FILL_BORDER_AFTER_DRAW;
}

UIScreen::~UIScreen()
{
    // remove screen from list
    for (List<UIScreen*>::iterator t = appScreens.begin(); t != appScreens.end(); ++t)
    {
        if (*t == this)
        {
            appScreens.erase(t);
            break;
        }
    }
}

void UIScreen::SystemScreenSizeChanged(const Rect& newFullScreenRect)
{
    SetSize(newFullScreenRect.GetSize());
    UIControl::SystemScreenSizeChanged(newFullScreenRect);
}

void UIScreen::SetFillBorderOrder(UIScreen::eFillBorderOrder fillOrder)
{
    fillBorderOrder = fillOrder;
}

void UIScreen::SystemDraw(const UIGeometricData& geometricData, const UIControlBackground* parentBackground)
{
    if (fillBorderOrder == FILL_BORDER_BEFORE_DRAW)
    {
        FillScreenBorders(geometricData);
        UIControl::SystemDraw(geometricData, parentBackground);
    }
    else if (fillBorderOrder == FILL_BORDER_AFTER_DRAW)
    {
        UIControl::SystemDraw(geometricData, parentBackground);
        FillScreenBorders(geometricData);
    }
    else
    {
        UIControl::SystemDraw(geometricData, parentBackground);
    }
}

void UIScreen::FillScreenBorders(const UIGeometricData& geometricData)
{
    static auto drawColor(Color::Black);

    UIGeometricData drawData;
    drawData.position = relativePosition;
    drawData.size = size;
    drawData.pivotPoint = GetPivotPoint();
    drawData.scale = scale;
    drawData.angle = angle;
    drawData.AddGeometricData(geometricData);

    Rect drawRect = drawData.GetUnrotatedRect();
    Rect fullRect = UIControlSystem::Instance()->vcs->GetFullScreenVirtualRect();
    Vector2 virtualSize = Vector2(static_cast<float32>(UIControlSystem::Instance()->vcs->GetVirtualScreenSize().dx),
                                  static_cast<float32>(UIControlSystem::Instance()->vcs->GetVirtualScreenSize().dy));
    if (fullRect.x < 0)
    {
        auto rect1 = Rect(fullRect.x, 0, -fullRect.x, virtualSize.y);
        RenderSystem2D::Instance()->FillRect(rect1, drawColor);
        auto rect2 = Rect(fullRect.dx - fullRect.x, 0, fullRect.x, virtualSize.y);
        RenderSystem2D::Instance()->FillRect(rect2, drawColor);
    }
    else
    {
        auto rect1 = Rect(0, fullRect.y, virtualSize.x + 1, -fullRect.y);
        RenderSystem2D::Instance()->FillRect(rect1, drawColor);
        auto rect2 = Rect(0, fullRect.dy, virtualSize.x + 1, -fullRect.y);
        RenderSystem2D::Instance()->FillRect(rect2, drawColor);
    }
}

void UIScreen::LoadGroup()
{
    //Logger::FrameworkDebug("load group started");
    //uint64 loadGroupStart = SystemTimer::GetMs();
    ShaderPreprocessScope preprocessScope;

    if (groupId < 0)
    {
        if (isLoaded)
            return;

        LoadResources();
        isLoaded = true;
    }
    else
    {
        int32 screenGroupId = groupId;
        for (List<UIScreen*>::iterator t = appScreens.begin(); t != appScreens.end(); ++t)
        {
            UIScreen* screen = *t;
            if ((screen->groupId == screenGroupId) && (!screen->isLoaded))
            {
                screen->LoadResources();
                screen->isLoaded = true;
            }
        }
    }
    //uint64 loadGroupEnd = SystemTimer::GetMs();
    //Logger::FrameworkDebug("load group finished: %lld", loadGroupEnd - loadGroupStart);
}

void UIScreen::UnloadGroup()
{
    if (groupId < 0)
    {
        if (!isLoaded)
            return;

        UnloadResources();
        isLoaded = false;
    }
    else
    {
        int32 screenGroupId = groupId;
        for (List<UIScreen*>::iterator t = appScreens.begin(); t != appScreens.end(); ++t)
        {
            UIScreen* screen = *t;
            if ((screen->groupId == screenGroupId) && (screen->isLoaded))
            {
                screen->UnloadResources();
                screen->isLoaded = false;
            }
        }
    }
}

bool UIScreen::IsLoaded()
{
    return isLoaded;
}

void UIScreen::AddToGroup(int32 _groupId)
{
    groupId = _groupId;
}

void UIScreen::RemoveFromGroup()
{
    groupId = groupIdCounter--;
}

int32 UIScreen::GetGroupId()
{
    return groupId;
}
};