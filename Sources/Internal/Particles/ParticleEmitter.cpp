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


#include "Particles/ParticleEmitter.h"
#include "Particles/Particle.h"
#include "Particles/ParticleLayer.h"
#include "Particles/ParticleLayer3D.h"
#include "Render/RenderManager.h"
#include "Utils/Random.h"
#include "Utils/StringFormat.h"
#include "Animation/LinearAnimation.h"
#include "Scene3D/Scene.h"
#include "FileSystem/FileSystem.h"
#include "Scene3D/SceneFileV2.h"

namespace DAVA 
{

#define PARTICLE_EMITTER_DEFAULT_LIFE_TIME 100.0f
#define PARTICLE_EMITTER_DEFERRED_UPDATE_INTERVAL 0.1f // in seconds


Map<String, ParticleEmitter::EmitterYamlCacheEntry> ParticleEmitter::emitterYamlCache;

ParticleEmitter::ParticleEmitter()
{
	type = TYPE_PARTICLE_EMTITTER;
	Cleanup(false);

	bbox = AABBox3(Vector3(), Vector3());
	parentParticle = NULL;
	deferredTimeElapsed = 0.0f;

	time = 0.0f;
	repeatCount = 0;
	currentLodLevel = 0;
	desiredLodLevel = 0;
	shortEffect = false;
	lodLevelLocked = false;
	particleCount = 0;		
	state = STATE_STOPPED;	
}

ParticleEmitter::~ParticleEmitter()
{
	CleanupLayers();
	ReleaseFromCache(emitterFileName);
}

void ParticleEmitter::Cleanup(bool needCleanupLayers)
{
	emitterType = EMITTER_POINT;
	emissionVector.Set(NULL);
	emissionVector = RefPtr<PropertyLineValue<Vector3> >(new PropertyLineValue<Vector3>(Vector3(1.0f, 0.0f, 0.0f)));
	emissionAngle.Set(NULL);
	emissionAngle = RefPtr<PropertyLineValue<float32> >(new PropertyLineValue<float32>(0.0f));
	emissionRange.Set(NULL);
	emissionRange = RefPtr<PropertyLineValue<float32> >(new PropertyLineValue<float32>(360.0f));
	size = RefPtr<PropertyLineValue<Vector3> >(0);
	colorOverLife = 0;
	radius = 0;

	// number = new PropertyLineValue<float>(1.0f);
	particleCount = 0;
	time = 0.0f;
	repeatCount = 0;
	lodLevelLocked = false;
	currentLodLevel = 0;
	desiredLodLevel = 0;
	lifeTime = PARTICLE_EMITTER_DEFAULT_LIFE_TIME;
	emitPointsCount = -1;
	isPaused = false;
	angle = 0.f;
	isAutorestart = true;
	particlesFollow = false;
    is3D = false;
	playbackSpeed = 1.0f;
	shouldBeDeleted = false;
	// Also cleanup layers, if needed.
	if (needCleanupLayers)
	{
		CleanupLayers();
	}
}

void ParticleEmitter::CleanupLayers()
{
    while(!layers.empty())
    {
        RemoveLayer(layers[0]);
    }
}

//ParticleEmitter * ParticleEmitter::Clone()
//{
//	ParticleEmitter * emitter = new ParticleEmitter();
//	for (int32 k = 0; k < (int32)layers.size(); ++k)
//	{
//		ParticleLayer * newLayer = layers[k]->Clone();
//		newLayer->SetEmitter(emitter);
//		emitter->layers.push_back(newLayer);
//	}
//    emitter->emissionVector = emissionVector;
//	if (emissionAngle)
//		emitter->emissionAngle = emissionAngle->Clone();
//	if (emissionRange)
//		emitter->emissionRange = emissionRange->Clone();
//	if(colorOverLife)
//		emitter->colorOverLife = colorOverLife->Clone();
//	if (radius)
//		emitter->radius = radius->Clone();
//    if (size)
//        emitter->size = size->Clone();
//	
//	emitter->type = type;
//	emitter->lifeTime = lifeTime;
//	emitter->emitPointsCount = emitPointsCount;
//	emitter->isPaused = isPaused;
//	emitter->isAutorestart = isAutorestart;
//	emitter->particlesFollow = particlesFollow;
//	return emitter;
//}

RenderObject * ParticleEmitter::Clone(RenderObject *newObject)
{
	if(!newObject)
	{
		DVASSERT_MSG(IsPointerToExactClass<ParticleEmitter>(this), "Can clone only ParticleEmitter");
		newObject = new ParticleEmitter();
	}

	((ParticleEmitter*)newObject)->LoadFromYaml(configPath);

	return newObject;
}

void ParticleEmitter::Save(KeyedArchive *archive, SceneFileV2 *sceneFile)
{
	RenderObject::Save(archive, sceneFile);

	if(NULL != archive)
	{
        String filename = configPath.GetRelativePathname(sceneFile->GetScenePath());
		archive->SetString("pe.configpath", filename);
	}
}

void ParticleEmitter::Load(KeyedArchive *archive, SceneFileV2 *sceneFile)
{
	RenderObject::Load(archive, sceneFile);

	if(NULL != archive)
	{
		String filename = archive->GetString("pe.configpath");
		if(!filename.empty())
		{
			configPath = sceneFile->GetScenePath() + filename;
			LoadFromYaml(configPath);
		}
	}
}

void ParticleEmitter::AddLayer(ParticleLayer * layer)
{
	if (!layer)
	{
		return;
	}

	// Don't allow the same layer to be added twice.
	Vector<DAVA::ParticleLayer*>::iterator layerIter = std::find(layers.begin(), layers.end(),
																 layer);
	if (layerIter != layers.end())
	{
		DVASSERT(false);
		return;
	}

	layer->Retain();
	if (layer->GetEmitter())
	{
		layer->GetEmitter()->RemoveLayer(layer);
	}
		
	layers.push_back(layer);
	layer->SetEmitter(this);
	AddRenderBatch(layer->GetRenderBatch());
}

void ParticleEmitter::InsertLayer(ParticleLayer * layer, ParticleLayer * beforeLayer)
{
	AddLayer(layer);
	if (beforeLayer)
	{
		MoveLayer(layer, beforeLayer);
	}
}
	
void ParticleEmitter::RemoveLayer(ParticleLayer * layer)
{
	if (!layer)
	{
		return;
	}

	Vector<DAVA::ParticleLayer*>::iterator layerIter = std::find(layers.begin(), layers.end(), layer);
	if (layerIter != this->layers.end())
	{
		layer->RemoveInnerEmitter();
		layers.erase(layerIter);

        RemoveRenderBatch(layer->GetRenderBatch());
		layer->SetEmitter(NULL);
		SafeRelease(layer);
	}
}
    
void ParticleEmitter::RemoveLayer(int32 index)
{
    DVASSERT(0 <= index && index < (int32)layers.size());
    RemoveLayer(layers[index]);
}

	
void ParticleEmitter::MoveLayer(ParticleLayer * layer, ParticleLayer * beforeLayer)
{
	Vector<DAVA::ParticleLayer*>::iterator layerIter = std::find(layers.begin(), layers.end(), layer);
	Vector<DAVA::ParticleLayer*>::iterator beforeLayerIter = std::find(layers.begin(), layers.end(),beforeLayer);

	if (layerIter == layers.end() || beforeLayerIter == layers.end() ||
		layerIter == beforeLayerIter)
	{
		return;
	}
		
	layers.erase(layerIter);

	// Look for the position again - an iterator might be changed.
	beforeLayerIter = std::find(layers.begin(), layers.end(), beforeLayer);
	layers.insert(beforeLayerIter, layer);
}

ParticleLayer* ParticleEmitter::GetNextLayer(ParticleLayer* layer)
{
	if (!layer || layers.size() < 2)
	{
		return NULL;
	}

	int32 layersToCheck = layers.size() - 1;
	for (int32 i = 0; i < layersToCheck; i ++)
	{
		ParticleLayer* curLayer = layers[i];
		if (curLayer == layer)
		{
			return layers[i + 1];
		}
	}
	
	return NULL;
}

/* float32 ParticleEmitter::GetCurrentNumberScale()
{
	return number->GetValue(time);
} */

void ParticleEmitter::SetDesiredLodLevel(int32 level)
{
	desiredLodLevel = level;
	if (!lodLevelLocked){
		currentLodLevel = desiredLodLevel;
	}
}

bool ParticleEmitter::IsShortEffect()
{
	return shortEffect;
}
void ParticleEmitter::SetShortEffect(bool isShort)
{
	shortEffect = isShort;
	if (!isShort)
	{
		lodLevelLocked = false; //once effect is not considered short anymore - unlock lod
		currentLodLevel = desiredLodLevel;
	}
}

Matrix3 ParticleEmitter::GetRotationMatrix()
{
	Matrix4 * worldTransformPtr = GetWorldTransformPtr();
	Matrix3 rotationMatrix;	

	if(worldTransformPtr)
	{	
		rotationMatrix = Matrix3(*worldTransformPtr);
	}
	else
	{
		rotationMatrix.Identity();
	}

	return rotationMatrix;
}

void ParticleEmitter::Pause(bool _isPaused)
{
	isPaused = _isPaused;

	// Also update Inner Emitters.
	int32 layersCount = layers.size();
	for (int32 i = 0; i < layersCount; i ++)
	{
		layers[i]->PauseInnerEmitter(_isPaused);
	}
}

void ParticleEmitter::DoRestart(bool isDeleteAllParticles)
{
	Vector<ParticleLayer*>::iterator it;
	for(it = layers.begin(); it != layers.end(); ++it)
	{
		(*it)->Restart(isDeleteAllParticles);
	}

	time = 0.0f;
	repeatCount = 0;
	lodLevelLocked = false;
	currentLodLevel = desiredLodLevel;
}

void ParticleEmitter::Play()
{
	isPaused = false;
	state = STATE_PLAYING;
	int32 layersCount = layers.size();
	for (int32 i = 0; i < layersCount; i ++)
	{
		layers[i]->PauseInnerEmitter(false);
	}
    /*Pause(false);
    DoRestart(false);*/
}
    
void ParticleEmitter::Stop(bool isDeleteAllParticles)
{
	state = STATE_STOPPING;
	time = 0.0f;
	repeatCount = 0;
	lodLevelLocked = false;
	currentLodLevel = desiredLodLevel;
	Vector<ParticleLayer*>::iterator it;
	for(it = layers.begin(); it != layers.end(); ++it)
	{
		(*it)->Restart(isDeleteAllParticles);
	}	

}

void ParticleEmitter::Restart(bool isDeleteAllParticles)
{
	state = STATE_PLAYING;
	isPaused = false;
	time = 0.0f;
	repeatCount = 0;
	lodLevelLocked = false;
	currentLodLevel = desiredLodLevel;
	Vector<ParticleLayer*>::iterator it;
	for(it = layers.begin(); it != layers.end(); ++it)
	{
		(*it)->Restart(isDeleteAllParticles);
	}	
}
    
bool ParticleEmitter::IsPaused()
{
	return isPaused;
}

bool ParticleEmitter::IsStopped()
{    
    return (state == STATE_STOPPED);
}
	

bool ParticleEmitter::DeferredUpdate(float32 timeElapsed)
{
	deferredTimeElapsed += timeElapsed;
	if (deferredTimeElapsed > PARTICLE_EMITTER_DEFERRED_UPDATE_INTERVAL)
	{
		Update(deferredTimeElapsed);
		deferredTimeElapsed = 0.0f;
		return true;
	}	
	return false;	
}
	
void ParticleEmitter::Update(float32 timeElapsed)
{	
	timeElapsed *= playbackSpeed;

	
	time += timeElapsed;
	float32 t = time / lifeTime;

	if (colorOverLife)
	{
		currentColor = colorOverLife->GetValue(t);
	}

	if(isAutorestart && (time > lifeTime))
	{
		time -= lifeTime;

		// Restart() resets repeatCount, so store it locally and then revert.
		int16 curRepeatCount = repeatCount;
		Restart(true);
		repeatCount = curRepeatCount;

		repeatCount ++;
	}


	particleCount = 0;
	Vector<ParticleLayer*>::iterator it;
	for(it = layers.begin(); it != layers.end(); ++it)
	{
        if(!(*it)->GetDisabled())
            (*it)->Update(timeElapsed, (*it)->IsLodActive(currentLodLevel));
		particleCount+=(*it)->GetParticleCount();
	}

	if (shortEffect)
		lodLevelLocked = true;
	if ((state == STATE_STOPPING) && (particleCount == 0))
	{
		state = STATE_STOPPED;
	}
}

void ParticleEmitter::PrepareRenderData(Camera * camera){
	for(Vector<ParticleLayer*>::iterator it = layers.begin(), e = layers.end(); it!=e; ++it)
	{
		(*it)->PrepareRenderData(camera);
	}
}

void ParticleEmitter::RenderUpdate(Camera *camera, float32 timeElapsed)
{
	eBlendMode srcMode = RenderManager::Instance()->GetSrcBlend();
	eBlendMode destMode = RenderManager::Instance()->GetDestBlend();

	// Yuri Coder, 2013/01/30. ParticleEmitter class can be now only 2D.
	if(particlesFollow)
	{
		RenderManager::Instance()->PushDrawMatrix();
		RenderManager::Instance()->SetDrawTranslate(position);
	}

	Vector<ParticleLayer*>::iterator it;
	for(it = layers.begin(); it != layers.end(); ++it)
	{
		if(!(*it)->GetDisabled())
			(*it)->Draw(camera);
	}

	if(particlesFollow)
	{
		RenderManager::Instance()->PopDrawMatrix();
	}

	RenderManager::Instance()->SetBlendMode(srcMode, destMode);
}

void ParticleEmitter::PrepareEmitterParameters(Particle * particle, float32 velocity, int32 emitIndex)
{
	// Yuri Coder, 2013/01/30. ParticleEmitter class can be now only 2D.
    Vector3 tempPosition = particlesFollow ? Vector3() : position;
    if (emitterType == EMITTER_POINT)
    {
        particle->position = tempPosition;
    }
    else if (emitterType == EMITTER_RECT)
    {
        // TODO: add emitter angle support
        float32 rand05_x = (float32)Random::Instance()->RandFloat() - 0.5f; // [-0.5f, 0.5f]
        float32 rand05_y = (float32)Random::Instance()->RandFloat() - 0.5f; // [-0.5f, 0.5f]
        float32 rand05_z = (float32)Random::Instance()->RandFloat() - 0.5f; // [-0.5f, 0.5f]
        Vector3 lineDirection(0, 0, 0);
        if(size)
            lineDirection = Vector3(size->GetValue(time).x * rand05_x, size->GetValue(time).y * rand05_y, size->GetValue(time).z * rand05_z);
		particle->position = tempPosition + lineDirection;
	}
    else if ((emitterType == EMITTER_ONCIRCLE_VOLUME) ||
			 (emitterType == EMITTER_ONCIRCLE_EDGES) ||
			 (emitterType == EMITTER_SHOCKWAVE))
    {
        // here just set particle position
		// Yuri Coder, 2013/04/18. Shockwave particle isn't implemented for 2D mode -
		// currently draw them in the same way as "onCircle" ones.
        particle->position = tempPosition;
    }
        
    Vector3 vel;
    //vel.x = (float32)((rand() & 255) - 128);
    //vel.y = (float32)((rand() & 255) - 128);
    //vel.Normalize();
        
    float32 rand05 = (float32)Random::Instance()->RandFloat() - 0.5f; // [-0.5f, 0.5f]

    float32 particleAngle = 0;
    if(emissionAngle)
        particleAngle = DegToRad(emissionAngle->GetValue(time) + angle);
        
    float32 range = 0.0f;
    if(emissionRange)
        range = DegToRad(emissionRange->GetValue(time));
        
    if (emitPointsCount == -1)
    {
        // if emitAtPoints property is not set just emit randomly in range
        particleAngle += range * rand05;
    }
    else
    {
        particleAngle += range * (float32)emitIndex / (float32)emitPointsCount;
    }
        
        
    vel.x = cosf(particleAngle);
    vel.y = sinf(particleAngle);
    vel.z = 0;
        
    // reuse particle velocity we've calculated
	// Yuri Coder, 2013/04/18. Shockwave particle isn't implemented for 2D mode -
	// currently draw them in the same way as "onCircle" ones.
    if ((emitterType == EMITTER_ONCIRCLE_VOLUME) ||
		(emitterType == EMITTER_ONCIRCLE_EDGES) ||
		(emitterType == EMITTER_SHOCKWAVE))
    {
        if(radius)
            particle->position += vel * radius->GetValue(time);
    }
        
    particle->speed.x = vel.x*velocity;
    particle->speed.y = vel.y*velocity;	
    particle->angle = particleAngle;
}

void ParticleEmitter::RetainInCache(const String& name)
{
	Map<String, EmitterYamlCacheEntry>::iterator it = emitterYamlCache.find(name);
	if (it!=emitterYamlCache.end())
	{
		(*it).second.refCount++;
	}
}

void ParticleEmitter::ReleaseFromCache(const String& name)
{
	Map<String, EmitterYamlCacheEntry>::iterator it = emitterYamlCache.find(name);
	if (it!=emitterYamlCache.end())
	{
		(*it).second.refCount--;
		if (!(*it).second.refCount)
		{
			SafeRelease((*it).second.parser);
			emitterYamlCache.erase(it);
		}
	}
}

YamlParser* ParticleEmitter::GetParser(const FilePath &filename)
{
	YamlParser *res = NULL;
	String name = filename.GetAbsolutePathname();
	Map<String, EmitterYamlCacheEntry>::iterator it = emitterYamlCache.find(name);
	if (it!=emitterYamlCache.end())
	{
		(*it).second.refCount++;
		res = (*it).second.parser;
	}
	else
	{
		res = YamlParser::Create(filename);
		EmitterYamlCacheEntry entry;
		entry.parser = res;
		entry.refCount = 1;
		emitterYamlCache[name] = entry;
	}
	ReleaseFromCache(emitterFileName);
	emitterFileName = name;
	return res;
}

void ParticleEmitter::LoadFromYaml(const FilePath & filename)
{
    Cleanup(true);
    
	YamlParser * parser = GetParser(filename);
	if(!parser)
	{
		Logger::Error("ParticleEmitter::LoadFromYaml failed (%s)", filename.GetAbsolutePathname().c_str());
		return;
	}

	configPath = filename;
	time = 0.0f;
	repeatCount = 0;
	lifeTime = PARTICLE_EMITTER_DEFAULT_LIFE_TIME;

	YamlNode * rootNode = parser->GetRootNode();

	const YamlNode * emitterNode = rootNode->Get("emitter");
	if (emitterNode)
	{
		if (emitterNode->Get("emissionAngle"))
			emissionAngle = PropertyLineYamlReader::CreatePropertyLine<float32>(emitterNode->Get("emissionAngle"));
        
		if (emitterNode->Get("emissionVector"))
			emissionVector = PropertyLineYamlReader::CreatePropertyLine<Vector3>(emitterNode->Get("emissionVector"));
        
		const YamlNode* emissionVectorInvertedNode = emitterNode->Get("emissionVectorInverted");
		if (!emissionVectorInvertedNode)
		{
			// Yuri Coder, 2013/04/12. This means that the emission vector in the YAML file is not inverted yet.
			// Because of [DF-1003] fix for such files we have to invert coordinates for this vector.
			InvertEmissionVectorCoordinates();
		}

		if (emitterNode->Get("emissionRange"))
			emissionRange = PropertyLineYamlReader::CreatePropertyLine<float32>(emitterNode->Get("emissionRange"));
        
		if (emitterNode->Get("colorOverLife"))
			colorOverLife = PropertyLineYamlReader::CreatePropertyLine<Color>(emitterNode->Get("colorOverLife"));
		if (emitterNode->Get("radius"))
			radius = PropertyLineYamlReader::CreatePropertyLine<float32>(emitterNode->Get("radius"));
		
		emitPointsCount = -1; 
		const YamlNode * emitAtPointsNode = emitterNode->Get("emitAtPoints");
		if (emitAtPointsNode)
			emitPointsCount = emitAtPointsNode->AsInt();
		
		const YamlNode * lifeTimeNode = emitterNode->Get("life");
		if (lifeTimeNode)
		{
			lifeTime = lifeTimeNode->AsFloat();
		}else
		{
			lifeTime = PARTICLE_EMITTER_DEFAULT_LIFE_TIME;
		}
        
        is3D = false;
		const YamlNode * _3dNode = emitterNode->Get("3d");
		if (_3dNode)
		{	
			is3D = _3dNode->AsBool();
		}
		const YamlNode * shortEffectNode = emitterNode->Get("shortEffect");
		if (shortEffectNode)
			shortEffect = shortEffectNode->AsBool();
        
		const YamlNode * typeNode = emitterNode->Get("type");
		if (typeNode)
		{	
			if (typeNode->AsString() == "point")
				emitterType = EMITTER_POINT;
			else if (typeNode->AsString() == "line")
			{
				// Yuri Coder, 2013/04/09. Get rid of the "line" node type -
				// it can be completely replaced by "rect" one.
				emitterType = EMITTER_RECT;
			}
			else if (typeNode->AsString() == "rect")
				emitterType = EMITTER_RECT;
			else if (typeNode->AsString() == "oncircle")
				emitterType = EMITTER_ONCIRCLE_VOLUME;
			else if (typeNode->AsString() == "oncircle_edges")
				emitterType = EMITTER_ONCIRCLE_EDGES;
			else if (typeNode->AsString() == "shockwave")
				emitterType = EMITTER_SHOCKWAVE;
			else
				emitterType = EMITTER_POINT;
		}else
			emitterType = EMITTER_POINT;
		
        size = PropertyLineYamlReader::CreatePropertyLine<Vector3>(emitterNode->Get("size"));
        
        if(size == 0)
        {
            Vector3 _size(0, 0, 0);
            const YamlNode * widthNode = emitterNode->Get("width");
            if (widthNode)
                _size.x = widthNode->AsFloat();

            const YamlNode * heightNode = emitterNode->Get("height");
            if (heightNode)
                _size.y = heightNode->AsFloat();

            const YamlNode * depthNode = emitterNode->Get("depth");
            if (depthNode)
                _size.y = depthNode->AsFloat();
            
            size = new PropertyLineValue<Vector3>(_size);
        }
        
		const YamlNode * autorestartNode = emitterNode->Get("autorestart");
		if(autorestartNode)
			isAutorestart = autorestartNode->AsBool();

		const YamlNode * particlesFollowNode = emitterNode->Get("particlesFollow");
		if(particlesFollowNode)
			particlesFollow = particlesFollowNode->AsBool();
	}

	int cnt = rootNode->GetCount();
	for (int k = 0; k < cnt; ++k)
	{
		const YamlNode * node = rootNode->Get(k);
		const YamlNode * typeNode = node->Get("type");
		
		const YamlNode * longNode = node->Get("isLong");
		bool isLong = false;
		if(longNode && (longNode->AsBool() == true))
		{
			isLong = true;
		}

		if (typeNode && typeNode->AsString() == "layer")
		{
			LoadParticleLayerFromYaml(node, isLong);
		}
	}
	
	// Yuri Coder, 2013/01/15. The "name" node for Layer was just added and may not exist for
	// old yaml files. Generate the default name for nodes with empty names.
	UpdateEmptyLayerNames();		
}

void ParticleEmitter::SaveToYaml(const FilePath & filename)
{
    YamlParser* parser = YamlParser::Create();
    if (!parser)
    {
        Logger::Error("ParticleEmitter::SaveToYaml() - unable to create parser!");
        return;
    }

	configPath = filename;

    YamlNode* rootYamlNode = new YamlNode(YamlNode::TYPE_MAP);
    YamlNode* emitterYamlNode = new YamlNode(YamlNode::TYPE_MAP);
    rootYamlNode->AddNodeToMap("emitter", emitterYamlNode);
    
    emitterYamlNode->Set("3d", this->is3D);
    emitterYamlNode->Set("type", GetEmitterTypeName());
	emitterYamlNode->Set("shortEffect", shortEffect);
    
    // Write the property lines.
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(emitterYamlNode, "emissionAngle", this->emissionAngle);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(emitterYamlNode, "emissionRange", this->emissionRange);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<Vector3>(emitterYamlNode, "emissionVector", this->emissionVector);

	// Yuri Coder, 2013/04/12. After the coordinates inversion for the emission vector we need to introduce the
	// new "emissionVectorInverted" flag to mark we don't need to invert coordinates after re-loading the YAML.
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<bool>(emitterYamlNode, "emissionVectorInverted", true);

    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(emitterYamlNode, "radius", this->radius);

    PropertyLineYamlWriter::WritePropertyLineToYamlNode<Color>(emitterYamlNode, "colorOverLife", this->colorOverLife);

    PropertyLineYamlWriter::WritePropertyLineToYamlNode<Vector3>(emitterYamlNode, "size", this->size);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(emitterYamlNode, "life", this->lifeTime);

    // Now write all the Layers. Note - layers are child of root node, not the emitter one.
    int32 layersCount = this->layers.size();
    for (int32 i = 0; i < layersCount; i ++)
    {
        this->layers[i]->SaveToYamlNode(rootYamlNode, i);
    }

    parser->SaveToYamlFile(filename, rootYamlNode, true);
    parser->Release();
}

void ParticleEmitter::GetModifableLines(List<ModifiablePropertyLineBase *> &modifiables)
{
	PropertyLineHelper::AddIfModifiable(emissionVector.Get(), modifiables);
	PropertyLineHelper::AddIfModifiable(emissionRange.Get(), modifiables);
	PropertyLineHelper::AddIfModifiable(radius.Get(), modifiables);
	PropertyLineHelper::AddIfModifiable(size.Get(), modifiables);
	PropertyLineHelper::AddIfModifiable(colorOverLife.Get(), modifiables);
	int32 layersCount = this->layers.size();
	for (int32 i = 0; i < layersCount; i ++)
	{
		layers[i]->GetModifableLines(modifiables);
	}
}
    
int32 ParticleEmitter::GetParticleCount()
{
	return particleCount;
}

int32 ParticleEmitter::GetRepeatCount()
{
	return repeatCount;
}

int32 ParticleEmitter::GetEmitPointsCount()
{
	return emitPointsCount;
}
	
Vector<ParticleLayer*> & ParticleEmitter::GetLayers()
{
	return layers;
}
	
float32 ParticleEmitter::GetLifeTime()
{
	return lifeTime;
}
    
void ParticleEmitter::SetLifeTime(float32 time)
{
    lifeTime = time;
	
	/*see DF-1686*/
	for (int32 i = 0, size = layers.size(); i<size; ++i)
	{
		if (lifeTime<layers[i]->endTime)
		{
			layers[i]->UpdateLayerTime(layers[i]->startTime, lifeTime);
		}
	}
}
    
float32 ParticleEmitter::GetTime()
{
    return time;
}    

bool ParticleEmitter::GetAutoRestart()
{
	return isAutorestart;
}

Vector3 ParticleEmitter::GetSize()
{
    if(size)
        return size->GetValue(0);
    return Vector3(0, 0, 0);
}
    
Vector3 ParticleEmitter::GetSize(float32 time)
{
    if(size)
        return size->GetValue(time);
    return Vector3(0, 0, 0);
}
    
void ParticleEmitter::SetSize(const Vector3& _size)
{
	size = new PropertyLineValue<Vector3>(_size);
}

Animation * ParticleEmitter::SizeAnimation(const Vector3 & newSize, float32 time, Interpolation::FuncType interpolationFunc /*= Interpolation::LINEAR*/, int32 track /*= 0*/)
{
    Vector3 _size(0, 0, 0);
    if(size)
        _size = size->GetValue(0);
	LinearAnimation<Vector3> * animation = new LinearAnimation<Vector3>(this, &_size, newSize, time, interpolationFunc);
	animation->Start(track);
	return animation;
}

String ParticleEmitter::GetEmitterTypeName()
{
    switch (this->emitterType)
    {
        case EMITTER_POINT:
        {
            return "point";
        }

        case EMITTER_RECT:
        {
            return "rect";
        }

        case EMITTER_ONCIRCLE_VOLUME:
        {
            return "oncircle";
        }

		case EMITTER_ONCIRCLE_EDGES:
        {
            return "oncircle_edges";
        }

		case EMITTER_SHOCKWAVE:
        {
            return "shockwave";
        }

        default:
        {
            return "unknown";
        }
    }
}

void ParticleEmitter::UpdateEmptyLayerNames()
{
	int32 layersCount = this->GetLayers().size();
	for (int i = 0; i < layersCount; i ++)
	{
		UpdateLayerNameIfEmpty(this->layers[i], i);
	}
}

void ParticleEmitter::UpdateLayerNameIfEmpty(ParticleLayer* layer, int32 index)
{
	if (layer && layer->layerName.empty())
	{
		layer->layerName = Format("Layer %i", index);
	}
}


void ParticleEmitter::LoadParticleLayerFromYaml(const YamlNode* yamlNode, bool isLong)
{
	ParticleLayer* layer = new ParticleLayer();
	AddLayer(layer);
	layer->LoadFromYaml(configPath, yamlNode);

	SafeRelease(layer);
}

bool ParticleEmitter::Is3DFlagCorrect()
{
	// ParticleEmitter class can be created only for non-3D Emitters.
	return (is3D == false);
}

void ParticleEmitter::SetPlaybackSpeed(float32 value)
{
	this->playbackSpeed = Clamp(value, PARTICLE_EMITTER_MIN_PLAYBACK_SPEED,
								PARTICLE_EMITTER_MAX_PLAYBACK_SPEED);
	int32 layersCount = this->GetLayers().size();
	for (int i = 0; i < layersCount; i ++)
	{
		this->layers[i]->SetPlaybackSpeed(this->playbackSpeed);
	}
}

float32 ParticleEmitter::GetPlaybackSpeed()
{
	return this->playbackSpeed;
}

void ParticleEmitter::InvertEmissionVectorCoordinates()
{
	if (!this->emissionVector)
	{
		return;
	}

	PropertyLine<Vector3> *pvk = emissionVector.Get();
	uint32 keysSize = pvk->keys.size();
	for (uint32 i = 0; i < keysSize; ++i)
	{
		pvk->keys[i].value *= -1;
	}
}

int32 ParticleEmitter::GetActiveParticlesCount()
{
	uint32 particlesCount = 0;
	int32 layersCount = this->GetLayers().size();
	for (int i = 0; i < layersCount; i ++)
	{
		particlesCount += this->layers[i]->GetActiveParticlesCount();
	}

	return particlesCount;
}

void ParticleEmitter::RememberInitialTranslationVector()
{
	if (GetWorldTransformPtr())
	{
		this->initialTranslationVector = GetWorldTransformPtr()->GetTranslationVector();
	}
}

const Vector3& ParticleEmitter::GetInitialTranslationVector()
{
	return this->initialTranslationVector;
}

void ParticleEmitter::SetDisabledForAllLayers(bool value)
{
	for (Vector<ParticleLayer*>::iterator iter = layers.begin(); iter != layers.end(); iter ++)
	{
		(*iter)->SetDisabled(value);
	}
}

void ParticleEmitter::RecalcBoundingBox()
{
	bbox = AABBox3(Vector3(), Vector3());
}

void ParticleEmitter::RecalculateWorldBoundingBox()
{
	worldBBox = bbox; //as ParticelEmmiter dont use world transform for particle rendering, just for generation
}

void ParticleEmitter::SetLongToAllLayers(bool isLong)
{
	for(Vector<ParticleLayer*>::iterator it = layers.begin(); it != layers.end(); ++it)
	{
		(*it)->SetLong(isLong);
	}
}

void ParticleEmitter::SetParentParticle(Particle* parent)
{
	parentParticle = parent;
}

Particle* ParticleEmitter::GetParentParticle()
{
	return parentParticle;
}

void ParticleEmitter::HandleRemoveFromSystem()
{
	Vector<ParticleLayer*>::iterator it;
	for(it = layers.begin(); it != layers.end(); ++it)
	{
		(*it)->HandleRemoveFromSystem();
	}
}

}; 
