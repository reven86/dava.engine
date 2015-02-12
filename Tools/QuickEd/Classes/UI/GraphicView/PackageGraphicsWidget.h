#ifndef __QUICKED_PACKAGE_GRAPHICS_WIDGET_H__
#define __QUICKED_PACKAGE_GRAPHICS_WIDGET_H__

#include <QWidget>

namespace Ui {
    class PackageGraphicsWidget;
}

class Document;
class PreviewContext;

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
    
    void SetDocument(Document *newDocument);

private slots:
    // Zoom.
	void OnScaleByComboIndex(int value);
	void OnScaleByComboText();
	void OnZoomInRequested();
	void OnZoomOutRequested();
    
    void OnCanvasScaleChanged(int newScale);
    void OnGLWidgetResized(int width, int height);

    void OnVScrollbarMoved(int position);
    void OnHScrollbarMoved(int position);
    void OnScrollPositionChanged(const QPoint &newPosition);
    void OnScrollAreaChanged(const QSize &viewSize, const QSize &contentSize);

private:
    void OnScaleByZoom(int scaleDelta);

    QSize GetGLViewSize() const;

private:
    Ui::PackageGraphicsWidget *ui;
    Document *document;
    PreviewContext *context;
};

#endif // __QUICKED_PACKAGE_GRAPHICS_WIDGET_H__
