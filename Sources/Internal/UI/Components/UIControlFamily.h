#ifndef __DAVAENGINE_UI_CONTROL_FAMILY_H__
#define __DAVAENGINE_UI_CONTROL_FAMILY_H__

#include "Entity/BaseFamily.h"
#include "UI/Components/UIComponent.h"

namespace DAVA
{
class UIControlFamily : public BaseFamily<UIComponent>
{
private:
    UIControlFamily(const Vector<UIComponent*>& components);

public:
    static UIControlFamily* GetOrCreate(const Vector<UIComponent*>& components);
    static void Release(UIControlFamily*& family);
};
}

#endif //__DAVAENGINE_UI_CONTROL_FAMILY_H__
