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



#include "GradientPickerWidget.h"
#include <QPainter>
#include <QPaintEvent>

#include "Main/QtUtils.h"
#include "Tools/ColorPicker/ColorPicker.h"

#define BACKGROUND_COLOR (Color(0x80, 0x80, 0x80, 0xff) / 255.f)
#define BORDER_COLOR Color::Black
#define EMPTY_WIDGET_COLOR Color::Black
#define DEFAULT_GRADIENT_COLOR Color::White
#define UNSELECTED_MARKER_COLOR Color::White
#define SELECTED_MARKER_COLOR Color(0.f, 1.f, 0.f, 1.f)
#define TEXT_COLOR Color::White

#define MARKER_SIZE 4
#define TILED_RECT_SIZE 20
#define WIDGET_MIN_HEIGHT 38
#define WIDGET_MAX_HEIGHT 50
#define LEGEND_HEIGHT 12

GradientPickerWidget::GradientPickerWidget(QWidget *parent) :
	QWidget(parent),
	minTime(0.f),
	maxTime(1.f),
	selectedPointIndex(-1),
	showCursorPos(false),
	legend("")
{
	setMinimumHeight(WIDGET_MIN_HEIGHT);
	setMaximumHeight(WIDGET_MAX_HEIGHT);
	setMouseTracking(true);

	backgroundBrush.setColor(ColorToQColor(BACKGROUND_COLOR));
	backgroundBrush.setStyle(Qt::SolidPattern);

	tiledPixmap = QPixmap(TILED_RECT_SIZE, TILED_RECT_SIZE);
	QPainter painter(&tiledPixmap);
	painter.setBrush(QBrush(Qt::lightGray));
	painter.setPen(Qt::NoPen);
	painter.drawRect(QRect(0, 0, TILED_RECT_SIZE, TILED_RECT_SIZE));
	painter.setBrush(QBrush(Qt::white));
	painter.drawRect(QRect(0, 0, TILED_RECT_SIZE / 2, TILED_RECT_SIZE / 2));
	painter.drawRect(QRect(TILED_RECT_SIZE / 2, TILED_RECT_SIZE / 2, TILED_RECT_SIZE / 2, TILED_RECT_SIZE / 2));
}

GradientPickerWidget::~GradientPickerWidget()
{
}

void GradientPickerWidget::Init(float32 minT, float32 maxT, const QString& legend)
{
	SetLimits(minT, maxT);
	this->legend = legend;
}

void GradientPickerWidget::SetLimits(float32 minT, float32 maxT)
{
	if (maxT - minT < 0.01f || minT > maxT)
		return;

	minTime = minT;
	maxTime = maxT;
}

void GradientPickerWidget::SetValues(const Vector<PropValue<Color> >& values)
{
	ClearPoints();
	for (uint32 i = 0; i < values.size(); ++i)
	{
		AddColorPoint(values[i].t, values[i].v);
	}
	update();
}

bool GradientPickerWidget::ComparePoints(const std::pair<float32, Color> &a, const std::pair<float32, Color> &b)
{
	return (a.first < b.first);
}

bool GradientPickerWidget::CompareIndices(int32 a, int32 b)
{
	return (a > b);
}

bool GradientPickerWidget::GetValues(Vector<PropValue<Color> >* values)
{
	Vector<std::pair<float32, Color> > sortedPoints = points;
	std::sort(sortedPoints.begin(), sortedPoints.end(), ComparePoints);

	for (uint32 i = 0; i < sortedPoints.size(); ++i)
	{
		values->push_back(PropValue<Color>(sortedPoints[i].first, sortedPoints[i].second));
	}
	
	return true;
}

void GradientPickerWidget::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	
	QFont font("Courier", 11, QFont::Normal);
	painter.setFont(font);

	painter.fillRect(this->rect(), backgroundBrush);
	painter.setPen(Qt::black);
	painter.drawRect(this->rect().adjusted(0, 0, -1, -1));

	QRectF graphRect = GetGraphRect();
	QPointF graphCenter = graphRect.center();

	painter.drawTiledPixmap(graphRect, tiledPixmap);

	QPen pen;
	QRect textRect = GetTextRect();
	if(legend != "")
	{
		pen.setColor(ColorToQColor(TEXT_COLOR));
		painter.setPen(pen);
		painter.drawText(textRect, Qt::AlignHCenter | Qt::AlignVCenter, legend);
	}

	pen.setColor(ColorToQColor(BORDER_COLOR));
	painter.setPen(pen);
	painter.drawRect(graphRect);

	QLinearGradient gradient(graphRect.left(), graphCenter.y(), graphRect.right(), graphCenter.y());
	if (points.empty())
		gradient.setColorAt(0.f, ColorToQColor(EMPTY_WIDGET_COLOR));
	for (uint32 i = 0; i < points.size(); ++i)
	{
		gradient.setColorAt(GetGradientPosFromTime(points[i].first), ColorToQColor(points[i].second));
	}

	painter.setBrush(QBrush(gradient));
	painter.drawRect(graphRect);

	painter.drawLine(graphRect.left(), graphCenter.y(), graphRect.right(), graphCenter.y());
	if(showCursorPos)
		painter.drawLine(cursorPos.x(), graphRect.top(), cursorPos.x(), graphRect.bottom());

	QBrush selectedBrush(ColorToQColor(SELECTED_MARKER_COLOR));
	QBrush unselectedBrush(ColorToQColor(UNSELECTED_MARKER_COLOR));
	Vector<QRectF> markers = GetMarkerRects();
	for (uint32 i = 0; i < markers.size(); ++i)
	{
		if (i == selectedPointIndex)
			painter.setBrush(selectedBrush);
		else
			painter.setBrush(unselectedBrush);
		painter.drawRect(markers[i]);
	}
}

void GradientPickerWidget::mouseMoveEvent(QMouseEvent *event)
{
	QPoint point = event->pos();
	QRect graphRect = GetGraphRect();

	int32 newX = qMin(graphRect.right(), point.x());
	newX = qMax(newX, graphRect.left());
	
	showCursorPos = true;
	cursorPos.setX(newX);
	cursorPos.setY(point.y());

	float32 x = GetTimeFromCursorPos(newX);

	if(selectedPointIndex != -1 && event->buttons() == Qt::LeftButton)
	{
		points[selectedPointIndex].first = x;
	}

	update();
}

void GradientPickerWidget::mousePressEvent(QMouseEvent *event)
{
	QPoint point = event->pos();
	Vector<int32> pointIds = GetMarkersFromCursorPos(point);
	int32 pointId = pointIds[0];

	if(event->button() == Qt::LeftButton)
	{
		selectedPointIndex = pointId;
		update();
	}
	else if(event->button() == Qt::RightButton)
	{
		if(pointId == -1)
		{
			QRect graphRect = GetGraphRect();
			if(graphRect.contains(point))
			{
				float32 x = GetTimeFromCursorPos(point.x());
				AddPoint(x);
				update();
			}
		}
		else
		{
			DeletePoint(pointId);
			update();
		}
	}
}

void GradientPickerWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
	QPoint point = event->pos();
	Vector<int32> pointIds = GetMarkersFromCursorPos(point);
	int32 pointId = pointIds[0];

	if(pointId != -1 && event->button() == Qt::LeftButton)
	{
        const Color oldColor = points[pointId].second;
        ColorPicker cp(this);
        cp.setWindowTitle("Marker color");
        cp.SetDavaColor(oldColor);
        const bool result = cp.Exec();
        const Color newColor = cp.GetDavaColor();

        if(result && newColor != oldColor)
		{
			SetCurrentPointColor(newColor);
			update();
			
			emit ValueChanged();
		}
	}
}

void GradientPickerWidget::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton && selectedPointIndex != -1)
	{
		QPoint point = event->pos();
		Vector<int32> pointIds = GetMarkersFromCursorPos(point);

		for (uint32 i = 0; i < pointIds.size(); ++i)
		{
			if (pointIds[i] == selectedPointIndex)
			{
				pointIds.erase(pointIds.begin() + i);
				break;
			}
		}
		DeletePoints(pointIds);
		update();
	}

	emit ValueChanged();
}

void GradientPickerWidget::leaveEvent(QEvent *)
{
	showCursorPos = false;
	update();
}

QRect GradientPickerWidget::GetGraphRect()
{
	int32 indent = MARKER_SIZE + 1;
	return this->rect().adjusted(indent, LEGEND_HEIGHT, -indent, -indent);
}

QRect GradientPickerWidget::GetTextRect()
{
	QRect r = this->rect().adjusted(1, 1, -1, 0);
	r.setHeight(LEGEND_HEIGHT - 2);
	return r;
}

bool GradientPickerWidget::AddPoint(float32 point)
{
	if(points.empty())
		point = 0.f;

	return AddColorPoint(point, GetCurrentPointColor());
}

bool GradientPickerWidget::AddColorPoint(float32 point, const Color &color)
{
	if(point < minTime || point > maxTime)
		return false;

	points.push_back(std::make_pair(point, color));
	return true;
}

bool GradientPickerWidget::SetCurrentPointColor(const Color& color)
{
	return SetPointColor(selectedPointIndex, color);
}

bool GradientPickerWidget::SetPointColor(uint32 index, const DAVA::Color &color)
{
	if (index >= points.size())
		return false;

	points[index].second = color;
	return true;
}

Color GradientPickerWidget::GetCurrentPointColor()
{
	return GetPointColor(selectedPointIndex);
}

Color GradientPickerWidget::GetPointColor(uint32 index)
{
	if(index >= points.size())
		return DEFAULT_GRADIENT_COLOR;
	
	return points[index].second;
}

bool GradientPickerWidget::DeleteCurrentPoint()
{
	return DeletePoint(selectedPointIndex);
}

bool GradientPickerWidget::DeletePoint(uint32 index)
{
	if (index >= points.size())
		return false;

	points.erase(points.begin() + index);

	if(selectedPointIndex == index)
	{
		selectedPointIndex = -1;
	}
	else if(selectedPointIndex >(int32) index)
	{
		--selectedPointIndex;
	}

	return true;
}

bool GradientPickerWidget::DeletePoints(Vector<int32> indices)
{
	std::sort(indices.begin(), indices.end(), CompareIndices);
	for (uint32 i = 0; i < indices.size(); ++i)
		if(!DeletePoint(indices[i]))
			return false;

	return true;
}

void GradientPickerWidget::ClearPoints()
{
	points.clear();
}

float32 GradientPickerWidget::GetTimeFromCursorPos(float32 xPos)
{
	QRectF graphRect = GetGraphRect();
	float32 timeLine = maxTime - minTime;
	xPos -= graphRect.left();

	float32 time = minTime + timeLine * xPos / graphRect.width();
	return time;
}

float32 GradientPickerWidget::GetCursorPosFromTime(float32 time)
{
	QRectF graphRect = GetGraphRect();
	float32 timeLine = maxTime - minTime;

	float32 pos = graphRect.left() + graphRect.width() * time / timeLine;
	return pos;
}

float32 GradientPickerWidget::GetGradientPosFromTime(float32 time)
{
	float32 timeLine = maxTime - minTime;

	float32 gradientPos = (time - minTime) / timeLine;
	return gradientPos;
}

Vector<QRectF> GradientPickerWidget::GetMarkerRects()
{
	Vector<QRectF> rects;

	QRectF graphRect = GetGraphRect();
	QPointF graphCenter = graphRect.center();
	for (uint32 i = 0; i < points.size(); ++i)
	{
		QRectF pointMarker;
		pointMarker.setSize(QSizeF(MARKER_SIZE * 2, MARKER_SIZE * 2));

		int32 x = GetCursorPosFromTime(points[i].first);
		pointMarker.moveCenter(QPointF(x, graphCenter.y()));

		rects.push_back(pointMarker);
	}

	return rects;
}

Vector<int32> GradientPickerWidget::GetMarkersFromCursorPos(const QPoint &point)
{
	Vector<QRectF> markers = GetMarkerRects();
	Vector<int32> res;

	for (uint32 i = 0; i < markers.size(); ++i)
	{
		if (markers[i].contains(point))
		{
			res.push_back(i);
		}
	}

	if (res.empty())
		res.push_back(-1);

	return res;
}
