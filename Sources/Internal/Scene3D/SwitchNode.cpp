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


#include "Scene3D/SwitchNode.h"
#include "FileSystem/KeyedArchive.h"

namespace DAVA
{


SwitchNode::SwitchNode()
:	oldSwitchIndex(0),
	newSwitchIndex(0)
{

}

Entity* SwitchNode::Clone(Entity *dstNode /*= NULL*/)
{
	if (!dstNode) 
	{
		dstNode = new SwitchNode();
	}

	return Entity::Clone(dstNode);
}

void SwitchNode::SetSwitchIndex(int32 _switchIndex)
{
	newSwitchIndex = _switchIndex;
}

int32 SwitchNode::GetSwitchIndex()
{
	return newSwitchIndex;
}

void SwitchNode::Update(float32 timeElapsed)
{
	if(oldSwitchIndex != newSwitchIndex)
	{
		int32 childrenCound = GetChildrenCount();
		for(int32 i = 0; i < childrenCound; ++i)
		{
			GetChild(i)->SetUpdatable(newSwitchIndex == i);
		}

		oldSwitchIndex = newSwitchIndex;
	}
}

void SwitchNode::AddNode(Entity * node)
{
	Entity::AddNode(node);

	ReapplySwitch();
}

void SwitchNode::ReapplySwitch()
{
	oldSwitchIndex = -1;
}

void SwitchNode::Save(KeyedArchive * archive, SerializationContext * serializationContext)
{
	Entity::Save(archive, serializationContext);

	archive->SetInt32("switchIndex", newSwitchIndex);
}

void SwitchNode::Load(KeyedArchive * archive, SerializationContext * serializationContext)
{
	Entity::Load(archive, serializationContext);

	int32 loadedSwitchIndex = archive->GetInt32("switchIndex");
	SetSwitchIndex(loadedSwitchIndex);
}

}
