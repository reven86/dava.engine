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
{}

FocusTracker::~FocusTracker()
{}

void FocusTracker::OnClick()
{
    if ( !isFocused )
    {
        glWindow->requestActivate();
    }
}

void FocusTracker::OnFocusIn()
{
    isFocused = true;
    emit focusIn();
}

void FocusTracker::OnFocusOut()
{
    isFocused = false;
    emit focusOut();
}
