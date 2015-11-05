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
    ~ScrollAreaController() = default;

    void SetNestedControl(DAVA::UIControl* nestedControl);
    void AdjustScale(qreal newScale, QPointF mousePos);

    QSize GetCanvasSize() const;
    QSize GetViewSize() const;
    QPoint GetPosition() const;
    qreal GetScale() const;

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

private:
    void UpdatePosition();
    DAVA::ScopedPtr<DAVA::UIControl> backgroundControl;
    DAVA::UIControl* nestedControl = nullptr;
    QSize canvasSize = QSize(0, 0);
    QSize viewSize = QSize(0, 0);
    QPoint position = QPoint(0, 0);
    qreal scale = 0.0f;
    const int Margin = 50;
};

#endif // __QUICKED_PREVIEW_SCROLL_AREA_CONTROLLER_H__
