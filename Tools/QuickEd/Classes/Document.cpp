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
#include "EditorSystems/EditorSystemsManager.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/ControlProperties/RootProperty.h"

#include "Ui/QtModelPackageCommandExecutor.h"
#include "EditorCore.h"

using namespace DAVA;

Document::Document(PackageNode* _package, QObject* parent)
    : QObject(parent)
    , package(SafeRetain(_package))
    , commandExecutor(new QtModelPackageCommandExecutor(this))
    , undoStack(new QUndoStack(this))
    , systemManager(_package)
{
    systemManager.SelectionChanged.Connect(this, &Document::OnSelectedControlNodesChanged);
    systemManager.CanvasSizeChanged.Connect(this, &Document::CanvasSizeChanged);

    systemManager.PropertiesChanged.Connect([this](const Vector<std::tuple<ControlNode*, AbstractProperty*, VariantType>>& properties, size_t hash) {
        commandExecutor->ChangeProperty(properties, hash);
    });

    connect(GetEditorFontSystem(), &EditorFontSystem::UpdateFontPreset, this, &Document::RefreshAllControlProperties);
}

Document::~Document()
{
    for (auto& context : contexts)
    {
        delete context.second;
    }
    SafeRelease(package);
}

EditorSystemsManager* Document::GetSystemManager()
{
    return &systemManager;
}

const FilePath& Document::GetPackageFilePath() const
{
    return package->GetPath();
}

QUndoStack* Document::GetUndoStack()
{
    return undoStack;
}

PackageNode* Document::GetPackage()
{
    return package;
}

QtModelPackageCommandExecutor* Document::GetCommandExecutor()
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
}

void Document::Deactivate()
{
    systemManager.Deactivate();
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

void Document::SetScale(float scale)
{
    DAVA::float32 realScale = scale / 100.0f;
    systemManager.GetScalableControl()->SetScale(Vector2(realScale, realScale));
    emit CanvasSizeChanged();
}

void Document::SetEmulationMode(bool arg)
{
    systemManager.SetEmulationMode(arg);
}

void Document::SetPixelization(bool hasPixelization)
{
    Texture::SetPixelization(hasPixelization);
}

void Document::SetDPR(double arg)
{
    systemManager.DPRChanged.Emit(std::move(arg));
}

void Document::RefreshAllControlProperties()
{
    package->GetPackageControlsNode()->RefreshControlProperties();
}

void Document::OnSelectionChanged(const SelectedNodes& selected, const SelectedNodes& deselected)
{
    SelectedNodes reallySelected, reallyDeselected;
    selectionContainer.GetOnlyExistedItems(deselected, reallyDeselected);
    selectionContainer.GetNotExistedItems(selected, reallySelected);
    selectionContainer.MergeSelection(selected, deselected);
    if (!reallySelected.empty() || !reallyDeselected.empty())
    {
        systemManager.SelectionChanged.Emit(selected, deselected);
    }
}

void Document::OnSelectedControlNodesChanged(const SelectedNodes& selected, const SelectedNodes& deselected)
{
    selectionContainer.MergeSelection(selected, deselected);
    SelectedNodesChanged(selected, deselected);
}
