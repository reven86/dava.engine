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
#include "Model/PackageHierarchy/ImportedPackagesNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/ControlProperties/RootProperty.h"
#include "Model/ControlProperties/SectionProperty.h"

#include "Ui/QtModelPackageCommandExecutor.h"
#include "EditorCore.h"

Document::Document(PackageNode *_package, QObject *parent)
    : QObject(parent)
    , package(SafeRetain(_package))
    , commandExecutor(new QtModelPackageCommandExecutor(this))
    , undoStack(new QUndoStack(this))
    , selectionSystem(this)
    , canvasSystem(this)
    , hudSystem(this)
    , treeSystem(this)
    , cursorSystem(this)
    , transformSystem(this)
{
    inputListeners << &selectionSystem << &hudSystem << &treeSystem << &transformSystem;
    selectionSystem.AddListener(this);
    selectionSystem.AddListener(&canvasSystem);
    selectionSystem.AddListener(&hudSystem);
    selectionSystem.AddListener(&transformSystem);
    hudSystem.AddListener(&cursorSystem);
    hudSystem.AddListener(&transformSystem);
    connect(GetEditorFontSystem(), &EditorFontSystem::UpdateFontPreset, this, &Document::RefreshAllControlProperties);
}

Document::~Document()
{
    for (auto context : contexts)
    {
        delete context;
    }
}

using namespace DAVA;

void Document::Detach()
{
    hudSystem.Detach();
    canvasSystem.Detach();
    emit SelectedNodesChanged(SelectedNodes(), selectedNodes);
}

void Document::Attach()
{
    emit SelectedNodesChanged(selectedNodes, SelectedNodes());
}

CanvasSystem* Document::GetCanvasSystem()
{
    return &canvasSystem;
}

HUDSystem* Document::GetHUDSystem()
{
    return &hudSystem;
}

const FilePath &Document::GetPackageFilePath() const
{
    return package->GetPath();
}

void Document::RefreshLayout()
{
    package->RefreshPackageStylesAndLayout(true);
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
    SelectedNodes selected_(selected.begin(), selected.end());
    SelectedNodes deselected_(deselected.begin(), deselected.end());
    SetSelectedNodes(selected_, deselected_);
}

bool Document::OnInput(UIEvent *currentInput)
{
    QListIterator<InputInterface*> it(inputListeners);
    it.toBack();
    while (it.hasPrevious())
    {
        if (it.previous()->OnInput(currentInput))
        {
            return true;
        }
    }
    return false;
}

ControlNode* Document::GetControlNodeByPos(const Vector2& pos, ControlNode* node) const
{
    if (node == nullptr)
    {
        auto controlsNode = package->GetPackageControlsNode();
        for (int index = 0; index < controlsNode->GetCount(); ++index)
        {
            auto tmpNode = controlsNode->Get(index);
            DVASSERT(nullptr != tmpNode);
            auto retVal = GetControlNodeByPos(pos, tmpNode);
            if (nullptr != retVal)
            {
                return retVal;
            }
        }
    }
    else
    {
        int count = node->GetCount();
        for (int i = 0; i < count; ++i)
        {
            auto retVal = GetControlNodeByPos(pos, node->Get(i));
            if (nullptr != retVal)
            {
                return retVal;
            }
        }
        auto control = node->GetControl();
        if (control->IsPointInside(pos) && control->GetVisible() && control->GetVisibleForUIEditor())
        {
            return node;
        }
    }
    return nullptr;
}

AbstractProperty* Document::GetPropertyByName(const ControlNode *node, const DAVA::String &name) const
{
    RootProperty *propertiesRoot = node->GetRootProperty();
    int propertiesCount = propertiesRoot->GetCount();
    for (int index = 0; index < propertiesCount; ++index)
    {
        SectionProperty *section = dynamic_cast<SectionProperty*>(propertiesRoot->GetProperty(index));
        if (section)
        {
            int sectionCount = section->GetCount();
            for (int prop = 0; prop < sectionCount; ++prop)
            {
                ValueProperty *valueProperty = dynamic_cast<ValueProperty*>(section->GetProperty(prop));
                if (nullptr != valueProperty && valueProperty->GetName() == name)
                {
                    return valueProperty;
                }
            }
        }
    }
    return nullptr;
}

void Document::RefreshAllControlProperties()
{
    package->GetPackageControlsNode()->RefreshControlProperties();
}

void Document::OnSelectedNodesChanged(const SelectedNodes &selected, const SelectedNodes &deselected)
{
    SetSelectedNodes(selected, deselected);
}

void Document::SetSelectedNodes(const SelectedNodes& selected, const SelectedNodes& deselected)
{
    if (selected.empty() && deselected.empty())
    {
        return;
    }
    auto tmpSelected = selectedNodes;
    UniteNodes(selected, tmpSelected);
    SubstractNodes(deselected, tmpSelected);
    if (selectedNodes != tmpSelected)
    {
        selectedNodes = tmpSelected;
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
        if (!selectedControls.empty() || !deselectedControls.empty())
        {
            selectionSystem.SelectionWasChanged(selectedControls, deselectedControls);
        }
        emit SelectedNodesChanged(selected, deselected);
    }
}
