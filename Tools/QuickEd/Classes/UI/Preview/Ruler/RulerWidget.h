#ifndef __RULER_WIDGET__H__
#define __RULER_WIDGET__H__

#include "RulerSettings.h"

#include <QWidget>
#include <QPixmap>
#include <QMouseEvent>

class RulerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RulerWidget(QWidget* parent = 0);
    ~RulerWidget() = default;

    //Set ruler orientation
    void SetRulerOrientation(Qt::Orientation orientation);

    // Set the initial Ruler Settings.
    void SetRulerSettings(const RulerSettings& rulerSettings);

    void paintEvent(QPaintEvent* event) override;

    QSize minimumSizeHint() const override;

    void resizeEvent(QResizeEvent* event) override;

public slots:
    // Ruler Settings are changed.
    void OnRulerSettingsChanged(const RulerSettings& rulerSettings);

    // Marker Position is changed.
    void OnMarkerPositionChanged(int position);

protected:
    // We are using double buffering to avoid flicker and excessive updates.
    void UpdateDoubleBufferImage();

    // Draw different types of scales.
    void DrawScale(QPainter& painter, int tickStep, int tickStartPos, int tickEndPos,
                   bool drawValues, bool isHorizontal);

private:
    // Ruler orientation.
    Qt::Orientation orientation = Qt::Horizontal;

    // Ruler settings.
    RulerSettings settings;

    // Ruler double buffer.
    QPixmap doubleBuffer;

    // Marker position.
    int markerPosition = 0;
};

#endif /* defined(__RULER_WIDGET__H__) */
