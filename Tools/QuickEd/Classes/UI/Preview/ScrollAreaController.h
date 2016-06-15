#ifndef __QUICKED_PREVIEW_SCROLL_AREA_CONTROLLER_H__
#define __QUICKED_PREVIEW_SCROLL_AREA_CONTROLLER_H__

#include <QObject>
#include <QPoint>
#include <QSize>

#include "Base/ScopedPtr.h"
#include "UI/UIControl.h"

namespace DAVA
{
class Vector2;
}

class ScrollAreaController : public QObject
{
    Q_OBJECT
public:
    Q_PROPERTY(QSize canvasSize READ GetCanvasSize NOTIFY CanvasSizeChanged);
    Q_PROPERTY(QSize viewSize READ GetViewSize WRITE SetViewSize NOTIFY ViewSizeChanged);
    Q_PROPERTY(QPoint position READ GetPosition WRITE SetPosition NOTIFY PositionChanged);
    Q_PROPERTY(qreal scale READ GetScale WRITE SetScale NOTIFY ScaleChanged);

    ScrollAreaController(QObject* parent = nullptr);
    ~ScrollAreaController();

    void SetNestedControl(DAVA::UIControl* nestedControl);
    void SetMovableControl(DAVA::UIControl* movableControl);
    void AdjustScale(qreal newScale, QPointF mousePos);

    QSize GetCanvasSize() const;
    QSize GetViewSize() const;
    QPoint GetPosition() const;
    qreal GetScale() const;
    qreal GetMinScale() const;
    qreal GetMaxScale() const;
    QPoint GetMinimumPos() const;
    QPoint GetMaximumPos() const;

public slots:
    void SetViewSize(QSize size);
    void SetPosition(QPoint position);
    void UpdateCanvasContentSize();
    void SetScale(qreal scale);

signals:
    void CanvasSizeChanged(QSize canvasSize);
    void ViewSizeChanged(QSize size);
    void PositionChanged(QPoint position);
    void ScaleChanged(qreal scale);
    void NestedControlPositionChanged(QPoint position);

private:
    void UpdatePosition();
    DAVA::ScopedPtr<DAVA::UIControl> backgroundControl;
    DAVA::UIControl* nestedControl = nullptr;
    DAVA::UIControl* movableControl = nullptr;
    QSize canvasSize = QSize(0, 0);
    QSize viewSize = QSize(0, 0);
    QPoint position = QPoint(0, 0);
    qreal scale = 0.0f;
    const qreal minScale = 0.25f;
    const qreal maxScale = 8.0f;
    const int Margin = 50;
};

#endif // __QUICKED_PREVIEW_SCROLL_AREA_CONTROLLER_H__
