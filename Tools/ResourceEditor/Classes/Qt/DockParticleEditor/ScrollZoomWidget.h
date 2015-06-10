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


#ifndef TIMELINE_WIDGET_BASE_H
#define TIMELINE_WIDGET_BASE_H


#include <DAVAEngine.h>
#include <QWidget>
#include <QString>
#include <QDialog>
#include <QScrollBar.h>
#include <qslider.h>

#define SCALE_WIDTH				25

#define GRAPH_HEIGHT			150
#define GRAPH_OFFSET_STEP		10

#define MINIMUM_DISPLAYED_TIME	0.02f
#define ZOOM_STEP				0.1f
#define UI_RECTANGLE_OFFSET		1.5

#define SCROLL_BAR_HEIGHT		12

#define MIN_ZOOM				1.0f
#define MAX_ZOOM				10.0f
#define ZOOM_SLIDER_LENGTH		40

#ifdef __DAVAENGINE_WIN32__
#define SLIDER_HEIGHT_EXPAND    0
#else
#define SLIDER_HEIGHT_EXPAND    5
#endif

using namespace DAVA;
#include <qpainter.h>

class ScrollZoomWidget : public QWidget
{
	Q_OBJECT

public:
	explicit ScrollZoomWidget(QWidget *parent = 0);
	~ScrollZoomWidget();
	
	virtual void Init(float32 minT, float32 maxT);
	float32 GetMinBoundary();
	float32 GetMaxBoundary();

signals:
	void ValueChanged();

protected:
	virtual void paintEvent(QPaintEvent *,QPainter& painter );
	virtual void mouseMoveEvent(QMouseEvent *);
	virtual void mousePressEvent(QMouseEvent *);
	virtual void mouseReleaseEvent(QMouseEvent *);

	virtual void wheelEvent(QWheelEvent*);

	virtual void UpdateSizePolicy()			= 0;
	virtual QRect GetGraphRect() const		= 0;
	virtual QRect GetIncreaseRect() const	= 0;
	virtual QRect GetScaleRect() const		= 0;
	virtual QRect GetDecreaseRect() const	= 0;
	virtual QRect GetSliderRect() const		= 0;

	virtual QRect GetScrollBarRect() const;

	void UpdateScrollBarPosition();
	void UpdateScrollBarSlider();

	void UpdateSliderPosition();
	void UpdateZoomSlider();

	QString float2QString(float32 value) const;
	
	int32 GetIntValue(float32 value) const;

	void PerformZoom(float newScale, bool moveScroll = true);

	void PerformOffset(float value, bool moveScroll = true);

	enum ePositionRelativelyToDrawRect
	{
		POSITION_LEFT,
		POSITION_RIGHT,
		POSITION_INSIDE
	};
	ePositionRelativelyToDrawRect GetPointPositionFromDrawingRect(QPoint point) const; 

protected slots:

	void HandleHorizontalScrollChanged(int value);
	void HandleZoomScrollChanged(int value);

protected:
	QPoint			mouseStartPos;

	float32			minValue;
	float32			maxValue;
	float32			minTime;
	float32			maxTime;
	float32			generalMinTime;
	float32			generalMaxTime;
	float32			minValueLimit;
	float32			maxValueLimit;
	
	enum eGridStyle
	{
		GRID_STYLE_ALL_POSITION,
		GRID_STYLE_LIMITS
	};
	eGridStyle		gridStyle;
	
	QBrush			backgroundBrush;
	float32			scale;
	float32			initialTimeInterval;

	QScrollBar	*	horizontalScrollBar;
	QSlider	*		zoomSlider;
};


#endif // TIMELINE_WIDGET_BASE_H
