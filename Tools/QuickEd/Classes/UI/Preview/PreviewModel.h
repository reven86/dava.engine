#ifndef __QUICKED_PREVIEW_MODEL_H__
#define __QUICKED_PREVIEW_MODEL_H__

#include <QObject>
#include <QPoint>
#include <QSize>
#include "DAVAEngine.h"

#include "ControlSelectionListener.h"


class PackageCanvas;
class ControlNode;
class CheckeredCanvas;


class PreviewModel: public QObject, ControlSelectionListener
{
    Q_OBJECT
public:
    PreviewModel(QObject *parent = nullptr);
    virtual ~PreviewModel();
    void SetData(DAVA::UIControl *view, PackageCanvas *canvas);
    bool IsValid() const;
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
signals:
    void CanvasPositionChanged(const QPoint &canvasPosition);
    void CanvasOrViewChanged(const QSize &viewSize, const QSize &scaledContentSize);
    void CanvasScaleChanged(int canvasScale);
    
    void ControlNodeSelected(ControlNode *node);
    void AllControlsDeselected();

    void ErrorOccurred(QString errorText);

public: // ControlSelectionListener
    virtual void OnControlSelected(DAVA::UIControl *rootControl, DAVA::UIControl *selectedControl) override;
    virtual void OnAllControlsDeselected() override;
    void ControlsDeactivated(const QList<ControlNode*> &deactivatedControls);
    void ControlsActivated(const QList<ControlNode *> &activatedControls);
    
private:
    CheckeredCanvas *FindControlContainer(DAVA::UIControl *control);
    
private:
    DAVA::Vector2 canvasPosition;

    PackageCanvas *canvas;
    DAVA::UIControl *view;

    DAVA::Map<DAVA::UIControl*, ControlNode*> rootNodes;
};

inline bool PreviewModel::IsValid() const
{
    return nullptr != canvas && nullptr != view;
}

inline DAVA::UIControl *PreviewModel::GetViewControl() const
{
    return view;
}


#endif // __QUICKED_PREVIEW_MODEL_H__
