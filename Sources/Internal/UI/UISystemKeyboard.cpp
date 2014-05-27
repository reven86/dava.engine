#include "UI/UISystemKeyboard.h"
#include "UI/UISystemKeyboardListener.h"
#include "UI/UISystemKeyboardImpl.h"
#include "Base/BaseMath.h"

namespace DAVA
{
UISystemKeyboard::~UISystemKeyboard()
{

}

UISystemKeyboard::UISystemKeyboard()
    : impl( NULL )
{
    impl = new UISystemKeyboardImpl( this );
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