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


#include "PreviewModel.h"

#include "UI/Preview/EditScreen.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Document.h"
#include "Base/Result.h"

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
    , emulationMode(false)
{
    view = new DAVA::UIControl();
    view->SetName("PreviewModel_View");

    canvas = new PackageCanvas();
    canvas->SetName("PreviewModel_PackageCanvas");

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
    ResultList resultList;
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
            if (selectedControl != rootControl)
            {
                UIControl *c = selectedControl;
                while (nullptr != c && c != rootControl)
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
                resultList.AddResult(Result::RESULT_FAILURE, ("selected control is equal to the current root control"));
            }
        }
        else
        {
            resultList.AddResult(Result::RESULT_FAILURE, ("rootControl not found!"));
        }
    }
    if (!selectedNodes.isEmpty())
    {
        emit ControlNodeSelected(selectedNodes);
    }
    if (!resultList)
    {
        emit ErrorOccurred(resultList);
        for (const auto &result : resultList.GetResults())
        {
            if (!result.message.empty())
            {
                switch (result.type)
                {
                case Result::RESULT_SUCCESS: Logger::FrameworkDebug("%s", result.message.c_str()); break;
                case Result::RESULT_FAILURE: Logger::Warning("%s", result.message.c_str());  break;
                case Result::RESULT_ERROR: Logger::Error("%s", result.message.c_str()); break;
                }
            }
        }
    }
}

void PreviewModel::SetRootControls(const QList<ControlNode*> &activatedControls)
{
    rootNodes.clear();
    canvas->RemoveAllControls();

    foreach(ControlNode *controlNode, activatedControls)
    {
        rootNodes[controlNode->GetControl()] = controlNode;

        ScopedPtr<CheckeredCanvas> checkeredCanvas(new CheckeredCanvas());
        checkeredCanvas->SetEmulationMode(emulationMode);
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

void PreviewModel::SetSelectedControls(const QList<ControlNode *> &selectedControls)
{
    for (auto &rootNode : rootNodes)
    {
        DynamicTypeCheck<CheckeredCanvas*>(rootNode.first->GetParent())->ClearSelections();
    }
    
    for (ControlNode *node : selectedControls)
    {
        UIControl *control = node->GetControl();
        CheckeredCanvas *rootContainer = FindControlContainer(control);
        if (rootContainer)
            rootContainer->SelectControl(control);
    }

}

void PreviewModel::SetEmulationMode(bool aEmulationMode)
{
    emulationMode = aEmulationMode;
    for (UIControl *control : canvas->GetChildren())
    {
        CheckeredCanvas *canvas = dynamic_cast<CheckeredCanvas*>(control);
        if (canvas)
        {
            canvas->SetEmulationMode(emulationMode);
        }
        else
        {
            DVASSERT(false);
        }
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
