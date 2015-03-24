#ifndef __QUICKED_PREVIEW_WIDGET_H__
#define __QUICKED_PREVIEW_WIDGET_H__

#include <QWidget>
#include <QPointer>

namespace Ui {
    class PreviewWidget;
}

class WidgetContext;
class DavaGLWidget;

enum ScreenId
{
    UNKNOWN_SCREEN = -1,
    EDIT_SCREEN = 0,
};

class PreviewWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PreviewWidget(QWidget *parent = nullptr);
    DavaGLWidget* GetGLWidget() const;
    virtual ~PreviewWidget();
public slots:
    void OnContextChanged(WidgetContext *context);
    void OnDataChanged(const QByteArray &role);
private slots:
    // Zoom.
	void OnScaleByComboIndex(int value);
	void OnScaleByComboText();
	void OnZoomInRequested();
	void OnZoomOutRequested();
    
    void OnCanvasScaleChanged(int newScale);
    void OnGLWidgetResized(int width, int height, int dpr);

    void OnVScrollbarMoved(int position);
    void OnHScrollbarMoved(int position);
    void OnScrollPositionChanged(const QPoint &newPosition);
    void OnScrollAreaChanged(const QSize &viewSize, const QSize &contentSize);
    
    void OnMonitorChanged();

private:
    void OnScaleByZoom(int scaleDelta);

    QSize GetGLViewSize() const;
    void setScale(int scale);

private:
    Ui::PreviewWidget *ui;
    WidgetContext *widgetContext;
};

#endif // __QUICKED_PREVIEW_WIDGET_H__
