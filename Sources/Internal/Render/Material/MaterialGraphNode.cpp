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

#include "Render/Material/MaterialGraphNode.h"
#include "Render/Material/MaterialGraph.h"

#include "FileSystem/YamlParser.h"
#include "FileSystem/YamlNode.h"
#include "FileSystem/FileSystem.h"
#include "Utils/StringFormat.h"
#include "Render/Shader.h"

namespace DAVA
{

/*
    Code for some of the nodes:

    Mul:
    var [name] = [a] * [b];
    
    Add:
    var [name] = [a] + [b];
    
    Lerp:
    var [name] = lerp([a], [b], [t]);
 
    <ambient diffuse>
    </>
    
    <diffuse> <diffuse>
 
 */
    
MaterialGraphNode::MaterialGraphNode(MaterialGraph * _graph)
    :   type(TYPE_NONE)
    ,   graph(_graph)
    ,   depthMarker(-1)
    ,   isVertexShaderNode(false)
    ,   usedByOthersModifier(0)
    ,   textureInputIndex(0)
    ,   textureChannelIndex(0)
    ,   shader(0)
{
}
    
MaterialGraphNode::~MaterialGraphNode()
{
    for (Map<String, MaterialGraphNodeConnector*>::iterator it = inputConnectors.begin(); it != inputConnectors.end(); ++it)
    {
        SafeRelease(it->second);
    }
    inputConnectors.clear();
}
    
void MaterialGraphNode::InitFromYamlNode(const YamlNode * graphNode)
{
    const YamlNode * typeNode = graphNode->Get("node");
    const YamlNode * nameNode = graphNode->Get("name");
    SetType(typeNode->AsString());
    SetName(nameNode->AsString());
        
    usage = types[type].usage;
    DVASSERT(usage == USE_VERTEX || usage == USE_BOTH || usage == USE_PIXEL);
    
    if (type == TYPE_SAMPLE_2D)
    {
        const YamlNode * textureChannelNode = graphNode->Get("channel");
        if (textureChannelNode)
            textureChannelIndex = textureChannelNode->AsInt();
    }
    if (type == TYPE_TEX_COORD_INPUT)
    {
        const YamlNode * inputNode = graphNode->Get("input");
        if (inputNode)
            textureInputIndex = inputNode->AsInt();
    }
    if (type == TYPE_CONST)
    {
        const YamlNode * valueNode = graphNode->Get("value");
        if (valueNode->GetType() == YamlNode::TYPE_STRING)
        {
            constValue.SetFloat(valueNode->AsFloat());
            returnType = Shader::UT_FLOAT;
        }
        else if (valueNode->GetType() == YamlNode::TYPE_ARRAY)
        {
            switch (valueNode->GetCount())
            {
                case 2:
                    constValue.SetVector2(valueNode->AsVector2());
                    returnType = Shader::UT_FLOAT_VEC2;
                    break;
                case 3:
                    constValue.SetVector3(valueNode->AsVector3());
                    returnType = Shader::UT_FLOAT_VEC3;
                    break;
                case 4:
                    constValue.SetVector4(valueNode->AsVector4());
                    returnType = Shader::UT_FLOAT_VEC4;
                    break;
                default:
                    break;
            }
        }
    }
}

void MaterialGraphNode::SetDepthMarker(uint32 _depthMarker)
{
    depthMarker = _depthMarker;
}
    
uint32 MaterialGraphNode::GetDepthMarker()
{
    return depthMarker;
}

Map<String, MaterialGraphNodeConnector*> & MaterialGraphNode::GetInputConnectors()
{
    return inputConnectors;
}

    
void MaterialGraphNode::ConnectToNode(const String & connectorName, MaterialGraphNode * node, const String & connectionModifier)
{
    MaterialGraphNodeConnector * connector = new MaterialGraphNodeConnector();
    connector->node = node;
    connector->modifier = connectionModifier;
    inputConnectors[connectorName] = connector;
    node->MergeConnectionModifiers(connectionModifier);
}
    
uint32 MaterialGraphNode::StringToRGBAModifier(const String & input)
{
    uint32 result = 0;
    for (uint32 k = 0; k < input.length(); ++k)
    {
        if (input[k] == 'r')result |= VAR_MODIFIER_R;
        if (input[k] == 'g')result |= VAR_MODIFIER_G;
        if (input[k] == 'b')result |= VAR_MODIFIER_B;
        if (input[k] == 'a')result |= VAR_MODIFIER_A;
    }
    return result;
}
    
String MaterialGraphNode::RGBAModifierToString(uint32 modifier)
{
    String result;
    if (modifier & VAR_MODIFIER_R)
        result += 'r';
    if (modifier & VAR_MODIFIER_G)
        result += 'g';
    if (modifier & VAR_MODIFIER_B)
        result += 'b';
    if (modifier & VAR_MODIFIER_A)
        result += 'a';
    
    return result;
}
    
static const char * returnTypes[] =
{
    "wrong type",
    "float",
    "vec2",
    "vec3",
    "vec4",
};
    
String MaterialGraphNode::RGBAModifierToType(uint32 modifier)
{
    uint32 bc = 0;
    if (modifier & VAR_MODIFIER_R)
        bc++;
    if (modifier & VAR_MODIFIER_G)
        bc++;
    if (modifier & VAR_MODIFIER_B)
        bc++;
    if (modifier & VAR_MODIFIER_A)
        bc++;
    
    return returnTypes[bc];
}

    
void MaterialGraphNode::MergeConnectionModifiers(const String & usedByOtherNode)
{
    uint32 otherNodeModifier = StringToRGBAModifier(usedByOtherNode);
    
    usedByOthersModifier |= otherNodeModifier;
}

MaterialGraphNodeConnector * MaterialGraphNode::GetInputConnector(const String & name)
{
    Map<String, MaterialGraphNodeConnector*>::iterator res = inputConnectors.find(name);
    if (res != inputConnectors.end())
        return res->second;
    return 0;
}

String MaterialGraphNode::GetResultFormat(const String & s1, const String & s2)
{
    uint32 s1Len = static_cast<uint32>(s1.length());
    uint32 s2Len = static_cast<uint32>(s2.length());
    if ((s1Len == 1) || (s2Len == 1))
    {
        uint32 formatBytes = Max(s1Len, s2Len);
        return returnTypes[formatBytes];
    }else if ((s1Len == 0) || (s2Len == 0))
    {
        uint32 formatBytes = Max(s1Len, s2Len);
        return returnTypes[formatBytes];
    }
    else if (s1Len == s2Len)return returnTypes[s1Len];
    return returnTypes[0];
}

const String & MaterialGraphNode::GetName() const
{
    return name;
}
    
void MaterialGraphNode::SetName(const String & _name)
{
    name = _name;
}

MaterialGraphNode::TypeUsageStruct MaterialGraphNode::types[] =
{
    {"TYPE_NONE", MaterialGraphNode::USE_BOTH},
    {"TYPE_FORWARD_MATERIAL", MaterialGraphNode::USE_PIXEL},
    {"TYPE_DEFERRED_MATERIAL", MaterialGraphNode::USE_PIXEL},
    {"TYPE_SAMPLE_2D", MaterialGraphNode::USE_PIXEL},
    {"TYPE_MUL", MaterialGraphNode::USE_BOTH},
    {"TYPE_ADD", MaterialGraphNode::USE_BOTH},
    {"TYPE_LERP", MaterialGraphNode::USE_BOTH},
    {"TYPE_TIME", MaterialGraphNode::USE_BOTH},
    {"TYPE_SIN", MaterialGraphNode::USE_BOTH},
    {"TYPE_COS", MaterialGraphNode::USE_BOTH},
    {"TYPE_CONST", MaterialGraphNode::USE_BOTH},
    {"TYPE_ROTATOR", MaterialGraphNode::USE_BOTH},
    {"TYPE_SHIFTER", MaterialGraphNode::USE_BOTH},
    {"TYPE_TEX_COORD_INPUT", MaterialGraphNode::USE_VERTEX},
};
    
void MaterialGraphNode::SetType(const String & _type)
{
    for (uint32 k = 0; k < TYPE_COUNT; ++k)
    {
        if (_type == types[k].name)
        {
            type = (eType)k;
            return;
        }
    }
    DVASSERT(0 && "MaterialGraphNode type not found");
}
    
void MaterialGraphNode::RecursiveSetRealUsageForward(MaterialGraphNode * node)
{
    Map<String, eNodeUsage> resultUsage;
    Map<String, MaterialGraphNodeConnector*> & inputConnectors = node->GetInputConnectors();
    
    for (Map<String, MaterialGraphNodeConnector*>::iterator it = inputConnectors.begin(); it != inputConnectors.end(); ++it)
    {
        MaterialGraphNodeConnector * connector = it->second;
        MaterialGraphNode * connectedNode = connector->node;
        if (connectedNode)
        {
            RecursiveSetRealUsageForward(connectedNode);
            
            if ((connectedNode->usage == USE_BOTH) && (node->usage != USE_BOTH))
                connectedNode->usage = node->usage;
        }
    }
}


MaterialGraphNode::eNodeUsage MaterialGraphNode::RecursiveSetRealUsageBack(MaterialGraphNode * node)
{
    
    // DEBUG
    //Logger::Debug("eval node: %s", node->GetName().c_str());

    //Map<String, eNodeUsage> resultUsage;
    Map<String, MaterialGraphNodeConnector*> & inputConnectors = node->GetInputConnectors();
    
    // return node usage if it does not have connectors
    if (inputConnectors.size() == 0)
    {
        return node->usage;
    }
    
    bool hasPixelShaderNode = false;
    bool hasVertexShaderNode = false;
    
    for (Map<String, MaterialGraphNodeConnector*>::iterator it = inputConnectors.begin(); it != inputConnectors.end(); ++it)
    {
        MaterialGraphNodeConnector * connector = it->second;
        MaterialGraphNode * connectedNode = connector->node;
        if (connectedNode)
        {
            eNodeUsage result = RecursiveSetRealUsageBack(connectedNode);
            if (result == USE_PIXEL)
            {
                hasPixelShaderNode = true;
            
            }else if (result == USE_VERTEX)
            {
                hasVertexShaderNode = true;
            }
        }
    }
    
    if (node->usage == USE_PIXEL || node->usage == USE_VERTEX)return node->usage;
    
    // usage is the same
//    int32 index = 0;
//    eNodeUsage firstInputUsage = USE_BOTH;
//    for (Map<String, eNodeUsage>::iterator it = resultUsage.begin(); it != resultUsage.end(); ++it)
//    {
//        if (index == 0)firstInputUsage = it->second;
//        else if (firstInputUsage != it->second)
//            firstInputUsage = USE_BOTH;
//    }
    
    // we run mechanism only for nodes where we are not sure that
    // node are in vertex shader or in pixel shader
    DVASSERT(node->usage == USE_BOTH);

    if (hasPixelShaderNode)node->usage = USE_PIXEL;
    else if (hasVertexShaderNode)node->usage = USE_VERTEX;
    
//    if (firstInputUsage != USE_BOTH)
//        node->usage = firstInputUsage;
//    DVASSERT(firstInputUsage != USE_BOTH);
    return node->usage;
}

};

#endif // RHI_COMPLETE