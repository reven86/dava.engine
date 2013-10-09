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



#include "Render/Material/NMaterial.h"
#include "Render/RenderManager.h"
#include "Render/RenderState.h"
#include "Render/3D/PolygonGroup.h"
#include "Render/RenderBase.h"
#include "FileSystem/YamlParser.h"
#include "Render/Shader.h"


namespace DAVA
{
    
    
NMaterialDescriptor::NMaterialDescriptor()
{
    
}
    
NMaterialDescriptor::~NMaterialDescriptor()
{
    
}

uint32 NMaterialDescriptor::GetTextureSlotByName(const String & textureName)
{
    Map<String, uint32>::iterator it = slotNameMap.find(textureName);
    if (it != slotNameMap.end())
    {
        return it->second;
    }
    // if we haven't found slot let's use first slot
    return 0;
}
    
uint32 NMaterialDescriptor::GetUniformSlotByName(const String & uniformName)
{
    Map<String, uint32>::iterator it = uniformNameMap.find(uniformName);
    if (it != uniformNameMap.end())
    {
        return it->second;
    }
    // if we haven't found slot let's use first slot
    return 0;
}
    
void NMaterialDescriptor::SetNameForTextureSlot(uint32 slot, const String & name)
{
    slotNameMap[name] = slot;
}

void NMaterialDescriptor::SetNameForUniformSlot(uint32 slot, const String & name)
{
    uniformNameMap[name] = slot;
}


NMaterialInstance::NMaterialInstance()
{
    DVASSERT(sizeof(UniformInfo) == 4);
    uniforms = 0;
    uniformData = 0;
    lightCount = 0;
    for (uint32 k = 0; k < 8; ++k)
    {
        lightNodes[k] = 0;
    }
    
    renderState.state = RenderState::DEFAULT_3D_STATE;
    shader = 0;
}
    
NMaterialInstance::~NMaterialInstance()
{
    SafeDeleteArray(uniforms);
    SafeDeleteArray(uniformData);
}
    
bool NMaterialInstance::LoadFromYaml(const String & pathname)
{
    YamlParser * parser = parser->Create(pathname);
    parser->GetRootNode();
    
    
    
    
    SafeRelease(parser);

	return true;
}
    
void NMaterialInstance::SetUniformData(uint32 uniformIndex, void * data, uint32 size)
{
    memcpy(&uniformData[uniforms[uniformIndex].shift], data, size);
}

    
void NMaterialInstance::PrepareInstanceForShader(Shader * _shader)
{
    if (shader == _shader)return;
    shader = _shader;
    /*
        TODO: rethink how to prepare instances for particular shaders of material.
     */
    
    SafeDeleteArray(uniforms);
    SafeDeleteArray(uniformData);
    
    uniformCount = shader->GetUniformCount();
    uniforms = new UniformInfo[uniformCount];
    memset(uniforms, 0, sizeof(UniformInfo) * uniformCount);
    
    uint32 uniformDataSize = 0;
    for (uint32 uniformIndex = 0; uniformIndex < uniformCount; ++uniformIndex)
    {
        Shader::eUniformType uniformType = shader->GetUniformType(uniformIndex);
        uint32 size = shader->GetUniformTypeSize(uniformType) * shader->GetUniformArraySize(uniformIndex) + 4;
        uniforms[uniformIndex].shift = uniformDataSize;
        uniforms[uniformIndex].arraySize = shader->GetUniformArraySize(uniformIndex);
        uniformDataSize += size;
    }
    
    uniformData = new uint8[uniformDataSize];

    for (uint32 uniformIndex = 0; uniformIndex < uniformCount; ++uniformIndex)
    {
        //Shader::eUniformType uniformType = shader->GetUniformType(uniformIndex);
        const String & uniformName = shader->GetUniformName(uniformIndex);
        //Logger::FrameworkDebug("Find uniform: %s", uniformName.c_str());
        
        if ((uniformName == "texture[0]") || (uniformName == "texture"))
        {
            uint32 textureIndexes[8] = {0, 1, 2, 3, 4, 5, 6, 7};
            SetUniformData(uniformIndex, textureIndexes, 4 *  shader->GetUniformArraySize(uniformIndex));
        }else if ((uniformName == "lightIntensity[0]") || (uniformName == "lightIntensity"))
        {
            float32 lightIntensity[8] = {500, 500, 500, 500, 100, 100, 100, 100};
            SetUniformData(uniformIndex, lightIntensity, 4 *  shader->GetUniformArraySize(uniformIndex));
        }else if ((uniformName == "lightPosition[0]") || (uniformName == "lightPosition"))
        {
            Vector3 lightPosition[8] = {
                Vector3(0.0f, 0.0f, 0.0f),
                Vector3(0.0f, 0.0f, 0.0f),
                Vector3(0.0f, 0.0f, 0.0f),
                Vector3(0.0f, 0.0f, 0.0f),
                Vector3(0.0f, 0.0f, 0.0f),
                Vector3(0.0f, 0.0f, 0.0f),
                Vector3(0.0f, 0.0f, 0.0f),
                Vector3(0.0f, 0.0f, 0.0f),
            };
            SetUniformData(uniformIndex, lightPosition, 12 *  shader->GetUniformArraySize(uniformIndex));
        }else
        {
            uniforms[uniformIndex].flags |= SKIP_UNIFORM;
        }
    }
}
    
// TODO: Platform specific code.
// Either rethink, or rewrite. 

void NMaterialInstance::BindUniforms()
{
    for (uint32 uniformIndex = 0; uniformIndex < uniformCount; ++uniformIndex)
    {
        Shader::eUniformType uniformType = shader->GetUniformType(uniformIndex);
        UniformInfo uniformInfo = uniforms[uniformIndex];
        
        if (uniformInfo.flags & SKIP_UNIFORM)continue;
        
//        Logger::FrameworkDebug("Bind uniform: %s", shader->GetUniformName(uniformIndex).c_str());
        
        uint8 * data = &uniformData[uniformInfo.shift];
        switch(uniformType)
        {
            case Shader::UT_FLOAT:
                RENDER_VERIFY(glUniform1fv(shader->GetUniformLocationByIndex(uniformIndex), uniformInfo.arraySize, (float*)data));
                break;
            case Shader::UT_FLOAT_VEC2:
                RENDER_VERIFY(glUniform2fv(shader->GetUniformLocationByIndex(uniformIndex), uniformInfo.arraySize, (float*)data));
                break;
            case Shader::UT_FLOAT_VEC3:
                RENDER_VERIFY(glUniform3fv(shader->GetUniformLocationByIndex(uniformIndex), uniformInfo.arraySize, (float*)data));
                break;
            case Shader::UT_FLOAT_VEC4:
                RENDER_VERIFY(glUniform4fv(shader->GetUniformLocationByIndex(uniformIndex), uniformInfo.arraySize, (float*)data));
                break;
            case Shader::UT_INT:
                RENDER_VERIFY(glUniform1iv(shader->GetUniformLocationByIndex(uniformIndex), uniformInfo.arraySize, (int32*)data));
                break;
            case Shader::UT_INT_VEC2:
                RENDER_VERIFY(glUniform2iv(shader->GetUniformLocationByIndex(uniformIndex), uniformInfo.arraySize, (int32*)data));
                break;
            case Shader::UT_INT_VEC3:
                RENDER_VERIFY(glUniform3iv(shader->GetUniformLocationByIndex(uniformIndex), uniformInfo.arraySize, (int32*)data));
                break;
            case Shader::UT_INT_VEC4:
                RENDER_VERIFY(glUniform4iv(shader->GetUniformLocationByIndex(uniformIndex), uniformInfo.arraySize, (int32*)data));
                break;
            case Shader::UT_BOOL:
                RENDER_VERIFY(glUniform1iv(shader->GetUniformLocationByIndex(uniformIndex), uniformInfo.arraySize, (int32*)data));
                break;
            case Shader::UT_BOOL_VEC2:
                RENDER_VERIFY(glUniform2iv(shader->GetUniformLocationByIndex(uniformIndex), uniformInfo.arraySize, (int32*)data));
                break;
            case Shader::UT_BOOL_VEC3:
                RENDER_VERIFY(glUniform3iv(shader->GetUniformLocationByIndex(uniformIndex), uniformInfo.arraySize, (int32*)data));
                break;
            case Shader::UT_BOOL_VEC4:
                RENDER_VERIFY(glUniform4iv(shader->GetUniformLocationByIndex(uniformIndex), uniformInfo.arraySize, (int32*)data));
                break;
            case Shader::UT_FLOAT_MAT2:
                RENDER_VERIFY(glUniformMatrix2fv(shader->GetUniformLocationByIndex(uniformIndex), uniformInfo.arraySize, GL_FALSE, (float32*)data));
                break;
            case Shader::UT_FLOAT_MAT3:
                RENDER_VERIFY(glUniformMatrix3fv(shader->GetUniformLocationByIndex(uniformIndex), uniformInfo.arraySize, GL_FALSE, (float32*)data));
                break;
            case Shader::UT_FLOAT_MAT4:
                RENDER_VERIFY(glUniformMatrix4fv(shader->GetUniformLocationByIndex(uniformIndex), uniformInfo.arraySize, GL_FALSE, (float32*)data));
                break;
            case Shader::UT_SAMPLER_2D:
                RENDER_VERIFY(glUniform1iv(shader->GetUniformLocationByIndex(uniformIndex), uniformInfo.arraySize, (int32*)data));
                break;
            case Shader::UT_SAMPLER_CUBE:
                RENDER_VERIFY(glUniform1iv(shader->GetUniformLocationByIndex(uniformIndex), uniformInfo.arraySize, (int32*)data));
                break;
        };
    }
}

    
void NMaterialInstance::UpdateUniforms()
{
    // Get global time
    //float32 time = 0.0;
    //shader->GetUniformLocationByName(FN_GLOBAL_TIME);
    
    
    /*
         Camera * camera = scene->GetCurrentCamera();
         LightNode * lightNode0 = instanceMaterialState->GetLight(0);
         if (lightNode0 && camera)
         {
         if (uniformLightPosition0 != -1)
         {
         const Matrix4 & matrix = camera->GetMatrix();
         Vector3 lightPosition0InCameraSpace = lightNode0->GetPosition() * matrix;
     */

}
    
NMaterial::NMaterial(uint32 _shaderCount)
    :   shaderCount(_shaderCount)
{
    shaderArray = new Shader*[shaderCount];
    descriptor = new NMaterialDescriptor();
}
    
NMaterial::~NMaterial()
{
    SafeRelease(descriptor);
    SafeDeleteArray(shaderArray);
}
    
NMaterialDescriptor * NMaterial::GetDescriptor()
{
    return descriptor;
}
    
void NMaterial::SetShader(uint32 index, Shader * shader)
{
    DVASSERT(shaderArray != 0);
    shaderArray[index] = shader;
}


void NMaterial::PrepareRenderState(PolygonGroup * polygonGroup, NMaterialInstance * instance)
{
    
//    if (1)
    {
//        shaderArray[0]->Dump();
    }
    
    
    Shader * activeShader = shaderArray[instance->GetLightCount()];
    DVASSERT(activeShader != 0)
    instance->renderState.SetShader(activeShader);
    instance->PrepareInstanceForShader(activeShader);
    
    // Bind shader & global uniforms
    RenderManager::Instance()->FlushState(&instance->renderState);

    // Here bind local uniforms of material
    instance->UpdateUniforms();
    instance->BindUniforms();
    
    RenderManager::Instance()->SetRenderData(polygonGroup->renderDataObject);
	RenderManager::Instance()->AttachRenderData();
};

void NMaterial::Draw(PolygonGroup * polygonGroup, NMaterialInstance * instance)
{
    // TODO: rethink this code
    if (polygonGroup->renderDataObject->GetIndexBufferID() != 0)
    {
        RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, polygonGroup->indexCount, EIF_16, 0);
    }
    else
    {
        RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, polygonGroup->indexCount, EIF_16, polygonGroup->indexArray);
    }
};
    



};
