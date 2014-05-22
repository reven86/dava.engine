#include "UI/UIKeyboard.h"
#include "UI/UIKeyboardListener.h"

namespace DAVA
{
UIKeyboard::~UIKeyboard()
{

}

UIKeyboard::UIKeyboard()
{

}

void UIKeyboard::OpenKeyboard()
{

}

void UIKeyboard::CloseKeyboard()
{

}

void UIKeyboard::AddListener( UIKeyboardListener * listener )
{
    listeners.insert( listener );
}

void UIKeyboard::RemoveListener( UIKeyboardListener * listener )
{
    listeners.erase( listener );
}

void UIKeyboard::NotifyKeyboardShownNotify( const Rect& keyboardRect )
{
    for( Set<UIKeyboardListener *>::iterator it = listeners.begin(); it != listeners.end(); ++it )
    {
        (*it)->OnKeyboardShown( keyboardRect );
    }
}

void UIKeyboard::NotifyKeyboardHiddenNotify()
{
    for( Set<UIKeyboardListener *>::iterator it = listeners.begin(); it != listeners.end(); ++it )
    {
        (*it)->OnKeyboardHidden();
    }
}

}