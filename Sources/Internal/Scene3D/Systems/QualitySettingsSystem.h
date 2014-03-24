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
    QualitySettingsSystem();

    void Load(const FilePath &path);

    // textures quality
    size_t GetTxQualityCount() const;
    FastName GetTxQualityName(size_t index) const;

    FastName GetCurTxQuality() const;
    void SetCurTxQuality(const FastName &name);

    const TextureQuality* GetTxQuality(const FastName &name) const;

    // materials quality
    size_t GetMaQualityGroupCount() const;
    FastName GetMaQualityGroupName(size_t index) const;
    
    size_t GetMaQualityCount(const FastName &group) const;
    FastName GetMaQualityName(const FastName &group, size_t index) const;

    FastName GetCurMaQuality(const FastName &group) const;
    void SetCurMaQuality(const FastName &group, const FastName &quality);

    const MaterialQuality* GetMaQuality(const FastName &group, const FastName &quality) const;

    // ------------------------------------------

	void EnableOption(const FastName & option, bool enabled);
    
	bool IsOptionEnabled(const FastName & option) const;

    bool NeedLoadEntity(const Entity *entity);
    
	void UpdateEntityAfterLoad(Entity *entity);

protected:

	void RemoveModelsByType(const Vector<Entity *> & models);


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

    // textures
    int curTextureQuality;
    Vector<TXQ> textureQualities;

    // materials
    FastNameMap<MAGrQ> materialGroups;

	FastNameMap<bool> qualityOptions;
};
	
}

#endif //__DAVAENGINE_SCENE3D_QUALITYSETTINGSSYSTEM_H__