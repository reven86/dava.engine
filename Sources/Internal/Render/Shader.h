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
    uint32 updateSemantic;
    rhi::Handle buffer;
    eShaderSemantic dynamicPropertySemantic;
};



class ShaderDescriptor
{
    
public:
    ShaderDescriptor(rhi::ShaderSource *vSource, rhi::ShaderSource *fSource);

    void UpdateDynamicParams();

    uint32 GetVertexConstBuffersCount();
    uint32 GetFragmentConstBuffersCount();

    rhi::Handle GetDynamicBuffer(ConstBufferDescriptor::Type type, uint32 index);
    inline rhi::Handle GetPiplineState(){ return piplineState; }


    //utility
    static uint32 CalculateRegsCount(rhi::ShaderProp::Type type, uint32 arraySize);
    static eShaderSemantic GetShaderSemanticByName(const FastName& name);

private:
    Vector<ConstBufferDescriptor> constBuffers;


    uint32 vertexConstBuffersCount, fragmentConstBuffersCount;
    Vector<DynamicPropertyBinding> dynamicPropertyBindings;

    Map<std::pair<ConstBufferDescriptor::Type, uint32>, rhi::Handle> dynamicBuffers;

    FastName vProgUid, fProgUid;

    rhi::Handle piplineState;

    rhi::ShaderSamplerList fragmentSamplerList; //no vertex samplers in rhi yet
};

};

#endif // __DAVAENGINE_SHADER_H__
