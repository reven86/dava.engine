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


#include "Scene3D/ParticleEmitterNode.h"
#include "Scene3D/Scene.h"
#include "Scene3D/SceneFileV2.h"


namespace DAVA
{


ParticleEmitterNode::ParticleEmitterNode()
:	emitter(0)
{
	SetName("Particle Emitter");
}

ParticleEmitterNode::~ParticleEmitterNode()
{
	SafeRelease(emitter);
}

void ParticleEmitterNode::Update(float32 timeElapsed)
{
	// Yuri Coder, 2013/04/10. This method isn't called anymore.
}

void ParticleEmitterNode::Draw()
{
	// Yuri Coder, 2013/04/10. This method isn't called anymore.
}

void ParticleEmitterNode::LoadFromYaml(const FilePath& _yamlPath)
{
	yamlPath = _yamlPath;
	SafeRelease(emitter);
	emitter = new ParticleEmitter();
	emitter->LoadFromYaml(yamlPath);
}

ParticleEmitter * ParticleEmitterNode::GetEmitter()
{
	return emitter;
}

Entity* ParticleEmitterNode::Clone(Entity *dstNode /*= NULL*/)
{
	if (!dstNode) 
	{
		DVASSERT_MSG(IsPointerToExactClass<ParticleEmitterNode>(this), "Can clone only ParticleEmitterNode");
		dstNode = new ParticleEmitterNode();
	}

	Entity::Clone(dstNode);
	ParticleEmitterNode *nd = (ParticleEmitterNode *)dstNode;

	nd->yamlPath = yamlPath;
	nd->LoadFromYaml(yamlPath);

	return dstNode;
}

void ParticleEmitterNode::Save(KeyedArchive * archive, SerializationContext * serializationContext)
{
	Entity::Save(archive, serializationContext);

	archive->SetString("yamlPath", yamlPath.GetRelativePathname(serializationContext->GetScenePath()));
}

void ParticleEmitterNode::Load(KeyedArchive * archive, SerializationContext * serializationContext)
{
	Entity::Load(archive, serializationContext);
	
	String path = archive->GetString("yamlPath");
	yamlPath = serializationContext->GetScenePath() + path;
	LoadFromYaml(yamlPath);
}

void ParticleEmitterNode::GetDataNodes(Set<DataNode*> & dataNodes)
{
	//VI: NMaterial is not a DataNode anymore
	/*if(emitter)
	{
		int32 layersCount = emitter->GetLayers().size();
		for(int32 i = 0; i < layersCount; ++i)
		{
			ParticleLayer3D * layer = dynamic_cast<ParticleLayer3D*>(emitter->GetLayers()[i]);
			dataNodes.insert(layer->GetMaterial());
		}
	}*/
	

	Entity::GetDataNodes(dataNodes);
}

};

