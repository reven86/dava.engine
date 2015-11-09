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

#include "DavaRenderer.h"
#include "Davaglwidget.h"

#include "ControlMapper.h"

#include <QKeyEvent>
#include <QScreen>
#include <QTimer>
#include <QBoxLayout>
#include <QApplication>
#include <QAction>

namespace
{
    const QSize cMinSize = QSize( 180, 180 );
}

DavaGLView::DavaGLView()
    : QQuickWindow()
    , controlMapper(new ControlMapper(this))
{
    setSurfaceType(QWindow::OpenGLSurface);

    setKeyboardGrabEnabled(true);
    setMouseGrabEnabled(true);

    setMinimumSize( cMinSize );
}

bool DavaGLView::event(QEvent* event)
{
    switch ( event->type() )
    {
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
            return QWindow::event(event);
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

    return QQuickWindow::event(event);
}

void DavaGLView::keyPressEvent(QKeyEvent* e)
{
    controlMapper->keyPressEvent(e);
}

void DavaGLView::keyReleaseEvent(QKeyEvent* e)
{
    controlMapper->keyReleaseEvent(e);
}

void DavaGLView::mouseMoveEvent(QMouseEvent* e)
{
    controlMapper->mouseMoveEvent(e);
}

void DavaGLView::mousePressEvent(QMouseEvent* e)
{
    requestActivate();
    controlMapper->mousePressEvent(e);
}

void DavaGLView::mouseReleaseEvent(QMouseEvent* e)
{
    controlMapper->mouseReleaseEvent(e);
}

void DavaGLView::mouseDoubleClickEvent(QMouseEvent* e)
{
    controlMapper->mouseDoubleClickEvent(e);
}

void DavaGLView::wheelEvent(QWheelEvent* e)
{
    if ( e->phase() != Qt::ScrollUpdate )
    {
        return;
    }

    controlMapper->wheelEvent(e);
    if ( e->orientation() == Qt::Vertical )
    {
        emit mouseScrolled(e->angleDelta().y());
    }
}

void DavaGLView::handleDragMoveEvent(QDragMoveEvent* e)
{
    controlMapper->dragMoveEvent(e);
}

///=======================
DavaGLWidget::DavaGLWidget(QWidget *parent)
    : QWidget(parent)
{
//configure Qt Scene Graph to single thread mode
#ifdef Q_OS_WIN
    _putenv_s("QSG_RENDER_LOOP", "basic");
#else
    setenv("QSG_RENDER_LOOP", "basic", 1);
#endif
    setAcceptDrops(true);
    setMouseTracking(true);

    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    setMinimumSize(cMinSize);

    davaGLView = new DavaGLView();

    connect(qApp, &QApplication::focusWindowChanged, [this](QWindow* now) //fix bug with actions focus scope
            {
                bool isActive = (now == davaGLView);
                for (auto& action : actions())
                {
                    action->setEnabled(isActive);
                }
            });

    davaGLView->setClearBeforeRendering(false);
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &DavaGLWidget::UpdateView);
    timer->start(16); //62.5 fps :)
    connect(davaGLView, &QWindow::screenChanged, this, &DavaGLWidget::OnResize);
    connect(davaGLView, &QWindow::screenChanged, this, &DavaGLWidget::ScreenChanged);
    connect(davaGLView, &QQuickWindow::beforeSynchronizing, this, &DavaGLWidget::OnSync, Qt::DirectConnection);
    connect(davaGLView, &QQuickWindow::sceneGraphInvalidated, this, &DavaGLWidget::OnCleanup);
    connect(davaGLView, &DavaGLView::mouseScrolled, this, &DavaGLWidget::mouseScrolled);
    connect(davaGLView, &DavaGLView::OnDrop, this, &DavaGLWidget::OnDrop);
    auto layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    layout->setMargin(0);
    setLayout(layout);

    QWidget* container = createWindowContainer(davaGLView);
    container->setAcceptDrops(true);
    container->setMouseTracking(true);
    container->setFocusPolicy(Qt::NoFocus);

    layout->addWidget(container);
    
#if defined(Q_OS_MAC)
    DAVA::Core::Instance()->SetNativeView((void*)davaGLView->winId());
#elif defined(Q_OS_WIN)
    DAVA::Core::Instance()->SetNativeView((void*)container->winId());
#endif //Q_OS_MAC / Q_OS_WIN
}

void DavaGLWidget::MakeInvisible()
{
    setWindowFlags( Qt::Window | Qt::FramelessWindowHint | Qt::CustomizeWindowHint | Qt::Tool );    // Remove border
    setAttribute( Qt::WA_TransparentForMouseEvents );   // Rethrow mouse events
    setAttribute( Qt::WA_ShowWithoutActivating );       // Do not get focus
    setWindowOpacity( 0.0 );
    setFixedSize( 1, 1 );
    setEnabled( false );
    move( 0, 0 );
}

qreal DavaGLWidget::GetDevicePixelRatio() const
{
    return davaGLView->devicePixelRatio();
}

QQuickWindow* DavaGLWidget::GetGLView()
{
    return davaGLView;
}

void DavaGLWidget::OnResize()
{
    if (nullptr != renderer)
    {
        int currentDPR = davaGLView->devicePixelRatio();
        DAVA::QtLayer::Instance()->Resize(width() * currentDPR, height() * currentDPR);
        emit Resized(width(), height(), currentDPR);
    }
}

void DavaGLWidget::OnSync()
{
    if (nullptr == renderer)
    {
        renderer = new DavaRenderer();
        OnResize();
        connect(davaGLView, &QQuickWindow::beforeRendering, renderer, &DavaRenderer::paint, Qt::DirectConnection);
        emit Initialized();
    }
}
void DavaGLWidget::resizeEvent(QResizeEvent*)
{
    if (nullptr != renderer)
    {
        OnResize();
    }
}

void DavaGLWidget::OnCleanup()
{
    DAVA::SafeDelete(renderer);
}

void DavaGLWidget::UpdateView()
{
    if (!DAVA::DVAssertMessage::IsMessageDisplayed())
    {
        davaGLView->update();
    }
}