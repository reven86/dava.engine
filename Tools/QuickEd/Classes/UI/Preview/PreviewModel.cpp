#include "PreviewModel.h"

#include "UI/Preview/EditScreen.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Document.h"
#include "Result.h"

using namespace DAVA;

static const Vector2 CANVAS_BORDER_SIZE(100.0f, 100.0f);

float32 CalcCanvasAxisPosition(float32 oldViewSize, float32 newViewSize,
    float32 oldCanvasScaledSize, float32 newCanvasScaledSize,
    float32 scalePoint, float32 oldCanvasPosition)
{
    float32 newCanvasPosition;

    if (newViewSize != oldViewSize ||
        newCanvasScaledSize != oldCanvasScaledSize)
    {
        if (newCanvasScaledSize < newViewSize)
        {
            newCanvasPosition = (newViewSize - newCanvasScaledSize) / 2.0f;
        }
        else if (oldViewSize == 0.0f || oldCanvasScaledSize == 0.0f)
        {
            newCanvasPosition = (newViewSize - newCanvasScaledSize) / 2.0f;
        }
        else// if(newCanvasScaledSize >= newViewSize)
        {
            newCanvasPosition = oldCanvasPosition + (newViewSize - oldViewSize) * scalePoint + (oldCanvasScaledSize - newCanvasScaledSize) * scalePoint;
            newCanvasPosition = Clamp(newCanvasPosition, newViewSize - newCanvasScaledSize, 0.0f);
        }
    }
    else
    {
        newCanvasPosition = oldCanvasPosition;
    }

    return newCanvasPosition;
}

Vector2 CalcCanvasPosition(const Vector2 &oldViewSize, const Vector2 &newViewSize,
    const Vector2 &oldCanvasScaledSize, const Vector2 &newCanvasScaledSize,
    const Vector2 &scalePoint, const Vector2 &oldCanvasPosition)
{
    Vector2 newCanvasPosition;

    newCanvasPosition.x = CalcCanvasAxisPosition(oldViewSize.x, newViewSize.x, oldCanvasScaledSize.x, newCanvasScaledSize.x, scalePoint.x, oldCanvasPosition.x);
    newCanvasPosition.y = CalcCanvasAxisPosition(oldViewSize.y, newViewSize.y, oldCanvasScaledSize.y, newCanvasScaledSize.y, scalePoint.y, oldCanvasPosition.y);

    return newCanvasPosition;
}

Vector2 CalcScaledCanvasSize(const DAVA::Vector2 &canvasSize, const DAVA::Vector2 &canvasScale, const DAVA::Vector2 &viewSize)
{
    Vector2 newCanvasScaledSize = canvasSize * canvasScale + CANVAS_BORDER_SIZE;
    return newCanvasScaledSize;
}

PreviewModel::PreviewModel(QObject *parent)
    : QObject(parent)
    , canvas(nullptr)
    , view(nullptr)
{
    view = new DAVA::UIControl();
    canvas = new PackageCanvas();
    view->AddControl(canvas);
}

PreviewModel::~PreviewModel()
{
    SafeRelease(view);
    SafeRelease(canvas);
}

void PreviewModel::SetViewControlSize(const QSize &qtSize)
{
    Vector2 oldSize = view->GetSize();
    Vector2 newSize(qtSize.width(), qtSize.height());
    view->SetSize(newSize);

    Vector2 oldCanvasScaledSize = CalcScaledCanvasSize(canvas->GetSize(), canvas->GetScale(), oldSize);
    Vector2 newCanvasScaledSize = CalcScaledCanvasSize(canvas->GetSize(), canvas->GetScale(), newSize);

    if (oldSize != newSize ||
        oldCanvasScaledSize != newCanvasScaledSize)
    {
        QSize viewSize(newSize.x, newSize.y);
        QSize contentSize(newCanvasScaledSize.x, newCanvasScaledSize.y);
        emit CanvasOrViewChanged(viewSize, contentSize);
    }

    Vector2 newPosition = CalcCanvasPosition(oldSize, newSize, oldCanvasScaledSize, newCanvasScaledSize, Vector2(0.5f, 0.5f), canvasPosition);
    SetCanvasPosition(QPoint(newPosition.x, newPosition.y));
}

void PreviewModel::SetCanvasControlSize(const QSize &qtSize)
{
    Vector2 oldSize = canvas->GetSize();
    Vector2 newSize(qtSize.width(), qtSize.height());
    canvas->SetSize(newSize);

    Vector2 oldCanvasScaledSize = CalcScaledCanvasSize(oldSize, canvas->GetScale(), view->GetSize());
    Vector2 newCanvasScaledSize = CalcScaledCanvasSize(newSize, canvas->GetScale(), view->GetSize());

    if (oldCanvasScaledSize != newCanvasScaledSize)
    {
        QSize viewSize(view->GetSize().x, view->GetSize().y);
        QSize contentSize(newCanvasScaledSize.x, newCanvasScaledSize.y);
        emit CanvasOrViewChanged(viewSize, contentSize);
    }

    Vector2 newPosition = CalcCanvasPosition(view->GetSize(), view->GetSize(), oldCanvasScaledSize, newCanvasScaledSize, Vector2(0.5f, 0.5f), canvasPosition);
    SetCanvasPosition(QPoint(newPosition.x, newPosition.y));
}

void PreviewModel::SetCanvasControlScale(int intNewScale)
{
    Vector2 oldScale = canvas->GetScale();
    Vector2 newScale(float32(intNewScale) / 100.0f, float32(intNewScale) / 100.0f);
    canvas->SetScale(newScale);
    int intOldScale = oldScale.x*100.0f;

    Vector2 oldCanvasScaledSize = CalcScaledCanvasSize(canvas->GetSize(), oldScale, view->GetSize());
    Vector2 newCanvasScaledSize = CalcScaledCanvasSize(canvas->GetSize(), newScale, view->GetSize());

    QSize viewSize(view->GetSize().x, view->GetSize().y);
    QSize contentSize(newCanvasScaledSize.x, newCanvasScaledSize.y);
    emit CanvasOrViewChanged(viewSize, contentSize);
    emit CanvasScaleChanged(intNewScale);

    Vector2 newPosition = CalcCanvasPosition(view->GetSize(), view->GetSize(), oldCanvasScaledSize, newCanvasScaledSize, Vector2(0.5f, 0.5f), canvasPosition);
    SetCanvasPosition(QPoint(newPosition.x, newPosition.y));
}

QSize PreviewModel::GetScaledCanvasSize() const
{
    Vector2 size = CalcScaledCanvasSize(canvas->GetSize(), canvas->GetScale(), view->GetSize());
    return QSize(size.x, size.y);
}

QSize PreviewModel::GetViewSize() const
{
    return QSize(view->GetSize().x, view->GetSize().y);
}

void PreviewModel::SetCanvasPosition(const QPoint &newCanvasPosition)
{
    QPoint oldCanvasPosition = QPoint(canvasPosition.x, canvasPosition.y);
    canvasPosition = Vector2(newCanvasPosition.x(), newCanvasPosition.y());

    if (oldCanvasPosition != newCanvasPosition)
    {
        canvas->SetPosition(canvasPosition + CANVAS_BORDER_SIZE / 2.0f);
        emit CanvasPositionChanged(newCanvasPosition);
    }
}

void PreviewModel::OnControlSelected(const DAVA::List<std::pair<DAVA::UIControl *, DAVA::UIControl*> > &selectedPairs)
{
    Result result;
    QList<ControlNode*> selectedNodes;
    for (auto pair : selectedPairs)
    {
        DAVA::UIControl* rootControl = pair.first;
        DAVA::UIControl* selectedControl = pair.second;
        auto it = rootNodes.find(rootControl);
        if (it != rootNodes.end())
        {
            Vector<UIControl*> path;
            ControlNode *node = it->second;
            if (selectedControl == rootControl)
            {
                // ok
            }
            else
            {
                UIControl *c = selectedControl;
                while (c && c != rootControl)
                {
                    path.push_back(c);
                    c = c->GetParent();
                }


                for (auto it = path.rbegin(); it != path.rend(); ++it)
                {
                    bool found = false;
                    for (int32 index = 0; index < node->GetCount(); index++)
                    {
                        ControlNode *child = node->Get(index);
                        if (child->GetControl() == *it)
                        {
                            node = child;
                            found = true;
                            break;
                        }
                    }

                    if (!found)
                    {
                        node = nullptr;
                        break;
                    }
                }
            }

            if (node)
            {
                selectedNodes.push_back(node);
            }
            else
            {
                result.addError(Result::Warning, tr("Node not changed"));
            }
        }
        else
        {
            result.addError(Result::Warning, tr("rootControl not found!"));
        }
    }
    ControlNodeSelected(selectedNodes);
    if (result)
    {
        emit ErrorOccurred(result);
    }
}

void PreviewModel::SetActiveRootControls(const QList<ControlNode*> &activatedControls)
{
    rootNodes.clear();
    canvas->RemoveAllControls();

    foreach(ControlNode *controlNode, activatedControls)
    {
        rootNodes[controlNode->GetControl()] = controlNode;

        ScopedPtr<CheckeredCanvas> checkeredCanvas(new CheckeredCanvas());
        checkeredCanvas->AddControlSelectionListener(this);
        checkeredCanvas->AddControl(controlNode->GetControl());
        checkeredCanvas->SetSize(controlNode->GetControl()->GetSize());
        canvas->AddControl(checkeredCanvas);
    }

    Vector2 oldSize = canvas->GetSize();
    canvas->LayoutCanvas();
    Vector2 newSize = canvas->GetSize();

    Vector2 oldCanvasScaledSize = CalcScaledCanvasSize(oldSize, canvas->GetScale(), view->GetSize());
    Vector2 newCanvasScaledSize = CalcScaledCanvasSize(newSize, canvas->GetScale(), view->GetSize());

    if (oldCanvasScaledSize != newCanvasScaledSize)
    {
        QSize viewSize(view->GetSize().x, view->GetSize().y);
        QSize contentSize(newCanvasScaledSize.x, newCanvasScaledSize.y);
        emit CanvasOrViewChanged(viewSize, contentSize);
    }

    Vector2 newPosition = CalcCanvasPosition(view->GetSize(), view->GetSize(), oldCanvasScaledSize, newCanvasScaledSize, Vector2(0.5f, 0.5f), canvasPosition);
    SetCanvasPosition(QPoint(newPosition.x, newPosition.y));
}

void PreviewModel::ControlsDeactivated(const QList<ControlNode*> &deactivatedControls)
{
    for (ControlNode *node : deactivatedControls)
    {
        UIControl *control = node->GetControl();
        CheckeredCanvas *rootContainer = FindControlContainer(control);
        if (rootContainer)
            rootContainer->RemoveSelection(control);
    }
}

void PreviewModel::ControlsActivated(const QList<ControlNode *> &activatedControls)
{
    for (ControlNode *node : activatedControls)
    {
        UIControl *control = node->GetControl();
        CheckeredCanvas *rootContainer = FindControlContainer(control);
        if (rootContainer)
            rootContainer->SelectControl(control);
    }
}

QPoint PreviewModel::GetCanvasPosition() const
{
    return QPoint(canvasPosition.x, canvasPosition.y);
}

int PreviewModel::GetCanvasScale() const
{
    return canvas->GetScale().x*100.0f;
}

CheckeredCanvas *PreviewModel::FindControlContainer(UIControl *control)
{
    UIControl *c = control;

    while (nullptr != c->GetParent() && c->GetParent() != canvas)
        c = c->GetParent();

    if (nullptr != c)
        return dynamic_cast<CheckeredCanvas*>(c);

    return nullptr;
}
