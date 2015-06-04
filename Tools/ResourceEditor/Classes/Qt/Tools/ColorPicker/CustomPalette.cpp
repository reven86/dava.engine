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


#include "CustomPalette.h"

#include <QPainter>
#include <QDebug>
#include <QDrag>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>

#include "ColorCell.h"


namespace
{
    const int BORDER = 1;
}


CustomPalette::CustomPalette(QWidget* parent)
    : QWidget(parent)
    , nRows(6)
    , nColumns(4)
    , cellSize(34, 34)
{
    CreateControls();
    AdjustControls();
}

CustomPalette::~CustomPalette()
{
}

void CustomPalette::SetCellSize(const QSize& _size)
{
    cellSize = _size;
}

void CustomPalette::SetCellCount(int w, int h)
{
    nColumns = w;
    nRows = h;
}

CustomPalette::Colors CustomPalette::GetColors() const
{
    Colors cs;
    cs.reserve(controls.size());
    for (int i = 0; i < controls.size(); i++)
    {
        if (controls[i])
        {
            cs.push_back(controls[i]->GetColor());
        }
    }

    return cs;
}

void CustomPalette::SetColors(const Colors& _colors)
{
    colors = _colors;
    CreateControls();
    AdjustControls();
}

void CustomPalette::resizeEvent(QResizeEvent* e)
{
    Q_UNUSED(e);
    AdjustControls();
}

void CustomPalette::CreateControls()
{
    qDeleteAll(controls);
    const int n = Count();

    controls.clear();
    controls.reserve(n);
    for (int i = 0; i < n; i++)
    {
        ColorCell *cell = new ColorCell(this);
        const QColor c = (i < colors.size()) ? colors[i] : Qt::transparent;
        cell->SetColor(c);

        connect(cell, SIGNAL( clicked(const QColor&) ), SIGNAL( selected(const QColor&) ));

        controls << cell;
    }
}

void CustomPalette::AdjustControls()
{
    const int xOfs = ( width() - BORDER * 2 - nColumns * cellSize.width() ) / nColumns;
    const int yOfs = ( height() - BORDER * 2 - nRows * cellSize.height() ) / nRows;

    int yPos = yOfs / 2;
    for (int y = 0; y < nRows; y++)
    {
        int xPos = xOfs / 2;
        for (int x = 0; x < nColumns; x++)
        {
            const int i = x + y * nColumns;
            ColorCell *cell = controls[i];
            if (cell)
            {
                cell->resize(cellSize);
                cell->move(xPos, yPos);
            }
            xPos += xOfs + cellSize.width();
        }
        yPos += yOfs + cellSize.height();
    }
}

int CustomPalette::Count() const
{
    return nRows * nColumns;
}
