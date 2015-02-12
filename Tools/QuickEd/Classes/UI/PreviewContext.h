#ifndef __GRAPHICSVIEWCONTEXT_H__
#define __GRAPHICSVIEWCONTEXT_H__

#include <QObject>
#include <QPoint>
#include <QSize>
#include "DAVAEngine.h"

#include "Preview/ControlSelectionListener.h"

class PackageCanvas;
class ControlNode;
class CheckeredCanvas;

class Document;

class PreviewContext: public QObject, ControlSelectionListener
{
    Q_OBJECT
public:
    PreviewContext(Document *document);
    virtual ~PreviewContext();

    void SetViewControlSize(const QSize &newSize);
    void SetCanvasControlSize(const QSize &newSize);
    void SetCanvasControlScale(int newScale);

    void SetCanvasPosition(const QPoint &newPosition);
    QPoint GetCanvasPosition() const;
    int GetCanvasScale() const;

    QSize GetScaledCanvasSize() const;
    QSize GetViewSize() const;

    inline DAVA::UIControl *GetViewControl() const { return view; }
signals:
    void CanvasPositionChanged(const QPoint &canvasPosition);
    void CanvasOrViewChanged(const QSize &viewSize, const QSize &scaledContentSize);
    void CanvasScaleChanged(int canvasScale);
    
    void ControlNodeSelected(ControlNode *node);
    void AllControlsDeselected();

public: // ControlSelectionListener
    virtual void OnControlSelected(DAVA::UIControl *rootControl, DAVA::UIControl *selectedControl);
    virtual void OnAllControlsDeselected();
    
public slots:
    void OnActiveRootControlsChanged(const QList<ControlNode *> &activatedControls, const QList<ControlNode*> &deactivatedControls);
    void OnSelectedControlsChanged(const QList<ControlNode *> &activatedControls, const QList<ControlNode*> &deactivatedControls);

private:
    CheckeredCanvas *FindControlContainer(DAVA::UIControl *control);
    
private:
    DAVA::Vector2 canvasPosition;

    PackageCanvas *canvas;
    DAVA::UIControl *view;

    DAVA::Map<DAVA::UIControl*, ControlNode*> rootNodes;
};


#endif // __GRAPHICSVIEWCONTEXT_H__
