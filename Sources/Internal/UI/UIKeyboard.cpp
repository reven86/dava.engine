#include "UI/UIKeyboard.h"
#include "UI/UIKeyboardListener.h"
#include "UIKeyboard_iOSImpl.h"
#include "Base/BaseMath.h"

namespace DAVA
{
UIKeyboard::~UIKeyboard()
{

}

UIKeyboard::UIKeyboard()
{
    impl = new UIKeyboardImpl( this );
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
void UIKeyboard::AddListener( UIKeyboardListener * listener )
{
    listeners.insert( listener );
}

void UIKeyboard::RemoveListener( UIKeyboardListener * listener )
{
    listeners.erase( listener );
}

void UIKeyboard::SendWillShowNotification( const Rect &keyboardRect )
{
    for( Set<UIKeyboardListener *>::iterator it = listeners.begin(); it != listeners.end(); ++it )
    {
        (*it)->OnKeyboardWillShow( keyboardRect );
    }
}
void UIKeyboard::SendDidShowNotification( const Rect &keyboardRect )
{
    for( Set<UIKeyboardListener *>::iterator it = listeners.begin(); it != listeners.end(); ++it )
    {
        (*it)->OnKeyboardDidShow( keyboardRect );
    }
}
void UIKeyboard::SendWillHideNotification()
{
    for( Set<UIKeyboardListener *>::iterator it = listeners.begin(); it != listeners.end(); ++it )
    {
        (*it)->OnKeyboardWillHide();
    }
}
void UIKeyboard::SendDidHideNotification()
{
    for( Set<UIKeyboardListener *>::iterator it = listeners.begin(); it != listeners.end(); ++it )
    {
        (*it)->OnKeyboardDidHide();
    }
}

}