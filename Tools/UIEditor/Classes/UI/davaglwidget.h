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


#ifndef DAVAGLWIDGET_H
#define DAVAGLWIDGET_H

#include <QWidget>
#include <QTimer>
#include "Math/Vector.h"

#include "QtLayer.h"

namespace Ui {
	class DavaGLWidget;
}

class DavaGLWidget : public QWidget, public DAVA::QtLayerDelegate
{
	Q_OBJECT

public:
	explicit DavaGLWidget(QWidget *parent = 0);
	~DavaGLWidget();

	void SetMaxFPS(int fps);
	int GetMaxFPS();
	int GetFPS();

	QSize GetPrevSize() const { return prevSize;};

protected:
	virtual QPaintEngine *paintEngine() const;
	virtual void paintEvent(QPaintEvent *);

	virtual void resizeEvent(QResizeEvent *);
	virtual void wheelEvent(QWheelEvent *);

	virtual void showEvent(QShowEvent *);
	virtual void hideEvent(QHideEvent *);

	virtual void focusInEvent(QFocusEvent *);
	virtual void focusOutEvent(QFocusEvent *);

	virtual void dropEvent(QDropEvent *event);
	virtual void dragMoveEvent(QDragMoveEvent *event);
	virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragLeaveEvent(QDragLeaveEvent * event);

	virtual void keyPressEvent(QKeyEvent *);
	virtual void keyReleaseEvent(QKeyEvent *);

	virtual void mouseMoveEvent(QMouseEvent *);

#if defined(Q_WS_WIN)
	virtual bool winEvent(MSG *message, long *result);
#endif //#if defined(Q_WS_WIN)

	protected slots:
		void Render();

signals:
    void DavaGLWidgetResized();

protected:
    // Recalculate "raw" guide coord to internal.
    DAVA::Vector2 GuideToInternal(const QPoint& pos);
    DAVA::float32 ToNearestInteger(DAVA::float32 value);

	virtual void Quit();

private:
	Ui::DavaGLWidget *ui;

	int maxFPS;
	int minFrameTimeMs;

	// Previous widget size.
	QSize prevSize;

};

#endif // DAVAGLWIDGET_H

