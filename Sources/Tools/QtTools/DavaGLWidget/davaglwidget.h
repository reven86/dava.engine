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
#include <QMimeData>
#include <QWidget>
#include <QScopedPointer>
#include <QQuickWindow>

class QDragMoveEvent;
class DavaGLWidget;
class ControlMapper;
class QResizeEvent;
class DavaRenderer;

class DavaGLView
: public QQuickWindow
{
    friend class DavaGLWidget;

    Q_OBJECT

public:
    DavaGLView();

signals:
    void mouseScrolled(int ofs);
    void OnDrop( const QMimeData *mimeData );

protected:
    bool event(QEvent *event) override;

    void keyPressEvent(QKeyEvent *) override;
    void keyReleaseEvent(QKeyEvent *) override;
    
    void mouseMoveEvent(QMouseEvent * event) override;
    void mousePressEvent(QMouseEvent * event) override;
    void mouseReleaseEvent(QMouseEvent * event) override;
    
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *) override;
    void handleDragMoveEvent(QDragMoveEvent * event);
    
private:
    ControlMapper* controlMapper = nullptr;
};


class DavaGLWidget
    : public QWidget
{
    Q_OBJECT
    friend class FocusTracker;

public:
    explicit DavaGLWidget(QWidget *parent = nullptr);
    void MakeInvisible();
    qreal GetDevicePixelRatio() const;
    QQuickWindow* GetGLView();
signals:
    void ScreenChanged();
    void mouseScrolled(int ofs);
    void Resized(int width, int height, int dpr);
    void Initialized();
    void OnDrop(const QMimeData* mimeData);
public slots:
    void OnSync();

private slots:
    void OnResize();
    void OnCleanup();

protected:
    void resizeEvent(QResizeEvent*) override;

private:
    DavaGLView* davaGLView = nullptr;
    DavaRenderer* renderer = nullptr;
};

#endif // DAVAGLWIDGET_H
