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

#include <QOpenGLWidget>
#include <QTimer>
#include <QMimeData>


#include "UI/UIEvent.h"
#include "Platform/Qt5/QtLayer.h"

class QOpenGLContext;
class QOffscreenSurface;


#include <QOpenGLFunctions>
#include <QWindow>

class QOpenGLPaintDevice;
class QOpenGLContext;
class QPainter;
class QExposeEvent;
class OpenGLWindow : public QWindow, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    OpenGLWindow();
    ~OpenGLWindow();
    
    void render(QPainter *painter);
    void render();
    
    void renderNow();

signals:
    
    void Exposed();
    
    
protected:
    
    bool event(QEvent *event) override;
    void exposeEvent(QExposeEvent *event) override;
    
    
    void keyPressEvent(QKeyEvent *) override;
    void keyReleaseEvent(QKeyEvent *) override;
    
    void mouseMoveEvent(QMouseEvent * event) override;
    void mousePressEvent(QMouseEvent * event) override;
    void mouseReleaseEvent(QMouseEvent * event) override;
    
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    
#ifndef QT_NO_WHEELEVENT
    void wheelEvent(QWheelEvent *) override;
#endif
    
    DAVA::UIEvent MapMouseEventToDAVA(const QMouseEvent *event) const;
    DAVA::UIEvent::eButtonID MapQtButtonToDAVA(const Qt::MouseButton button) const;
    
    DAVA::char16 MapQtKeyToDAVA(const QKeyEvent *event);
    
private:
    
    int currentDPR;
    
    QOpenGLContext *context;
    QOpenGLPaintDevice *paintDevice;
};



class DavaGLWidget
    : public QWidget
	, public DAVA::QtLayerDelegate
{
    Q_OBJECT
    
public:
    explicit DavaGLWidget(QWidget *parent = 0);
    ~DavaGLWidget();

    void SetFPS(int fps);
    int GetFPS() const;

    bool IsInitialized() const;
    
signals:
    
    void Initialized();
    void Resized(int width, int height, int dpr);
	void OnDrop(const QMimeData *mimeData);

protected slots:
    
    void OnWindowExposed();
    void OnRenderTimer();
    
private:
    
    void resizeEvent(QResizeEvent *) override;
    
	virtual void dropEvent(QDropEvent *);
	virtual void dragMoveEvent(QDragMoveEvent *);
	virtual void dragEnterEvent(QDragEnterEvent *);

	virtual void Quit();
    DAVA_DEPRECATED(virtual void ShowAssertMessage(const char * message));
    
    void PerformSizeChange();
    
private:
    
    QTimer * renderTimer;
    int fps;
    
    bool isInitialized;
    
    int currentDPR;
    int currentWidth;
    int currentHeight;

    DAVA::String assertMessage;
    
    OpenGLWindow *openGlWindow;
};


inline int DavaGLWidget::GetFPS() const
{
    return fps;
}


inline bool DavaGLWidget::IsInitialized() const
{
    return isInitialized;
}

#endif // DAVAGLWIDGET_H
