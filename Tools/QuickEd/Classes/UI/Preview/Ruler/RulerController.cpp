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

void RulerController::SetViewPos(QPoint viewPos)
{
    viewPos *= -1;
    if (viewPos.x() != screenViewPos.x())
    {
        screenViewPos.setX(viewPos.x());
        horisontalRulerSettings.startPos = screenViewPos.x();
        emit HorisontalRulerSettingsChanged(horisontalRulerSettings);
    }

    if (viewPos.y() != screenViewPos.y())
    {
        screenViewPos.setY(viewPos.y());
        verticalRulerSettings.startPos = screenViewPos.y();
        emit VerticalRulerSettingsChanged(verticalRulerSettings);
    }
}

void RulerController::SetScale(float scale)
{
    screenScale = scale / 100.f;
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

    for (int i = 0; i < sizeof(ticksMap); i++)
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