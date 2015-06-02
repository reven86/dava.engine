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


#ifndef VALUESLIDER_H
#define VALUESLIDER_H

#include <QWidget>
#include <QPointer>


class QLineEdit;
class MouseHelper;

//
//  Parent widget must have Click focus policy
//

class ValueSlider
    : public QWidget
{
    Q_OBJECT

    signals:
    void started(double);
    void changing(double);
    void changed(double);
    void canceled();

public:
    explicit ValueSlider(QWidget* parent = NULL);
    ~ValueSlider();

    void SetDigitsAfterDot(int c);
    void SetRange(double min, double max);
    void SetValue(double val);
    double GetValue() const;

protected:
    virtual void DrawBackground(QPainter* p) const;
    virtual void DrawForeground(QPainter* p) const;
    virtual QRect PosArea() const;

    void paintEvent(QPaintEvent* e);
    void resizeEvent(QResizeEvent* e);

    bool eventFilter(QObject* obj, QEvent* e);

    bool IsEditorMode() const;

private slots:
    void OnMousePress(const QPoint& pos);
    void OnMouseMove(const QPoint& pos);
    void OnMouseRelease(const QPoint& pos);
    void OnMouseClick();

private:
    void normalize();
    void undoEditing();
    void acceptEditing();

    double minVal;
    double maxVal;
    double val;
    int digitsAfterDot;

    QPointer<MouseHelper> mouse;
    QPoint clickPos;
    double clickVal;
    mutable QPixmap arrows;

    QPointer<QLineEdit> editor;
};


#endif // VALUESLIDER_H
