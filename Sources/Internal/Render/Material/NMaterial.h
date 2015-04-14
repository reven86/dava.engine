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

#ifndef __DAVAENGINE_NMATERIAL_H__
#define __DAVAENGINE_NMATERIAL_H__

#include <memory>
#include "Base/FastNameMap.h"
#include "NMaterialNames.h"
#include "Render/Shader.h"
#include "Scene3D/DataNode.h"

namespace DAVA
{

struct NMaterialProperty
{
    FastName name;
    rhi::ShaderProp::Type type;
    uint32 arraySize;
    std::unique_ptr<float32[]> data;
    uint32 updateSemantic;

    inline void SetPropertyValue(float32 *newValue);    
    inline static uint32 GetCurrentUpdateSemantic(){ return globalPropertyUpdateSemanticCounter; }
private:
    static uint32 globalPropertyUpdateSemanticCounter;
};

struct MaterialPropertyBinding
{
    rhi::ShaderProp::Type type;
    uint32 reg;
    uint32 updateSemantic;
    NMaterialProperty *source;
};

struct MaterialBufferBinding
{
    rhi::Handle constBuffer;
    Vector<MaterialPropertyBinding> propBindings;
    uint32 lastValidPropertySemantic;
};


class RenderVariantInstance
{
    friend class NMaterialV3;
    ShaderDescriptor *shader;


    rhi::Handle depthState;
    rhi::Handle samplerState;
    rhi::Handle textureSet;

    Vector<rhi::Handle> vertexConstBuffers;
    Vector<rhi::Handle> fragmentConstBuffers;

    Vector<MaterialBufferBinding *> materialBufferBindings;
};

class NMaterial : public DataNode
{

public:
    NMaterial();    
    ~NMaterial();

    void Load(KeyedArchive * archive, SerializationContext * serializationContext) override;

    /*properties*/
    void AddProperty(const FastName& propName, float32 *propData, rhi::ShaderProp::Type type, uint32 arraySize = 1);
    void RemoveProperty(const FastName& propName);
    void SetPropertyValue(const FastName& propName, float32 *propData);
    bool HasLocalProperty(const FastName& flagName);

    /*textures*/
    void AddTexture(const FastName& slotName, Texture *texture);
    void RemoveTexture(const FastName& slotName);
    void SetTexture(const FastName& slotName, Texture *texture);
    bool HasLocalTexture(const FastName& flagName);

    /*flags*/
    void AddFlag(const FastName& flagName, int32 value);
    void RemoveFlag(const FastName& flagName);
    void SetFlag(const FastName& flagName, int32 value);
    bool HasLocalFlag(const FastName& flagName);

    void SetParent(NMaterial *parent);
    NMaterial* GetParent();       

    inline uint32 GetRenderLayerID() const;
    inline uint32 GetSortingKey() const;
    inline uint64 GetMaterialKey() const;

    void BindParams(rhi::BatchDescriptor & target);

private:

    void InvalidateBufferBindings();
    void InvalidateTextureBindings();
    void InvalidateShader();

    void RebuildBindings();
    void RebuildTextureBindings();
    void RebuildShader();

    bool NeedLocalOverride(UniquePropertyLayout propertyLayout);
    void ClearLocalBuffers();
    void  InjectChildBuffer(UniquePropertyLayout propLayoutId, MaterialBufferBinding* buffer);

    MaterialBufferBinding* GetConstBufferBinding(UniquePropertyLayout propertyLayout);
    NMaterialProperty* GetMaterialProperty(const FastName& propName);
    void CollectMaterialFlags(HashMap<FastName, int32>& target);

    rhi::Handle GetMaterialTexture(const FastName& slotName);

private:
    HashMap<FastName, NMaterialProperty *> localProperties;
    HashMap<FastName, Texture*> localTextures; //this is runtime only state, filepath storing will be separately done later
    HashMap<FastName, int32> localFlags; //integer flags are just more generic then boolean (eg. #if SHADING == HIGH), it has nothing in common with eFlagValue bullshit from old NMaterial    

    Vector<NMaterial *> children;
    NMaterial *parent;

    //runtime
    HashMap<UniquePropertyLayout, MaterialBufferBinding*> localConstBuffers;

    /*this is for render passes - not used right now - only active variant instance*/
    HashMap<FastName, RenderVariantInstance *> renderVariants;

    FastName activeVariantName;
    RenderVariantInstance *activeVariantInstance;

    bool needRebuildBindings;
    bool needRebuildTextures;
    bool needRebuildShader;

};



void NMaterialProperty::SetPropertyValue(float32 *newValue)
{
    //4 is because register size is float4
    Memcpy(data.get(), newValue, sizeof(float32) * 4 * ShaderDescriptor::CalculateRegsCount(type, arraySize));
    updateSemantic = ++globalPropertyUpdateSemanticCounter;
}
};

#endif // __DAVAENGINE_MATERIAL_H__

