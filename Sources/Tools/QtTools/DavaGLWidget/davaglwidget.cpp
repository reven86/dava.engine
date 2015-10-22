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

#include "Platform/Qt5/QtLayer.h"

#include "davaglwidget.h"
#include "FocusTracker.h"
#include "ControlMapper.h"
#include "QtTools/FrameworkBinding/FrameworkLoop.h"

#include <QKeyEvent>
#include <QScreen>
#include <QDebug>

#include <QOpenGLContext>
#include <QBoxLayout>

namespace
{
    const QSize cMinSize = QSize( 180, 180 );
}

OpenGLWindow::OpenGLWindow()
    : QWindow()
    , controlMapper(new ControlMapper(this))
{
    setSurfaceType(QWindow::OpenGLSurface);
    
    setKeyboardGrabEnabled(true);
    setMouseGrabEnabled(true);

    setMinimumSize( cMinSize );
}

OpenGLWindow::~OpenGLWindow()
{
}

void OpenGLWindow::renderNow()
{
    if (!isExposed())
    {
        return;
    }

    auto context = FrameworkLoop::Instance()->Context();
    context->swapBuffers( this );
}

void OpenGLWindow::exposeEvent(QExposeEvent *event)
{
    Q_UNUSED(event);
    
    if (isExposed())
    {
        //just initialize DAVAGL context
        FrameworkLoop::Instance()->Context();

        emit Exposed();
    }
}

bool OpenGLWindow::event(QEvent *event)
{
    switch ( event->type() )
    {
    // Render
    case QEvent::UpdateRequest:
        renderNow();
        return true;

    // Drag-n-drop
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
            handleDragMoveEvent( e );
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

    // Focus
    case QEvent::FocusOut:
        controlMapper->releaseKeyboard();
        break;

    default:
        break;
    }
    
    return QWindow::event(event);
}

void OpenGLWindow::keyPressEvent(QKeyEvent *e)
{
    controlMapper->keyPressEvent(e);
}

void OpenGLWindow::keyReleaseEvent(QKeyEvent *e)
{
    controlMapper->keyReleaseEvent(e);
}

void OpenGLWindow::mouseMoveEvent(QMouseEvent * e)
{
    controlMapper->mouseMoveEvent(e);
}

void OpenGLWindow::mousePressEvent(QMouseEvent * e)
{
    controlMapper->mousePressEvent(e);
    emit mousePressed();
}

void OpenGLWindow::mouseReleaseEvent(QMouseEvent * e)
{
    controlMapper->mouseReleaseEvent(e);
}

void OpenGLWindow::mouseDoubleClickEvent(QMouseEvent *e)
{
    controlMapper->mouseDoubleClickEvent(e);
}

void OpenGLWindow::wheelEvent(QWheelEvent *e)
{
    if ( e->phase() != Qt::ScrollUpdate )
        return;

    controlMapper->wheelEvent(e);
    if ( e->orientation() == Qt::Vertical )
    {
        emit mouseScrolled( e->delta() );
    }
}

void OpenGLWindow::handleDragMoveEvent(QDragMoveEvent* e)
{
    controlMapper->dragMoveEvent(e);
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
    connect( openGlWindow.data(), &OpenGLWindow::Exposed, this, &DavaGLWidget::OnWindowExposed );
    connect( openGlWindow.data(), &QWindow::screenChanged, this, &DavaGLWidget::PerformSizeChange );
    connect( openGlWindow.data(), &OpenGLWindow::OnDrop, this, &DavaGLWidget::OnDrop );
    
    auto l = new QBoxLayout(QBoxLayout::TopToBottom, this);
    l->setMargin( 0 );
    setLayout( l );
    
    container = createWindowContainer( openGlWindow );
    container->setAcceptDrops(true);
    container->setMouseTracking(true);
    container->setFocusPolicy(Qt::NoFocus);

    layout()->addWidget(container);

    focusTracker = new FocusTracker(this);

//temporary workaround
#if defined(Q_OS_MAC)
    DAVA::Core::Instance()->SetNativeView((void*)openGlWindow->winId());
#elif defined(Q_OS_WIN)
    DAVA::Core::Instance()->SetNativeView((void*)container->winId());
#endif //Q_OS_MAC / Q_OS_WIN
}

DavaGLWidget::~DavaGLWidget()
{
}

OpenGLWindow* DavaGLWidget::GetGLWindow() const
{
    return openGlWindow;
}

bool DavaGLWidget::IsInitialized() const
{
    return isInitialized;
}

void DavaGLWidget::MakeInvisible()
{
    setWindowFlags( Qt::Window | Qt::FramelessWindowHint | Qt::CustomizeWindowHint | Qt::Tool );    // Remove border
#ifdef Q_OS_WIN
   setAttribute( Qt::WA_DontShowOnScreen );             // Under OS X gl widget will be not initialized with this flag
#endif
    setAttribute( Qt::WA_TranslucentBackground );       // Transparent background
    setAttribute( Qt::WA_TransparentForMouseEvents );   // Rethrow mouse events
    setAttribute( Qt::WA_ShowWithoutActivating );       // Do not get focus
    setWindowOpacity( 0.0 );
    setFixedSize( 1, 1 );
    setEnabled( false );
    move( 0, 0 );
}

void DavaGLWidget::OnWindowExposed()
{
    disconnect( openGlWindow.data(), &OpenGLWindow::Exposed, this, &DavaGLWidget::OnWindowExposed );

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

void DavaGLWidget::PerformSizeChange()
{
    if(isInitialized)
    {   //INFO: this magic helps us with OSX OpenGL Context on File dialog
        FrameworkLoop::Instance()->Context();
    }
    
    currentDPR = openGlWindow->devicePixelRatio();
    if (isInitialized)
    {
        DAVA::QtLayer::Instance()->Resize(currentWidth * currentDPR, currentHeight * currentDPR);
    }
    
    emit Resized(currentWidth, currentHeight, currentDPR);
}
