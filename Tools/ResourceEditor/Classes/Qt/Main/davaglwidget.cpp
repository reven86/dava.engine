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
    #include "Platform/Qt5/Win32/CorePlatformWin32Qt.h"
#endif //#if defined (__DAVAENGINE_MACOS__)

#include "davaglwidget.h"

#include <QMessageBox>
#include <QTimer>
#include <QElapsedTimer>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QDebug>


#include <QOpenGLContext>
#include <QOpenGLFunctions>



#include "Main/mainwindow.h"
#include "ui_mainwindow.h"



DavaGLWidget::DavaGLWidget(QWidget *parent)
	: QOpenGLWidget(parent)
    , renderTimer(nullptr)
    , fps(60)
{
    setFocusPolicy( Qt::StrongFocus );
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    setMinimumSize(100, 100);
    
    setAttribute(Qt::WA_TranslucentBackground);

    DAVA::QtLayer::Instance()->SetDelegate(this);

	setAcceptDrops(true);
    setMouseTracking(true);
    
    SetFPS(60);
    
    renderTimer = new QTimer(this);
    renderTimer->singleShot(1000 / fps, this, SLOT(OnRenderTimer()));
}

DavaGLWidget::~DavaGLWidget()
{
}

void DavaGLWidget::SetFPS(int _fps)
{
    DVASSERT(0 != fps);
    
    DAVA::RenderManager::Instance()->SetFPS(fps);
    fps = _fps;
}


void DavaGLWidget::initializeGL()
{
    DAVA::QtLayer::Instance()->InitializeGlWindow();
}

void DavaGLWidget::resizeGL(int w, int h)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    DAVA::QtLayer::Instance()->Resize(w * 2, h * 2);
    
    emit Resized(w* 2, h* 2);
}


void DavaGLWidget::paintGL()
{
    DAVA::QtLayer::Instance()->ProcessFrame();
}


void DavaGLWidget::OnRenderTimer()
{
    update();
    
    DAVA::float32 requestedFrameDelta = 1000.f / fps;
    DAVA::float32 frameDelta = DAVA::SystemTimer::Instance()->FrameDelta();

    float32 nextFrameDelta = Max(1.f, requestedFrameDelta - frameDelta);
    renderTimer->singleShot((int) nextFrameDelta, this, SLOT(OnRenderTimer()));
}


void DavaGLWidget::keyPressEvent(QKeyEvent *e)
{
    char16 keyChar = e->nativeVirtualKey();
    DAVA::QtLayer::Instance()->KeyPressed(keyChar, e->count(), e->timestamp());
}

void DavaGLWidget::keyReleaseEvent(QKeyEvent *e)
{
    char16 keyChar = e->nativeVirtualKey();
    DAVA::QtLayer::Instance()->KeyReleased(keyChar);
}


void DavaGLWidget::mouseMoveEvent(QMouseEvent * event)
{
    DAVA::UIEvent davaEvent = MapMouseEventToDAVA(event);
    davaEvent.phase = DAVA::UIEvent::PHASE_DRAG;

    DAVA::QtLayer::Instance()->MouseEvent(davaEvent);
}

void DavaGLWidget::mousePressEvent(QMouseEvent * event)
{
    DAVA::UIEvent davaEvent = MapMouseEventToDAVA(event);
    davaEvent.phase = DAVA::UIEvent::PHASE_BEGAN;

    DAVA::QtLayer::Instance()->MouseEvent(davaEvent);
}

void DavaGLWidget::mouseReleaseEvent(QMouseEvent * event)
{
    DAVA::UIEvent davaEvent = MapMouseEventToDAVA(event);
    davaEvent.phase = DAVA::UIEvent::PHASE_ENDED;
    
    DAVA::QtLayer::Instance()->MouseEvent(davaEvent);
}

void DavaGLWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    DAVA::UIEvent davaEvent = MapMouseEventToDAVA(event);
    davaEvent.phase = DAVA::UIEvent::PHASE_ENDED;
    davaEvent.tapCount = 2;
    
    DAVA::QtLayer::Instance()->MouseEvent(davaEvent);
}

DAVA::UIEvent DavaGLWidget::MapMouseEventToDAVA(const QMouseEvent *event)
{
    DAVA::UIEvent davaEvent;
    
    QPoint pos = event->pos();
    davaEvent.point = davaEvent.physPoint = Vector2(pos.x()* 2, pos.y()* 2);
    davaEvent.tid = MapQtButtonToDAVA(event->button());
    davaEvent.timestamp = event->timestamp();
    davaEvent.tapCount = 1;

    return davaEvent;
}


DAVA::UIEvent::eButtonID DavaGLWidget::MapQtButtonToDAVA(const Qt::MouseButton button)
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

void DavaGLWidget::dragEnterEvent(QDragEnterEvent *event)
{
    event->setDropAction(Qt::LinkAction);
	event->accept();
}

void DavaGLWidget::dragMoveEvent(QDragMoveEvent *event)
{
	DAVA::Vector<DAVA::UIEvent> touches;
	DAVA::Vector<DAVA::UIEvent> emptyTouches;

	DAVA::UIEvent newTouch;
	newTouch.tid = 1;
	newTouch.physPoint.x = event->pos().x();
	newTouch.physPoint.y = event->pos().y();
	newTouch.phase = DAVA::UIEvent::PHASE_MOVE;
	touches.push_back(newTouch);

	DAVA::UIControlSystem::Instance()->OnInput(DAVA::UIEvent::PHASE_MOVE, emptyTouches, touches);

    event->setDropAction(Qt::LinkAction);
	event->accept();
}

void DavaGLWidget::dropEvent(QDropEvent *event)
{
	const QMimeData *mimeData = event->mimeData();
	emit OnDrop(mimeData);

    event->setDropAction(Qt::LinkAction);
	event->accept();
}


void DavaGLWidget::Quit()
{
    exit(0);
}

void DavaGLWidget::ShowAssertMessage(const char * message)
{
    QMessageBox::critical(this, "", message);
}

