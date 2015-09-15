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


#include "Document.h"
#include "Systems/SystemsManager.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/ControlProperties/RootProperty.h"

#include "Ui/QtModelPackageCommandExecutor.h"
#include "EditorCore.h"
#include "UI/IconHelper.h"
#include "UI/MainWindow.h"
#include "QtTools/DavaGLWidget/DavaGLWidget.h"

using namespace DAVA;

Document::Document(PackageNode *_package, QObject *parent)
    : QObject(parent)
    , package(SafeRetain(_package))
    , commandExecutor(new QtModelPackageCommandExecutor(this))
    , undoStack(new QUndoStack(this))
    , systemManager(_package)
{
    systemManager.SelectionChanged.Connect(this, &Document::OnSelectionChanged);
    systemManager.EmulationModeChangedSignal.Connect(this, &Document::EmulationModeChanged);
    systemManager.CanvasSizeChanged.Connect(this, &Document::CanvasSizeChanged);
    systemManager.SelectionByMenuRequested.Connect(this, &Document::OnSelectControlByMenu);
    systemManager.PropertyChanged.Connect([this](ControlNode *node, AbstractProperty *property, const DAVA::VariantType &value) {
        commandExecutor->ChangeProperty(node, property, value);
    });
    connect(GetEditorFontSystem(), &EditorFontSystem::UpdateFontPreset, this, &Document::RefreshAllControlProperties);
}

Document::~Document()
{
    for (auto &context : contexts)
    {
        delete context.second;
    }
}

int Document::GetScale() const
{
    return scale;
}

bool Document::IsInEmulationMode() const
{
    return systemManager.IsInEmulationMode();
}

SystemsManager* Document::GetSystemManager()
{
    return &systemManager;
}

const FilePath &Document::GetPackageFilePath() const
{
    return package->GetPath();
}

QUndoStack *Document::GetUndoStack()
{
    return undoStack;
}

PackageNode *Document::GetPackage()
{
    return package;
}

QtModelPackageCommandExecutor *Document::GetCommandExecutor()
{
    return commandExecutor;
}

WidgetContext* Document::GetContext(QObject* requester) const
{
    auto iter = contexts.find(requester);
    if (iter != contexts.end())
    {
        return iter->second;
    }
    return nullptr;
}

void Document::Activate()
{
    systemManager.Activate();
    emit SelectedNodesChanged(selectionTracker.selectedNodes, SelectedNodes());
    emit ScaleChanged(scale);
    emit EmulationModeChanged(IsInEmulationMode());
}


void Document::Deactivate()
{
    systemManager.Deactivate();
    emit SelectedNodesChanged(SelectedNodes(), selectionTracker.selectedNodes);
}

void Document::SetContext(QObject* requester, WidgetContext* widgetContext)
{
    auto iter = contexts.find(requester);
    if (iter != contexts.end())
    {
        DVASSERT_MSG(false, "document already have this context");
        delete iter->second;
        contexts.erase(iter);
    }
    contexts.insert(std::pair<QObject*, WidgetContext*>(requester, widgetContext));
}

void Document::RefreshLayout()
{
    package->RefreshPackageStylesAndLayout(true);
}

void Document::OnSelectControlByMenu(const Vector<ControlNode*>& nodesUnderPoint, const Vector2& point, ControlNode*& selectedNode)
{
    selectedNode = nullptr;
    auto view = EditorCore::Instance()->GetMainWindow()->GetGLWidget()->GetGLWindow();
    QPoint globalPos = view->mapToGlobal(QPoint(point.x, point.y)/ view->devicePixelRatio());
    QMenu menu;
    for (auto it = nodesUnderPoint.rbegin(); it != nodesUnderPoint.rend(); ++it)
    {
        ControlNode *controlNode = *it;
        QString className = QString::fromStdString(controlNode->GetControl()->GetClassName());
        QIcon icon(IconHelper::GetIconPathForClassName(className));
        QAction *action = new QAction(icon, QString::fromStdString(controlNode->GetName()), &menu);
        menu.addAction(action);
        void* ptr = static_cast<void*>(controlNode);
        action->setData(QVariant::fromValue(ptr));
        auto selectedNodes = selectionTracker.selectedNodes;
        if (selectedNodes.find(controlNode) != selectedNodes.end())
        {
            auto font = action->font();
            font.setBold(true);
            action->setFont(font);
        }
    }
    QAction *selectedAction = menu.exec(globalPos);
    if (nullptr != selectedAction)
    {
        void *ptr = selectedAction->data().value<void*>();
        selectedNode = static_cast<ControlNode*>(ptr);
    }
}

void Document::SetScale(int arg)
{
    if (scale != arg)
    {
        scale = arg;
        DAVA::float32 realScale = scale / 100.0f;
        systemManager.GetScalableControl()->SetScale(Vector2(realScale, realScale));
        emit ScaleChanged(scale);
        emit CanvasSizeChanged();
    }
}

void Document::ResetScale()
{
    SetScale(defaultScale);
}

void Document::SetEmulationMode(bool arg)
{
    if (IsInEmulationMode() != arg)
    {
        systemManager.SetEmulationMode(arg);
        emit EmulationModeChanged(IsInEmulationMode());
    }
}

void Document::RefreshAllControlProperties()
{
    package->GetPackageControlsNode()->RefreshControlProperties();
}

void Document::OnSelectionChanged(const SelectedNodes& selected, const SelectedNodes& deselected)
{
    SelectedNodes reallySelected;
    SelectedNodes reallyDeselected;

    selectionTracker.GetOnlyExistedItems(deselected, reallyDeselected);
    selectionTracker.GetNotExistedItems(selected, reallySelected);
    selectionTracker.MergeSelection(reallySelected, reallyDeselected);

    if (!reallySelected.empty() || !reallyDeselected.empty())
    {
        systemManager.SelectionChanged.Emit(reallySelected, reallyDeselected);
        emit SelectedNodesChanged(reallySelected, reallyDeselected);
    }
}
