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
    Q_PROPERTY(float scale READ GetScale WRITE SetScale NOTIFY ScaleChanged);

    ScrollAreaController(QObject* parent = nullptr);
    ~ScrollAreaController();

    void SetNestedControl(DAVA::UIControl* nestedControl);
    void SetMovableControl(DAVA::UIControl* movableControl);
    void AdjustScale(float newScale, const QPointF& mousePos);

    QSize GetCanvasSize() const;
    QSize GetViewSize() const;
    QPoint GetPosition() const;
    float GetScale() const;
    float GetMinScale() const;
    float GetMaxScale() const;
    QPoint GetMinimumPos() const;
    QPoint GetMaximumPos() const;

public slots:
    void SetViewSize(const QSize& size);
    void SetViewSize(DAVA::int32 width, DAVA::int32 height);
    void SetPosition(const QPoint& position);
    void UpdateCanvasContentSize();
    void SetScale(float scale);

signals:
    void CanvasSizeChanged(const QSize& canvasSize);
    void ViewSizeChanged(const QSize& size);
    void PositionChanged(const QPoint& position);
    void ScaleChanged(float scale);
    void NestedControlPositionChanged(const QPoint& position);

private:
    void UpdatePosition();
    DAVA::ScopedPtr<DAVA::UIControl> backgroundControl;
    DAVA::UIControl* nestedControl = nullptr;
    DAVA::UIControl* movableControl = nullptr;
    QSize canvasSize = QSize(0, 0);
    QSize viewSize = QSize(0, 0);
    QPoint position = QPoint(0, 0);
    float scale = 0.0f;
    const float minScale = 0.25f;
    const float maxScale = 8.0f;
    const int Margin = 50;
};

#endif // __QUICKED_PREVIEW_SCROLL_AREA_CONTROLLER_H__
