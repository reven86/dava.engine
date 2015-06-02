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


#ifndef ABSTRACTSLIDER_H
#define ABSTRACTSLIDER_H

#include <QWidget>
#include <QPointer>


class MouseHelper;


class AbstractSlider
    : public QWidget
{
    Q_OBJECT

signals:
    void started( const QPointF& );
    void changing( const QPointF& );
    void changed( const QPointF& );
    void canceled();

public:
    explicit AbstractSlider(QWidget *parent);
    ~AbstractSlider();

    QPointF PosF() const;
    void SetPosF( const QPointF& posF );

protected:
    void paintEvent( QPaintEvent* e ) override;
    void resizeEvent( QResizeEvent* e ) override;

    virtual void DrawBackground( QPainter *p ) const;
    virtual void DrawForeground( QPainter *p ) const;
    virtual QRect PosArea() const;

    QPoint Pos() const;
    void SetPos( const QPoint& pos );
    MouseHelper *Mouse() const;

private slots:
    void OnMousePress( const QPoint& pos );
    void OnMouseMove( const QPoint& pos );
    void OnMouseRelease( const QPoint& pos );

private:
    QPointF posF;
    QPoint pressPos;
    QSize lastSize;     //  остыль, т.к. Qt присылает неверный oldSize в первый resizeEvent
    QPointer< MouseHelper > mouse;
};


#endif // ABSTRACTSLIDER_H
