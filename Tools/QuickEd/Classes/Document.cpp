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
#include <QLineEdit>

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/PackageHierarchy/ControlNode.h"

#include "Model/ControlProperties/RootProperty.h"
#include "Model/ControlProperties/SectionProperty.h"
#include "Model/ControlProperties/ValueProperty.h"
#include "Model/ControlProperties/LocalizedTextValueProperty.h"
#include "Model/ControlProperties/FontValueProperty.h"

#include "Ui/QtModelPackageCommandExecutor.h"
#include "EditorCore.h"

#include "Systems/SelectionSystem.h"
#include "Systems/CanvasSystem.h"

using namespace DAVA;

Document::Document(PackageNode *_package, QObject *parent)
    : QObject(parent)
    , package(SafeRetain(_package))
    , commandExecutor(new QtModelPackageCommandExecutor(this))
    , undoStack(new QUndoStack(this))
    , selectionSystem(new SelectionSystem())
    , canvasSystem(new CanvasSystem(this))
{
    selectionSystem->AddListener(this);
    selectionSystem->AddListener(canvasSystem);
    connect(GetEditorFontSystem(), &EditorFontSystem::UpdateFontPreset, this, &Document::RefreshAllControlProperties);
}

Document::~Document()
{
    SafeRelease(package);
    SafeRelease(commandExecutor);
    delete selectionSystem;
    delete canvasSystem;
}

void Document::Detach()
{
    emit SelectedNodesChanged(SelectedNodes(), selectedNodes);
}

void Document::Attach()
{
    emit SelectedNodesChanged(selectedNodes, SelectedNodes());
}

const DAVA::FilePath &Document::GetPackageFilePath() const
{
    return package->GetPath();
}

CanvasSystem* Document::GetCanvasSystem() const
{
    return canvasSystem;
}


void Document::RefreshLayout()
{
    package->RefreshLayout();
}

WidgetContext* Document::GetContext(QObject* requester) const
{
    return contexts.value(requester, nullptr);
}

void Document::SetContext(QObject* requester, WidgetContext* widgetContext)
{
    if(contexts.contains(requester))
    {
        DVASSERT_MSG(false, "document already have this context");
        delete contexts.take(requester);
    }
    contexts.insert(requester, widgetContext);
}

void Document::SelectionWasChanged(const SelectedControls &selected, const SelectedControls &deselected)
{
    auto tmpSelected = selectedNodes;
    SelectedNodes selected_(selected.begin(), selected.end());
    SelectedNodes deselected_(deselected.begin(), deselected.end());
    UniteNodes(selected_, tmpSelected);
    SubstractNodes(deselected_, tmpSelected);
    if (tmpSelected != selectedNodes)
    {
        emit SelectedNodesChanged(selected_, deselected_);
    }
}

void Document::RefreshAllControlProperties()
{
    package->GetPackageControlsNode()->RefreshControlProperties();
}

void Document::OnSelectedNodesChanged(const SelectedNodes &selected, const SelectedNodes &deselected)
{
    auto tmpSelected = selectedNodes;
    UniteNodes(selected, tmpSelected);
    SubstractNodes(deselected, tmpSelected);
    SelectedControls selectedControls;
    SelectedControls deselectedControls;
    for (auto node : selected)
    {
        ControlNode *controlNode = dynamic_cast<ControlNode*>(node);
        if (nullptr != controlNode)
        {
            selectedControls.insert(controlNode);
        }
    }
    for (auto node : deselected)
    {
        ControlNode *controlNode = dynamic_cast<ControlNode*>(node);
        if (nullptr != controlNode)
        {
            deselectedControls.insert(controlNode);
        }
    }
    selectionSystem->SelectionWasChanged(selectedControls, deselectedControls);
    if (tmpSelected != selectedNodes)
    {
        emit SelectedNodesChanged(selected, deselected);
    }
}

