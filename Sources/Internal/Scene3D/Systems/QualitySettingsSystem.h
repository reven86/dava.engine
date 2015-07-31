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


#ifndef __DAVAENGINE_SCENE3D_QUALITYSETTINGSSYSTEM_H__
#define __DAVAENGINE_SCENE3D_QUALITYSETTINGSSYSTEM_H__

#include "Base/StaticSingleton.h"
#include "Base/FastNameMap.h"
#include "Math/Vector.h"

namespace DAVA
{

struct TextureQuality
{
    size_t albedoBaseMipMapLevel;
    size_t normalBaseMipMapLevel;
    Vector2 minSize;
    size_t weight;
};

struct MaterialQuality
{
    FastName qualityName;
    size_t weight;
};

class QualitySettingsComponent;
class QualitySettingsSystem: public StaticSingleton<QualitySettingsSystem>
{
public:
    static const FastName QUALITY_OPTION_VEGETATION_ANIMATION;
    static const FastName QUALITY_OPTION_STENCIL_SHADOW;
    static const FastName QUALITY_OPTION_WATER_DECORATIONS;

    QualitySettingsSystem();

    void Load(const FilePath &path);

    // textures quality
    size_t GetTextureQualityCount() const;
    FastName GetTextureQualityName(size_t index) const;

    FastName GetCurTextureQuality() const;
    void SetCurTextureQuality(const FastName &name);

    const TextureQuality* GetTxQuality(const FastName &name) const;

    // materials quality
    size_t GetMaterialQualityGroupCount() const;
    FastName GetMaterialQualityGroupName(size_t index) const;
    
    size_t GetMaterialQualityCount(const FastName &group) const;
    FastName GetMaterialQualityName(const FastName &group, size_t index) const;

    FastName GetCurMaterialQuality(const FastName &group) const;
    void SetCurMaterialQuality(const FastName &group, const FastName &quality);

    const MaterialQuality* GetMaterialQuality(const FastName &group, const FastName &quality) const;

    // sound quality
    size_t GetSFXQualityCount() const;
    FastName GetSFXQualityName(size_t index) const;

    FastName GetCurSFXQuality() const;
    void SetCurSFXQuality(const FastName &name);

    FilePath GetSFXQualityConfigPath(const FastName &name) const;
    FilePath GetSFXQualityConfigPath(size_t index) const;

    // ------------------------------------------

	void EnableOption(const FastName & option, bool enabled);
	bool IsOptionEnabled(const FastName & option) const;
    int32 GetOptionsCount() const;
    FastName GetOptionName(int32 index) const;

    bool IsQualityVisible(const Entity *entity);
    
	void UpdateEntityAfterLoad(Entity *entity);

    bool GetAllowCutUnusedVertexStreams();
    void SetAllowCutUnusedVertexStreams(bool cut);

    void SetKeepUnusedEntities(bool keep);
    bool GetKeepUnusedEntities();
         
    void UpdateEntityVisibility(Entity *e);    

protected:
    void UpdateEntityVisibilityRecursively(Entity *e, bool qualityVisible);	

protected:
    struct TXQ
    {
        FastName name;
        TextureQuality quality;
    };

    struct MAGrQ
    {
        size_t curQuality;
        Vector<MaterialQuality> qualities;
    };

    struct SFXQ
    {
        FastName name;
        FilePath configPath;
    };

    // textures
    int32 curTextureQuality;
    Vector<TXQ> textureQualities;

    // materials
    FastNameMap<MAGrQ> materialGroups;

    //sounds
    int32 curSoundQuality;
    Vector<SFXQ> soundQualities;

	FastNameMap<bool> qualityOptions;

    bool cutUnusedVertexStreams;

    bool keepUnusedQualityEntities; //for editor to prevent cutting entities with unused quality
};


inline void QualitySettingsSystem::SetKeepUnusedEntities(bool keep)
{
    keepUnusedQualityEntities = keep;
}

inline bool QualitySettingsSystem::GetKeepUnusedEntities()
{
    return keepUnusedQualityEntities;
}
	
}

#endif //__DAVAENGINE_SCENE3D_QUALITYSETTINGSSYSTEM_H__