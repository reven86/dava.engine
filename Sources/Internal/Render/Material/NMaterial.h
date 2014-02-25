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

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Scene3D/DataNode.h"
#include "Render/RenderState.h"
#include "Render/ShaderUniformArray.h"
#include "Render/Material/NMaterialConsts.h"

namespace DAVA
{

class UberShader;
class Shader;
class Texture;    
class SceneFileV2;
class Light;
class PolygonGroup;
class RenderDataObject;
class RenderState;
class Light;

class NMaterialDescriptor : public BaseObject
{
protected:
    virtual ~NMaterialDescriptor();
public:
    NMaterialDescriptor();

    uint32 GetTextureSlotByName(const String & textureName);
    uint32 GetUniformSlotByName(const String & uniformName);
    
    void SetNameForTextureSlot(uint32 slot, const String & name);
    void SetNameForUniformSlot(uint32 slot, const String & name);
    
private:
    Map<String, uint32> slotNameMap;
    Map<String, uint32> uniformNameMap;
};

/*
    Sorting should be done by NMaterialInstance shader, because it can be changed by material
 */
class NMaterialInstance : public BaseObject
{
protected:
    virtual ~NMaterialInstance();
public:
    NMaterialInstance();
    
    bool LoadFromYaml(const String & pathname);
    uint32 GetLightCount() { return lightCount; };
    void SetLightNode(uint32 index, Light * lightNode) { lightNodes[index] = lightNode; };
    RenderState * GetRenderState() { return &renderState; };

    void PrepareInstanceForShader(Shader * shader);

    void SetUniformData(uint32 uniformIndex, void * data, uint32 size);
    
    void UpdateUniforms();
    void BindUniforms();
    
private:
    
    static const uint32 SKIP_UNIFORM = 1 << 0;
    
    
    struct UniformInfo
    {
        uint32  flags: 8;
        uint32  arraySize : 8;
        uint32  shift : 16;
    };
    
    
    uint32                  uniformCount;
    UniformInfo *           uniforms;
    uint8 *                 uniformData;
    uint32                  lightCount;
    Light              *lightNodes[8];
    RenderState        renderState;
    Shader *                shader;
    friend class NMaterial;
};
 
class NMaterial : public BaseObject
{
protected:
    virtual ~NMaterial();
public:
    NMaterial(uint32 shaderCount);
    
    NMaterialDescriptor * GetDescriptor();
    void SetShader(uint32 index, Shader * shader);
    void PrepareRenderState(PolygonGroup * polygonGroup, NMaterialInstance * instance);
    void Draw(PolygonGroup * polygonGroup, NMaterialInstance * instance);

private:
    
    
    uint32 shaderCount;
    Shader ** shaderArray;
    NMaterialDescriptor * descriptor;
};


};

#endif // __DAVAENGINE_MATERIAL_H__

