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


#ifndef __DAVAENGINE_PARTICLE_LAYER_H__
#define __DAVAENGINE_PARTICLE_LAYER_H__

#include "Base/BaseTypes.h"
#include "Base/RefPtr.h"
#include "Base/DynamicObjectCache.h"
#include "Render/2D/Sprite.h"

#include "FileSystem/YamlParser.h"
#include "Particles/Particle.h"
#include "Particles/ParticleForce.h"
#include "Particles/ParticlePropertyLine.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{

class ParticleEmitter;
	
/**	
	In most cases you'll not need to use this class directly 
	and should use ParticleEffect instead. 
	
	Few cases when you actually need ParticleLayers: 
	- You want to get information about layer lifeTime or layer sprite
	- You want to change something on the fly inside layer
 */
struct ParticleLayer : public BaseObject
{	
	enum eType 
	{
		TYPE_SINGLE_PARTICLE,
		TYPE_PARTICLES,				// default for any particle layer loaded from yaml file
		TYPE_SUPEREMITTER_PARTICLES
	};

	enum eParticleOrientation
	{
		PARTICLE_ORIENTATION_CAMERA_FACING = 1<<0, //default
		PARTICLE_ORIENTATION_X_FACING = 1<<1,
		PARTICLE_ORIENTATION_Y_FACING = 1<<2,
		PARTICLE_ORIENTATION_Z_FACING = 1<<3,
		PARTICLE_ORIENTATION_WORLD_ALIGN = 1<<4 
	};

    enum eDegradeStrategy
    {
        DEGRADE_KEEP = 0,
        DEGRADE_CUT_PARTICLES = 1,
        DEGRADE_REMOVE = 2
    };


	ParticleLayer();
	virtual ~ParticleLayer();		
	virtual ParticleLayer * Clone();
	
	void LoadFromYaml(const FilePath & configPath, const YamlNode * node, bool preserveInheritPosition);	
    void SaveToYamlNode(const FilePath & configPath, YamlNode* parentNode, int32 layerIndex);
	void SaveForcesToYamlNode(YamlNode* layerNode);

	void AddForce(ParticleForce* force);
	void RemoveForce(ParticleForce* force);
	void RemoveForce(int32 forceIndex);
	void CleanupForces();
		

	void GetModifableLines(List<ModifiablePropertyLineBase *> &modifiables);
	
	// Convert from Layer Type to its name and vice versa.
	eType StringToLayerType(const String& layerTypeName, eType defaultLayerType);
	String LayerTypeToString(eType layerType, const String& defaultLayerTypeName);

	void UpdateLayerTime(float32 startTime, float32 endTime);

	bool IsLodActive(int32 lod);	
	void SetLodActive(int32 lod, bool active);
	
	Sprite 			* sprite;
	void SetSprite(Sprite * sprite);
	Vector2		layerPivotPoint;
	Vector2		layerPivotSizeOffsets; //precached for faster bbox computation
	void SetPivotPoint(Vector2 pivot);

	FilePath		spritePath;		
	bool isLooped;
	bool isLong;
    eBlending blending;
    bool enableFog;
    bool enableFrameBlend;
	bool inheritPosition;  //for super emitter - if true the whole emitter would be moved, otherwise just emission point	
	
	bool isDisabled;	

	
	Vector<bool> activeLODS;		

	String			layerName;

	/*
	 Properties of particle layer that describe particle system logic
	 */
	RefPtr< PropertyLine<float32> > life;				// in seconds
	RefPtr< PropertyLine<float32> > lifeVariation;		// variation part of life that added to particle life during generation of the particle
	
	RefPtr< PropertyLine<float32> > number;				// number of particles per second
	RefPtr< PropertyLine<float32> > numberVariation;	// variation part of number that added to particle count during generation of the particle
	
	RefPtr< PropertyLine<Vector2> > size;				// size of particles in pixels 
	RefPtr< PropertyLine<Vector2> > sizeVariation;		// size variation in pixels
	RefPtr< PropertyLine<Vector2> > sizeOverLifeXY;	
	
	RefPtr< PropertyLine<float32> > velocity;			// velocity in pixels
	RefPtr< PropertyLine<float32> > velocityVariation;	
	RefPtr< PropertyLine<float32> > velocityOverLife;
	
	Vector<ParticleForce*> forces;
	
	RefPtr< PropertyLine<float32> > spin;				// spin of angle / second
	RefPtr< PropertyLine<float32> > spinVariation;
	RefPtr< PropertyLine<float32> > spinOverLife;
	bool randomSpinDirection;
		
	
	RefPtr< PropertyLine<Color> > colorRandom;		
	RefPtr< PropertyLine<float32> > alphaOverLife;	
	RefPtr< PropertyLine<Color> > colorOverLife;	

	RefPtr< PropertyLine<float32> > angle;				// sprite angle in degrees
	RefPtr< PropertyLine<float32> > angleVariation;		// variations in degrees

	RefPtr< PropertyLine<float32> > animSpeedOverLife;	

	

	float32		startTime;
	float32		endTime;
	// Layer loop paremeters
	float32		deltaTime;
	float32 	deltaVariation;
	float32 	loopVariation;
	float32 	loopEndTime;		
	

	eType		type;

    eDegradeStrategy degradeStrategy;

	int32 particleOrientation;

	bool		frameOverLifeEnabled;
	float32		frameOverLifeFPS;
	bool		randomFrameOnStart;
	bool		loopSpriteAnimation;

	//for long particles
	float32 scaleVelocityBase;
	float32 scaleVelocityFactor;

	ParticleEmitter* innerEmitter;
	FilePath	innerEmitterPath;

private:
	struct LayerTypeNamesInfo
	{
		eType layerType;
		String layerTypeName;
	};
    static const LayerTypeNamesInfo layerTypeNamesInfoMap[];

    void FillSizeOverlifeXY(RefPtr<PropertyLine<float32>> sizeOverLife);
    void UpdateSizeLine(PropertyLine<Vector2> *line, bool rescaleSize, bool swapXY); //conversion from old format
};
}

#endif // __DAVAENGINE_PARTICLE_LAYER_H__