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


#include "RulerWidget.h"

#include <QPainter>
#include <QApplication>
#include <QDrag>
#include <QMimeData>

using namespace DAVA;

RulerWidget::RulerWidget(QWidget* parent)
    : QWidget(parent)
{
    UpdateDoubleBufferImage();
}

void RulerWidget::SetRulerOrientation(Qt::Orientation orientation_)
{
    orientation = orientation_;
    UpdateDoubleBufferImage();
    update();
}

void RulerWidget::SetRulerSettings(const RulerSettings& rulerSettings)
{
    settings = rulerSettings;
    UpdateDoubleBufferImage();
    update();
}

void RulerWidget::OnRulerSettingsChanged(const RulerSettings& rulerSettings)
{
    SetRulerSettings(rulerSettings);
}

void RulerWidget::OnMarkerPositionChanged(int32 position)
{
    markerPosition = position;
    update();
}

void RulerWidget::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, doubleBuffer);
    if (markerPosition == 0)
    {
        return;
    }

    // Draw the marker.
    QPen pen(Qt::black, 1);
    pen.setStyle(Qt::DashLine);
    painter.setPen(pen);

    if (orientation == Qt::Horizontal)
    {
        painter.drawLine(markerPosition, 0, markerPosition, rect().height());
    }
    else
    {
        painter.drawLine(0, markerPosition, rect().width(), markerPosition);
    }
}

QSize RulerWidget::minimumSizeHint() const
{
    static const int minimumSize = 16;
    switch (orientation)
    {
    default:
        return QSize(0, minimumSize);
    case Qt::Vertical:
        return QSize(minimumSize, 0);
    }
}

void RulerWidget::resizeEvent(QResizeEvent*)
{
    UpdateDoubleBufferImage();
}

void RulerWidget::DrawScale(QPainter& painter, int tickStep, int tickStartPos, int tickEndPos,
                            bool drawValues, bool isHorizontal)
{
    if (tickStep <= 0)
    {
        return;
    }

    int numberOffset = ceilf((float)(settings.startPos) / (float)(tickStep)) * (float)tickStep;
    int tickOffset = numberOffset - settings.startPos;

    int fontPos = 0;
    int endPos = 0;
    if (isHorizontal)
    {
        endPos = rect().width();
        fontPos = rect().height() / 2;
    }
    else
    {
        endPos = rect().height();
        fontPos = 1;
    }

    const QFont& font = painter.font();
    int fontSize = font.pixelSize();
    char nextDigit[2] = { 0x00, 0x00 };

    QVector<QLine> linesVector;
    linesVector.reserve(endPos / tickStep);
    for (int i = 0; i < endPos; i += tickStep)
    {
        int curPos = i + tickOffset;
        int curPosValue = (int)(i + numberOffset) / settings.zoomLevel;
        if (isHorizontal)
        {
            linesVector.append(QLine(curPos, tickStartPos, curPos, tickEndPos));
            if (drawValues)
            {
                painter.drawText(curPos + 2, fontPos, QString::number(curPosValue));
            }
        }
        else
        {
            linesVector.append(QLine(tickStartPos, curPos, tickEndPos, curPos));
            if (drawValues)
            {
                // In this case draw value digit-by-digit vertically UNDER the tick.
                std::stringstream digits;
                digits << curPosValue;

                int digitPos = curPos + fontSize - 1;
                int32 digitsSize = digits.str().size();
                for (int32 j = 0; j < digitsSize; j++)
                {
                    nextDigit[0] = digits.str().at(j); // next char is always 0x00
                    painter.drawText(fontPos, digitPos, QString::fromLatin1(nextDigit));
                    digitPos += fontSize;
                }
            }
        }
    }

    painter.drawLines(linesVector);
}

void RulerWidget::UpdateDoubleBufferImage()
{
    doubleBuffer = QPixmap(size());
    doubleBuffer.fill();

    static const QColor rulerBackgroundColor = QColor(0xFF, 0xFF, 0xFF);
    static const QColor rulerTicksColor = QColor(0x00, 0x00, 0x00);

    static const int rulerFontSize = 10;

    static const int borderWidth = 2;
    static const int borderOffset = borderWidth / 2;
    static const int ticksWidth = 1;

    QPainter painter(&doubleBuffer);
    painter.fillRect(rect(), rulerBackgroundColor);

    // Draw the thick right and top borders.
    QPen curPen(rulerTicksColor, borderWidth);
    painter.setPen(curPen);

    if (orientation == Qt::Horizontal)
    {
        painter.drawLine(borderOffset, 0, borderOffset, rect().height());
        curPen.setWidth(ticksWidth);
        painter.setPen(curPen);
        painter.drawLine(0, rect().height() - ticksWidth, rect().width(), rect().height() - ticksWidth);
    }
    else
    {
        painter.drawLine(0, borderOffset, rect().width(), borderOffset);
        curPen.setWidth(ticksWidth);
        painter.setPen(curPen);

        painter.drawLine(rect().width() - ticksWidth, 0, rect().width() - ticksWidth, rect().height());
    }

    // Calculate the tick coords depending on orientation.
    int smallTickStart = 0;
    int bigTickStart = 0;
    int tickEnd = 0;

    if (orientation == Qt::Horizontal)
    {
        smallTickStart = (rect().height() / 5) * 3;
        bigTickStart = 0;
        tickEnd = rect().height();
    }
    else
    {
        smallTickStart = (rect().width() / 5) * 3;
        bigTickStart = 0;
        tickEnd = rect().height();
    }

    // Draw small ticks.
    bool isHorizontal = (orientation == Qt::Horizontal);
    int tickStep = settings.smallTicksDelta * settings.zoomLevel;
    DrawScale(painter, tickStep, smallTickStart, tickEnd, false, isHorizontal);

    // Draw big ticks and values.
    QFont font = painter.font();
    font.setPixelSize(rulerFontSize);
    painter.setFont(font);

    tickStep = settings.bigTicksDelta * settings.zoomLevel;
    DrawScale(painter, tickStep, bigTickStart, tickEnd, true, isHorizontal);
}
