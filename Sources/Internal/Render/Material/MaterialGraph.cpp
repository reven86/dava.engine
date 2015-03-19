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


#include "Render/Material/MaterialGraph.h"
#include "Render/Material/MaterialGraphNode.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/YamlNode.h"

namespace DAVA
{

    
// MaterialGraph
MaterialGraph::MaterialGraph()
:   usedTextureCoordsCount(0)
,   usedTextures(0)
{
    
}

MaterialGraph::~MaterialGraph()
{
    RemoveAllNodes();
}

void MaterialGraph::RemoveAllNodes()
{
    for(auto& node : allNodes)
    {
        SafeRelease(node);
    }
    allNodes.clear();
}
    
void MaterialGraph::AddNode(MaterialGraphNode * node)
{
    allNodes.push_back(SafeRetain(node));
}
    
void MaterialGraph::RemoveNode(MaterialGraphNode * node)
{
    allNodes.erase(std::remove(allNodes.begin(), allNodes.end(), node));
    SafeRelease(node);
}
    
void MaterialGraph::RemoveNodeRecursive(MaterialGraphNode * node)
{
//    Map<String, MaterialGraphNodeConnector*> & inputConnectors = node->GetInputConnectors();
//    for (Map<String, MaterialGraphNodeConnector*>::iterator it = inputConnectors.begin(); it != inputConnectors.end(); ++it)
//    {
//        MaterialGraphNodeConnector * connector = it->second;
//        MaterialGraphNode * connectedNode = connector->node;
//        RemoveNodeRecursive(connectedNode);
//    }
    
    RemoveNode(node);
}

bool MaterialGraph::LoadFromFile(const FilePath & pathname)
{
    
    YamlParser * materialFileParser = YamlParser::Create(pathname);
    if (!materialFileParser)return false;
    
    materialPathname = pathname;
        
    
    const YamlNode * rootNode = materialFileParser->GetRootNode();
    const YamlNode * materialNode = rootNode->Get("material");
    
    const YamlNode * vertexShaderFileNode = materialNode->Get("vertexShader");
    vertexShaderFilename = vertexShaderFileNode->AsString();
    
    const YamlNode * pixelShaderFileNode = materialNode->Get("pixelShader");
    pixelShaderFilename = pixelShaderFileNode->AsString();
    
    const YamlNode * nodes = rootNode->Get("nodes");
    if (nodes && nodes->GetType() == YamlNode::TYPE_ARRAY)
    {
        for (uint32 k = 0; k < nodes->GetCount(); ++k)
        {
            const YamlNode * graphNode = nodes->Get(k);
            bool result = LoadNode(graphNode);
            if (!result)break;
        }
    }
    
    SafeRelease(materialFileParser);
    return true;
}

bool MaterialGraph::LoadNode(const YamlNode * graphNode)
{

    MaterialGraphNode * node = new MaterialGraphNode(this);
    node->InitFromYamlNode(graphNode);
    AddNode(node);
    
    usedTextureCoordsCount = Max(usedTextureCoordsCount, node->GetTextureInputIndex() + 1);
    
    if (node->GetType() == MaterialGraphNode::TYPE_SAMPLE_2D)
        usedTextures++;
    
    //YamlNode *
    const YamlNode * nameNode = graphNode->Get("name");
    const YamlNode * typeNode = graphNode->Get("node");
    Logger::Debug("- Read Node %s %s", typeNode->AsString().c_str(), nameNode->AsString().c_str());

    // Parse inputs
    const MultiMap<String, YamlNode*> & map = graphNode->AsMap();
    std::pair<MultiMap<String, YamlNode*>::const_iterator, MultiMap<String, YamlNode*>::const_iterator> inputs = map.equal_range("input");
    
    uint32 count = 0;
    for (MultiMap<String, YamlNode*>::const_iterator it = inputs.first; it != inputs.second; ++it)
    {
        const YamlNode * inputNode = it->second;
        const String & inputName = inputNode->Get(0)->AsString();
        const String & nodeName = inputNode->Get(1)->AsString();
        const String & connectionModifier = inputNode->Get(2)->AsString();
        
        MaterialGraphNode * connectNode = GetNodeByName(nodeName);
        if (connectNode)
        {
            Logger::Debug("- Read Input: %s connected:%s filter:%s",
                          inputNode->Get(0)->AsString().c_str(),
                          inputNode->Get(1)->AsString().c_str(),
                          inputNode->Get(2)->AsString().c_str());

            node->ConnectToNode(inputName, connectNode, connectionModifier);
            count++;
        }
    }
    SafeRelease(node);
    return true;
}
    
    
uint32 MaterialGraph::GetNodeCount() const
{
    return (uint32)allNodes.size();
}
MaterialGraphNode * MaterialGraph::GetNode(uint32 index) const
{
    return allNodes[index];
}
MaterialGraphNode * MaterialGraph::GetNodeByName(const String & name) const
{
    uint32 size = static_cast<uint32>(allNodes.size());
    for (uint32 k = 0; k < size; ++k)
    {
        if (allNodes[k]->GetName() == name)return allNodes[k];
    }
    return 0;
}

bool MaterialGraph::SortByDepthMarkerDescending(MaterialGraphNode * node1, MaterialGraphNode * node2)
{
    return (node1->GetDepthMarker() > node2->GetDepthMarker());
}

void MaterialGraph::SortByDepthMarkerAndRemoveUnused()
{
    for (Vector<MaterialGraphNode*>::iterator it = allNodes.begin(); it != allNodes.end();)
    {
        MaterialGraphNode * node = *it;
        if (node->GetDepthMarker() == static_cast<uint32>(-1))
        {
            Logger::Debug("- remove empty node: %s", node->GetName().c_str());
            RemoveNodeRecursive(node);
            it = allNodes.begin();
            continue;
        }
        ++it;
    }
    
    std::sort(allNodes.begin(), allNodes.end(), SortByDepthMarkerDescending);
}


};

