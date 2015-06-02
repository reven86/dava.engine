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


#ifndef GRADIENDWIDGET_H
#define GRADIENDWIDGET_H

#include <QWidget>


class GradientWidget
    : public QWidget
{
    Q_OBJECT

protected:
    struct Offset
    {
        int left;
        int top;
        int right;
        int bottom;

        Offset() : left(0), top(0), right(0), bottom(0) {}
    };

public:
    explicit GradientWidget(QWidget *parent);
    ~GradientWidget();

    void SetColorRange( const QColor& start, const QColor& stop );
    void SetRenderDimensions( bool hor, bool ver );
    void SetBgPadding( int left, int top, int right, int bottom );
    void SetGrid( bool enabled, const QSize& size = QSize() );

    QColor GetColorAt( const QPoint& pos ) const;

protected:
    virtual QPixmap drawBackground() const;
    virtual QPixmap drawContent() const;

    const Offset& padding() const;

    // QWidget
    void paintEvent( QPaintEvent* e ) override;
    void resizeEvent( QResizeEvent* e ) override;

private:

    mutable QPixmap cacheBg;
    mutable QImage cacheBgImage;    // To fast GetColor of any pixel
    QColor startColor;
    QColor stopColor;
    Offset paddingOfs;
    bool hor;   // Direction
    bool ver;   // Direction
    bool fillBg;
    QSize gridSize;
};


#endif // GRADIENDWIDGET_H
