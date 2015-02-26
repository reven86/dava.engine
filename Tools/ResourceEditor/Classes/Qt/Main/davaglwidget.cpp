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

#include "davaglwidget.h"

#include <QApplication>
#include <QMessageBox>
#include <QResizeEvent>
#include <QTimer>
#include <QElapsedTimer>
#include <QMoveEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QFocusEvent>
#include <QDateTime>
#include <QAbstractEventDispatcher>
#include <QDebug>

#if defined (__DAVAENGINE_MACOS__)
	#include "MacOS/QtLayerMacOS.h"
    #include "MacOS/CoreMacOSPlatformQt.h"
#elif defined (__DAVAENGINE_WIN32__)
	#include "Win32/QtLayerWin32.h"
	#include "Win32/CorePlatformWin32Qt.h"
#endif //#if defined (__DAVAENGINE_MACOS__)

#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__)
#include "Network/NetCore.h"
#endif

#include "Classes/Qt/Main/mainwindow.h"
#include "ui_mainwindow.h"


DavaGLWidget::DavaGLWidget(QWidget *parent)
	: QWidget(parent)
	, maxFPS(60)
	, fps(0)
	, fpsCountTime(0)
	, fpsCount(0)
	, minFrameTimeMs(0)
    , eventFilterCount(0)
{
	setAttribute(Qt::WA_OpaquePaintEvent, true);
	setAttribute(Qt::WA_NoSystemBackground, true);
	setAttribute(Qt::WA_PaintOnScreen, true);
	setAttribute(Qt::WA_TranslucentBackground, true);
	setAttribute(Qt::WA_NativeWindow, true);

    setFocusPolicy( Qt::StrongFocus );
	setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    
    setMinimumSize(100, 100);

	// Init OS-specific layer
	{

#if defined (__DAVAENGINE_MACOS__)
		setMouseTracking(true);
		DAVA::QtLayerMacOS *qtLayer = (DAVA::QtLayerMacOS *) DAVA::QtLayer::Instance();
		qtLayer->InitializeGlWindow((void *)this->winId(), this->size().width(), this->size().height());
#elif defined (__DAVAENGINE_WIN32__)
		DAVA::QtLayerWin32 *qtLayer = (DAVA::QtLayerWin32 *) DAVA::QtLayer::Instance();
		HINSTANCE hInstance = (HINSTANCE)::GetModuleHandle(NULL);
		qtLayer->SetWindow(hInstance, (HWND)this->winId(), this->size().width(), this->size().height());
		qtLayer->OnResume();
#else
		DVASSERT(false && "Wrong platform");
#endif //#if defined (__DAVAENGINE_MACOS__)

		DAVA::QtLayer::Instance()->SetDelegate(this);
		DAVA::QtLayer::Instance()->Resize(size().width(), size().height());
	}

	//EnableCustomPaintFlags(true);
	setAcceptDrops(true);

	// Setup FPS
	SetMaxFPS(maxFPS);
    
    RegisterEventFilter();
    
	// start render
	QTimer::singleShot(0, this, SLOT(Render()));
}

DavaGLWidget::~DavaGLWidget()
{
    if (eventFilterCount > 0)
    {
        QAbstractEventDispatcher::instance()->removeNativeEventFilter( this );
        eventFilterCount = 0;
    }
}

QPaintEngine *DavaGLWidget::paintEngine() const
{
	return NULL;
}

bool DavaGLWidget::nativeEventFilter(const QByteArray& eventType, void* msg, long* result)
{
    Q_UNUSED(eventType);
    
#if defined(Q_OS_WIN)

	MSG *message = static_cast<MSG *>(msg);
	DAVA::CoreWin32PlatformQt *core = static_cast<DAVA::CoreWin32PlatformQt *>(DAVA::CoreWin32PlatformQt::Instance());
	DVASSERT(core);

	bool processMessage = false;

	if ( message->hwnd == reinterpret_cast<HWND>(winId()))
	{
		switch ( message->message )
		{
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
			core->SetFocused(true);
			break;

		default:
			break;
		}

		processMessage = true;
	}
	else
	{
		// Qt5 doesn't pass WM_CHAR, WM_KEY** messages to child QWidget::nativeEvent callback,
		// so handle this messages globaly
		switch ( message->message )
		{
		case WM_KEYUP:
		case WM_KEYDOWN:
		case WM_CHAR:
			processMessage = core->IsFocused() && hasFocus();
			break;

		default:
			break;
		}
	}

	if (processMessage)
	{
		return core->WinEvent(message, result);
	}

#elif defined(Q_OS_MAC)
    Q_UNUSED(result);

    if (hasFocus())
    {
        DAVA::QtLayerMacOS *qtLayer = static_cast<DAVA::QtLayerMacOS *>(DAVA::QtLayer::Instance());
        qtLayer->HandleEvent(msg);
    }
#endif

	return false;
}

/*

// Helper for debugging QDockWidgets

bool DavaGLWidget::eventFilter( QObject* watched, QEvent* e )
{
    switch ( e->type() )
    {
    case QEvent::MouseButtonPress:
        qDebug() << "QEvent::MouseButtonPress";
        break;
    case QEvent::MouseButtonRelease:
        qDebug() << "QEvent::MouseButtonRelease";
        break;
    case QEvent::Show:
        qDebug() << "QEvent::Show";
        break;
    case QEvent::Hide:
        qDebug() << "QEvent::Hide";
        break;
    case QEvent::ShowToParent:
        qDebug() << "QEvent::ShowToParent";
        break;
    case QEvent::HideToParent:
        qDebug() << "QEvent::HideToParent";
        break;
    case QEvent::NonClientAreaMouseButtonPress:
        qDebug() << "QEvent::NonClientAreaMouseButtonPress";
        break;
    case QEvent::NonClientAreaMouseButtonRelease:
        qDebug() << "QEvent::NonClientAreaMouseButtonRelease";
        break;
    case QEvent::ZOrderChange:
        qDebug() << "QEvent::ZOrderChange";
        break;
    case QEvent::WinIdChange:
        qDebug() << "QEvent::WinIdChange";
        break;
    case QEvent::Destroy:
    case QEvent::NonClientAreaMouseMove:
    case QEvent::InputMethodQuery:
    case QEvent::FocusIn:
    case QEvent::FocusOut:
    case QEvent::MouseMove:
    case QEvent::Move:
    case QEvent::Enter:
    case QEvent::Leave:
    case QEvent::Paint:
    case QEvent::PolishRequest:
    case QEvent::LayoutRequest:
    case QEvent::ChildRemoved:
    case QEvent::WindowBlocked:
    case QEvent::ChildAdded:
    case QEvent::EnabledChange:
    case QEvent::WindowUnblocked:
    case QEvent::WindowActivate:
    case QEvent::WindowDeactivate:
    case QEvent::UpdateLater:
    case QEvent::Resize:
    case QEvent::ToolTip:
    case QEvent::UpdateRequest:
    case QEvent::ActivationChange:
    case QEvent::FocusAboutToChange:
        break;
    default:
        qDebug() << "Event: " << e->type();
        break;
    }

    return QWidget::eventFilter( watched, e );
}
*/

void DavaGLWidget::paintEvent(QPaintEvent *event)
{
	//Q_UNUSED(event);

    if (!isEnabled())
    {
        QWidget::paintEvent(event);
    }
}

void DavaGLWidget::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
	DAVA::QtLayer::Instance()->Resize(e->size().width(), e->size().height());

	emit Resized(e->size().width(), e->size().height());
}

void DavaGLWidget::showEvent(QShowEvent *e)
{
	QWidget::showEvent(e);

	DAVA::QtLayer::Instance()->OnResume();
}

void DavaGLWidget::hideEvent(QHideEvent *e)
{
	QWidget::hideEvent(e);

	DAVA::QtLayer::Instance()->OnSuspend();
}

void DavaGLWidget::focusInEvent(QFocusEvent *e)
{
    // RegisterEventFilter();
    QWidget::focusInEvent( e );
    
    DAVA::QtLayer::Instance()->LockKeyboardInput( true );

#if defined(Q_OS_WIN)
	DAVA::CoreWin32PlatformQt *core = static_cast<DAVA::CoreWin32PlatformQt *>(DAVA::CoreWin32PlatformQt::Instance());
	DVASSERT(core);
    core->SetFocused(true);
#endif

}

void DavaGLWidget::focusOutEvent(QFocusEvent *e)
{
    // UnregisterEventFilter();
    QWidget::focusOutEvent( e );
    
    DAVA::InputSystem::Instance()->GetKeyboard().ClearAllKeys();
	DAVA::QtLayer::Instance()->LockKeyboardInput(false);

#if defined(Q_OS_WIN)
	DAVA::CoreWin32PlatformQt *core = static_cast<DAVA::CoreWin32PlatformQt *>(DAVA::CoreWin32PlatformQt::Instance());
	DVASSERT(core);
    core->SetFocused(false);
#endif
}

void DavaGLWidget::dragEnterEvent(QDragEnterEvent *event)
{
    event->setDropAction(Qt::LinkAction);
	event->accept();
}

void DavaGLWidget::changeEvent(QEvent* e)
{
    switch (e->type())
    {
    case QEvent::EnabledChange:
	    setAttribute(Qt::WA_OpaquePaintEvent, isEnabled());
	    setAttribute(Qt::WA_NoSystemBackground, isEnabled());
	    setAttribute(Qt::WA_PaintOnScreen, isEnabled());
	    setAttribute(Qt::WA_TranslucentBackground, isEnabled());
        update();
        break;

    default:
        break;
    }
}

void DavaGLWidget::enterEvent(QEvent* e)
{
    // RegisterEventFilter();
}

void DavaGLWidget::leaveEvent(QEvent* e)
{
    // Bug in OS X, leave event called twice in some cases.
    // UnregisterEventFilter();
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

#if defined (Q_OS_MAC)
void DavaGLWidget::mouseMoveEvent(QMouseEvent *e)
{
    DAVA::QtLayerMacOS *qtLayer = static_cast<DAVA::QtLayerMacOS *>(DAVA::QtLayer::Instance());
	DVASSERT(qtLayer != NULL);
    qtLayer->MouseMoved(e->x(), e->y());
    QWidget::mouseMoveEvent(e);
}
#endif //#if defined (Q_OS_MAC)

void DavaGLWidget::SetMaxFPS(int fps)
{
	maxFPS = fps;

	if(0 != fps)
	{
		minFrameTimeMs = 1000 / fps;
	}

	DAVA::RenderManager::Instance()->SetFPS(maxFPS);
}

int DavaGLWidget::GetMaxFPS()
{
	return maxFPS;
}

int DavaGLWidget::GetFPS() const
{
	return fps;
}

void DavaGLWidget::Render()
{
	QElapsedTimer frameTimer;
	frameTimer.start();

    DAVA::QtLayer::Instance()->ProcessFrame();

	if(QDateTime::currentMSecsSinceEpoch() >= fpsCountTime)
	{
		fps = fpsCount;
		fpsCount = 0;
		fpsCountTime = QDateTime::currentMSecsSinceEpoch() + 1000.0;
	}
	else
	{
		fpsCount++;
	}

	qint64 waitUntilNextFrameMs = (qint64) minFrameTimeMs - frameTimer.elapsed();
	if(waitUntilNextFrameMs < 0)
	{
		waitUntilNextFrameMs = 0;
	}

	QTimer::singleShot(waitUntilNextFrameMs, this, SLOT(Render()));
}

void DavaGLWidget::Quit()
{
    exit(0);
}

void DavaGLWidget::RegisterEventFilter()
{
    if ( eventFilterCount == 0 )
    {
        QAbstractEventDispatcher::instance()->installNativeEventFilter( this );
    }
    eventFilterCount++;
}

void DavaGLWidget::UnregisterEventFilter()
{
    DVASSERT( eventFilterCount > 0 );
    if ( eventFilterCount == 1 )
    {
        QAbstractEventDispatcher::instance()->removeNativeEventFilter( this );
    }
    eventFilterCount--;
}
