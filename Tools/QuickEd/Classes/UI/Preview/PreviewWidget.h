#ifndef __QUICKED_PREVIEW_WIDGET_H__
#define __QUICKED_PREVIEW_WIDGET_H__

#include <QWidget>
#include <QPointer>
#include "ui_PreviewWidget.h"
#include "Result.h"

namespace Ui {
    class PreviewWidget;
}

class SharedData;
class PreviewModel;
class DavaGLWidget;
class ControlNode;

enum ScreenId
{
    UNKNOWN_SCREEN = -1,
    EDIT_SCREEN = 0,
};

class PreviewWidget : public QWidget, public Ui::PreviewWidget
{
    Q_OBJECT
public:
    explicit PreviewWidget(QWidget *parent = nullptr);
    ~PreviewWidget() = default;
    DavaGLWidget *GetDavaGLWidget();
public slots:
    void OnDocumentChanged(SharedData *context);
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

    void OnControlNodeSelected(QList<ControlNode*> selectedNodes);

    void OnError(const Result &result);

private:
    void OnScaleByZoom(int scaleDelta); 
    void UpdateActiveRootControls();

private:
    DavaGLWidget *davaGLWidget;
    SharedData *sharedData;
    PreviewModel *model;
};

#endif // __QUICKED_PREVIEW_WIDGET_H__
