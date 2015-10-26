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
    RulerController();
    ~RulerController();

    // Set the screen view pos and scale.
    void SetViewPos(QPoint startPos);
    void SetScale(float scale);

    // Update the rulers by sending "settings changed" signal to them.
    void UpdateRulers();

    // Update the ruler markers with the mouse position.
    void UpdateRulerMarkers(QPoint curMousePos);

signals:
    // Horizontal/Vertical ruler settings are changed.
    void HorisontalRulerSettingsChanged(const RulerSettings& settings);
    void VerticalRulerSettingsChanged(const RulerSettings& settings);

    // Horizontal/Vertical mark positions are changed.
    void HorisontalRulerMarkPositionChanged(int position);
    void VerticalRulerMarkPositionChanged(int position);

protected:
    void SetupInitialRulerSettings(RulerSettings& settings);

    // Recalculate the ruler settings depending on position/zoom level and emit signals.
    void RecalculateRulerSettings();

private:
    // Screen view pos and scale.
    QPoint screenViewPos;
    float screenScale;

    // Ruler settings.
    RulerSettings horisontalRulerSettings;
    RulerSettings verticalRulerSettings;
};

#endif /* defined(__RULERCONTROLLER__H__) */
