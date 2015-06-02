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



#ifndef __UIEditor__ScreenWrapper__
#define __UIEditor__ScreenWrapper__

#include "DAVAEngine.h"
#include <QObject>
#include <QPoint>
#include <QRect>
#include "HierarchyTreeControlNode.h"
#include "HierarchyTreeController.h"

#include "CursorsManager.h"

using namespace DAVA;

class DefaultScreen;

class ScreenWrapper: public QObject, public Singleton<ScreenWrapper>
{
    Q_OBJECT
    
public:
	enum CursorType
	{
		CursorTypeNormal,
		CursorTypeSize
	};
	
    explicit ScreenWrapper(QObject *parent = 0);
    ~ScreenWrapper();
	
	void SetQtScreen(QWidget* widget);
    
	QRect GetRect() const;
	void SetViewPos(int posX, int posY, const QRect& size);
	void RequestViewMove(const Vector2& delta);

	void SetScale(float scale);
	float GetScale() const;
	void UpdateScale(float scaleDelta);
		
	DefaultScreen* GetActiveScreen();
	
	bool IsDropEnable(const QPoint& pos);
	
	void RequestUpdateCursor();
	void SetCursor(const Vector2& position, Qt::CursorShape cursor);
	Vector2 GetCursorPosition();

   	void SetApplicationCursor(Qt::CursorShape cursor);
    void RestoreApplicationCursor();

    void SetBackgroundFrameRect(const Rect& rect);
    Rect GetBackgroundFrameRect() const;

    void SetBackgroundFrameColor(const Color& color);
    Color GetBackgroundFrameColor() const {return backgroundFrameColor;};

    Vector2 LocalToInternal(const Vector2& point);

signals:
	void UpdateScaleRequest(float scaleDelta);
	void UpdateScreenPositionRequest(const QPoint& posDelta);
	void UpdateView();

private:
	QWidget* GetMainWindow();
	
	QWidget* qtScreen;
	QWidget* mainWindow;
	
	Vector2 cursorPosition;
    
    // The rect and color of background frame (around the screen control).
    Rect backgroundFrameRect;
    Color backgroundFrameColor;
    
    // Cursors manager.
    CursorsManager cursorsManager;
};

#endif /* defined(__UIEditor__ScreenWrapper__) */
