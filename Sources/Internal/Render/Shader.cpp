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


#include "Render/Shader.h"
#include "Render/RHI/rhi_ShaderCache.h"
#include "Render/RenderBase.h"


namespace DAVA
{

struct BufferPropertyLayout
{
    rhi::ShaderPropList props;
};

bool operator == (const BufferPropertyLayout& lhs, const BufferPropertyLayout& rhs)
{
    if (lhs.props.size() != rhs.props.size())
        return false;
    for (size_t i = 0, sz = lhs.props.size(); i < sz; ++i)
    {
        if ((lhs.props[i].uid != rhs.props[i].uid) ||
            (lhs.props[i].type != rhs.props[i].type) ||
            (lhs.props[i].bufferReg != rhs.props[i].bufferReg) ||
            (lhs.props[i].defaultValue != rhs.props[i].defaultValue) ||   //should we really compare defaultValue HERE!?!?!?!?!?
            (lhs.props[i].bufferRegCount != rhs.props[i].bufferRegCount))
        {
            return false;
        }
    }

    return true;
}

namespace
{
    UniqueStateSet<BufferPropertyLayout> propertyLayoutSet;
}

uint32 ShaderDescriptor::CalculateRegsCount(rhi::ShaderProp::Type type, uint32 arraySize)
{
    switch (type)
    {
    case rhi::ShaderProp::TYPE_FLOAT1:
        DVASSERT(false); //for some reason float1 is not yet supported in rhi
        return 0;
        break;
    case rhi::ShaderProp::TYPE_FLOAT4:
        return arraySize; //1 float4 register per array element
        break;
    case rhi::ShaderProp::TYPE_FLOAT4X4:
        return arraySize * 4; //4 float4 register per array element
        break;
    default:
        DVASSERT(false); //how did we get here? unknown property type
        return 0;
        break;
    }
}

const rhi::ShaderPropList& ShaderDescriptor::GetProps(UniquePropertyLayout layout)
{
    return propertyLayoutSet.GetUnique(layout).props;
}

	
	
void ShaderDescriptor::UpdateDynamicParams()
{
    //Logger::Info( " upd-dyn-params" );
    for (auto& dynamicBinding : dynamicPropertyBindings)
    {
        float32 *data = (float32*)(Renderer::GetDynamicBindings().GetDynamicParam(dynamicBinding.dynamicPropertySemantic));
        pointer_size updateSemantic = Renderer::GetDynamicBindings().GetDynamicParamUpdateSemantic(dynamicBinding.dynamicPropertySemantic);
        if (dynamicBinding.updateSemantic != updateSemantic)
        {
            uint32 arraySize = 1; //for now 1, later move it to something similar to RenderManager::GetDynamicParamArraySize from instancing branch            
            rhi::UpdateConstBuffer4fv(dynamicBinding.buffer, dynamicBinding.reg, data, CalculateRegsCount(dynamicBinding.type, arraySize));
            dynamicBinding.updateSemantic = updateSemantic;
        }
    }
}

uint32 ShaderDescriptor::GetVertexConstBuffersCount()
{
    return vertexConstBuffersCount;
}
uint32 ShaderDescriptor::GetFragmentConstBuffersCount()
{
    return fragmentConstBuffersCount;
}

rhi::HConstBuffer ShaderDescriptor::GetDynamicBuffer(ConstBufferDescriptor::Type type, uint32 index)
{
    DVASSERT(dynamicBuffers.find(std::make_pair(type, index)) != dynamicBuffers.end()); //dynamic buffer not found
    return dynamicBuffers[std::make_pair(type, index)];
}


ShaderDescriptor::ShaderDescriptor(rhi::ShaderSource *vSource, rhi::ShaderSource *fSource, rhi::HPipelineState _pipelineState)
    :piplineState(_pipelineState)
{    
    vertexConstBuffersCount = vSource->ConstBufferCount();
    fragmentConstBuffersCount = fSource->ConstBufferCount();        

    Vector<BufferPropertyLayout> bufferPropertyLayouts;
    bufferPropertyLayouts.resize(vertexConstBuffersCount + fragmentConstBuffersCount);
    constBuffers.resize(vertexConstBuffersCount + fragmentConstBuffersCount);
    for (auto &prop : vSource->Properties())
    {
        bufferPropertyLayouts[prop.bufferindex].props.push_back(prop);
    }
    for (auto &prop : fSource->Properties())
    {
        bufferPropertyLayouts[prop.bufferindex + vertexConstBuffersCount].props.push_back(prop);
    }
    for (size_t i = 0, sz = constBuffers.size(); i < sz; ++i)
    {
        if (i < vertexConstBuffersCount)
        {
            constBuffers[i].type = ConstBufferDescriptor::Type::Vertex;
            constBuffers[i].targetSlot = i;
        }
        else
        {
            constBuffers[i].type = ConstBufferDescriptor::Type::Fragment;
            constBuffers[i].targetSlot = i - vertexConstBuffersCount;
        }

        constBuffers[i].propertyLayoutId = propertyLayoutSet.MakeUnique(bufferPropertyLayouts[i]);
        constBuffers[i].updateType = ConstBufferDescriptor::UpdateType::Static;
        for (auto& prop : bufferPropertyLayouts[i].props)
        {

            if (DynamicBindings::GetShaderSemanticByName(prop.uid) != DynamicBindings::UNKNOWN_SEMANTIC)
            {
                constBuffers[i].updateType = ConstBufferDescriptor::UpdateType::Dynamic;
            }
        }


    }


    for (size_t i = 0, sz = constBuffers.size(); i < sz; ++i)
    {
        if (constBuffers[i].updateType == ConstBufferDescriptor::UpdateType::Dynamic)
        {
            rhi::HConstBuffer dynamicBufferHandle;
            if (constBuffers[i].type == ConstBufferDescriptor::Type::Vertex)
                dynamicBufferHandle = rhi::CreateVertexConstBuffer(piplineState, constBuffers[i].targetSlot);
            else
                dynamicBufferHandle = rhi::CreateFragmentConstBuffer(piplineState, constBuffers[i].targetSlot);

            dynamicBuffers[std::make_pair(constBuffers[i].type, constBuffers[i].targetSlot)] = dynamicBufferHandle;
            for (auto &prop : bufferPropertyLayouts[i].props)
            {
                /*for some reason c++11 cant initialize inherited data*/
                DynamicPropertyBinding binding;
                binding.type = prop.type;
                binding.buffer = dynamicBufferHandle;
                binding.reg = prop.bufferReg;
                binding.updateSemantic = 0;
                binding.dynamicPropertySemantic = DynamicBindings::GetShaderSemanticByName(prop.uid);
                dynamicPropertyBindings.push_back(binding);
            }
        }
    }

    fragmentSamplerList = fSource->Samplers();
}
}
