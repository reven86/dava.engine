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
#if RHI_COMPLETE

#ifndef __DAVAENGINE_MATERIAL_BUILDER_H__
#define __DAVAENGINE_MATERIAL_BUILDER_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Scene3D/DataNode.h"
#include "FileSystem/YamlParser.h"
#include "Render/Shader.h"

namespace DAVA
{
class MaterialGraph;
class MaterialGraphNode;
class NMaterial;
class PolygonGroup;

struct MaterialShaders
{
    Shader * shaderForwardLight[4];
    Shader * shaderDeferred;
};

class MaterialCompiler : public BaseObject
{
protected:
    ~MaterialCompiler(){}
public:
    enum eCompileResult
    {
        COMPILATION_SUCCESS = 1,
        COMPILATION_FAILED = 0,
    };
    
    enum eCompileError
    {
        COMPILE_NO_ERROR = 1,
        ERROR_NOT_ENOUGH_CONNECTORS,
        ERROR_UNUSED_NODE,
    };
    
    eCompileResult Compile(MaterialGraph * materialGraph, PolygonGroup * polygonGroup, uint32 maxLights, NMaterial ** resultMaterial);
    eCompileError GenerateCodeForNode(MaterialGraphNode * node, String & vertexShader, String & pixelShader);

    String GetCompiledVertexShaderPathname() const;
    String GetCompiledFragmentShaderPathname() const;
    
    
    
private:
    /**
        Plan: 
        1. Remove unused nodes.
        2. Split vertex shader nodes, from pixel shader nodes. 
        3. Add varying nodes.
        4. Sort graph. 
        5. Generate plain code for vertex, and pixel shader. 
     
        IN_DIFFUSE, IN_SPECULAR, IN_EMISSIVE
        PIXEL SHADER | VARYING VARIABLES | VERTEX_SHADER.
        node0, node1, node2 | var1, var2, var3, var4 | node5, node6, node7.
     */
    void GenPlainCodeWithPositionDefines(List<MaterialGraphNode*> & requiredNodes);
    
    
    void GenerateCode(MaterialGraph * graph);
    void RecursiveSetDepthMarker(MaterialGraphNode * node, uint32 depthMarker);
    void FixNodesWithoutProperInputs();

    MaterialGraph * materialGraph;
    PolygonGroup * polygonGroup;
    NMaterial * currentMaterial;
    
    String pixelShaderCode;
    String vertexShaderCode;
    String finalPixelShaderCode;
    String finalVertexShaderCode;
    FilePath materialCompiledVshName;
    FilePath materialCompiledFshName;
    
    Map<String, Shader::eUniformType> vertexShaderAdditionaUniforms;
    Map<String, Shader::eUniformType> pixelShaderAdditionaUniforms;
    Map<String, Shader::eUniformType> additionalVaryings;
};
     

};

#endif // __DAVAENGINE_MATERIAL_H__

#endif //RHI_COMPLETE
