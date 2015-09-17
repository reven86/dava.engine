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

#include "DAVAEngine.h"

#include <QOpenGLWidget>
#include <QTimer>
#include <QMimeData>
#include <QWindow>
#include <QPointer>
#include <QScopedPointer>


class QOpenGLContext;
class QOpenGLPaintDevice;
class QExposeEvent;
class DavaGLWidget;
class FocusTracker;
class ControlMapper;


class OpenGLWindow
    : public QWindow
{
    friend class DavaGLWidget;

    Q_OBJECT
    
signals:
    void mousePressed();
    void mouseScrolled( int ofs );
    
public:
    OpenGLWindow();
    ~OpenGLWindow();
    
    void renderNow();

signals:
    void Exposed();
    void OnDrop( const QMimeData *mimeData );
    
protected:
    bool event(QEvent *event) override;
    void exposeEvent(QExposeEvent *event) override;
    
    void keyPressEvent(QKeyEvent *) override;
    void keyReleaseEvent(QKeyEvent *) override;
    
    void mouseMoveEvent(QMouseEvent * event) override;
    void mousePressEvent(QMouseEvent * event) override;
    void mouseReleaseEvent(QMouseEvent * event) override;
    
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *) override;
    void handleDragMoveEvent(QDragMoveEvent * event);
    
private:
    QScopedPointer< ControlMapper > controlMapper;
};


class DavaGLWidget
    : public QWidget
{
    Q_OBJECT

signals :
    void Initialized();
    void Resized( int width, int height, int dpr );
    void OnDrop( const QMimeData *mimeData );

public:
    explicit DavaGLWidget(QWidget *parent = nullptr);
    ~DavaGLWidget();

    OpenGLWindow *GetGLWindow() const;
    bool IsInitialized() const;

    void MakeInvisible();

public slots:
    void OnWindowExposed();
    
private:
    void resizeEvent(QResizeEvent *) override;

    void PerformSizeChange();
    
    bool isInitialized;
    int currentDPR;
    int currentWidth;
    int currentHeight;

    QPointer< OpenGLWindow > openGlWindow;
    QPointer< QWidget > container;
    QPointer< FocusTracker > focusTracker;
};



#endif // DAVAGLWIDGET_H
