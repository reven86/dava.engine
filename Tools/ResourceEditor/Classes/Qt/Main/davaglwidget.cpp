/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



#include "DAVAEngine.h"
#include "UI/UIEvent.h"
#include "UI/UIControlSystem.h"

#include "Platform/Qt5/QtLayer.h"

#if defined (__DAVAENGINE_MACOS__)
    #include "Platform/Qt5/MacOS/CoreMacOSPlatformQt.h"
#elif defined (__DAVAENGINE_WIN32__)
#endif

#include "davaglwidget.h"

#include <QMessageBox>
#include <QTimer>
#include <QElapsedTimer>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QDebug>
#include <QScreen>


#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__)
#include "Network/NetCore.h"
#endif

#include "Main/mainwindow.h"
#include "ui_mainwindow.h"

#include <QOpenGLContext>
#include <QOpenGLPaintDevice>

#include <QBoxLayout>

#if defined( Q_OS_WIN )
#include <QtPlatformheaders/QWGLNativeContext>
#elif defined( Q_OS_MAC )
#endif

#include "Classes/Qt/FrameworkBinding/FrameworkLoop.h"


namespace
{
    const QSize cMinSize = QSize( 180, 180 );
}



OpenGLWindow::OpenGLWindow()
    : QWindow()
{
    paintDevice = nullptr;
    setSurfaceType(QWindow::OpenGLSurface);
    
    setKeyboardGrabEnabled(true);
    setMouseGrabEnabled(true);

    setMinimumSize( cMinSize );
}

OpenGLWindow::~OpenGLWindow()
{
}

void OpenGLWindow::render()
{
    if (!paintDevice)
    {
        paintDevice = new QOpenGLPaintDevice();
    }

    if(paintDevice->size() != size())
    {
        paintDevice->setSize(size());
    }
}

void OpenGLWindow::renderNow()
{
    if (!isExposed())
        return;

    render();
    auto context = FrameworkLoop::Instance()->Context();
    context->swapBuffers( this );
}

void OpenGLWindow::exposeEvent(QExposeEvent *event)
{
    Q_UNUSED(event);
    
    if (isExposed())
    {
        renderNow();
        emit Exposed();
    }
}

bool OpenGLWindow::event(QEvent *event)
{
    switch ( event->type() )
    {
    case QEvent::UpdateRequest:
        renderNow();
        return true;
    case QEvent::FocusOut:
        DAVA::InputSystem::Instance()->GetKeyboard().ClearAllKeys();
        break;
    default:
        break;
    }
    
    return QWindow::event(event);
}

void OpenGLWindow::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
    case Qt::Key_Alt:
        DAVA::InputSystem::Instance()->GetKeyboard().OnKeyPressed( DVKEY_ALT );
        return;
    case Qt::Key_Control:
        DAVA::InputSystem::Instance()->GetKeyboard().OnKeyPressed( DVKEY_CTRL );
        return;
    case Qt::Key_Shift:
        DAVA::InputSystem::Instance()->GetKeyboard().OnKeyPressed( DVKEY_SHIFT );
        return;
    case Qt::Key_CapsLock:
        DAVA::InputSystem::Instance()->GetKeyboard().OnKeyPressed( DVKEY_CAPSLOCK );
        return;
    case Qt::Key_Meta:
        // Ignore Win key on windows, Ctrl key on OSX
        return;
    default:
        break;
    }
    
    const auto davaKey = DAVA::InputSystem::Instance()->GetKeyboard().GetDavaKeyForSystemKey( e->nativeVirtualKey() );
    if (davaKey != DVKEY_UNKNOWN)
    {
        DAVA::QtLayer::Instance()->KeyPressed( davaKey, e->count(), e->timestamp() );
    }
}

void OpenGLWindow::keyReleaseEvent(QKeyEvent *e)
{
    switch (e->key())
    {
    case Qt::Key_Alt:
        DAVA::InputSystem::Instance()->GetKeyboard().OnKeyUnpressed( DVKEY_ALT );
        return;
    case Qt::Key_Control:
        DAVA::InputSystem::Instance()->GetKeyboard().OnKeyUnpressed( DVKEY_CTRL );
        return;
    case Qt::Key_Shift:
        DAVA::InputSystem::Instance()->GetKeyboard().OnKeyUnpressed( DVKEY_SHIFT );
        return;
    case Qt::Key_CapsLock:
        DAVA::InputSystem::Instance()->GetKeyboard().OnKeyUnpressed( DVKEY_CAPSLOCK );
        return;
    case Qt::Key_Meta:
        // Ignore Win key on windows, Ctrl key on OSX
        return;
    default:
        break;
    }
    
    const auto davaKey = DAVA::InputSystem::Instance()->GetKeyboard().GetDavaKeyForSystemKey( e->nativeVirtualKey() );
    if (davaKey != DVKEY_UNKNOWN)
    {
        DAVA::InputSystem::Instance()->GetKeyboard().OnKeyUnpressed(davaKey);
    }
}

void OpenGLWindow::mouseMoveEvent(QMouseEvent * event)
{
    const Qt::MouseButtons buttons = event->buttons();
    DAVA::UIEvent davaEvent = MapMouseEventToDAVA(event);
    bool dragWasApplied = false;

    davaEvent.phase = DAVA::UIEvent::PHASE_DRAG;
    
    if(buttons & Qt::LeftButton)
    {
        dragWasApplied = true;
        davaEvent.tid = MapQtButtonToDAVA(Qt::LeftButton);
        DAVA::QtLayer::Instance()->MouseEvent(davaEvent);
    }
    if(buttons & Qt::RightButton)
    {
        dragWasApplied = true;
        davaEvent.tid = MapQtButtonToDAVA(Qt::RightButton);
        DAVA::QtLayer::Instance()->MouseEvent(davaEvent);
    }
    if(buttons & Qt::MiddleButton)
    {
        dragWasApplied = true;
        davaEvent.tid = MapQtButtonToDAVA(Qt::MiddleButton);
        DAVA::QtLayer::Instance()->MouseEvent(davaEvent);
    }
    
    if(!dragWasApplied)
    {
        davaEvent.phase = DAVA::UIEvent::PHASE_MOVE;
        davaEvent.tid = MapQtButtonToDAVA(Qt::LeftButton);
        DAVA::QtLayer::Instance()->MouseEvent(davaEvent);
    }
}

void OpenGLWindow::mousePressEvent(QMouseEvent * event)
{
    DAVA::UIEvent davaEvent = MapMouseEventToDAVA(event);
    davaEvent.phase = DAVA::UIEvent::PHASE_BEGAN;
    
    DAVA::QtLayer::Instance()->MouseEvent(davaEvent);
    
    emit mousePressed();
}

void OpenGLWindow::mouseReleaseEvent(QMouseEvent * event)
{
    DAVA::UIEvent davaEvent = MapMouseEventToDAVA(event);
    davaEvent.phase = DAVA::UIEvent::PHASE_ENDED;
    
    DAVA::QtLayer::Instance()->MouseEvent(davaEvent);
}

void OpenGLWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    DAVA::UIEvent davaEvent = MapMouseEventToDAVA(event);
    davaEvent.phase = DAVA::UIEvent::PHASE_ENDED;
    davaEvent.tapCount = 2;
    
    DAVA::QtLayer::Instance()->MouseEvent(davaEvent);
}

void OpenGLWindow::wheelEvent(QWheelEvent *event)
{
    DAVA::UIEvent davaEvent;
    
    int numDegrees = event->delta() / 8;
    int numSteps = numDegrees / 15;
    
    davaEvent.point = davaEvent.physPoint = Vector2(numSteps, numSteps);
    davaEvent.tid = DAVA::UIEvent::PHASE_WHEEL;
    davaEvent.phase = DAVA::UIEvent::PHASE_WHEEL;
    
    davaEvent.timestamp = event->timestamp();
    davaEvent.tapCount = 1;
    
    DAVA::QtLayer::Instance()->MouseEvent(davaEvent);
}



DAVA::UIEvent OpenGLWindow::MapMouseEventToDAVA(const QMouseEvent *event) const
{
    DAVA::UIEvent davaEvent;
    QPoint pos = event->pos();
    
    int currentDPR = devicePixelRatio();
    davaEvent.point = davaEvent.physPoint = Vector2(pos.x() * currentDPR, pos.y() * currentDPR);
    davaEvent.tid = MapQtButtonToDAVA(event->button());
    davaEvent.timestamp = event->timestamp();
    davaEvent.tapCount = 1;
    
    return davaEvent;
}

DAVA::UIEvent::eButtonID OpenGLWindow::MapQtButtonToDAVA(const Qt::MouseButton button) const
{
    switch (button)
    {
        case Qt::LeftButton:
            return DAVA::UIEvent::BUTTON_1;
            
        case Qt::RightButton:
            return DAVA::UIEvent::BUTTON_2;
            
        case Qt::MiddleButton:
            return DAVA::UIEvent::BUTTON_3;
            
        default:
            break;
    }
    
    return DAVA::UIEvent::BUTTON_NONE;
}

///=======================
DavaGLWidget::DavaGLWidget(QWidget *parent)
    : QWidget(parent)
    , isInitialized(false)
    , currentDPR(1)
    , currentWidth(0)
    , currentHeight(0)
{
    setAcceptDrops(true);
    setMouseTracking(true);

    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    setMinimumSize(cMinSize);
    
    openGlWindow = new OpenGLWindow();
    connect( openGlWindow, &OpenGLWindow::Exposed, this, &DavaGLWidget::OnWindowExposed );
    connect( openGlWindow, &QWindow::screenChanged, this, &DavaGLWidget::PerformSizeChange );
    
    auto l = new QBoxLayout(QBoxLayout::TopToBottom, this);
    l->setMargin( 0 );
    setLayout( l );
    
    container = createWindowContainer( openGlWindow );
    container->setAcceptDrops(true);
    container->setMouseTracking(true);
    container->setFocusPolicy(Qt::WheelFocus);
    //container->setFocusPolicy(Qt::NoFocus);

    openGlWindow->installEventFilter(this);

    layout()->addWidget(container);

    focusTracker = new FocusTracker(this);
}

DavaGLWidget::~DavaGLWidget()
{
}

OpenGLWindow* DavaGLWidget::GetGLWindow()
{
    return openGlWindow;
}

void DavaGLWidget::OnWindowExposed()
{
    disconnect( openGlWindow, &OpenGLWindow::Exposed, this, &DavaGLWidget::OnWindowExposed );

    currentDPR = devicePixelRatio();
    
    const auto contextId = FrameworkLoop::Instance()->GetRenderContextId();
    DAVA::QtLayer::Instance()->InitializeGlWindow( contextId );
    
    isInitialized = true;

    PerformSizeChange();
    emit Initialized();
}

void DavaGLWidget::resizeEvent(QResizeEvent *e)
{
    currentWidth = e->size().width();
    currentHeight = e->size().height();
    
    QWidget::resizeEvent(e);
    PerformSizeChange();
}

bool DavaGLWidget::eventFilter( QObject* watched, QEvent* event )
{
    if ( watched == openGlWindow )
    {
        switch ( event->type() )
        {
        case QEvent::DragEnter:
            {
                auto e = static_cast<QDragEnterEvent *>( event );
                e->setDropAction( Qt::LinkAction );
                e->accept();
            }
            break;
        case QEvent::DragMove:
            {
                auto e = static_cast<QDragMoveEvent *>( event );
                e->setDropAction( Qt::LinkAction );
                e->accept();
            }
            break;
        case QEvent::DragLeave:
            break;
        case QEvent::Drop:
            {
                auto e = static_cast<QDropEvent *>( event );
                emit OnDrop( e->mimeData() );
                e->setDropAction( Qt::LinkAction );
                e->accept();
            }
            break;

        case QEvent::MouseButtonPress:
            focusTracker->OnClick();
            break;
        case QEvent::Enter:
            focusTracker->OnEnter();
            break;
        case QEvent::Leave:
            focusTracker->OnLeave();
            break;
        case QEvent::FocusIn:
            focusTracker->OnFocusIn();
            break;
        case QEvent::FocusOut:
            focusTracker->OnFocusOut();
            break;
        default:
            break;
        }
    }

    return QWidget::eventFilter( watched, event );
}

void DavaGLWidget::PerformSizeChange()
{
    currentDPR = openGlWindow->devicePixelRatio();
    if (isInitialized)
    {
        DAVA::QtLayer::Instance()->Resize(currentWidth * currentDPR, currentHeight * currentDPR);
    }
    
    emit Resized(currentWidth, currentHeight, currentDPR);
}


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
    if ( qobject_cast<QSpinBox *>( w ) != nullptr )
        return true;

    return false;
}
