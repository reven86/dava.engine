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
#include "Render/Highlevel/ParticleLayerBatch.h"

#include "FileSystem/YamlParser.h"
#include "Particles/Particle.h"
#include "Particles/ParticleForce.h"
#include "Particles/ParticlePropertyLine.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{

class ParticleEmitter;
	
/**	
	\ingroup particlesystem
	\brief This class is core of our particle system. It does all ground work. 
	ParticleEmitter contain an array of ParticleLayers. In most cases you'll not need to use this class directly 
	and should use ParticleEmitter instead. 
	
	Few cases when you actually need ParticleLayers: 
	- You want to get information about layer lifeTime or layer sprite
	- You want to change something on the fly inside layer
 */
class ParticleLayer : public BaseObject
{
public:
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
protected:
	virtual ~ParticleLayer();
public:
	ParticleLayer();
	
	/**
		\brief Function to clone particle layer
		This function used inside ParticleEmitter class to clone layers. You can use 
		\returns particle layer with same properties as this one
	 */	
	virtual ParticleLayer * Clone(ParticleLayer * dstLayer = 0);

	/**
		\brief This function restarts this layer.
		You can delete particles at restart. 
		\param[in] isDeleteAllParticles if it's set to true layer deletes all previous particles that was generated
	 */
	void Restart(bool isDeleteAllParticles = true);
	
	/**
		\brief This function restarts this layer. It should be used only if layer is looped
		You can delete particles at restart. 
		\param[in] isDeleteAllParticles if it's set to true layer deletes all previous particles that was generated
	 */
	void RestartLoop(bool isDeleteAllParticles = true);
	
	/**
	 \brief Enable/disable loop otion.
	 If loop option is enabled, layer will automatically restart after it's lifeTime ends.
	 Option is disbled by default.
	 \param[in] autoRestart enable autorestart if true
	 */
	void SetLooped(bool isLopped);

	/**
	 \brief Get isLooped state.
	 \returns current layer autorestart state.
	 */
	bool GetLooped();
	
	/**
	 \brief Set looped layer delta time.
	 DeltaTime handle time delay between each layer loop
	 \param[in] deltaTime time delay between layer restarts
	 */
	void SetDeltaTime(float32 deltaTime);
	
	/**
	 \brief Get delta time.
	 \returns current deltaTime delay
	 */
	float32 GetDeltaTime();
	
	/**
	 \brief Set delta time variation
	 DeltaVariation defines maximum shift of deltaTime
	 For each loop we should calculate random deltaTime shift in range [0..deltaVariation]
	 \param[in] deltaVariation time variation for deltaTime
	 */
	void SetDeltaVariation(float32 deltaVariation);
	
	/**
	 \brief Get delta variation.
	 \returns current delta variation maximum shift
	 */
	float32 GetDeltaVariation();
	
	/**
	 \brief Set loop time variation
	 loopVariation defines maximum shift of loop life (loopLife = endTime - startTime)
	 For each loop we should calculate random loopLife shift in range [0..loopVariation]
	 \param[in] loopVariation time variation for loop life
	 */
	void SetLoopVariation(float32 loopVariation);
	
	/**
	 \brief Get loop variation.
	 \returns current loop variation maximum shift
	 */
	float32 GetLoopVariation();

	/**
	 \brief Set end time of looped layer.
	 endTime defines the time at which looped layer should stop "playing" and
	 wait for ParticleEmitter restart
	 \param[in] deltaTime time delay between layer restarts
	 */
	void SetLoopEndTime(float32 endTime);
	
	/**
	 \brief Get isLooped state.
	 \returns current autorestart state.
	 */
	float32 GetLoopEndTime();
	
	/**
		\brief This function retrieve current particle count from current layer.
		\returns particle count
	 */
	inline int32 GetParticleCount();
	
	/**
		\brief This function updates layer properties and layer particles. 
	 */
	void Update(float32 timeElapsed, bool generateNewParticles = true);
	
	/**
		\brief This function draws layer properties and layer particles. 
	 */
	virtual void Draw(Camera * camera);

	/**
		\brief it is not implemented for old 2d particles yet - they just use draw.
		ParticleLayer3d uses it to prepare render data.
	 */
	virtual void PrepareRenderData(Camera * camera);
	
	/** 
		\brief Function to set emitter for layer. 
		IMPORTANT: This function save weak pointer to parent emitter. Emitter hold strong references to all child layers.
		This function used internally in emitter, but in some situations. 
	*/
	ParticleEmitter* GetEmitter() const;
	void SetEmitter(ParticleEmitter * emitter);
	
	/**
		\brief Set sprite for the current layer
	 */
	void SetSprite(Sprite * sprite);
	/**
		\brief Get current layer sprite.
	 */
	Sprite * GetSprite();
	
	/**
		\brief Function to load layer from yaml node.
		Normally this function is called from ParticleEmitter. 	 
	 */
	virtual void LoadFromYaml(const FilePath & configPath, const YamlNode * node);

	/**
     \brief Function to save layer to yaml node.
     Normally this function is called from ParticleEmitter.
	 */
    void SaveToYamlNode(YamlNode* parentNode, int32 layerIndex);

	/**
		\brief Get head(first) particle of the layer.
		Can be used to iterate through the particles'.
	 */
	Particle * GetHeadParticle();

    float32 GetLayerTime();

    // Whether this layer is Long Layer?
    virtual bool IsLong() {return false;};
	virtual void SetLong(bool /*value*/) {};
    
	RenderBatch * GetRenderBatch();

	DAVA_DEPRECATED(void SetAdditive(bool additive));
	DAVA_DEPRECATED(bool GetAdditive() const);
	virtual void SetBlendMode(eBlendMode sFactor, eBlendMode dFactor);	
	eBlendMode GetBlendSrcFactor();
	eBlendMode GetBlendDstFactor();

	virtual void SetFog(bool enable);
	bool IsFogEnabled();

	virtual void SetFrameBlend(bool enable);
	bool IsFrameBlendEnabled();

	void SetInheritPosition(bool inherit);
	bool GetInheritPosition() const {return inheritPosition;}

	// Logic to work with Particle Forces.
	void AddForce(ParticleForce* force);
	void RemoveForce(ParticleForce* force);
	void RemoveForce(int32 forceIndex);

	void UpdateForce(int32 forceIndex, RefPtr< PropertyLine<Vector3> > force,
							 RefPtr< PropertyLine<Vector3> > forceVariation,
							 RefPtr< PropertyLine<float32> > forceOverLife);

	// Playback speed.
	void SetPlaybackSpeed(float32 value);
	float32 GetPlaybackSpeed();

	// Statistics for particles - count and area they use.
	int32 GetActiveParticlesCount();
	float32 GetActiveParticlesArea();

	// Create the inner emitter where needed.
	virtual void CreateInnerEmitter();

	// Get thhe inner emitter, if exists.
	ParticleEmitter* GetInnerEmitter();

	// Stop and remove Inner Emitter.
	virtual void RemoveInnerEmitter();

	// Control the Inner Emitter.
	virtual void PauseInnerEmitter(bool _isPaused);

	// Enable/disable the layer.
	inline bool GetDisabled();
	void SetDisabled(bool value);

	// Get/set the Pivot Point for the layer.
	Vector2 GetPivotPoint() const;
	void SetPivotPoint(const Vector2& value);

	// Handle the situation when layer is removed from the system.
	void HandleRemoveFromSystem();

	bool IsLodActive(int32 lod);
	void SetLodActive(int32 lod, bool active);	

	void GetModifableLines(List<ModifiablePropertyLineBase *> &modifiables);

protected:
	void GenerateNewParticle(int32 emitIndex);
	void GenerateSingleParticle();

	void RestartLayerIfNeed();

	void DeleteAllParticles();
	
	void AddToList(Particle * particle);
	void RemoveFromList(Particle * particle);
	
	void RunParticle(Particle * particle);
	void ProcessParticle(Particle * particle, float32 timeElapsed);	
	
    void SaveForcesToYamlNode(YamlNode* layerNode);

	void UpdateFrameTimeline();
	
	void CleanupForces();
	
	void FillSizeOverlifeXY(RefPtr< PropertyLine<float32> > sizeOverLife);
	
	// Convert from Layer Type to its name and vice versa.
	eType StringToLayerType(const String& layerTypeName, eType defaultLayerType);
	String LayerTypeToString(eType layerType, const String& defaultLayerTypeName);

	// Update the playback speed for all Inner Emitters.
	void UpdatePlaybackSpeedForInnerEmitters(float value);

	// Get the draw pivot point (mid of sprite + layer pivot point).
	inline Vector2 GetDrawPivotPoint();

	// list of particles
	Particle *	head;
	int32		count;
	int32		limit;
	

	// time properties for the particle layer
	float32 particlesToGenerate;
	float32 layerTime;
	float32 loopLayerTime;
	
	// parent emitter (required to know emitter params during generation)
	ParticleEmitter * emitter;
	// particle layer sprite
	Sprite 			* sprite;
	FilePath		spritePath;

	ParticleLayerBatch * renderBatch;

	bool		isDisabled;	
	bool		isLooped;

	eBlendMode srcBlendFactor, dstBlendFactor;

	bool enableFog;

	bool enableFrameBlend;

	bool inheritPosition;  //for super emitter - if true the whole emitter would be moved, otherwise just emission point

	float32		playbackSpeed;

	Vector2		layerPivotPoint;

	Vector<bool> activeLODS;	

	List<std::pair<String, ModifiablePropertyLineBase *> > modifiables;

public:
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
	//RefPtr< PropertyLine<float32> > sizeOverLife;
	
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

	float32		alignToMotion;

	float32		startTime;
	float32		endTime;
	// Layer loop paremeters
	float32		deltaTime;
	float32 	deltaVariation;
	float32 	loopVariation;
	float32 	loopEndTime;
	
	void		UpdateLayerTime(float32 startTime, float32 endTime);
	

	eType		type;

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
	void RecalculateVariation();
	float32 GetRandomFactor();
	float32 currentLoopVariation;
	float32 currentDeltaVariation;

public:
    
    INTROSPECTION_EXTEND(ParticleLayer, BaseObject,
                         NULL
//        MEMBER(particlesToGenerate, "Particles To Generate", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//        MEMBER(layerTime, "Layer Time", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//        MEMBER(relativeSpriteName, "Relative Sprite Name", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//
//        MEMBER(renderBatch, "Render Batch", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//                         
//        MEMBER(pivotPoint, "Pivot Point", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//                         
//        MEMBER(life, "Life", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//        MEMBER(lifeVariation, "Life Variation", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
                         
//        MEMBER(number, "Number", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//        MEMBER(numberVariation, "Number Variation", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//                         
//        MEMBER(size, "Size", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//        MEMBER(sizeVariation, "Size Variation", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//        MEMBER(sizeOverLife, "Size Over Life", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//                         
//                         
//        MEMBER(velocity, "Velocity", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//        MEMBER(velocityVariation, "Velocity Variation", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//        MEMBER(velocityOverLife, "Velocity Over Life", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//                         
//        MEMBER(forces, "Forces", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//        MEMBER(forcesVariation, "Forces Variation", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//        MEMBER(forcesOverLife, "Forces Over Life", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//                         
//        MEMBER(spin, "Spin", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//        MEMBER(spinVariation, "Spin Variation", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//        MEMBER(spinOverLife, "Spin Over Life", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//
//        MEMBER(colorRandom, "Color Random", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//        MEMBER(alphaOverLife, "Alpha Over Life", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//        MEMBER(colorOverLife, "Color Over Life", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//        MEMBER(frameOverLife, "Frame Over Life", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//        MEMBER(alignToMotion, "Align To Motion", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//        MEMBER(additive, "Additive", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//        MEMBER(startTime, "Start Time", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//        MEMBER(endTime, "End Time", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//        MEMBER(frameStart, "Frame Start", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//        MEMBER(frameEnd, "Frame End", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//                         
//      MEMBER(type, "Type", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//                         
//        MEMBER(isDisabled, "Is Disabled", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
    );
};

inline int32 ParticleLayer::GetParticleCount()
{
	return count;
}
	
inline bool ParticleLayer::GetDisabled()
{
	return this->isDisabled;
}

inline Vector2 ParticleLayer::GetDrawPivotPoint()
{
	if (this->sprite)
	{
		return Vector2(	sprite->GetWidth() / 2 + layerPivotPoint.x, sprite->GetHeight() / 2 + layerPivotPoint.y);
	}
	
	return layerPivotPoint;
}
};

#endif // __DAVAENGINE_PARTICLE_LAYER_H__