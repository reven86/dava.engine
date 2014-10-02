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



#include "ScreenWrapper.h"
#include "HierarchyTreeController.h"
#include "mainwindow.h"
#include "DefaultScreen.h"
#include "ScreenManager.h"
#include <QWidget>
#include <QApplication>

ScreenWrapper::ScreenWrapper(QObject *parent) :
    QObject(parent)
{
	qtScreen = NULL;
	mainWindow = NULL;
	cursorPosition.SetZero();
    
    backgroundFrameColor = EditorSettings::Instance()->GetCurrentBackgroundFrameColor();
}

ScreenWrapper::~ScreenWrapper()
{
    
}

void ScreenWrapper::SetQtScreen(QWidget* widget)
{
	this->qtScreen = widget;
}

QRect ScreenWrapper::GetRect() const
{
	QRect rect;
	HierarchyTreeScreenNode* activeScreen = HierarchyTreeController::Instance()->GetActiveScreen();
	if (activeScreen)
	{
		float scale = GetScale();
		Rect orgRect = activeScreen->GetRect();
		orgRect.x *= scale;
		orgRect.y *= scale;
		orgRect.dx *= scale;
		orgRect.dy *= scale;
		rect = QRect(orgRect.x, orgRect.y, orgRect.dx, orgRect.dy);
	}
	return rect;
}

void ScreenWrapper::SetScale(float scale)
{
	HierarchyTreeScreenNode* activeScreen = HierarchyTreeController::Instance()->GetActiveScreen();
	DefaultScreen* screen = GetActiveScreen();
	if (!activeScreen || !screen)
		return;
	
	activeScreen->SetScale(scale);
	screen->SetScale(Vector2(scale, scale));
}

void ScreenWrapper::UpdateScale(float scaleDelta)
{
	emit UpdateScaleRequest(scaleDelta);
}

float ScreenWrapper::GetScale() const
{
	HierarchyTreeScreenNode* activeScreen = HierarchyTreeController::Instance()->GetActiveScreen();
	if (!activeScreen)
		return 1;
	
	return activeScreen->GetScale();
}

DefaultScreen* ScreenWrapper::GetActiveScreen()
{
	DefaultScreen* screen = dynamic_cast<DefaultScreen*>(ScreenManager::Instance()->GetScreen());
	return screen;
}

QWidget* ScreenWrapper::GetMainWindow()
{
	if (!mainWindow)
	{
		QWidgetList list = qApp->topLevelWidgets();
		for (QWidgetList::iterator iter = list.begin(); iter != list.end(); ++iter)
		{
			QWidget* widget = (*iter);
			MainWindow* mainWindow = dynamic_cast<MainWindow*>(widget);
			if (mainWindow)
			{
				this->mainWindow = mainWindow;
				break;
			}
		}
	}
	return mainWindow;
}

void ScreenWrapper::RequestViewMove(const Vector2& delta)
{
	emit UpdateScreenPositionRequest(QPoint(delta.x, delta.y));
}

void ScreenWrapper::SetViewPos(int posX, int posY, const QRect& size)
{
	DefaultScreen* activeScreen = GetActiveScreen();
	if (!activeScreen)
		return;

	HierarchyTreeScreenNode* screenNode = HierarchyTreeController::Instance()->GetActiveScreen();
	if (screenNode)
	{
		screenNode->SetPosX(posX);
		screenNode->SetPosY(posY);
	}
	
	QRect screenRect = GetRect();
	
	if (screenRect.width() < size.width())
	{
		posX = (size.width() - screenRect.width()) / 2 - screenRect.topLeft().x();
		posX /= GetScale();
	}
	else
	{
		posX = -(screenRect.topLeft().x() + posX) / GetScale();
	}

	if (screenRect.height() < size.height())
	{
		posY = (size.height() - screenRect.height()) / 2 - screenRect.topLeft().y();
		posY /= GetScale();
	}
	else
	{
		posY = -(screenRect.topLeft().y() + posY) / GetScale();
	}
	
	activeScreen->SetPos(Vector2(posX, posY));
}

void ScreenWrapper::RequestUpdateCursor()
{
	QCursor::setPos(QCursor::pos()); //emulate mouse move for update cursor
}

bool ScreenWrapper::IsDropEnable(const QPoint& pos)
{
	DefaultScreen* activeScreen = GetActiveScreen();
	if (!activeScreen)
		return false;
	
	Vector2 _pos = Vector2(pos.x(), pos.y());
	bool res = activeScreen->IsDropEnable(_pos);
	activeScreen->BacklightControl(_pos);
	return res;
}

void ScreenWrapper::SetCursor(const Vector2& position, Qt::CursorShape cursor)
{
	cursorPosition = position;
	if (qtScreen)
		qtScreen->setCursor(cursorsManager.GetCursor(cursor));
}

Vector2 ScreenWrapper::GetCursorPosition()
{
	return cursorPosition;
}

void ScreenWrapper::SetApplicationCursor(Qt::CursorShape cursor)
{
    QApplication::setOverrideCursor(cursor);
}

void ScreenWrapper::RestoreApplicationCursor()
{
    QApplication::restoreOverrideCursor();
}

void ScreenWrapper::SetBackgroundFrameRect(const Rect& rect)
{
    backgroundFrameRect = rect;
}

Rect ScreenWrapper::GetBackgroundFrameRect() const
{
    HierarchyTreeScreenNode* activeScreen = HierarchyTreeController::Instance()->GetActiveScreen();
	if (!activeScreen)
    {
        // No screen is loaded yet, so scale should not be applied.
        return backgroundFrameRect;
    }

    Rect resultRect = backgroundFrameRect;
    resultRect.SetSize(backgroundFrameRect.GetSize() / activeScreen->GetScale());
    return resultRect;
}

void ScreenWrapper::SetBackgroundFrameColor(const Color& color)
{
    backgroundFrameColor = color;
}

Vector2 ScreenWrapper::LocalToInternal(const Vector2& point)
{
    if (!GetActiveScreen())
    {
        return point;
    }

    return GetActiveScreen()->LocalToInternal(point);
}
