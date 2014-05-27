#include "UI/UISystemKeyboardImpl.h"
#include "UI/UISystemKeyboard.h"

namespace DAVA
{
UISystemKeyboardImpl::UISystemKeyboardImpl( UISystemKeyboard *kb )
{
    keyboard = kb;
}
    
UISystemKeyboardImpl::~UISystemKeyboardImpl()
{
}

void UISystemKeyboardImpl::SendWillShowNotification( const Rect &kbRect )
{
    keyboard->SendWillShowNotification( kbRect );
}
    
void UISystemKeyboardImpl::SendDidShowNotification( const Rect &kbRect )
{
    keyboard->SendDidShowNotification( kbRect );
}
    
void UISystemKeyboardImpl::SendWillHideNotification()
{
    keyboard->SendWillHideNotification();
}
    
void UISystemKeyboardImpl::SendDidHideNotification()
{
    keyboard->SendDidHideNotification();
}
    
};
