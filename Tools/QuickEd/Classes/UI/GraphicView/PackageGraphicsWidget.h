//
//  PackageGraphicsWidget.h
//  UIEditor
//
//  Created by Alexey Strokachuk on 9/17/14.
//
//

#ifndef __UIEditor__ScreenTabWidget__
#define __UIEditor__ScreenTabWidget__

#include <QWidget>

namespace Ui {
    class PackageGraphicsWidget;
}

class PackageDocument;
class GraphicsViewContext;

enum ScreenId
{
    UNKNOWN_SCREEN = -1,
    EDIT_SCREEN = 0,
};

class PackageGraphicsWidget: public QWidget
{
    Q_OBJECT
public:
    PackageGraphicsWidget(QWidget *parent = NULL);
    ~PackageGraphicsWidget();
    
    void SetDocument(PackageDocument *newDocument);

private slots:
    // Zoom.
	void OnScaleByComboIndex(int value);
	void OnScaleByComboText();
	void OnZoomInRequested();
	void OnZoomOutRequested();
    
    void OnCanvasScaleChanged(int newScale);
    void OnGLWidgetResized();

    void OnVScrollbarMoved(int position);
    void OnHScrollbarMoved(int position);
    void OnScrollPositionChanged(const QPoint &newPosition);
    void OnScrollAreaChanged(const QSize &viewSize, const QSize &contentSize);

private:
    void OnScaleByZoom(int scaleDelta);

    QSize GetGLViewSize() const;

private:
    Ui::PackageGraphicsWidget *ui;
    PackageDocument *document;
    GraphicsViewContext *context;
};

#endif /* defined(__UIEditor__ScreenTabWidget__) */
