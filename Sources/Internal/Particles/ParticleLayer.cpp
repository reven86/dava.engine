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


#include "Particles/ParticleLayer.h"
#include "Particles/ParticleEmitter.h"
#include "Utils/StringFormat.h"
#include "Render/Image/Image.h"
#include "FileSystem/FileSystem.h"

namespace DAVA
{

const ParticleLayer::LayerTypeNamesInfo ParticleLayer::layerTypeNamesInfoMap[] =
{
	{ TYPE_SINGLE_PARTICLE, "single" },
	{ TYPE_PARTICLES, "particles" },
	{ TYPE_SUPEREMITTER_PARTICLES, "superEmitter" }
};

/*the following code is legacy compatibility to load original particle blending nodes*/
enum eBlendMode
{
    BLEND_NONE = 0,
    BLEND_ZERO,
    BLEND_ONE,
    BLEND_DST_COLOR,
    BLEND_ONE_MINUS_DST_COLOR,
    BLEND_SRC_ALPHA,
    BLEND_ONE_MINUS_SRC_ALPHA,
    BLEND_DST_ALPHA,
    BLEND_ONE_MINUS_DST_ALPHA,
    BLEND_SRC_ALPHA_SATURATE,
    BLEND_SRC_COLOR,
    BLEND_ONE_MINUS_SRC_COLOR,

    BLEND_MODE_COUNT,
};
const String BLEND_MODE_NAMES[BLEND_MODE_COUNT] =
{
  "BLEND_NONE",
  "BLEND_ZERO",
  "BLEND_ONE",
  "BLEND_DST_COLOR",
  "BLEND_ONE_MINUS_DST_COLOR",
  "BLEND_SRC_ALPHA",
  "BLEND_ONE_MINUS_SRC_ALPHA",
  "BLEND_DST_ALPHA",
  "BLEND_ONE_MINUS_DST_ALPHA",
  "BLEND_SRC_ALPHA_SATURATE",
  "BLEND_SRC_COLOR",
  "BLEND_ONE_MINUS_SRC_COLOR"
};

eBlendMode GetBlendModeByName(const String& blendStr)
{
    for (uint32 i = 0; i < BLEND_MODE_COUNT; i++)
        if (blendStr == BLEND_MODE_NAMES[i])
            return (eBlendMode)i;

    return BLEND_MODE_COUNT;
}

/*end of legacy compatibility code*/

ParticleLayer::ParticleLayer() 	
	: sprite(0)
	, innerEmitter(NULL)
{	
	life = 0;
	lifeVariation = 0;

	number = 0;	
	numberVariation = 0;		

	size = 0;
	sizeVariation = 0;

	velocity = 0;
	velocityVariation = 0;	
	velocityOverLife = 0;

	spin = 0;			
	spinVariation = 0;
	spinOverLife = 0;
	animSpeedOverLife = 0;
	randomSpinDirection = false;	
	
	colorOverLife = 0;
	colorRandom = 0;
	alphaOverLife = 0;	
	
	angle = 0;
	angleVariation = 0;

    blending = BLENDING_ALPHABLEND;
    enableFog = true;
    enableFrameBlend = false;
    inheritPosition = false;
    type = TYPE_PARTICLES;

    degradeStrategy = DEGRADE_KEEP;
    
    endTime = 100.0f;
	deltaTime = 0.0f;
	deltaVariation = 0.0f;
	loopVariation = 0.0f;
	loopEndTime = 0.0f;	

	frameOverLifeEnabled = false;
	frameOverLifeFPS = 0;
	randomFrameOnStart = false;
	loopSpriteAnimation = true;

	scaleVelocityBase = 1;
	scaleVelocityFactor = 0;

	particleOrientation = PARTICLE_ORIENTATION_CAMERA_FACING;
    
	isLooped = false;	

	isLong = false;
	
	isDisabled = false;

	activeLODS.resize(4, true);
}

ParticleLayer::~ParticleLayer()
{
	
	SafeRelease(sprite);
	SafeRelease(innerEmitter);
	
	CleanupForces();
	// dynamic cache automatically delete all particles
}

ParticleLayer * ParticleLayer::Clone()
{
	
	ParticleLayer *	dstLayer = new ParticleLayer();
	

	if (life)
		dstLayer->life.Set(life->Clone());
	
	if (lifeVariation)
		dstLayer->lifeVariation.Set(lifeVariation->Clone());

	if (number)
		dstLayer->number.Set(number->Clone());
	
	if (numberVariation)
		dstLayer->numberVariation.Set(numberVariation->Clone());

	if (size)
		dstLayer->size.Set(size->Clone());
	
	if (sizeVariation)
		dstLayer->sizeVariation.Set(sizeVariation->Clone());
	
	if (sizeOverLifeXY)
		dstLayer->sizeOverLifeXY.Set(sizeOverLifeXY->Clone());
	
	if (velocity)
		dstLayer->velocity.Set(velocity->Clone());
	
	if (velocityVariation)
		dstLayer->velocityVariation.Set(velocityVariation->Clone());
	
	if (velocityOverLife)
		dstLayer->velocityOverLife.Set(velocityOverLife->Clone());

	// Copy the forces.
	dstLayer->CleanupForces();
    dstLayer->forces.reserve(forces.size());
	for (int32 f = 0; f < (int32)forces.size(); ++ f)
	{
		ParticleForce* clonedForce = this->forces[f]->Clone();
		dstLayer->AddForce(clonedForce);
		clonedForce->Release();
	}

	if (spin)
		dstLayer->spin.Set(spin->Clone());
	
	if (spinVariation)
		dstLayer->spinVariation.Set(spinVariation->Clone());
	
	if (spinOverLife)
		dstLayer->spinOverLife.Set(spinOverLife->Clone());
	dstLayer->randomSpinDirection = randomSpinDirection;

	if (animSpeedOverLife)
		dstLayer->animSpeedOverLife.Set(animSpeedOverLife->Clone());		
	
	if (colorOverLife)
		dstLayer->colorOverLife.Set(colorOverLife->Clone());
	
	if (colorRandom)
		dstLayer->colorRandom.Set(colorRandom->Clone());
	
	if (alphaOverLife)
		dstLayer->alphaOverLife.Set(alphaOverLife->Clone());
	
	if (angle)
		dstLayer->angle.Set(angle->Clone());
	
	if (angleVariation)
		dstLayer->angleVariation.Set(angleVariation->Clone());

	SafeRelease(dstLayer->innerEmitter);
	if (innerEmitter)
		dstLayer->innerEmitter = static_cast<ParticleEmitter*>(innerEmitter->Clone());
	
	dstLayer->layerName = layerName;

    dstLayer->blending = blending;
    dstLayer->enableFog = enableFog;
    dstLayer->enableFrameBlend = enableFrameBlend;
    dstLayer->inheritPosition = inheritPosition;
    dstLayer->startTime = startTime;
    dstLayer->endTime = endTime;

    dstLayer->isLooped = isLooped;
	dstLayer->deltaTime = deltaTime;
	dstLayer->deltaVariation = deltaVariation;
	dstLayer->loopVariation = loopVariation;
	dstLayer->loopEndTime = loopEndTime;
	
	dstLayer->isDisabled = isDisabled;

	dstLayer->type = type;
    dstLayer->degradeStrategy = degradeStrategy;
	SafeRelease(dstLayer->sprite);
	dstLayer->sprite = SafeRetain(sprite);
	dstLayer->layerPivotPoint = layerPivotPoint;	
	dstLayer->layerPivotSizeOffsets = layerPivotSizeOffsets;

	dstLayer->frameOverLifeEnabled = frameOverLifeEnabled;
	dstLayer->frameOverLifeFPS = frameOverLifeFPS;
	dstLayer->randomFrameOnStart = randomFrameOnStart;
	dstLayer->loopSpriteAnimation = loopSpriteAnimation;
	dstLayer->particleOrientation = particleOrientation;

	dstLayer->scaleVelocityBase = scaleVelocityBase;
	dstLayer->scaleVelocityFactor = scaleVelocityFactor;
    
	dstLayer->spritePath = spritePath;
	dstLayer->activeLODS = activeLODS;
	dstLayer->isLong = isLong;

	return dstLayer;
}

bool ParticleLayer::IsLodActive(int32 lod)
{
	if ((lod>=0)&&(lod<(int32)activeLODS.size()))
		return activeLODS[lod];
	
	return false;
}

void ParticleLayer::SetLodActive(int32 lod, bool active)
{
	if ((lod>=0)&&(lod<(int32)activeLODS.size()))
		activeLODS[lod] = active;
}

template <class T> void UpdatePropertyLineKeys(PropertyLine<T> * line, float32 startTime, float32 translateTime, float32 endTime)
{	
	if (!line)	
		return;	
	Vector<typename PropertyLine<T>::PropertyKey> &keys = line->GetValues();
	int32 size = static_cast<int32>(keys.size());
	int32 i;
	for (i=0; i<size; ++i)
	{
		keys[i].t += translateTime;
		if (keys[i].t>endTime)		
			break;		
		
	}
	if (i==0)
		i+=1; //keep at least 1
	keys.erase(keys.begin()+i, keys.end());
	if (keys.size() == 1)
	{
		keys[0].t = startTime;
	}
}

template <class T> void UpdatePropertyLineOnLoad(PropertyLine<T> * line, float32 startTime, float32 endTime)
{
	if (!line)
		return;
	Vector<typename PropertyLine<T>::PropertyKey> &keys = line->GetValues();
	int32 size = static_cast<int32>(keys.size());
	int32 i;
	/*drop keys before*/
	for (i=0; i<size; ++i)
	{
		if (keys[i].t>=startTime)
			break;
	}
	if (i!=0)
	{
		T v0 = line->GetValue(startTime);
		keys.erase(keys.begin(), keys.begin()+i);
		typename PropertyLine<T>::PropertyKey key;
		key.t = startTime;
		key.value = v0;
		keys.insert(keys.begin(), key);
	}	
	
	/*drop keys after*/
	size = static_cast<int32>(keys.size());
	for (i=0; i<size; i++)
	{
		if (keys[i].t>endTime)
			break;
	}
	if (i!=size)
	{
		T v1 = line->GetValue(endTime);
		keys.erase(keys.begin()+i, keys.end());
		typename PropertyLine<T>::PropertyKey key;
		key.t = endTime;
		key.value = v1;
		keys.push_back(key);
	}
}

void ParticleLayer::UpdateLayerTime(float32 startTime, float32 endTime)
{
	float32 translateTime = startTime-this->startTime;
	this->startTime = startTime;
	this->endTime = endTime;
	/*validate all time depended property lines*/	
	UpdatePropertyLineKeys(PropertyLineHelper::GetValueLine(life).Get(), startTime, translateTime, endTime);
	UpdatePropertyLineKeys(PropertyLineHelper::GetValueLine(lifeVariation).Get(), startTime, translateTime, endTime);
	UpdatePropertyLineKeys(PropertyLineHelper::GetValueLine(number).Get(), startTime, translateTime, endTime);
	UpdatePropertyLineKeys(PropertyLineHelper::GetValueLine(numberVariation).Get(), startTime, translateTime, endTime);
	UpdatePropertyLineKeys(PropertyLineHelper::GetValueLine(size).Get(), startTime, translateTime, endTime);
	UpdatePropertyLineKeys(PropertyLineHelper::GetValueLine(sizeVariation).Get(), startTime, translateTime, endTime);
	UpdatePropertyLineKeys(PropertyLineHelper::GetValueLine(velocity).Get(), startTime, translateTime, endTime);
	UpdatePropertyLineKeys(PropertyLineHelper::GetValueLine(velocityVariation).Get(), startTime, translateTime, endTime);
	UpdatePropertyLineKeys(PropertyLineHelper::GetValueLine(spin).Get(), startTime, translateTime, endTime);
	UpdatePropertyLineKeys(PropertyLineHelper::GetValueLine(spinVariation).Get(), startTime, translateTime, endTime);
	UpdatePropertyLineKeys(PropertyLineHelper::GetValueLine(angle).Get(), startTime, translateTime, endTime);
	UpdatePropertyLineKeys(PropertyLineHelper::GetValueLine(angleVariation).Get(), startTime, translateTime, endTime);
}



void ParticleLayer::SetSprite(Sprite * _sprite)
{    
	SafeRelease(sprite);
	sprite = SafeRetain(_sprite);

	if(sprite)
	{
		spritePath = sprite->GetRelativePathname();
	}
}

void ParticleLayer::SetPivotPoint(Vector2 pivot)
{
	layerPivotPoint = pivot;
	layerPivotSizeOffsets = Vector2(1+fabs(layerPivotPoint.x), 1+fabs(layerPivotPoint.y));
	layerPivotSizeOffsets*=0.5f;
}


void ParticleLayer::LoadFromYaml(const FilePath & configPath, const YamlNode * node, bool preserveInheritPosition)
{		
	// format processing
	int32 format = 0;
	const YamlNode * formatNode = node->Get("effectFormat");
	if (formatNode)
	{
		format = formatNode->AsInt32();
	}

	type = TYPE_PARTICLES;
	const YamlNode * typeNode = node->Get("layerType");
	if (typeNode)
	{
		type = StringToLayerType(typeNode->AsString(), TYPE_PARTICLES);
	}

    degradeStrategy = DEGRADE_KEEP;
    const YamlNode * degradeNode = node->Get("degradeStrategy");
    if (degradeNode)
    {
        degradeStrategy = (eDegradeStrategy)(degradeNode->AsInt());
    }

	const YamlNode * nameNode = node->Get("name");
	if (nameNode)
	{
		layerName = nameNode->AsString();
	}

	const YamlNode * longNode = node->Get("isLong");
	if (longNode)
	{
		isLong = longNode->AsBool();
	}

	const YamlNode * pivotPointNode = node->Get("pivotPoint");
	
    SetSprite(NULL);
	const YamlNode * spriteNode = node->Get("sprite");
	if (spriteNode && !spriteNode->AsString().empty())
	{
		// Store the absolute path to sprite.
		spritePath = FilePath(configPath.GetDirectory(), spriteNode->AsString());

        if (type != TYPE_SUPEREMITTER_PARTICLES)
        {
		    Sprite * _sprite = Sprite::Create(spritePath);
		    SetSprite(_sprite);
            SafeRelease(_sprite);
        }
	}	
	if(pivotPointNode)
	{
		Vector2 _pivot = pivotPointNode->AsPoint();
		if ((format == 0)&&sprite)
		{
			
			float32 ny=-_pivot.x/sprite->GetWidth()*2;
			float32 nx=-_pivot.y/sprite->GetHeight()*2;
			_pivot.Set(nx, ny);
		}

		SetPivotPoint(_pivot);
	}

	const YamlNode *lodsNode = node->Get("activeLODS");
	if (lodsNode)
	{
		const Vector<YamlNode*> & vec = lodsNode->AsVector();
		for (uint32 i=0; i<(uint32)vec.size(); ++i)
			SetLodActive(i, (vec[i]->AsInt()) != 0); //as AddToArray has no override for bool, flags are stored as int
	}


	colorOverLife = PropertyLineYamlReader::CreatePropertyLine<Color>(node->Get("colorOverLife"));
	colorRandom = PropertyLineYamlReader::CreatePropertyLine<Color>(node->Get("colorRandom"));
	alphaOverLife = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("alphaOverLife"));
	
	const YamlNode* frameOverLifeEnabledNode = node->Get("frameOverLifeEnabled");
	if (frameOverLifeEnabledNode)
	{
		frameOverLifeEnabled = frameOverLifeEnabledNode->AsBool();
	}

	const YamlNode* randomFrameOnStartNode = node->Get("randomFrameOnStart");
	if (randomFrameOnStartNode)
	{
		randomFrameOnStart = randomFrameOnStartNode->AsBool();
	}
	const YamlNode* loopSpriteAnimationNode = node->Get("loopSpriteAnimation");
	if (loopSpriteAnimationNode)
	{
		loopSpriteAnimation = loopSpriteAnimationNode->AsBool();
	}

	const YamlNode* particleOrientationNode = node->Get("particleOrientation");
	if (particleOrientationNode)
	{
		particleOrientation = particleOrientationNode->AsInt32();
	}


	const YamlNode* frameOverLifeFPSNode = node->Get("frameOverLifeFPS");
	if (frameOverLifeFPSNode)
	{
		frameOverLifeFPS = frameOverLifeFPSNode->AsFloat();
	}

	const YamlNode* scaleVelocityBaseNode = node->Get("scaleVelocityBase");
	if (scaleVelocityBaseNode)
	{
		scaleVelocityBase = scaleVelocityBaseNode->AsFloat();
	}

	const YamlNode* scaleVelocityFactorNode = node->Get("scaleVelocityFactor");
	if (scaleVelocityFactorNode)
	{
		scaleVelocityFactor = scaleVelocityFactorNode->AsFloat();
	}

	life = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("life"));	
	lifeVariation = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("lifeVariation"));	

	number = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("number"));	
	numberVariation = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("numberVariation"));	

	
	size = PropertyLineYamlReader::CreatePropertyLine<Vector2>(node->Get("size"));		

	sizeVariation = PropertyLineYamlReader::CreatePropertyLine<Vector2>(node->Get("sizeVariation"));

	sizeOverLifeXY = PropertyLineYamlReader::CreatePropertyLine<Vector2>(node->Get("sizeOverLifeXY"));

	// Yuri Coder, 2013/04/03. sizeOverLife is outdated and kept here for the backward compatibility only.
	// New property is sizeOverlifeXY and contains both X and Y components.
	RefPtr< PropertyLine<float32> > sizeOverLife = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("sizeOverLife"));
	if (sizeOverLife)
	{
		if (sizeOverLifeXY)
		{
			// Both properties can't be present in the same config.
			Logger::Error("Both sizeOverlife and sizeOverlifeXY are defined for Particle Layer %s, taking sizeOverlifeXY as default",
						  configPath.GetAbsolutePathname().c_str());
			DVASSERT(false);
		}
		else
		{
			// Only the outdated sizeOverlife is defined - create sizeOverlifeXY property based on outdated one.
			FillSizeOverlifeXY(sizeOverLife);
		}
	}

	velocity = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("velocity"));
	velocityVariation = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("velocityVariation"));	
	velocityOverLife = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("velocityOverLife"));
	
	angle = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("angle"));
	angleVariation = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("angleVariation"));
	
	int32 forceCount = 0;
	const YamlNode * forceCountNode = node->Get("forceCount");
	if (forceCountNode)
		forceCount = forceCountNode->AsInt();

	for (int k = 0; k < forceCount; ++k)
	{
        // Any of the Force Parameters might be NULL, and this is acceptable.
		RefPtr< PropertyLine<Vector3> > force = PropertyLineYamlReader::CreatePropertyLine<Vector3>(node->Get(Format("force%d", k) ));		
		RefPtr< PropertyLine<float32> > forceOverLife = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get(Format("forceOverLife%d", k)));
        
        if (force&&(!format)) //as no forceVariation anymore - add it directly to force
        {
             RefPtr< PropertyLine<Vector3> > forceVariation = PropertyLineYamlReader::CreatePropertyLine<Vector3>(node->Get(Format("forceVariation%d", k)));
             if (forceVariation)
             {
                 Vector3 varriationToAdd = forceVariation->GetValue(0);
                 Vector<typename PropertyLine<Vector3>::PropertyKey> &keys = force->GetValues();		                 
                 for (size_t i=0, sz = keys.size(); i<sz; ++i)
                 {			
                     keys[i].value+=varriationToAdd;
                 }
             }
             
        }

		ParticleForce* particleForce = new ParticleForce(force, forceOverLife);
		AddForce(particleForce);
        particleForce->Release();
	}

	spin = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("spin"));
	spinVariation = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("spinVariation"));	
	spinOverLife = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("spinOverLife"));	
	animSpeedOverLife = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("animSpeedOverLife"));	
	const YamlNode* randomSpinDirectionNode = node->Get("randomSpinDirection");
	if (randomSpinDirectionNode)
	{
		randomSpinDirection = randomSpinDirectionNode->AsBool();
    }

    blending = BLENDING_ALPHABLEND; //default

    //read blend node for backward compatibility with old effect files
    const YamlNode* blend = node->Get("blend");
    if (blend)
    {
        if (blend->AsString() == "alpha")
        {
            blending = BLENDING_ALPHABLEND;
        }
        if (blend->AsString() == "add")
        {
            blending = BLENDING_ALPHA_ADDITIVE;
        }
    }

    const YamlNode* blendSrcNode = node->Get("srcBlendFactor");
    const YamlNode* blendDestNode = node->Get("dstBlendFactor");

    if (blendSrcNode && blendDestNode)
    {
        eBlendMode srcBlendFactor = GetBlendModeByName(blendSrcNode->AsString());
        eBlendMode dstBlendFactor = GetBlendModeByName(blendDestNode->AsString());

        if ((srcBlendFactor == BLEND_ONE) && (dstBlendFactor == BLEND_ONE))
            blending = BLENDING_ADDITIVE;
        else if ((srcBlendFactor == BLEND_SRC_ALPHA) && (dstBlendFactor == BLEND_ONE))
            blending = BLENDING_ALPHA_ADDITIVE;
        else if ((srcBlendFactor == BLEND_ONE_MINUS_DST_COLOR) && (dstBlendFactor == BLEND_ONE))
            blending = BLENDING_SOFT_ADDITIVE;
        else if ((srcBlendFactor == BLEND_DST_COLOR) && (dstBlendFactor == BLEND_ZERO))
            blending = BLENDING_MULTIPLICATIVE;
        else if ((srcBlendFactor == BLEND_DST_COLOR) && (dstBlendFactor == BLEND_SRC_COLOR))
            blending = BLENDING_STRONG_MULTIPLICATIVE;
    }

    //end of legacy

    const YamlNode* blendingNode = node->Get("blending");
    if (blendingNode)
        blending = (eBlending)blendingNode->AsInt();
    const YamlNode* fogNode = node->Get("enableFog");
    if (fogNode)
    {
        enableFog = fogNode->AsBool();
    }

    const YamlNode * frameBlendNode = node->Get("enableFrameBlend");	
	if (frameBlendNode)
	{
        enableFrameBlend = frameBlendNode->AsBool();
    }

    startTime = 0.0f;
    endTime = 100000000.0f;
    const YamlNode* startTimeNode = node->Get("startTime");
    if (startTimeNode)
        startTime = startTimeNode->AsFloat();

	const YamlNode * endTimeNode = node->Get("endTime");
	if (endTimeNode)
		endTime = endTimeNode->AsFloat();
		
	isLooped = false;	
	deltaTime = 0.0f;
	deltaVariation = 0.0f;
	loopVariation = 0.0f;
	
	const YamlNode * isLoopedNode = node->Get("isLooped");
	if (isLoopedNode)
		isLooped = isLoopedNode->AsBool();
		
	const YamlNode * deltaTimeNode = node->Get("deltaTime");
	if (deltaTimeNode)
		deltaTime = deltaTimeNode->AsFloat();
		
	const YamlNode * deltaVariationNode = node->Get("deltaVariation");
	if (deltaVariationNode)
		deltaVariation = deltaVariationNode->AsFloat();
		
	const YamlNode * loopVariationNode = node->Get("loopVariation");
	if (loopVariationNode)
		loopVariation = loopVariationNode->AsFloat();
		
	const YamlNode * loopEndTimeNode = node->Get("loopEndTime");
	if (loopEndTimeNode)
		loopEndTime = loopEndTimeNode->AsFloat();				

	/*validate all time depended property lines*/	
	UpdatePropertyLineOnLoad(life.Get(), startTime, endTime);
	UpdatePropertyLineOnLoad(lifeVariation.Get(), startTime, endTime);
	UpdatePropertyLineOnLoad(number.Get(), startTime, endTime);
	UpdatePropertyLineOnLoad(numberVariation.Get(), startTime, endTime);
	UpdatePropertyLineOnLoad(size.Get(), startTime, endTime);
	UpdatePropertyLineOnLoad(sizeVariation.Get(), startTime, endTime);
	UpdatePropertyLineOnLoad(velocity.Get(), startTime, endTime);
	UpdatePropertyLineOnLoad(velocityVariation.Get(), startTime, endTime);
	UpdatePropertyLineOnLoad(spin.Get(), startTime, endTime);
	UpdatePropertyLineOnLoad(spinVariation.Get(), startTime, endTime);
	UpdatePropertyLineOnLoad(angle.Get(), startTime, endTime);
	UpdatePropertyLineOnLoad(angleVariation.Get(), startTime, endTime);

	const YamlNode * inheritPositionNode = node->Get("inheritPosition");
	if (inheritPositionNode)
	{
		inheritPosition = inheritPositionNode->AsBool();
	}

	// Load the Inner Emitter parameters.
	const YamlNode * innerEmitterPathNode = node->Get("innerEmitterPath");
	if ((type == TYPE_SUPEREMITTER_PARTICLES) && innerEmitterPathNode)
	{
		SafeRelease(innerEmitter);
		innerEmitter = new ParticleEmitter();       
		// Since Inner Emitter path is stored as Relative, convert it to absolute when loading.
		innerEmitterPath = FilePath(configPath.GetDirectory(), innerEmitterPathNode->AsString());
		innerEmitter->LoadFromYaml(this->innerEmitterPath, true);
	}		
	if (format == 0) //update old stuff
	{
		UpdateSizeLine(size.Get(), true, !isLong);
		UpdateSizeLine(sizeVariation.Get(), true, !isLong);
		UpdateSizeLine(sizeOverLifeXY.Get(), false, !isLong);
		inheritPosition &= preserveInheritPosition;
	}
}

void ParticleLayer::UpdateSizeLine(PropertyLine<Vector2> *line, bool rescaleSize, bool swapXY)
{
	//conversion from old format
	if (!line) return;
	if ((!rescaleSize)&&(!swapXY)) return; //nothing to update
    
	Vector<typename PropertyLine<Vector2>::PropertyKey> &keys = PropertyLineHelper::GetValueLine(line)->GetValues();		
	for (size_t i=0, sz = keys.size(); i<sz; ++i)
	{			
		if (rescaleSize)
		{
			keys[i].value.x*=0.5f;
			keys[i].value.y*=0.5f;
		}
		if (swapXY)
		{
			float x = keys[i].value.x;
			keys[i].value.x=keys[i].value.y;
			keys[i].value.y=x;
		}		
	}
}

void ParticleLayer::SaveToYamlNode(const FilePath & configPath, YamlNode* parentNode, int32 layerIndex)
{
    YamlNode* layerNode = new YamlNode(YamlNode::TYPE_MAP);
    String layerNodeName = Format("layer%d", layerIndex);
    parentNode->AddNodeToMap(layerNodeName, layerNode);

    PropertyLineYamlWriter::WritePropertyValueToYamlNode<String>(layerNode, "name", layerName);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<String>(layerNode, "type", "layer");
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<String>(layerNode, "layerType",
																 LayerTypeToString(type, "particles"));
    

    PropertyLineYamlWriter::WritePropertyValueToYamlNode<int32>(layerNode, "degradeStrategy", (int32)degradeStrategy);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<bool>(layerNode, "isLong", isLong);
    

	PropertyLineYamlWriter::WritePropertyValueToYamlNode<Vector2>(layerNode, "pivotPoint", layerPivotPoint);

    // Truncate an extension of the sprite file.
    FilePath savePath = spritePath;
    if (!savePath.IsEmpty())
    {        
        savePath.TruncateExtension();
	    String relativePath = savePath.GetRelativePathname(configPath.GetDirectory());
	    PropertyLineYamlWriter::WritePropertyValueToYamlNode<String>(layerNode, "sprite", relativePath);
    }

    layerNode->Add("blending", blending);

    layerNode->Add("enableFog", enableFog);
    layerNode->Add("enableFrameBlend", enableFrameBlend);

    layerNode->Add("scaleVelocityBase", scaleVelocityBase);
    layerNode->Add("scaleVelocityFactor", scaleVelocityFactor);

    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "life", this->life);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "lifeVariation", this->lifeVariation);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "number", this->number);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "numberVariation", this->numberVariation);
    
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<Vector2>(layerNode, "size", this->size);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<Vector2>(layerNode, "sizeVariation", this->sizeVariation);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<Vector2>(layerNode, "sizeOverLifeXY", this->sizeOverLifeXY);
    
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "velocity", this->velocity);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "velocityVariation", this->velocityVariation);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "velocityOverLife", this->velocityOverLife);

    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "spin", this->spin);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "spinVariation", this->spinVariation);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "spinOverLife", this->spinOverLife);
	PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "animSpeedOverLife", this->animSpeedOverLife);
	PropertyLineYamlWriter::WritePropertyValueToYamlNode<bool>(layerNode, "randomSpinDirection", this->randomSpinDirection);
    
	PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "angle", this->angle);
	PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "angleVariation", this->angleVariation);

    PropertyLineYamlWriter::WritePropertyLineToYamlNode<Color>(layerNode, "colorRandom", this->colorRandom);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "alphaOverLife", this->alphaOverLife);

    PropertyLineYamlWriter::WritePropertyLineToYamlNode<Color>(layerNode, "colorOverLife", this->colorOverLife);
	PropertyLineYamlWriter::WritePropertyValueToYamlNode<bool>(layerNode, "frameOverLifeEnabled", this->frameOverLifeEnabled);
	PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(layerNode, "frameOverLifeFPS", this->frameOverLifeFPS);
	PropertyLineYamlWriter::WritePropertyValueToYamlNode<bool>(layerNode, "randomFrameOnStart", this->randomFrameOnStart);
	PropertyLineYamlWriter::WritePropertyValueToYamlNode<bool>(layerNode, "loopSpriteAnimation", this->loopSpriteAnimation);
    

    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(layerNode, "startTime", this->startTime);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(layerNode, "endTime", this->endTime);
	
	PropertyLineYamlWriter::WritePropertyValueToYamlNode<bool>(layerNode, "isLooped", this->isLooped);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(layerNode, "deltaTime", this->deltaTime);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(layerNode, "deltaVariation", this->deltaVariation);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(layerNode, "loopVariation", this->loopVariation);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(layerNode, "loopEndTime", this->loopEndTime);

	layerNode->Set("inheritPosition", inheritPosition);	

	layerNode->Set("particleOrientation", particleOrientation);

	YamlNode *lodsNode = new YamlNode(YamlNode::TYPE_ARRAY);
	for (int32 i =0; i<4; i++)
		lodsNode->Add((int32)activeLODS[i]); //as for now AddValueToArray has no bool type - force it to int
	layerNode->SetNodeToMap("activeLODS", lodsNode);

	if ((type == TYPE_SUPEREMITTER_PARTICLES) && innerEmitter)
	{
		String innerRelativePath = innerEmitterPath.GetRelativePathname(configPath.GetDirectory());
		PropertyLineYamlWriter::WritePropertyValueToYamlNode<String>(layerNode, "innerEmitterPath", innerRelativePath);
	}

	PropertyLineYamlWriter::WritePropertyValueToYamlNode<int32>(layerNode, "effectFormat", 1);

    // Now write the forces.
    SaveForcesToYamlNode(layerNode);
}

void ParticleLayer::SaveForcesToYamlNode(YamlNode* layerNode)
{
    int32 forceCount = (int32)this->forces.size();
    if (forceCount == 0)
    {
        // No forces to write.
        return;
    }

    PropertyLineYamlWriter::WritePropertyValueToYamlNode<int32>(layerNode, "forceCount", forceCount);
    for (int32 i = 0; i < forceCount; i ++)
    {
		ParticleForce* currentForce = this->forces[i];
        
		String forceDataName = Format("force%d", i);		
        PropertyLineYamlWriter::WritePropertyLineToYamlNode<Vector3>(layerNode, forceDataName, currentForce->force);
		
        forceDataName = Format("forceOverLife%d", i);
        PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, forceDataName, currentForce->forceOverLife);
    }
}


void ParticleLayer::GetModifableLines(List<ModifiablePropertyLineBase *> &modifiables)
{
	PropertyLineHelper::AddIfModifiable(life.Get(), modifiables);
	PropertyLineHelper::AddIfModifiable(lifeVariation.Get(), modifiables);
	PropertyLineHelper::AddIfModifiable(number.Get(), modifiables);
	PropertyLineHelper::AddIfModifiable(numberVariation.Get(), modifiables);
	PropertyLineHelper::AddIfModifiable(size.Get(), modifiables);
	PropertyLineHelper::AddIfModifiable(sizeVariation.Get(), modifiables);
	PropertyLineHelper::AddIfModifiable(sizeOverLifeXY.Get(), modifiables);
	PropertyLineHelper::AddIfModifiable(velocity.Get(), modifiables);
	PropertyLineHelper::AddIfModifiable(velocityVariation.Get(), modifiables);
	PropertyLineHelper::AddIfModifiable(velocityOverLife.Get(), modifiables);	
	PropertyLineHelper::AddIfModifiable(spin.Get(), modifiables);
	PropertyLineHelper::AddIfModifiable(spinVariation.Get(), modifiables);
	PropertyLineHelper::AddIfModifiable(spinOverLife.Get(), modifiables);	
	PropertyLineHelper::AddIfModifiable(colorRandom.Get(), modifiables);	
	PropertyLineHelper::AddIfModifiable(alphaOverLife.Get(), modifiables);	
	PropertyLineHelper::AddIfModifiable(colorOverLife.Get(), modifiables);	
	PropertyLineHelper::AddIfModifiable(angle.Get(), modifiables);		
	PropertyLineHelper::AddIfModifiable(angleVariation.Get(), modifiables);		
	PropertyLineHelper::AddIfModifiable(animSpeedOverLife.Get(), modifiables);	

	int32 forceCount = (int32)this->forces.size();	
	for (int32 i = 0; i < forceCount; i ++)
	{
		forces[i]->GetModifableLines(modifiables);
	}

	if ((type == TYPE_SUPEREMITTER_PARTICLES)&&innerEmitter)
	{
		innerEmitter->GetModifableLines(modifiables);
	}

}


void ParticleLayer::AddForce(ParticleForce* force)
{
	SafeRetain(force);
	this->forces.push_back(force);
}

void ParticleLayer::RemoveForce(ParticleForce* force)
{
	Vector<ParticleForce*>::iterator iter = std::find(this->forces.begin(),
													  this->forces.end(),
													  force);
	if (iter != this->forces.end())
	{
		SafeRelease(*iter);
		this->forces.erase(iter);
	}
}

void ParticleLayer::RemoveForce(int32 forceIndex)
{
	if (forceIndex <= (int32)this->forces.size())
	{
		SafeRelease(this->forces[forceIndex]);
		this->forces.erase(this->forces.begin() + forceIndex);
	}
}

void ParticleLayer::CleanupForces()
{
	for (Vector<ParticleForce*>::iterator iter = this->forces.begin();
		 iter != this->forces.end(); iter ++)
	{
		SafeRelease(*iter);
	}
	
	this->forces.clear();
}

void ParticleLayer::FillSizeOverlifeXY(RefPtr< PropertyLine<float32> > sizeOverLife)
{
	Vector<PropValue<float32> > wrappedPropertyValues = PropLineWrapper<float32>(sizeOverLife).GetProps();
	if (wrappedPropertyValues.empty())
	{
		this->sizeOverLifeXY = NULL;
		return;
	}
	else if (wrappedPropertyValues.size() == 1)
	{
		Vector2 singleValue(wrappedPropertyValues[0].v, wrappedPropertyValues[0].v);
		this->sizeOverLifeXY = RefPtr< PropertyLine<Vector2> >(new PropertyLineValue<Vector2>(singleValue));
		return;
	}

	RefPtr<PropertyLineKeyframes<Vector2> > sizeOverLifeXYKeyframes =
		RefPtr<PropertyLineKeyframes<Vector2> >(new PropertyLineKeyframes<Vector2>);
	size_t propsCount = wrappedPropertyValues.size();
	for (size_t i = 0; i < propsCount; i ++)
	{
		Vector2 curValue(wrappedPropertyValues[i].v, wrappedPropertyValues[i].v);
		sizeOverLifeXYKeyframes->AddValue(wrappedPropertyValues[i].t, curValue);
	}
	
	this->sizeOverLifeXY = sizeOverLifeXYKeyframes;
}


ParticleLayer::eType ParticleLayer::StringToLayerType(const String& layerTypeName, eType defaultLayerType)
{
	int32 layerTypesCount = sizeof(layerTypeNamesInfoMap) / sizeof(*layerTypeNamesInfoMap);
	for (int32 i = 0; i < layerTypesCount; i ++)
	{
		if (layerTypeNamesInfoMap[i].layerTypeName == layerTypeName)
		{
			return layerTypeNamesInfoMap[i].layerType;
		}
	}
	
	return defaultLayerType;
}

String ParticleLayer::LayerTypeToString(eType layerType, const String& defaultLayerTypeName)
{
	int32 layerTypesCount = sizeof(layerTypeNamesInfoMap) / sizeof(*layerTypeNamesInfoMap);
	for (int32 i = 0; i < layerTypesCount; i ++)
	{
		if (layerTypeNamesInfoMap[i].layerType == layerType)
		{
			return layerTypeNamesInfoMap[i].layerTypeName;
		}
	}
	
	return defaultLayerTypeName;
}
};
