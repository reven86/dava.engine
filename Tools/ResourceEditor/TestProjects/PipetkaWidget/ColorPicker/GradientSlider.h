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


#ifndef GRADIENTSLIDER_H
#define GRADIENTSLIDER_H

#include <QWidget>
#include <QMap>


#include "GradientWidget.h"
#include "IColorEditor.h"
#include "MouseHelper.h"


class GradientSlider
    : public GradientWidget
    , public IColorEditor
{
    Q_OBJECT

signals:
    // IColorEditor overrides
    void begin();
    void changing( QColor const& c );
    void changed( QColor const& c );
    void canceled();

public:
    explicit GradientSlider(QWidget *parent);
    ~GradientSlider();

    void setEditorDimensions( Qt::Edges flags = (Qt::LeftEdge | Qt::RightEdge) );
    void setPrefferableArrows();

    QPointF GetPosF() const;

    // IColorEditor overrides
    QColor GetColor() const override;
    void setColor( QColor const& c ) override;

protected:
    // GradientWidget overrides
    QPixmap drawContent() const override;

    // QWidget overrides
    void resizeEvent( QResizeEvent* e ) override;

private slots:
    void onMousePress( const QPoint& pos );
    void onMouseMove( const QPoint& pos );
    void onMouseRelease( const QPoint& pos );
    void onClick();

private:
    QPoint fitInBackground( const QPoint& pos ) const;
    void setPos( const QPoint& pos );

    void drawArrows( QPainter *p ) const;
    void drawArrow( Qt::Edge arrow, QPainter *p ) const;

    QPointer<MouseHelper> mouse;
    QSize arrowSize;
    Qt::Edges arrows;
    QPoint currentPos;
    QColor startColor;
    QColor color;

    mutable QMap< Qt::Edge, QPixmap > arrowCache;
};

#endif // GRADIENTSLIDER_H
