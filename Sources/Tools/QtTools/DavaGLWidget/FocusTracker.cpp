#include "FocusTracker.h"

#include "DavaGLWidget.h"

#include <QApplication>
#include <QLineEdit>
#include <QSpinBox>


FocusTracker::FocusTracker( DavaGLWidget* _glWidget )
    : QObject( _glWidget )
    , glWidget( _glWidget )
    , glWindow( _glWidget->GetGLWindow() )
    , isFocused( false )
    , needToRestoreFocus( false )
{}

FocusTracker::~FocusTracker()
{}

void FocusTracker::OnClick()
{
    needToRestoreFocus = false;
    if ( !isFocused )
    {
        glWindow->requestActivate();
    }
}

void FocusTracker::OnEnter()
{
    auto rootWidget = glWidget->window();
    if ( rootWidget == nullptr )
        return;
    
    needToRestoreFocus = (!isFocused);
    prevWidget = QApplication::focusWidget();
    if ( prevWidget.isNull() )
    {
        prevWidget = QApplication::activeWindow();
    }
    
    const bool needToSetFocus =
    !prevWidget.isNull() &&
    !isEditor( prevWidget ) &&
    prevWidget->window() == rootWidget;
    
    if ( !isFocused && needToSetFocus )
    {
        glWindow->requestActivate();
    }
}

void FocusTracker::OnLeave()
{
    if ( needToRestoreFocus && !prevWidget.isNull() )
    {
        prevWidget->setFocus();
    }
    
    needToRestoreFocus = false;
}

void FocusTracker::OnFocusIn()
{
    isFocused = true;
}

void FocusTracker::OnFocusOut()
{
    isFocused = false;
}

bool FocusTracker::isEditor( QWidget* w )
{
    if ( w == nullptr )
        return false;
    
    if ( qobject_cast<QLineEdit *> ( w ) != nullptr )
        return true;
    if ( qobject_cast<QAbstractSpinBox *>( w ) != nullptr )
        return true;
    
    return false;
}