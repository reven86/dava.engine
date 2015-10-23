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


#include "Render/3D/StaticMesh.h"
#include "Scene3D/DataNode.h"
#include "Scene3D/Scene.h"
#include "FileSystem/Logger.h"
#include "Render/Shader.h"
#include "Scene3D/SceneFileV2.h"

namespace DAVA 
{
    
StaticMesh::StaticMesh(Scene * _scene)
	: DataNode()
{
//    if (scene)
//    {
//        DataNode * staticMeshes = scene->GetStaticMeshes();
//        staticMeshes->AddNode(this);
//    }
}
    
void StaticMesh::SetScene(Scene * _scene)
{
    DVASSERT(scene == 0);
    scene = _scene;
//    if (scene)
//    {
//        DataNode * staticMeshes = scene->GetStaticMeshes();
//        staticMeshes->AddNode(this);
//    }
}
    
int32 StaticMesh::Release()
{
    int32 retainCount = BaseObject::Release();
//    if (retainCount == 1)
//    {
//        DataNode * staticMeshes = scene->GetStaticMeshes();
//        staticMeshes->RemoveNode(this);
//    }
    return retainCount;
}
	
StaticMesh::~StaticMesh()
{
    
}   
    
void StaticMesh::AddNode(DataNode * node)
{
    PolygonGroup * group = dynamic_cast<PolygonGroup*>(node);
    DVASSERT(group != 0);
    if (group)
    {
        group->Retain();
        children.push_back(node);
    }
}
	
uint32 StaticMesh::GetPolygonGroupCount()
{
    return (uint32)children.size();
}

PolygonGroup * StaticMesh::GetPolygonGroup(uint32 index)
{
    return reinterpret_cast<PolygonGroup*>(children[index]);
}

};