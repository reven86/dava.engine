#include "SceneRenderTest.h"

rhi::Handle GetRenderDataObjectVertexStream(RenderDataObject *rdo){ return rhi::InvalidHandle; }
rhi::Handle GetRenderDataObjectIndexBuffer(RenderDataObject *rdo){ return rhi::InvalidHandle; }
rhi::Handle GetMaterialRenderPiplineState(NMaterial *material){ return rhi::InvalidHandle; }
rhi::DepthStencilState GetMaterialDepthStencilState(NMaterial *material){ return rhi::DepthStencilState(); }
rhi::Handle GetMaterialTextureState(NMaterial *material){ return rhi::InvalidHandle; }

Shader* GetMaterialShader(NMaterial *material){ return NULL; } //for now lats assume we can get shader for curr material render pass


struct MaterialConstBufferProperty
{
    enum ConstBufferType {Vertex, Fragment};
    ConstBufferType constBufferType;
    uint32 slot;
    rhi::Handle handle;

    static Vector<MaterialConstBufferProperty> empty;
};

Vector<MaterialConstBufferProperty> MaterialConstBufferProperty::empty;


struct ShaderConstBufferProperty
{
    enum ConstBufferType { Vertex, Fragment };
    ConstBufferType constBufferType;
    uint32 slot;
    rhi::Handle handle;    

    int32 constCount;
    Vector<uint32> updateSemantic;
    Vector<uint32> shaderSemantic;    
    Vector<uint32> targetSlot;

    static Vector<ShaderConstBufferProperty> empty;
};
Vector<ShaderConstBufferProperty> ShaderConstBufferProperty::empty;

Vector<MaterialConstBufferProperty>& GetMaterialProperties(NMaterial *material){ return MaterialConstBufferProperty::empty; }
Vector<ShaderConstBufferProperty>& GetShaderProperties(Shader *shader){ return ShaderConstBufferProperty::empty; }



void SceneRenderTest::Init()
{
    rhi::RenderPassConfig passConfig;

    mainRenderPass = AllocateRenderPass(passConfig, 1, &sceneCommandBuffer);        
}

void SceneRenderTest::Draw()
{
    rhi::BeginRenderPass(mainRenderPass);
    
    rhi::BatchDescriptor batchDescriptor;
    /*
            
    Handle              vertexConst[MAX_CONST_BUFFER_COUNT];    
    Handle              fragmentConst[MAX_CONST_BUFFER_COUNT];   

    */

    for (int32 i = 0, sz = batchArray.size(); i < sz; ++i)
    {
        RenderBatch *batch = batchArray[i];

        batch->GetRenderObject()->BindDynamicParameters(NULL);
        /*for now let us assume that we've already selected corresponding render pass in material*/

        batchDescriptor.vertexStreamCount = 1;
        /*note - for following tricks batch rdo should be rebuild once pg is set*/
        batchDescriptor.vertexStream[0] = GetRenderDataObjectVertexStream(batch->renderDataObject);
        batchDescriptor.indexBuffer = GetRenderDataObjectIndexBuffer(batch->renderDataObject);

        batchDescriptor.renderPipelineState = GetMaterialRenderPiplineState(batch->GetMaterial());        
        batchDescriptor.depthState = GetMaterialDepthStencilState(batch->GetMaterial());
        
        /*sampler state should be carefully thought over again - right now i am not sure how is it intended to work*/
        /*at least in gl it is bound to texture unit afiak*/
        batchDescriptor.samplerState = rhi::SamplerState(); 

        /*as we already passed renderPipelineState this to look like not needed*/
        uint32 vconstBuffCount = 1;// rhi::PipelineState::VertexConstBufferCount(batchDescriptor.renderPipelineState);
        uint32 fconstBuffCount = 3;// rhi::PipelineState::FragmentConstBufferCount(batchDescriptor.renderPipelineState);
        batchDescriptor.vertexConstCount = vconstBuffCount;
        batchDescriptor.fragmentConstCount = fconstBuffCount;

        /*i don't like this if... may be passing const buffers as single array would be better idea - anyway we know there type*/
        Vector<MaterialConstBufferProperty>& materialProps = GetMaterialProperties(batch->material);
        for (uint32 i = 0, sz = materialProps.size(); i < sz; ++i)
        {
            if (materialProps[i].constBufferType == MaterialConstBufferProperty::Vertex)
                batchDescriptor.vertexConst[materialProps[i].slot] = materialProps[i].handle;
            else
                batchDescriptor.fragmentConst[materialProps[i].slot] = materialProps[i].handle;
        }

        /*it would be not shader in final - for now its what we call RightMaterialVariant*/
        Shader *shader = GetMaterialShader(batch->material);
        Vector<ShaderConstBufferProperty>& shaderProps = GetShaderProperties(shader);
        for (uint32 i = 0, sz = shaderProps.size(); i < sz; ++i)
        {
            
            //its like what we have now with binding dynamic params
            for (uint32 slot = 0, slotCount = shaderProps[i].constCount; slot < slotCount; ++slot)
            {
                const void *data = RenderManager::GetDynamicParam(eShaderSemantic(shaderProps[i].shaderSemantic[slot])); //as it can recompute param and change update semantic
                uint32 _updateSemantic = GET_DYNAMIC_PARAM_UPDATE_SEMANTIC(eShaderSemantic(shaderProps[i].shaderSemantic[slot]));
                if (_updateSemantic != shaderProps[i].updateSemantic[slot])
                {
                    uint32 count = 1; //assume we can get it from rander manager                    
                    rhi::ConstBuffer::SetConst(shaderProps[i].handle, shaderProps[i].targetSlot[slot], count, (const float *)data);
                    shaderProps[i].updateSemantic[slot] = _updateSemantic;
                }
            }
            
            /*again - lines below are just copy-pasted from material properties setup above*/
            if (shaderProps[i].constBufferType == ShaderConstBufferProperty::Vertex)
                batchDescriptor.vertexConst[shaderProps[i].slot] = shaderProps[i].handle;
            else
                batchDescriptor.fragmentConst[shaderProps[i].slot] = shaderProps[i].handle;
        }
        
        batchDescriptor.textureSet = GetMaterialTextureState(batch->GetMaterial());        
        
        /*we do not have primitive type in RenderDataObject - so it might be some problem*/
        batchDescriptor.primitiveType = rhi::PRIMITIVE_TRIANGLELIST;

        batchDescriptor.primitiveCount = batch->renderDataObject->indexCount;

        rhi::DrawBatch(sceneCommandBuffer, batchDescriptor);
    }

    rhi::EndRenderPass(mainRenderPass);
}