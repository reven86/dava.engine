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
#include <QMimeData>
#include <QAbstractNativeEventFilter>

#include "QtLayer.h"


class DavaGLWidget
	: public QWidget
	, public DAVA::QtLayerDelegate
    , public QAbstractNativeEventFilter
{
    Q_OBJECT
    
public:
    explicit DavaGLWidget(QWidget *parent = 0);
    ~DavaGLWidget();

	void SetMaxFPS(int fps);
	int GetMaxFPS();
	int GetFPS() const;
    
	virtual QPaintEngine *paintEngine() const;
	bool nativeEventFilter(const QByteArray& eventType, void * message, long * result);
   
signals:
	void OnDrop(const QMimeData *mimeData);
	void Resized(int width, int height);

private slots:
	void Render();

private:
	virtual void paintEvent(QPaintEvent *);
	virtual void resizeEvent(QResizeEvent *);

	virtual void showEvent(QShowEvent *);
	virtual void hideEvent(QHideEvent *);

    virtual void focusInEvent(QFocusEvent *);
    virtual void focusOutEvent(QFocusEvent *);

	virtual void dropEvent(QDropEvent *);
	virtual void dragMoveEvent(QDragMoveEvent *);
	virtual void dragEnterEvent(QDragEnterEvent *);

    virtual void changeEvent(QEvent *e);
    virtual void enterEvent(QEvent *e);
    virtual void leaveEvent(QEvent *e);
    
#if defined (Q_OS_MAC)
    virtual void mouseMoveEvent(QMouseEvent *);
#endif //#if defined (Q_OS_MAC)

	virtual void Quit();

    void RegisterEventFilter();
    void UnregisterEventFilter();

	int maxFPS;
    int minFrameTimeMs;
	int fps;
    int eventFilterCount;

	qint64 fpsCountTime;
	int fpsCount;
};

#endif // DAVAGLWIDGET_H
