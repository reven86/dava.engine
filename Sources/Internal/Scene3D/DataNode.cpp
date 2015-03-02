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


#include "Scene3D/DataNode.h"
#include "FileSystem/KeyedArchive.h"
#include "Scene3D/SceneFileV2.h"

namespace DAVA 
{

const uint16 DataNode::NodeRuntimeFlag = 1;
    
    
DataNode::DataNode()
:   pointer(0),
    scene(0),
    index(-1),
    nodeFlags(0)
{
}

DataNode::~DataNode()
{
    //RemoveAllChildren();
}
    
int32 DataNode::Release()
{
    int32 retainCount = BaseObject::Release();
    return retainCount;
}

    
void DataNode::SetScene(Scene * _scene)
{
    DVASSERT(scene == 0 || scene == _scene);
    scene = _scene;
}
  
void DataNode::AddNode(DataNode * node)
{
    DVASSERT(false && "DataNode::AddNode is deprecated!");
}

int32  DataNode::GetNodeIndex()
{
    return index;
}

uint64 DataNode::GetPreviousPointer()
{
    return pointer;
}

    
void DataNode::Load(KeyedArchive * archive, SerializationContext * serializationContext)
{
    BaseObject::LoadObject(archive);
    
    index = archive->GetInt32("#index", -1);
    pointer = archive->GetByteArrayAsType("#id", (uint64)0);
}

void DataNode::Save(KeyedArchive * archive, SerializationContext * serializationContext)
{
    BaseObject::SaveObject(archive);
    archive->SetInt32("#index", index);
    
    pointer = (uint64)this;
    archive->SetByteArrayAsType("#id", pointer);
}



}