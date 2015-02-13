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

#include "davaglwidget.h"
#include "ui_davaglwidget.h"

#include <QApplication>
#include <QMessageBox>
#include <QResizeEvent>
#include <QTimer>
#include <QElapsedTimer>
#include <QMoveEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QFocusEvent>

#include "controllist.h"
#include "ScreenWrapper.h"
#include "HierarchyTreeController.h"
#include "ItemsCommand.h"
#include "GuideCommands.h"
#include "CommandsController.h"

#include "Guides/GuideMimeData.h"

#if defined (__DAVAENGINE_MACOS__)
#include "QtLayerMacOS.h"
#elif defined (__DAVAENGINE_WIN32__)
#include "QtLayerWin32.h"
#include "CorePlatformWin32Qt.h"
#endif //#if defined (__DAVAENGINE_MACOS__)

DavaGLWidget::DavaGLWidget(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::DavaGLWidget)
	, maxFPS(60)
	, minFrameTimeMs(0)
{
	ui->setupUi(this);

	// Widget will try to expand to maximum available size
	setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

	// Init OS-specific layer
	{

#if defined (__DAVAENGINE_MACOS__)
		setMouseTracking(true);
		DAVA::QtLayerMacOS *qtLayer = (DAVA::QtLayerMacOS *) DAVA::QtLayer::Instance();
		qtLayer->InitializeGlWindow((void *)this->winId(), this->size().width(), this->size().height());
#elif defined (__DAVAENGINE_WIN32__)
		DAVA::QtLayerWin32 *qtLayer = (DAVA::QtLayerWin32 *) DAVA::QtLayer::Instance();
		HINSTANCE hInstance = (HINSTANCE)::GetModuleHandle(NULL);
		qtLayer->SetWindow(hInstance, this->winId(), this->size().width(), this->size().height());
		qtLayer->OnResume();
#else
		DVASSERT(false && "Wrong platform");
#endif //#if defined (__DAVAENGINE_MACOS__)

		DAVA::QtLayer::Instance()->SetDelegate(this);
		DAVA::QtLayer::Instance()->Resize(size().width(), size().height());

	}

	// Disable Widget blinking
	setAttribute(Qt::WA_OpaquePaintEvent, true);
	setAttribute(Qt::WA_NoSystemBackground, true);
	setAttribute(Qt::WA_PaintOnScreen, true);

	// Setup FPS
	SetMaxFPS(maxFPS);

	// start render in 1 ms
	QTimer::singleShot(1, this, SLOT(Render()));

	// acept drops
	this->setAcceptDrops(true);	
	setMouseTracking(true);

	// Remember the previous size as current one.
	prevSize = size();
	ScreenWrapper::Instance()->SetQtScreen(this);
}

DavaGLWidget::~DavaGLWidget()
{
	delete ui;
}

QPaintEngine *DavaGLWidget::paintEngine() const
{
	return NULL;
}

void DavaGLWidget::paintEvent(QPaintEvent *)
{
	// We have custom rendering (by timer), so here nothing to do
	return;
}

void DavaGLWidget::resizeEvent(QResizeEvent *e)
{
	// Remember previous size of the widget.
	prevSize = e->oldSize();

	QWidget::resizeEvent(e);
	DAVA::QtLayer::Instance()->Resize(e->size().width(), e->size().height());

	//YZ fix load resource
	
	VirtualCoordinatesSystem::Instance()->UnregisterAllAvailableResourceSizes();
    VirtualCoordinatesSystem::Instance()->RegisterAvailableResourceSize(size().width(), size().height(), "Gfx");
    VirtualCoordinatesSystem::Instance()->ScreenSizeChanged();
    
    emit DavaGLWidgetResized();
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
	QWidget::focusInEvent(e);

	DAVA::QtLayer::Instance()->LockKeyboardInput(true);
}

void DavaGLWidget::focusOutEvent(QFocusEvent *e)
{
	QWidget::focusOutEvent(e);

	DAVA::InputSystem::Instance()->GetKeyboard().ClearAllKeys();
	DAVA::QtLayer::Instance()->LockKeyboardInput(false);
}

#if defined(Q_WS_WIN)
bool DavaGLWidget::winEvent(MSG *message, long *result)
{
	DAVA::CoreWin32PlatformQt *core = dynamic_cast<DAVA::CoreWin32PlatformQt *>(DAVA::CoreWin32PlatformQt::Instance());
	if (NULL != core)
	{
		if(NULL != message && 
			(message->message == WM_LBUTTONDOWN ||
			message->message == WM_RBUTTONDOWN ||
			message->message == WM_MBUTTONDOWN))
		{
			core->SetFocused(true);
		}

		return core->WinEvent(message, result);
	}

	return false;
}
#endif //#if defined(Q_WS_WIN)

void DavaGLWidget::mouseMoveEvent(QMouseEvent *e)
{
#if defined (Q_WS_MAC)
	DAVA::QtLayerMacOS *qtLayer = dynamic_cast<DAVA::QtLayerMacOS *>(DAVA::QtLayer::Instance());
	if(qtLayer)
	{
        QPointF p = e->posF();
		qtLayer->MouseMoved((float32) p.x(), (float32) p.y());
	}
#endif //#if defined (Q_WS_MAC)

    // Check whether we are dragging the guide outside the screen, if yes - cancel the move.
    HierarchyTreeScreenNode* activeScreen = HierarchyTreeController::Instance()->GetActiveScreen();

    if (activeScreen && (e->buttons() & Qt::LeftButton) && !rect().contains(e->pos()) &&
       activeScreen->AreGuidesEnabled() && activeScreen->GetMoveGuide())
    {
        const GuideData moveGuideData = *(activeScreen->CancelMoveGuide());

        DeleteSingleGuideCommand* command = new DeleteSingleGuideCommand(activeScreen, moveGuideData);
        CommandsController::Instance()->ExecuteCommand(command);
        SafeRelease(command);

        HierarchyTreeController::Instance()->ResetSelectedControl();
    }

	QWidget::mouseMoveEvent(e);
}


void DavaGLWidget::wheelEvent(QWheelEvent *e)
{
	QWidget::wheelEvent(e);

	if (e->modifiers() & Qt::ControlModifier)
	{
		float delta = e->delta() * 0.001;
		ScreenWrapper::Instance()->UpdateScale(delta);
	}
}

void DavaGLWidget::dropEvent(QDropEvent *event)
{
	const QMimeData* data = event->mimeData();
	const ControlMimeData* controlData = dynamic_cast<const ControlMimeData*>(data);
	if (!controlData)
		return;

	CreateControlCommand* cmd = new CreateControlCommand(controlData->GetControlId(), event->pos());
	CommandsController::Instance()->ExecuteCommand(cmd);
	SafeRelease(cmd);
}

void DavaGLWidget::dragMoveEvent(QDragMoveEvent *event)
{
	const QMimeData* data = event->mimeData();
	const ControlMimeData* controlData = dynamic_cast<const ControlMimeData*>(data);
    const GuideMimeData* guideData = dynamic_cast<const GuideMimeData*>(data);
    HierarchyTreeScreenNode* activeScreen = HierarchyTreeController::Instance()->GetActiveScreen();
    
	if (controlData && ScreenWrapper::Instance()->IsDropEnable(event->pos()))
	{
		event->accept();
	}
	else if (activeScreen && activeScreen->AreGuidesEnabled() && guideData)
    {
        Vector2 curPos = GuideToInternal(event->pos());
        activeScreen->MoveNewGuide(curPos);
        event->accept();
    }
    else
    {
		event->ignore();
	}
}

void DavaGLWidget::dragEnterEvent(QDragEnterEvent *event)
{
    HierarchyTreeScreenNode* activeScreen = HierarchyTreeController::Instance()->GetActiveScreen();
    if (!activeScreen)
    {
        return;
    }

	const QMimeData* data = event->mimeData();
	const ControlMimeData* controlData = dynamic_cast<const ControlMimeData*>(data);
	if (controlData)
    {
		event->acceptProposedAction();
        return;
    }

    const GuideMimeData* guideData = dynamic_cast<const GuideMimeData*>(data);
    if (guideData && activeScreen->AreGuidesEnabled())
    {
        activeScreen->StartNewGuide(guideData->GetGuideType());
        HierarchyTreeController::Instance()->ResetSelectedControl();
        event->acceptProposedAction();
    }
}

void DavaGLWidget::dragLeaveEvent(QDragLeaveEvent* /*event*/)
{
    // Reset the selected guide when we are leaving screen.
    HierarchyTreeScreenNode* activeScreen = HierarchyTreeController::Instance()->GetActiveScreen();
    if (activeScreen->AreGuidesEnabled())
    {
        activeScreen->CancelNewGuide();
        HierarchyTreeController::Instance()->ResetSelectedControl();
    }
}

void DavaGLWidget::keyPressEvent(QKeyEvent *)
{
	//qDebug("DavaGLWidget::keyPressEvent");
}

void DavaGLWidget::keyReleaseEvent(QKeyEvent *)
{
	//qDebug("DavaGLWidget::keyReleaseEvent");
}

void DavaGLWidget::SetMaxFPS(int fps)
{
	maxFPS = fps;

	if(0 != fps)
	{
		minFrameTimeMs = 1000 / fps;
	}

	DAVA::RenderManager::Instance()->SetFPS(maxFPS);
}

void DavaGLWidget::Render()
{
	QElapsedTimer frameTimer;
	frameTimer.start();

	DAVA::QtLayer::Instance()->ProcessFrame();

	qint64 waitUntilNextFrameMs = (qint64) minFrameTimeMs - frameTimer.elapsed();

	if(waitUntilNextFrameMs <= 0)
	{
		// our render is too slow to reach maxFPS,
		// so we can wait a minimum time
		waitUntilNextFrameMs = 1;
	}

	QTimer::singleShot(waitUntilNextFrameMs, this, SLOT(Render()));
}

void DavaGLWidget::Quit()
{
	DAVA::Logger::Info("[QUIT]");
	exit(0);
}

Vector2 DavaGLWidget::GuideToInternal(const QPoint& pos)
{
    Vector2 curPos = Vector2(pos.x(), pos.y());
    Vector2 internalPos = ScreenWrapper::Instance()->LocalToInternal(curPos);
    
    // Allow moving per-pixel only.
    internalPos.x =  Round(internalPos.x);
    internalPos.y = Round(internalPos.y);
    
    return internalPos;
}
