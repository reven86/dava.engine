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

#ifndef __DAVAENGINE_MATERIAL_GRAPH_H__
#define __DAVAENGINE_MATERIAL_GRAPH_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Scene3D/DataNode.h"
#include "FileSystem/YamlParser.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{

class MaterialGraphNode;
class NMaterial;

class MaterialGraph : public BaseObject
{
protected:
    ~MaterialGraph();
public:
    MaterialGraph();
    
    bool LoadFromFile(const FilePath & pathname);
    bool LoadNode(const YamlNode * graphNode);
    
    void SortByDepthMarkerAndRemoveUnused();
    
    void RemoveAllNodes();
    void AddNode(MaterialGraphNode * node);
    void RemoveNode(MaterialGraphNode * node);
    void RemoveNodeRecursive(MaterialGraphNode * node);

    
    uint32 GetNodeCount() const;
    MaterialGraphNode * GetNode(uint32 index) const;
    MaterialGraphNode * GetNodeByName(const String & name) const;
    
    uint32 GetUsedTextureCoordsCount() { return usedTextureCoordsCount; };
    uint32 GetUsedTextures() { return usedTextures; };
    
    const FilePath & GetMaterialPathname() const { return materialPathname; };
    
    const String & GetVertexShaderFilename() const { return vertexShaderFilename; };
    const String & GetPixelShaderFilename() const { return pixelShaderFilename; };
    
protected:
    FilePath materialPathname;
    
    String vertexShaderFilename;
    String pixelShaderFilename;
    
    static bool SortByDepthMarkerDescending(MaterialGraphNode * node1, MaterialGraphNode * node2);
    Vector<MaterialGraphNode*> allNodes;
    uint32 usedTextureCoordsCount;
    uint32 usedTextures;
};

};

#endif // __DAVAENGINE_MATERIAL_GRAPH_H__

#endif // RHI_COMPLETE