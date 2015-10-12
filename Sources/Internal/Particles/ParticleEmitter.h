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


#ifndef __DAVAENGINE_PARTICLE_EMITTER_H__
#define __DAVAENGINE_PARTICLE_EMITTER_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/BaseObject.h"
#include "Base/DynamicObjectCache.h"
#include "Base/RefPtr.h"
#include "Particles/ParticlePropertyLine.h"
#include "Animation/Animation.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/Highlevel/IRenderUpdatable.h"
#include "Particles/ParticleLayer.h"
#include "FileSystem/FilePath.h"
#include "Scene3D/SceneFile/SerializationContext.h"

namespace DAVA 
{	
/*this class is proxy to load old effect hierarchy
  */
class PartilceEmitterLoadProxy : public RenderObject
{
public:
	PartilceEmitterLoadProxy();
	String emitterFilename;
	void Load(KeyedArchive *archive, SerializationContext *serializationContext); 
};

class ParticleEmitter : public BaseObject
{
public:
    static ParticleEmitter *LoadEmitter(const FilePath & filename);

	enum eType
	{
		EMITTER_POINT,
		EMITTER_RECT,
		EMITTER_ONCIRCLE_VOLUME,
		EMITTER_ONCIRCLE_EDGES,
		EMITTER_SHOCKWAVE
	};

	enum eState
	{
		STATE_PLAYING,    
		STATE_STOPPING, //emitter is stopping - no new particle generation, still need to update and recalculate
		STATE_STOPPED   //emitter is completely stopped - no processing at all
	};

	ParticleEmitter();	
	ParticleEmitter * Clone();
	
	bool LoadFromYaml(const FilePath & pathName, bool preserveInheritPosition=false);
    void SaveToYaml(const FilePath & pathName);    	
	
	void AddLayer(ParticleLayer * layer);
	ParticleLayer* GetNextLayer(ParticleLayer* layer);
	virtual void InsertLayer(ParticleLayer * layer, ParticleLayer * beforeLayer);	
	void RemoveLayer(ParticleLayer * layer);
	void RemoveLayer(int32 index);	
	void MoveLayer(ParticleLayer * layer, ParticleLayer * beforeLayer);					    	      

	void UpdateEmptyLayerNames();
	void UpdateLayerNameIfEmpty(ParticleLayer* layer, int32 index);			
	void LoadParticleLayerFromYaml(const YamlNode* yamlNode, bool preserveInheritPosition);
	
	// Invert the emission vector coordinates for backward compatibility.
	void InvertEmissionVectorCoordinates();

    String GetEmitterTypeName();

	void GetModifableLines(List<ModifiablePropertyLineBase *> &modifiables);

	void Cleanup(bool needCleanupLayers = true);
	void CleanupLayers();


	FilePath configPath;	
	eType	emitterType;	    	

	Vector<ParticleLayer*> layers;	
	bool shortEffect;	
	
	float32 lifeTime;

	FastName name;

	RefPtr< PropertyLine<Vector3> > emissionVector;    
	RefPtr< PropertyLine<float32> > emissionRange;
	RefPtr< PropertyLine<float32> > radius;
	RefPtr< PropertyLine<Color> > colorOverLife;
	RefPtr< PropertyLine<Vector3> > size;	    

    RefPtr< PropertyLine<float32> > emissionAngle;
    RefPtr< PropertyLine<float32> > emissionAngleVariation;

protected:
    virtual ~ParticleEmitter();

private:
	bool requireDeepClone;


#if defined (USE_FILEPATH_IN_MAP)
    using EmitterCacheMap = Map<FilePath, ParticleEmitter*>;
#else //#if defined (USE_FILEPATH_IN_MAP)
    using EmitterCacheMap = Map<String, ParticleEmitter*>;
#endif //#if defined (USE_FILEPATH_IN_MAP)		    
    void ReleaseFromCache(const FilePath& name);

    static EmitterCacheMap emitterCache;
public:
    static bool FORCE_DEEP_CLONE;
};

}

#endif // __DAVAENGINE_PARTICLE_EMITTER_H__
