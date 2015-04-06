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

#ifndef __DAVAENGINE_MATERIAL_GRAPH_NODE_H__
#define __DAVAENGINE_MATERIAL_GRAPH_NODE_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Scene3D/DataNode.h"
#include "FileSystem/YamlParser.h"
#include "Render/Shader.h"
#include "FileSystem/VariantType.h"

namespace DAVA
{

class MaterialGraph;
class MaterialGraphNode;
class YamlNode;
class Shader;
    
class MaterialGraphNodeConnector : public BaseObject
{
protected:
    ~MaterialGraphNodeConnector(){}
public:
    MaterialGraphNode * GetNode() { return node; }
    
    MaterialGraphNode * node;
    String modifier;
};

class MaterialGraphNode : public BaseObject
{
public:
    enum eType
    {
        TYPE_NONE = 0,
        TYPE_FORWARD_MATERIAL,
        TYPE_DEFERRED_MATERIAL,
        TYPE_SAMPLE_2D,
        TYPE_MUL,
        TYPE_ADD,
        TYPE_LERP,
        TYPE_TIME,
        TYPE_SIN,
        TYPE_COS,
        TYPE_CONST,
        TYPE_ROTATOR,
        TYPE_SHIFTER,
        TYPE_TEX_COORD_INPUT,
        
        TYPE_COUNT,
    };
    
    enum eNodeUsage
    {
        USE_VERTEX = 0,
        USE_PIXEL = 1,
        USE_BOTH = 2,
    };
protected:
    ~MaterialGraphNode();
public:
    MaterialGraphNode(MaterialGraph * graph);
    
    void InitFromYamlNode(const YamlNode * graphNode);
    
    void SetDepthMarker(uint32 depthMarker);
    uint32 GetDepthMarker();
    
    MaterialGraphNodeConnector * GetInputConnector(const String & name);
    Map<String, MaterialGraphNodeConnector*> & GetInputConnectors();
    void ConnectToNode(const String & inputName, MaterialGraphNode * node, const String & connectionModifier);

    uint32 GetOutputCount();
    uint32 GetConstCount() const;
    
    const String & GetName() const;
    void SetName(const String & name);
    void SetType(const String & type);
    eType GetType() { return type; };
    
    void SetUsage(eNodeUsage _usage) { usage = _usage; };
    eNodeUsage GetUsage() const { return usage; };
    
    void MergeConnectionModifiers(const String & usedByOtherNode);
    
    uint32 GetTextureInputIndex() { return textureInputIndex; };
    
    //const String & GetVertexShaderCode() { return vertexShaderCode; }
    //const String & GetFragmentShaderCode() { return fragmentShaderCode; }
    
    /*
        Here we try to set nodes shader according to the input connectors. So firstly from vertex shader => pixel shader. 
     */
    static MaterialGraphNode::eNodeUsage RecursiveSetRealUsageBack(MaterialGraphNode * node);
    /*
        Here we mark remaining nodes according to the nodes we try to use values in. For example const nodes.
        If const is used in pixel shader, we mark it for pixel shader, if in vertex shader we mark if for vertex shader.
     */
    static void RecursiveSetRealUsageForward(MaterialGraphNode * node);

protected:
    String GetResultFormat(const String & s1, const String & s2);

    struct TypeUsageStruct
    {
        const char * name;
        MaterialGraphNode::eNodeUsage usage;
    };
    
    eType type;
    eNodeUsage usage;
    MaterialGraph * graph;
    Shader::eUniformType returnType;
    VariantType constValue;
    

    uint32 depthMarker;
    String name;
    bool isVertexShaderNode;
    //String usedByOthersModifier;
    
    enum
    {
        VAR_MODIFIER_R = 1,
        VAR_MODIFIER_G = 2,
        VAR_MODIFIER_B = 4,
        VAR_MODIFIER_A = 8,
    };
    
    uint32 StringToRGBAModifier(const String & input);
    uint32 usedByOthersModifier;
    String RGBAModifierToString(uint32 modifier);
    String RGBAModifierToType(uint32 modifier);
    
    uint32 textureInputIndex;
    uint32 textureChannelIndex;
    Shader * shader;
    Map<String, MaterialGraphNodeConnector*> inputConnectors;
    
    String nodeCode;
    String nodeGenVarying;
//    String vertexShaderCode;
//    String fragmentShaderCode;
    
    static TypeUsageStruct types[];
    
    friend class MaterialCompiler;
};

};

#endif // __DAVAENGINE_MATERIAL_GRAPH_NODE_H__

#endif // RHI_COMPLETE