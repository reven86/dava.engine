#ifndef __RULERCONTROLLER__H__
#define __RULERCONTROLLER__H__

#include "RulerSettings.h"

#include <QObject>
#include <QPoint>

class RulerController : public QObject
{
    Q_OBJECT

public:
    // Construction/destruction.
    RulerController(QObject* parent = nullptr);
    ~RulerController() = default;

    // Set the screen view pos and scale.
    void SetScale(float scale);

public slots:
    // Update the ruler markers with the mouse position.
    void UpdateRulerMarkers(QPoint curMousePos);
    void SetViewPos(QPoint pos);

signals:
    // Horizontal/Vertical ruler settings are changed.
    void HorisontalRulerSettingsChanged(const RulerSettings& settings);
    void VerticalRulerSettingsChanged(const RulerSettings& settings);

    // Horizontal/Vertical mark positions are changed.
    void HorisontalRulerMarkPositionChanged(int position);
    void VerticalRulerMarkPositionChanged(int position);

protected:
    // Update the rulers by sending "settings changed" signal to them.
    void UpdateRulers();

    void SetupInitialRulerSettings(RulerSettings& settings);

    // Recalculate the ruler settings depending on position/zoom level and emit signals.
    void RecalculateRulerSettings();

private:
    // Screen view pos and scale.
    QPoint viewPos;
    float screenScale = 1.0f;

    // Ruler settings.
    RulerSettings horisontalRulerSettings;
    RulerSettings verticalRulerSettings;
};

#endif /* defined(__RULERCONTROLLER__H__) */
