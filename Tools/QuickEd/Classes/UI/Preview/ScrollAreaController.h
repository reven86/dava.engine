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

namespace DAVA{
    class UIControl;
}

class ScrollAreaController : public QObject
{
    Q_OBJECT
public:
    Q_PROPERTY(QSize size READ GetSize WRITE SetSize NOTIFY SizeChanged);
    Q_PROPERTY(int scale READ GetScale WRITE SetScale NOTIFY ScaleChanged);
    Q_PROPERTY(QPoint position READ GetPosition WRITE SetPosition NOTIFY PositionChanged);

    ScrollAreaController(QObject *parent = nullptr);
    ~ScrollAreaController() = default;

    QSize GetSize() const;
    int GetScale() const;
    QPoint GetPosition() const;

public slots:
    void SetSize(const QSize &size);
    void SetScale(int scale);
    void SetPosition(const QPoint &position);
signals:
    void SizeChanged(const QSize &size);
    void ScaleChanged(int scale);
    void PositionChanged(const QPoint &position);
private:
    static DAVA::UIControl *Root();
    QSize size = QSize(0, 0);
    int scale = 0;
    QPoint position = QPoint(0, 0);
};

#endif // __QUICKED_PREVIEW_SCROLL_AREA_CONTROLLER_H__
