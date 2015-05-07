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


#ifndef __DAVAENGINE_SHADER_H__
#define __DAVAENGINE_SHADER_H__

#include "Base/BaseTypes.h"
#include "Render/Renderer.h"
#include "Render/RHI/rhi_ShaderSource.h"
#include "Render/UniqueStateSet.h"
#include "Render/DynamicBindings.h"

namespace DAVA
{
using UniquePropertyLayout = UniqueHandle;

struct ConstBufferDescriptor
{
    enum class Type{ Vertex, Fragment };
    enum class UpdateType{ Static, Dynamic };

    Type type;
    UpdateType updateType;
    uint32 targetSlot;

    UniquePropertyLayout propertyLayoutId;
};



struct DynamicPropertyBinding
{
    rhi::ShaderProp::Type type;
    uint32 reg;
    uint32 regCount; //offset for props less than 1 reg size
    uint32 updateSemantic;
    rhi::HConstBuffer buffer;
    DynamicBindings::eShaderSemantic dynamicPropertySemantic;
};



class ShaderDescriptor
{
public://utility    
    static const rhi::ShaderPropList& GetProps(UniquePropertyLayout layout);
    static uint32 CalculateRegsCount(rhi::ShaderProp::Type type, uint32 arraySize);  //return in registers  
    static uint32 CalculateDataSize(rhi::ShaderProp::Type type, uint32 arraySize); //return in float  
    
public:
    ShaderDescriptor(rhi::ShaderSource *vSource, rhi::ShaderSource *fSource, rhi::HPipelineState pipelineState);

    void UpdateDynamicParams();

    uint32 GetVertexConstBuffersCount();
    uint32 GetFragmentConstBuffersCount();

    rhi::HConstBuffer GetDynamicBuffer(ConstBufferDescriptor::Type type, uint32 index);
    inline rhi::HPipelineState GetPiplineState(){ return piplineState; }

    

private:
    Vector<ConstBufferDescriptor> constBuffers;


    uint32 vertexConstBuffersCount, fragmentConstBuffersCount;
    Vector<DynamicPropertyBinding> dynamicPropertyBindings;

    Map<std::pair<ConstBufferDescriptor::Type, uint32>, rhi::HConstBuffer> dynamicBuffers;

    rhi::HPipelineState piplineState;

    rhi::ShaderSamplerList fragmentSamplerList; //no vertex samplers in rhi yet

    friend class NMaterial;
};

};

#endif // __DAVAENGINE_SHADER_H__
