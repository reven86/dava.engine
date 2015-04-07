#ifndef __QUICKED_PREVIEW_MODEL_H__
#define __QUICKED_PREVIEW_MODEL_H__

#include <QObject>
#include <QPoint>
#include <QSize>
#include "DAVAEngine.h"

#include "Result.h"
#include "ControlSelectionListener.h"

class PackageCanvas;
class ControlNode;
class CheckeredCanvas;

class PreviewModel : public QObject, ControlSelectionListener
{
    Q_OBJECT
public:
    PreviewModel(QObject *parent);
    virtual ~PreviewModel();

    void SetViewControlSize(const QSize &newSize);
    void SetCanvasControlSize(const QSize &newSize);
    void SetCanvasControlScale(int newScale);

    void SetCanvasPosition(const QPoint &newPosition);
    QPoint GetCanvasPosition() const;
    int GetCanvasScale() const;

    QSize GetScaledCanvasSize() const;
    QSize GetViewSize() const;
    DAVA::UIControl *GetViewControl() const;

    void SetActiveRootControls(const QList<ControlNode*> &activatedControls);
    void ControlsDeactivated(const QList<ControlNode*> &deactivatedControls);
    void ControlsActivated(const QList<ControlNode *> &activatedControls);

    // ControlSelectionListener
    virtual void OnControlSelected(const DAVA::List<std::pair<DAVA::UIControl *, DAVA::UIControl*> > &selectedPairs) override;
signals:
    void CanvasPositionChanged(const QPoint &canvasPosition);
    void CanvasOrViewChanged(const QSize &viewSize, const QSize &scaledContentSize);
    void CanvasScaleChanged(int canvasScale);

    void ControlNodeSelected(const QList<ControlNode *> &selectedNodes);

    void ErrorOccurred(const Result &error);

private:
    CheckeredCanvas *FindControlContainer(DAVA::UIControl *control);

private:
    DAVA::Vector2 canvasPosition;

    PackageCanvas *canvas;
    DAVA::UIControl *view;

    DAVA::Map<DAVA::UIControl*, ControlNode*> rootNodes;
};

inline DAVA::UIControl *PreviewModel::GetViewControl() const 
{
    return view;
}


#endif // __QUICKED_PREVIEW_MODEL_H__
