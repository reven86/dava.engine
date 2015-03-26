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


#include "Render/Material/MaterialCompiler.h"
#include "Render/Material/MaterialGraph.h"
#include "Render/Material/MaterialGraphNode.h"
#include "Render/Material/NMaterialConsts.h"
#include "Render/Material/NMaterial.h"
#include "Render/3D/PolygonGroup.h"
#include "FileSystem/FileSystem.h"
#include "Render/Shader.h"
#include "Utils/StringFormat.h"

namespace DAVA
{
    
MaterialCompiler::eCompileResult MaterialCompiler::Compile(MaterialGraph * _materialGraph, PolygonGroup * _polygonGroup, uint32 maxLights, NMaterial ** resultMaterial)
{
    materialGraph = _materialGraph;
    polygonGroup = _polygonGroup;
    
    MaterialGraphNode * rootResultNode = materialGraph->GetNodeByName("material");
    if (!rootResultNode)
    {
        return COMPILATION_FAILED;
    }

    currentMaterial = new NMaterial();

    RecursiveSetDepthMarker(rootResultNode, 0);
    materialGraph->SortByDepthMarkerAndRemoveUnused();

    MaterialGraphNode::RecursiveSetRealUsageBack(rootResultNode);
    MaterialGraphNode::RecursiveSetRealUsageForward(rootResultNode);

    
    materialCompiledVshName = FilePath::CreateWithNewExtension(materialGraph->GetMaterialPathname(), ".vsh");
    materialCompiledFshName = FilePath::CreateWithNewExtension(materialGraph->GetMaterialPathname(), ".fsh");

#if 1
    materialCompiledVshName = "~doc:/temp.vsh";
    materialCompiledFshName = "~doc:/temp.fsh";
#endif
    
    GenerateCode(materialGraph);

    return COMPILATION_SUCCESS;
};
    
String MaterialCompiler::GetCompiledVertexShaderPathname() const
{	//VK: why we return string instead of FilePath
    return materialCompiledVshName.GetAbsolutePathname();
}

String MaterialCompiler::GetCompiledFragmentShaderPathname() const
{	//VK: why we return string instead of FilePath
    return materialCompiledFshName.GetAbsolutePathname();
}

    
void MaterialCompiler::RecursiveSetDepthMarker(MaterialGraphNode * node, uint32 depthMarker)
{
    node->SetDepthMarker(depthMarker);
    
    Map<String, MaterialGraphNodeConnector*> & inputConnectors = node->GetInputConnectors();
    for (Map<String, MaterialGraphNodeConnector*>::iterator it = inputConnectors.begin(); it != inputConnectors.end(); ++it)
    {
        MaterialGraphNodeConnector * connector = it->second;
        MaterialGraphNode * connectedNode = connector->node;
        if (connectedNode)
        {
            RecursiveSetDepthMarker(connectedNode, depthMarker + 1);
        }
    }
}

void MaterialCompiler::GenerateCode(MaterialGraph * materialGraph)
{
    // Generate pixel shader code
    String pixelShader;
    String vertexShader;
    
    for (uint32 nodeIndex = 0; nodeIndex < materialGraph->GetNodeCount(); ++nodeIndex)
    {
        MaterialGraphNode * node = materialGraph->GetNode(nodeIndex);
        GenerateCodeForNode(node, vertexShader, pixelShader);
    }
    
    //MaterialGraphNode * rootResultNode = materialGraph->GetNodeByName("material");

    FilePath materialPath(materialGraph->GetMaterialPathname().GetDirectory());
    FilePath vertexShaderPath(materialPath + materialGraph->GetVertexShaderFilename());
    FilePath fragmentShaderPath(materialPath + materialGraph->GetPixelShaderFilename());

    String originalVertexShader = FileSystem::Instance()->ReadFileContents(vertexShaderPath);
    String originalFragmentShader = FileSystem::Instance()->ReadFileContents(fragmentShaderPath);
    
    
    File * resultVsh = File::Create(materialCompiledVshName, File::CREATE | File::WRITE);
    resultVsh->WriteString(finalVertexShaderCode, false); // Save without null terminator
    resultVsh->WriteString(originalVertexShader);
    SafeRelease(resultVsh);
    
    File * resultFsh = File::Create(materialCompiledFshName, File::CREATE | File::WRITE);
    resultFsh->WriteString(finalPixelShaderCode, false); // Save without null terminator
    resultFsh->WriteString(originalFragmentShader);
    //resultFsh->Write(fragmentShaderData->GetPtr(), fragmentShaderData->GetSize());
    SafeRelease(resultFsh);
};
    
void MaterialCompiler::FixNodesWithoutProperInputs()
{
    
}


MaterialCompiler::eCompileError MaterialCompiler::GenerateCodeForNode(MaterialGraphNode * node, String & vertexShader, String & pixelShader)
{
    Logger::Debug("Generate Code: %s %d", node->GetName().c_str(), node->GetType());
    
    
    //uint32 usedByOthersCount = usedByOthersModifier.length();
    /*     if (usedByOthersCount == 0)
     {
     return ERROR_UNUSED_NODE;
     }
     */
    //
    
    DVASSERT(node->usage != MaterialGraphNode::USE_BOTH);
    
    MaterialGraphNode::eType type = node->GetType();
    
    String * destinationCode = &pixelShaderCode;
    if (node->usage == MaterialGraphNode::USE_VERTEX)
        destinationCode = &vertexShaderCode;
        
    if (type == MaterialGraphNode::TYPE_TEX_COORD_INPUT)
    {
        //
        node->nodeGenVarying = Format("var_inTexCoord%d", node->textureInputIndex);
        node->nodeCode = Format("var_inTexCoord%d = inTexCoord%d;", node->textureInputIndex, node->textureInputIndex);
        *destinationCode += node->nodeCode;
    }
    
    if (type == MaterialGraphNode::TYPE_SHIFTER)
    {
        MaterialGraphNodeConnector * connectorTexCoord = node->GetInputConnector("texCoord");
//        int32 texCoordIndex = 0;
//        if (connectorTexCoord)
//            texCoordIndex = connectorTexCoord->GetNode()->textureInputIndex;
        
        // Add uniform
        vertexShaderAdditionaUniforms[NMaterialConsts::UNIFORM_GLOBAL_TIME] = Shader::UT_FLOAT;
        node->nodeGenVarying = Format("var_%s", node->GetName().c_str());
        node->nodeCode = Format("var_%s = %s + 0.25 * globalTime;", node->GetName().c_str(), connectorTexCoord->GetNode()->GetName().c_str());
        *destinationCode += node->nodeCode;
    }
    
    if (type == MaterialGraphNode::TYPE_ROTATOR)
    {
        // Add uniform
        vertexShaderAdditionaUniforms[NMaterialConsts::UNIFORM_GLOBAL_TIME] = Shader::UT_FLOAT;
    }
    
    if (type == MaterialGraphNode::TYPE_SAMPLE_2D)
    {
        MaterialGraphNodeConnector * connectorTexCoord = node->GetInputConnector("texCoord");
        String inputVarName;
        if (connectorTexCoord)
        {
            MaterialGraphNode * inputCoordNode = connectorTexCoord->GetNode();
            
            // Get from varying
            if (inputCoordNode->usage == MaterialGraphNode::USE_VERTEX)
            {
                additionalVaryings[inputCoordNode->nodeGenVarying] = Shader::UT_FLOAT_VEC2;
                inputVarName = "var_" + connectorTexCoord->GetNode()->GetName();
            }else
            {
                inputVarName = node->GetName();
            }
        }
        node->nodeCode = Format("%s %s = texture2D(texture[%d], %s).%s;", node->RGBAModifierToType(node->usedByOthersModifier).c_str(),
                          node->name.c_str(),
                          node->textureChannelIndex,
                          inputVarName.c_str(),
                          node->RGBAModifierToString(node->usedByOthersModifier).c_str());
        Logger::Debug("%s", node->nodeCode.c_str());
        *destinationCode += node->nodeCode;
        
        //NMaterialDescriptor * descriptor = currentMaterial->GetDescriptor();
        //descriptor->SetNameForTextureSlot(node->textureChannelIndex, node->name);
    }
    
    if (type == MaterialGraphNode::TYPE_CONST)
    {
        String value;
        if (node->constValue.GetType() == VariantType::TYPE_FLOAT)
            value = Format("(%f)", node->constValue.AsFloat());
        else if (node->constValue.GetType() == VariantType::TYPE_VECTOR2)
            value = Format("(%f, %f)", node->constValue.AsVector2().x, node->constValue.AsVector2().y);
        else if (node->constValue.GetType() == VariantType::TYPE_VECTOR3)
            value = Format("(%f, %f, %f)", node->constValue.AsVector3().x, node->constValue.AsVector3().y, node->constValue.AsVector3().z);
        else if (node->constValue.GetType() == VariantType::TYPE_VECTOR4)
            value = Format("(%f, %f, %f, %f)", node->constValue.AsVector4().x, node->constValue.AsVector4().y, node->constValue.AsVector4().z, node->constValue.AsVector4().w);
        
        node->nodeCode = Format("%s %s = %s%s;", Shader::GetUniformTypeSLName(node->returnType), node->name.c_str(), Shader::GetUniformTypeSLName(node->returnType), value.c_str());
        *destinationCode += node->nodeCode;
    }
    
    if (type == MaterialGraphNode::TYPE_MUL)
    {
        MaterialGraphNodeConnector * connectorA = node->GetInputConnector("a");
        MaterialGraphNodeConnector * connectorB = node->GetInputConnector("b");
        
        if (!connectorA || !connectorB)
            return ERROR_NOT_ENOUGH_CONNECTORS;
        String resultFormat = node->GetResultFormat(connectorA->modifier, connectorB->modifier);
        
        String op1 = connectorA->node->GetName();
        if (!connectorA->modifier.empty())op1 += "." + connectorA->modifier;
        String op2 = connectorB->node->GetName();
        if (!connectorB->modifier.empty())op2 += "." + connectorB->modifier;
        
        node->nodeCode = Format("%s %s = %s * %s;",  resultFormat.c_str(),
                          node->name.c_str(),
                          op1.c_str(),
                          op2.c_str());
        Logger::Debug("%s", node->nodeCode.c_str());
        *destinationCode += node->nodeCode;
    }
    if (type == MaterialGraphNode::TYPE_ADD)
    {
        MaterialGraphNodeConnector * connectorA = node->GetInputConnector("a");
        MaterialGraphNodeConnector * connectorB = node->GetInputConnector("b");
        
        if (!connectorA || !connectorB)
            return ERROR_NOT_ENOUGH_CONNECTORS;
        String resultFormat = node->GetResultFormat(connectorA->modifier, connectorB->modifier);
        
        node->nodeCode = Format("%s %s = %s.%s + %s.%s;", resultFormat.c_str(),
                          node->name.c_str(),
                          connectorA->node->GetName().c_str(),
                          connectorA->modifier.c_str(),
                          connectorB->node->GetName().c_str(),
                          connectorB->modifier.c_str());
        
        Logger::Debug("%s", node->nodeCode.c_str());
        *destinationCode += node->nodeCode;
    }
    
    if (type == MaterialGraphNode::TYPE_FORWARD_MATERIAL)
    {
        // shader = new Shader();
        MaterialGraphNodeConnector * connectorEmissive = node->GetInputConnector("emissive");
        MaterialGraphNodeConnector * connectorDiffuse = node->GetInputConnector("diffuse");
        MaterialGraphNodeConnector * connectorSpecular = node->GetInputConnector("specular");
        MaterialGraphNodeConnector * connectorNormal = node->GetInputConnector("normal");
        //      MaterialGraphNodeConnector * connectorAlpha = GetInputConnector("alpha");        
        
        // vertexShader +=
        finalPixelShaderCode += "#define GRAPH_CUSTOM_PIXEL_CODE ";
        finalPixelShaderCode += pixelShaderCode;
        finalPixelShaderCode += "\n";

        finalVertexShaderCode += "#define GRAPH_CUSTOM_VERTEX_CODE ";
        finalVertexShaderCode += vertexShaderCode;
        finalVertexShaderCode += "\n";
        
        finalPixelShaderCode += Format("#define NUM_TEXTURES %d\n", materialGraph->GetUsedTextures());
        finalPixelShaderCode += Format("#define NUM_TEX_COORDS %d\n", materialGraph->GetUsedTextureCoordsCount());
        
        finalVertexShaderCode += Format("#define NUM_TEX_COORDS %d\n", materialGraph->GetUsedTextureCoordsCount());
        
        
        /*
         #define ATTRIBUTE_POSITION
         #define ATTRIBUTE_NORMAL
         #define ATTRIBUTE_TEX0
         #define ATTRIBUTE_TEX1
         #define ATTRIBUTE_TANGENT
         #define ATTRIBUTE_COLOR
         */
        
        if (polygonGroup->GetFormat() & EVF_VERTEX)
            finalVertexShaderCode += "#define ATTRIBUTE_POSITION\n";
        if (polygonGroup->GetFormat() & EVF_NORMAL)
            finalVertexShaderCode += "#define ATTRIBUTE_NORMAL\n";
        if (polygonGroup->GetFormat() & EVF_TEXCOORD0)
            finalVertexShaderCode += "#define ATTRIBUTE_TEX0\n";
        if (polygonGroup->GetFormat() & EVF_TEXCOORD1)
            finalVertexShaderCode += "#define ATTRIBUTE_TEX1\n";
        if (polygonGroup->GetFormat() & EVF_TEXCOORD2)
            finalVertexShaderCode += "#define ATTRIBUTE_TEX2\n";
        if (polygonGroup->GetFormat() & EVF_TEXCOORD3)
            finalVertexShaderCode += "#define ATTRIBUTE_TEX3\n";
        if (polygonGroup->GetFormat() & EVF_TANGENT)
            finalVertexShaderCode += "#define ATTRIBUTE_TANGENT\n";
        if (polygonGroup->GetFormat() & EVF_COLOR)
            finalVertexShaderCode += "#define ATTRIBUTE_COLOR\n";
        
        if (connectorEmissive)
            finalPixelShaderCode += Format("#define IN_EMISSIVE %s\n", connectorEmissive->GetNode()->GetName().c_str());
        if (connectorDiffuse)
            finalPixelShaderCode += Format("#define IN_DIFFUSE %s\n", connectorDiffuse->GetNode()->GetName().c_str());
        if (connectorSpecular)
            finalPixelShaderCode += Format("#define IN_SPECULAR %s\n", connectorSpecular->GetNode()->GetName().c_str());
        if (connectorNormal)
            finalPixelShaderCode += Format("#define IN_NORMAL %s\n", connectorNormal->GetNode()->GetName().c_str());
        
        if (connectorNormal)
        {
            finalPixelShaderCode += "#define PIXEL_LIT\n";
            finalVertexShaderCode += "#define PIXEL_LIT\n";
        }
        else if (connectorDiffuse || connectorSpecular)
        {
            finalPixelShaderCode += "#define VERTEX_LIT\n";
            finalVertexShaderCode += "#define VERTEX_LIT\n";
        }

        finalVertexShaderCode += "#define ADDITIONAL_UNIFORMS ";
        for (Map<String, Shader::eUniformType>::iterator it = vertexShaderAdditionaUniforms.begin(); it != vertexShaderAdditionaUniforms.end(); ++it)
        {
            finalVertexShaderCode += Format("uniform %s %s;", Shader::GetUniformTypeSLName(it->second), it->first.c_str());
        }
        finalVertexShaderCode += "\n";

        finalPixelShaderCode += "#define ADDITIONAL_UNIFORMS ";
        for (Map<String, Shader::eUniformType>::iterator it = pixelShaderAdditionaUniforms.begin(); it != pixelShaderAdditionaUniforms.end(); ++it)
        {
            finalPixelShaderCode += Format("uniform %s %s;", Shader::GetUniformTypeSLName(it->second), it->first.c_str());
        }
        finalPixelShaderCode += "\n";
        
        finalVertexShaderCode += "#define ADDITIONAL_VARYINGS ";
        finalPixelShaderCode += "#define ADDITIONAL_VARYINGS ";
        for (Map<String, Shader::eUniformType>::iterator it = additionalVaryings.begin(); it != additionalVaryings.end(); ++it)
        {
            finalVertexShaderCode += Format("varying %s %s;", Shader::GetUniformTypeSLName(it->second), it->first.c_str());
            finalPixelShaderCode += Format("varying %s %s;", Shader::GetUniformTypeSLName(it->second), it->first.c_str());
        }
        finalVertexShaderCode += "\n";
        finalPixelShaderCode += "\n";
    }

        
    return COMPILE_NO_ERROR;
}



};

