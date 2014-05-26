#include "UI/UISystemKeyboard.h"
#include "UI/UISystemKeyboardListener.h"
#include "Base/BaseMath.h"

#if defined (__DAVAENGINE_ANDROID__)
#include "UIKeyboard_iOSImpl.h"
#elif defined (__DAVAENGINE_IPHONE__)
#include "UIKeyboard_iOSImpl.h"
#endif

namespace DAVA
{
UISystemKeyboard::~UISystemKeyboard()
{

}

UISystemKeyboard::UISystemKeyboard()
    : impl( NULL )
{
#if defined (__DAVAENGINE_ANDROID__) || defined (__DAVAENGINE_IPHONE__)
    impl = new UISystemKeyboardImpl( this );
#endif
}
/*
void UIKeyboard::Show( UITextField * textField )
{
    impl->Show( textField );
}

void UIKeyboard::Hide( UITextField * textField )
{
    impl->Hide( textField );
}
*/
void UISystemKeyboard::AddListener( UISystemKeyboardListener * listener )
{
    listeners.insert( listener );
}

void UISystemKeyboard::RemoveListener( UISystemKeyboardListener * listener )
{
    listeners.erase( listener );
}

void UISystemKeyboard::SendWillShowNotification( const Rect &keyboardRect )
{
    for( Set<UISystemKeyboardListener *>::iterator it = listeners.begin(); it != listeners.end(); ++it )
    {
        (*it)->OnKeyboardWillShow( keyboardRect );
    }
}
void UISystemKeyboard::SendDidShowNotification( const Rect &keyboardRect )
{
    for( Set<UISystemKeyboardListener *>::iterator it = listeners.begin(); it != listeners.end(); ++it )
    {
        (*it)->OnKeyboardDidShow( keyboardRect );
    }
}
void UISystemKeyboard::SendWillHideNotification()
{
    for( Set<UISystemKeyboardListener *>::iterator it = listeners.begin(); it != listeners.end(); ++it )
    {
        (*it)->OnKeyboardWillHide();
    }
}
void UISystemKeyboard::SendDidHideNotification()
{
    for( Set<UISystemKeyboardListener *>::iterator it = listeners.begin(); it != listeners.end(); ++it )
    {
        (*it)->OnKeyboardDidHide();
    }
}

}