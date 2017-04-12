#include <cmath>
#include "RulerController.h"

RulerController::RulerController(QObject* parent)
    : QObject(parent)
    , screenScale(0.0f)
{
    SetupInitialRulerSettings(horisontalRulerSettings);
    SetupInitialRulerSettings(verticalRulerSettings);
}

void RulerController::SetupInitialRulerSettings(RulerSettings& settings)
{
    static const int defaultSmallTicksDelta = 10;
    static const int defaultBigTicksDelta = 50;

    settings.smallTicksDelta = defaultSmallTicksDelta;
    settings.bigTicksDelta = defaultBigTicksDelta;
    settings.startPos = 0;
    settings.zoomLevel = 1.0f;
}

void RulerController::SetViewPos(QPoint pos)
{
    if (viewPos != pos)
    {
        viewPos = pos;
        horisontalRulerSettings.startPos = viewPos.x();
        verticalRulerSettings.startPos = viewPos.y();

        UpdateRulers();
    }
}

void RulerController::SetScale(float scale)
{
    screenScale = scale;
    horisontalRulerSettings.zoomLevel = screenScale;
    verticalRulerSettings.zoomLevel = screenScale;

    RecalculateRulerSettings();
}

void RulerController::UpdateRulerMarkers(QPoint curMousePos)
{
    emit HorisontalRulerMarkPositionChanged(curMousePos.x());
    emit VerticalRulerMarkPositionChanged(curMousePos.y());
}

void RulerController::UpdateRulers()
{
    emit HorisontalRulerSettingsChanged(horisontalRulerSettings);
    emit VerticalRulerSettingsChanged(verticalRulerSettings);
}

void RulerController::RecalculateRulerSettings()
{
    static const struct
    {
        float scaleLevel;
        int smallTicksDelta;
        int bigTicksDelta;
    } ticksMap[] =
    {
      { 0.1f, 100, 500 },
      { 0.25f, 64, 320 },
      { 0.5f, 40, 200 },
      { 0.75f, 16, 80 },
      { 1.0f, 10, 50 },
      { 2.0f, 10, 50 },
      { 4.0f, 2, 20 },
      { 8.0f, 1, 10 },
      { 12.0f, 1, 10 },
      { 16.0f, 1, 10 }
    };

    // Look for the closest value.
    int closestValueIndex = 0;
    float closestScaleDistance = std::numeric_limits<float>::max();

    for (int i = 0; i < sizeof(ticksMap) / sizeof(ticksMap[0]); i++)
    {
        float curScaleDistance = std::fabs(ticksMap[i].scaleLevel - screenScale);
        if (curScaleDistance < closestScaleDistance)
        {
            closestScaleDistance = curScaleDistance;
            closestValueIndex = i;
        }
    }

    horisontalRulerSettings.smallTicksDelta = ticksMap[closestValueIndex].smallTicksDelta;
    horisontalRulerSettings.bigTicksDelta = ticksMap[closestValueIndex].bigTicksDelta;
    verticalRulerSettings.smallTicksDelta = ticksMap[closestValueIndex].smallTicksDelta;
    verticalRulerSettings.bigTicksDelta = ticksMap[closestValueIndex].bigTicksDelta;

    UpdateRulers();
}
