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
#include "ScreenTransform.h"

#include "Metadata/GraphNode.mpp"

#include <core_dependency_system/i_interface.hpp>
#include <core_reflection/i_definition_manager.hpp>
#include <core_reflection/property_accessor.hpp>

#include <QDebug>

PropertyAccessor bindProperty(GraphNode* node, const char* propertyName)
{
    IDefinitionManager* defMng = Context::queryInterface<IDefinitionManager>();
    assert(defMng != nullptr);

    IClassDefinition* definition = defMng->getDefinition<GraphNode>();
    assert(definition != nullptr);

    return definition->bindProperty(propertyName, node);
}

std::string const& GraphNode::GetTitle() const
{
    return title;
}

void GraphNode::SetTitle(std::string const& title_)
{
    title = title_;
}

float GraphNode::GetPosX() const
{
    return pixelX;
}

void GraphNode::SetPosX(const float& x)
{
    bindProperty(this, "nodePosX").setValue(x);
}

float GraphNode::GetPosY() const
{
    return pixelY;
}

void GraphNode::SetPosY(const float& y)
{
    bindProperty(this, "nodePosY").setValue(y);
}

float GraphNode::GetScale() const
{
    return scale;
}

void GraphNode::SetScale(const float& scale)
{
    bindProperty(this, "nodeScale").setValue(scale);
}

void GraphNode::Shift(float pixelShiftX, float pixelShiftY)
{
    ScreenTransform& transform = ScreenTransform::Instance();
    QPointF zeroPoint = transform.PtoG(QPointF(0.0f, 0.0f));
    QPointF shiftPoint = transform.PtoG(QPointF(pixelShiftX, pixelShiftY));
    QPointF globalShift = shiftPoint - zeroPoint;

    modelX += globalShift.x();
    modelY += globalShift.y();

    ApplyTransform();
}

void GraphNode::ApplyTransform()
{
    ScreenTransform& transform = ScreenTransform::Instance();
    QPointF pt = transform.GtoP(QPointF(modelX, modelY));
    SetPosX(pt.x());
    SetPosY(pt.y());
    SetScale(transform.GetScale());
}

void GraphNode::SetPosXImpl(const float& x)
{
    pixelX = x;
}

void GraphNode::SetPosYImpl(const float& y)
{
    pixelY = y;
}

void GraphNode::SetScaleImpl(const float& scale_)
{
    scale = scale_;
}
