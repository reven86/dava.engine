#ifndef __GRAPHICSVIEWCONTEXT_H__
#define __GRAPHICSVIEWCONTEXT_H__

#include <QObject>
#include <QPoint>
#include <QSize>
#include "DAVAEngine.h"

class PackageCanvas;
class ControlNode;

class GraphicsViewContext: public QObject
{
    Q_OBJECT
public:
    GraphicsViewContext();
    ~GraphicsViewContext();

    void SetViewControlSize(const QSize &newSize);
    void SetCanvasControlSize(const QSize &newSize);
    void SetCanvasControlScale(int newScale);

    void SetCanvasPosition(const QPoint &newPosition);
    QPoint GetCanvasPosition() const;
    int GetCanvasScale() const;

    QSize GetScaledCanvasSize() const;
    QSize GetViewSize() const;

    inline DAVA::UIControl *GetViewControl() const { return view; }
signals:
    void CanvasPositionChanged(const QPoint &canvasPosition);
    void CanvasOrViewChanged(const QSize &viewSize, const QSize &scaledContentSize);
    void CanvasScaleChanged(int canvasScale);

public slots:
    void OnActiveRootControlsChanged(const QList<ControlNode *> &activatedControls, const QList<ControlNode*> &deactivatedControls);
private:
    DAVA::Vector2 canvasPosition;

    PackageCanvas *canvas;
    DAVA::UIControl *view;
};


#endif // __GRAPHICSVIEWCONTEXT_H__
