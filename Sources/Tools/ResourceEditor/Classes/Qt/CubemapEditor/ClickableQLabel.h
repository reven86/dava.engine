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


#ifndef __QT_CLICKABLE_QLABEL_H__
#define __QT_CLICKABLE_QLABEL_H__

#include <QLabel.h>

class ClickableQLabel : public QLabel
{
	Q_OBJECT
	
public:
	
	ClickableQLabel(QWidget *parent = 0);
	~ClickableQLabel();
	
	void SetRotation(int rotation);
	int GetRotation();
    
    void SetVisualRotation(int rotation);
    int GetVisualRotation();
    
	void SetFaceLoaded(bool loaded);
	bool GetFaceLoaded();

	void OnParentMouseMove(QMouseEvent *ev);
	
protected:
	
	void mousePressEvent(QMouseEvent *ev);
	void enterEvent(QEvent *ev);
    void leaveEvent(QEvent *ev);
    void paintEvent(QPaintEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);

signals:

	void OnLabelClicked();
	void OnRotationChanged();
	
private:
	
	enum RotateButtonDrawFlags
	{
		None = 0,
		RotateClockwise = 1,
		RotateCounterclockwise = 2
	};
	
	bool IsPointInsideClockwiseRotationArea(QMouseEvent *ev);
	bool IsPointInsideCounterclockwiseRotationArea(QMouseEvent *ev);
	bool IsPointOutsideControl(QMouseEvent *ev);
	void DrawRotationIcon(QPaintEvent *ev, QPoint position, float opacity, bool flipped);
	void DrawFaceImage(QPaintEvent *ev);
	QPoint GetPointForButton(RotateButtonDrawFlags flag);
	
private:
	
	bool faceLoaded;
	bool mouseEntered;
	int buttonDrawFlags;
	int currentRotation;
    int visualRotation;
	
	static QImage rotateClockwiseImage;
	static QImage rotateCounterclockwiseImage;
};

#endif /* defined(__QT_CLICKABLE_QLABEL_H__) */
