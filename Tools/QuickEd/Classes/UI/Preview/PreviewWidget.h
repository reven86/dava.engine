#ifndef __QUICKED_PREVIEW_WIDGET_H__
#define __QUICKED_PREVIEW_WIDGET_H__

#include <QWidget>
#include <QPointer>

namespace Ui {
    class PreviewWidget;
}

class Document;
class PreviewContext;

enum ScreenId
{
    UNKNOWN_SCREEN = -1,
    EDIT_SCREEN = 0,
};

class PreviewWidget : public QWidget
{
    Q_OBJECT
public:
    PreviewWidget(QWidget *parent = nullptr);
    virtual ~PreviewWidget();
    
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
    Ui::PreviewWidget *ui;
    Document *document;
    QPointer<PreviewContext> context;
};

#endif // __QUICKED_PREVIEW_WIDGET_H__
