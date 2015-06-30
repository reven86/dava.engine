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


#ifndef __DAVAENGINE_RENDER_TECHNIQUE_H__
#define __DAVAENGINE_RENDER_TECHNIQUE_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/HashMap.h"
#include "Base/FastNameMap.h"
#include "Scene3D/DataNode.h"
#include "Render/RenderState.h"
#include "Render/Material/NMaterialConsts.h"
#include "Render/Shader.h"
#include "Render/RenderState.h"
#include "Base/Introspection.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Base/AllocatorFactory.h"

namespace DAVA
{
class YamlNode;
class RenderTechniquePass
{
public:
    IMPLEMENT_POOL_ALLOCATOR(RenderTechniquePass, 64);
    
    RenderTechniquePass(const FastName & _shaderName,
						const FastNameSet & _uniqueDefines,
						RenderState * _renderState);
    ~RenderTechniquePass();
    
    Shader * CompileShader(const FastNameSet& materialDefines);
    
    inline const FastName & GetShaderName() const { return shaderName; }
    inline RenderState * GetRenderState() const { return renderState; }
    inline const FastNameSet & GetUniqueDefineSet() { return uniqueDefines; }
    
protected:
    FastName shaderName;
    RenderState * renderState;
    FastNameSet uniqueDefines;
};

class RenderTechnique : public BaseObject
{
public:
	
    IMPLEMENT_POOL_ALLOCATOR(RenderTechnique, 64);

    RenderTechnique(const FastName & name);
    ~RenderTechnique();

    void AddRenderTechniquePass(const FastName& passName,
								const FastName & _shaderName,
								const FastNameSet & _uniqueDefines,
								RenderState * _renderState);

    inline const FastName & GetName() const { return name; };
    inline uint32 GetIndexByName(const FastName & fastName) const { return nameIndexMap.at(fastName); };
    inline RenderTechniquePass * GetPassByIndex(uint32 index) const { return renderTechniquePassArray[index]; }
	inline const FastNameSet & GetLayersSet() const { return layersSet; };
//    inline const Vector<RenderLayerID> & GetLayersIDs() const { return layerIDs; };

	inline const FastName& GetPassName(uint32 index) const
	{
		return nameIndexMap.keyByIndex(index);
	}
	
	inline RenderTechniquePass* GetPassByName(const FastName & fastName) const
	{
		uint32 index = GetIndexByName(fastName);
		return GetPassByIndex(index);
	}
	
	inline uint32 GetPassCount() const
	{
		return static_cast<uint32>(renderTechniquePassArray.size());
	}
	
	inline uint16 GetTechniqueId() const
	{
		return techniqueId;
	}
    
protected:
	
	static uint16 techinqueSequenceId;
	
protected:
	
    FastName name;
    Vector<RenderTechniquePass*> renderTechniquePassArray;
    HashMap<FastName, uint32> nameIndexMap;
	FastNameSet layersSet;
	uint16 techniqueId;
	
//    Vector<RenderLayerID> layerIDs;
    
    friend class RenderTechniqueSingleton;
};

class RenderTechniqueSingleton : public StaticSingleton<RenderTechniqueSingleton>
{
public:
    RenderTechnique * CreateTechniqueByName(const FastName & renderTechniquePathname);
    void ReleaseRenderTechnique(RenderTechnique * renderTechnique);

    bool PreloadRenderTechnique(const FastName & fastName);
    bool UnloadRenderTechnique(const FastName & fastName);
    
    // Remove all unreferenced render techniques
    void ClearRenderTechniques();

protected:
    bool LoadRenderTechnique(const FastName & fastName, RenderTechnique * targetTechnique);
    bool LoadRenderTechniqueFromYamlNode(const YamlNode * rootNode, RenderTechnique * targetTechnique);
    HashMap<FastName, RenderTechnique*> renderTechniqueMap;
};

};

#endif // __DAVAENGINE_RENDER_TECHNIQUE_H__

