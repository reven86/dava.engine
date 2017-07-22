#pragma once

#include "UI/Preview/Ruler/RulerSettings.h"
#include "UI/Preview/Guides/IRulerListener.h"

#include <QWidget>
#include <QPixmap>
#include <QMouseEvent>

class LazyUpdater;

class RulerWidget final : public QWidget
{
    Q_OBJECT

public:
    explicit RulerWidget(IRulerListener* listener, QWidget* parent = 0);
    ~RulerWidget() = default;

    //Set ruler orientation
    void SetRulerOrientation(Qt::Orientation orientation);

signals:
    void GeometryChanged();

public slots:
    // Ruler Settings are changed.
    void OnRulerSettingsChanged(const RulerSettings& rulerSettings);

    // Marker Position is changed.
    void OnMarkerPositionChanged(int position);

private slots:
    void ShowContextMenu(const QPoint& point);

private:
    void paintEvent(QPaintEvent* paintEvent) override;

    QSize minimumSizeHint() const override;

    void resizeEvent(QResizeEvent* resizeEvent) override;
    void moveEvent(QMoveEvent* moveEvent) override;

    // We are using double buffering to avoid flicker and excessive updates.
    void UpdateDoubleBufferImage();

    // Draw different types of scales.
    void DrawScale(QPainter& painter, int tickStep, int tickStartPos, int tickEndPos,
                   bool drawValues);

    void mouseMoveEvent(QMouseEvent* mouseEvent) override;
    void leaveEvent(QEvent* event) override;

    void mousePressEvent(QMouseEvent* mouseEvent) override;
    void mouseReleaseEvent(QMouseEvent* mouseEvent) override;

    DAVA::float32 GetMousePos(const QPoint& pos) const;

    // Ruler orientation.
    Qt::Orientation orientation = Qt::Horizontal;

    // Ruler settings.
    RulerSettings settings;

    // Ruler double buffer.
    QPixmap doubleBuffer;

    // Marker position.
    int markerPosition = 0;

    LazyUpdater* lazyUpdater = nullptr;

    IRulerListener* listener = nullptr;
};
