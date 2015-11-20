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

#include "GraphEditor.h"
#include "Metadata/GraphEditor.mpp"

#include "Action.h"
#include "NodeModel.h"
#include "ContextMenuModel.h"

#include <core_dependency_system/i_interface.hpp>
#include <core_reflection/i_definition_manager.hpp>

GraphEditor::GraphEditor()
{
    IDefinitionManager* defMng = Context::queryInterface<IDefinitionManager>();
    assert(defMng != nullptr);

    std::vector<ObjectHandle> nodes;
    {
        ObjectHandleT<GraphNode> node = defMng->create<GraphNode>();
        node->SetTitle("Real node from C++");
        nodes.push_back(ObjectHandle(node));
        nodeModel.reset(new NodeModel(std::move(nodes)));
    }

    {
        ObjectHandleT<GraphNode> node = defMng->create<GraphNode>();
        node->SetTitle("Second node");
        node->SetPosX(30.0);
        node->SetPosY(0.0);
        nodes.push_back(ObjectHandle(node));
        nodeModel.reset(new NodeModel(std::move(nodes)));
    }

    std::vector<ObjectHandle> actions;
    for (size_t i = 0; i < 3; ++i)
    {
        ObjectHandleT<Action> action = defMng->create<Action>();
        action->SetParams("Action : " + std::to_string(i), std::bind(&GraphEditor::OnActionTriggered, this));
        actions.emplace_back(action);
    }

    contextMenuModel.reset(new ContextMenuModel(std::move(actions)));
}

void GraphEditor::SizeChanged(int width, int height)
{
    if (width == 0 || height == 0)
        return;

    ScreenTransform::Instance().Resize(QSizeF(width, height));
    ApplyTransform();
}

IListModel* GraphEditor::GetContextMenuModel() const
{
    return static_cast<IListModel*>(contextMenuModel.get());
}

IListModel* GraphEditor::GetNodeModel() const
{
    return static_cast<IListModel*>(nodeModel.get());
}

void GraphEditor::Scale(float factor, float x, float y)
{
    ScreenTransform::Instance().Scale(factor, x, y);
    ApplyTransform();
}

void GraphEditor::Shift(float x, float y)
{
    ScreenTransform::Instance().Shift(QPointF(x, y));
    ApplyTransform();
}

void GraphEditor::OnActionTriggered()
{
}

void GraphEditor::ApplyTransform()
{
    NodeModel* model = static_cast<NodeModel*>(nodeModel.get());
    model->ApplyTransform();
}
