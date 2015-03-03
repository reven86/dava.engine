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
    
    const QSize cMinSize = QSize( 200, 200 );

    const std::map< quint32, DAVA::char16 > keyMap_WIN =
    {

        { 65, DVKEY_A },
        { 66, DVKEY_B },
        { 67, DVKEY_C },
        { 68, DVKEY_D },
        { 69, DVKEY_E },
        { 70, DVKEY_F },
        { 71, DVKEY_G },
        { 72, DVKEY_H },
        { 73, DVKEY_I },
        { 74, DVKEY_J },
        { 75, DVKEY_K },
        { 76, DVKEY_L },
        { 77, DVKEY_M },
        { 78, DVKEY_N },
        { 79, DVKEY_O },
        { 80, DVKEY_P },
        { 81, DVKEY_Q },
        { 82, DVKEY_R },
        { 83, DVKEY_S },
        { 84, DVKEY_T },
        { 85, DVKEY_U },
        { 86, DVKEY_V },
        { 87, DVKEY_W },
        { 88, DVKEY_X },
        { 89, DVKEY_Y },
        { 90, DVKEY_Z },

        { 48, DVKEY_0 },
        { 49, DVKEY_1 },
        { 50, DVKEY_2 },
        { 51, DVKEY_3 },
        { 52, DVKEY_4 },
        { 53, DVKEY_5 },
        { 54, DVKEY_6 },
        { 55, DVKEY_7 },
        { 56, DVKEY_8 },
        { 57, DVKEY_9 },

    };

    const std::map< quint32, DAVA::char16 > keyMap_OSX = 
    {

        { 0,  DVKEY_A },
        { 11, DVKEY_B },
        { 8,  DVKEY_C },
        { 2,  DVKEY_D },
        { 14, DVKEY_E },
        { 3,  DVKEY_F },
        { 5,  DVKEY_G },
        { 4,  DVKEY_H },
        { 34, DVKEY_I },
        { 38, DVKEY_J },
        { 40, DVKEY_K },
        { 37, DVKEY_L },
        { 46, DVKEY_M },
        { 45, DVKEY_N },
        { 31, DVKEY_O },
        { 35, DVKEY_P },
        { 12, DVKEY_Q },
        { 15, DVKEY_R },
        { 1,  DVKEY_S },
        { 17, DVKEY_T },
        { 32, DVKEY_U },
        { 9,  DVKEY_V },
        { 13, DVKEY_W },
        { 7,  DVKEY_X },
        { 16, DVKEY_Y },
        { 6,  DVKEY_Z },

        { 29, DVKEY_0 },
        { 18, DVKEY_1 },
        { 19, DVKEY_2 },
        { 20, DVKEY_3 },
        { 21, DVKEY_4 },
        { 23, DVKEY_5 },
        { 22, DVKEY_6 },
        { 26, DVKEY_7 },
        { 28, DVKEY_8 },
        { 25, DVKEY_9 },

    };

#if defined( Q_OS_WIN )
    const std::map< quint32, DAVA::char16 > keyMap = keyMap_WIN;
#elif defined( Q_OS_MAC )
    const std::map< quint32, DAVA::char16 > keyMap = keyMap_OSX;
#endif

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
    if(event->type() == QEvent::UpdateRequest)
    {
        renderNow();
        return true;
    }
    
    return QWindow::event(event);
}

void OpenGLWindow::keyPressEvent(QKeyEvent *e)
{
    const Qt::KeyboardModifiers modifiers = e->modifiers();
    
    if(modifiers & Qt::ShiftModifier)
    {
        DAVA::InputSystem::Instance()->GetKeyboard().OnKeyPressed(DAVA::DVKEY_SHIFT);
    }
    if(modifiers & Qt::ControlModifier)
    {
        DAVA::InputSystem::Instance()->GetKeyboard().OnKeyPressed(DAVA::DVKEY_CTRL);
    }
    if(modifiers & Qt::AltModifier)
    {
        DAVA::InputSystem::Instance()->GetKeyboard().OnKeyPressed(DAVA::DVKEY_ALT);
    }
    
    char davaKey = MapQtKeyToDAVA(e);
    if(davaKey)
    {
        DAVA::QtLayer::Instance()->KeyPressed(davaKey, e->count(), e->timestamp());
    }
}

void OpenGLWindow::keyReleaseEvent(QKeyEvent *e)
{
    int key = e->key();
    
    if(Qt::Key_Shift == key)
    {
        DAVA::InputSystem::Instance()->GetKeyboard().OnKeyUnpressed(DAVA::DVKEY_SHIFT);
    }
    else if(Qt::Key_Control == key)
    {
        DAVA::InputSystem::Instance()->GetKeyboard().OnKeyUnpressed(DAVA::DVKEY_CTRL);
    }
    else if(Qt::Key_Alt == key)
    {
        DAVA::InputSystem::Instance()->GetKeyboard().OnKeyUnpressed(DAVA::DVKEY_ALT);
    }

    char davaKey = MapQtKeyToDAVA(e);
    if(davaKey)
    {
        DAVA::QtLayer::Instance()->KeyReleased(davaKey);
    }
}

DAVA::char16 OpenGLWindow::MapQtKeyToDAVA(const QKeyEvent *event)
{
    const quint32 nativeKey = event->nativeVirtualKey();
    DAVA::char16 davaKey = DVKEY_UNKNOWN;

    const auto it = keyMap.find( nativeKey );
    if ( it != keyMap.end() )
    {
        davaKey = it->second;
    }

    return davaKey;
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
    
    openGlWindow->installEventFilter(this);

    layout()->addWidget( container );
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
