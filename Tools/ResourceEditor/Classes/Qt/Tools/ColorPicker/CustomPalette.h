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


#ifndef CUSTOMPALETTE_H
#define CUSTOMPALETTE_H

#include <QWidget>
#include <QPointer>


class ColorCell;
class MouseHelper;

class CustomPalette
    : public QWidget
{
    Q_OBJECT

public:
    typedef QVector<QColor> Colors;

signals:
    void selected(const QColor& c);

public:
    explicit CustomPalette(QWidget *parent = NULL);
    ~CustomPalette();

    void SetColors(const Colors& colors);
    void SetCellSize(const QSize& size);
    void SetCellCount(int w, int h);
    Colors GetColors() const;

private:
    void resizeEvent(QResizeEvent* e);

    void CreateControls();
    void AdjustControls();
    int Count() const;

    int nRows;
    int nColumns;
    Colors colors;
    QList< QPointer<ColorCell> > controls;
    QSize cellSize;
};



#endif // PALETTEHSV_H
