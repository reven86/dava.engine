#ifndef __DAVAENGINE_UI_SCREEN_H__
#define __DAVAENGINE_UI_SCREEN_H__

#include "Base/BaseTypes.h"
#include "UI/UIControl.h"
#include "Utils/Utils.h"
#include "Core/Core.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"

namespace DAVA
{
class UIScreenTransition;
class UIControlSystem;
class UIScreen : public UIControl
{
public:
    enum eFillBorderOrder
    {
        FILL_BORDER_BEFORE_DRAW = 0
        ,
        FILL_BORDER_AFTER_DRAW
        ,
        FILL_BORDER_NONE
    };

protected:
    virtual ~UIScreen();

public:
    UIScreen(const Rect& rect = Rect(0.0f,
                                     0.0f,
                                     static_cast<float32>(VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize().dx),
                                     static_cast<float32>(VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize().dy)
                                     ));

    /**
	 \brief Sets the fill border ordrer. Borders fills Only when they are present on the screen. You can change filling type by overloading FillScreenBorders method.
	 
	 \param fillOrder Sets the borders filling order.
	 */
    void SetFillBorderOrder(UIScreen::eFillBorderOrder fillOrder);

    /* 
		This is block of functions used by transition
	 */
    virtual void LoadGroup();
    virtual void UnloadGroup();
    virtual bool IsLoaded();

    virtual void AddToGroup(int32 groupId);
    virtual void RemoveFromGroup();

    virtual int32 GetGroupId();
    virtual void SystemDraw(const UIGeometricData& geometricData); // Internal method used by ControlSystem

    virtual void SystemScreenSizeChanged(const Rect& newFullScreenSize);

protected:
    virtual void LoadResources()
    {
    }
    virtual void UnloadResources()
    {
    }

    /**
	 \brief Fills borders thats appears in non proportional screen scaling.
	 
	 \param geometricData Base geometric data. This parameter is'n used in the default realisation.
	 */
    virtual void FillScreenBorders(const UIGeometricData& geometricData);

    int32 groupId;

private:
    bool isLoaded;
    static List<UIScreen*> appScreens;
    static int32 groupIdCounter;
    eFillBorderOrder fillBorderOrder;
};
};

#endif