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

#include "GraphNode.h"
#include "ConnectionSlot.h"
#include "BaseModel.h"

#include "Metadata/GraphNode.mpp"

#include <core_dependency_system/i_interface.hpp>
#include <core_reflection/i_definition_manager.hpp>
#include <core_reflection/property_accessor.hpp>

class ConnectionSlotsModel : public BaseModel<ConnectionSlot>
{
    using TBase = BaseModel<ConnectionSlot>;

public:
    ConnectionSlotsModel(std::vector<ObjectHandleT<ConnectionSlot>>&& objects)
        : TBase(std::move(objects))
    {
    }
};

template <typename T>
void setProperty(GraphNode* node, const char* propertyName, const T& value)
{
    IDefinitionManager* defMng = Context::queryInterface<IDefinitionManager>();
    assert(defMng != nullptr);

    IClassDefinition* definition = defMng->getDefinition<GraphNode>();
    assert(definition != nullptr);

    definition->bindProperty(propertyName, node).setValue(value);
}

std::string const& GraphNode::GetTitle() const
{
    return title;
}

void GraphNode::SetTitle(std::string const& title_)
{
    title = title_;
}

size_t GraphNode::GetUID() const
{
    return reinterpret_cast<size_t>(this);
}

GraphNode::GraphNode()
{
}

GraphNode::~GraphNode()
{
}

void GraphNode::Init(Params&& params)
{
    inputSlots.reset(new ConnectionSlotsModel(std::move(params.inputSlots)));
    outputSlots.reset(new ConnectionSlotsModel(std::move(params.outputSlots)));
    typeId = std::move(params.typeId);
}

float GraphNode::GetPosX() const
{
    return modelX;
}

void GraphNode::SetPosX(float x)
{
    modelX = x;
    setProperty(this, "nodePosX", modelY);
}

float GraphNode::GetPosY() const
{
    return modelY;
}

void GraphNode::SetPosY(float y)
{
    modelY = y;
    setProperty(this, "nodePosY", modelY);
}

void GraphNode::Shift(float modelShiftX, float modelShiftY)
{
    //MoveNodes(modelShiftX, modelShiftY);
    ShiftImpl(modelShiftX, modelShiftY);
}

void GraphNode::ShiftImpl(float modelShiftX, float modelShiftY)
{
    SetPosX(GetPosX() + modelShiftX);
    SetPosY(GetPosY() + modelShiftY);

    Changed(this);
}

IListModel* GraphNode::GetInputSlots() const
{
    return inputSlots.get();
}

IListModel* GraphNode::GetOutputSlots() const
{
    return outputSlots.get();
}

void GraphNode::PosXChanged(const float& x)
{

}

void GraphNode::PosYChanged(const float& y)
{

}
