#ifndef __DAVAGLWIDGET_H__
#define __DAVAGLWIDGET_H__

#include "Render/RenderBase.h" // need to include glew.h in right order

#include "QtTools/WarningGuard/QtWarningsHandler.h"
PUSH_QT_WARNING_SUPRESSOR
#include <QWidget>
#include <QMimeData>
#include <QScopedPointer>
#include <QQuickWindow>
POP_QT_WARNING_SUPRESSOR

class QDragMoveEvent;
class DavaGLWidget;
class ControlMapper;
class QResizeEvent;
class DavaRenderer;

class DavaGLView
: public QQuickWindow
{
    friend class DavaGLWidget;

    PUSH_QT_WARNING_SUPRESSOR
    Q_OBJECT
    POP_QT_WARNING_SUPRESSOR

public:
    DavaGLView();

signals:
    void mouseScrolled(int ofs);
    void OnDrop(const QMimeData* mimeData);

protected:
    bool event(QEvent* event) override;

    void keyPressEvent(QKeyEvent*) override;
    void keyReleaseEvent(QKeyEvent*) override;

    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void handleDragMoveEvent(QDragMoveEvent* event);

private:
    ControlMapper* controlMapper = nullptr;
};

class DavaGLWidget
: public QWidget
{
    PUSH_QT_WARNING_SUPRESSOR
    Q_OBJECT
    POP_QT_WARNING_SUPRESSOR
    friend class FocusTracker;

public:
    explicit DavaGLWidget(QWidget* parent = nullptr);
    void MakeInvisible();
    QQuickWindow* GetGLView();
    QCursor GetCursor() const;
    void SetCursor(const QCursor& cursor);
    void UnsetCursor();

signals:
    void ScreenChanged();
    void mouseScrolled(int ofs);
    void Resized(int width, int height);
    void Initialized();
    void OnDrop(const QMimeData* mimeData);

public slots:
    void OnPaint();

private slots:
    void OnResize();
    void OnCleanup();

protected:
    void resizeEvent(QResizeEvent*) override;

private:
    DavaGLView* davaGLView = nullptr;
    DavaRenderer* renderer = nullptr;
};

#endif // __DAVAGLWIDGET_H__
