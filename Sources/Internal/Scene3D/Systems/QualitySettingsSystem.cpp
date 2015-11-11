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


#include "Scene3D/Systems/QualitySettingsSystem.h"
#include "Scene3D/Components/QualitySettingsComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Scene.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/YamlParser.h"
#include "FileSystem/YamlNode.h"
#include "Render/Highlevel/RenderObject.h"

namespace DAVA
{

const FastName QualitySettingsSystem::QUALITY_OPTION_VEGETATION_ANIMATION("Vegetation Animation");
const FastName QualitySettingsSystem::QUALITY_OPTION_STENCIL_SHADOW("Stencil Shadows");
const FastName QualitySettingsSystem::QUALITY_OPTION_WATER_DECORATIONS("Water Decorations");

QualitySettingsSystem::QualitySettingsSystem()
    : curTextureQuality(0)
    , curSoundQuality(0)
    , cutUnusedVertexStreams(false) //default set to keep all streams
    , keepUnusedQualityEntities(false)
{
    Load("~res:/quality.yaml");

    EnableOption(QUALITY_OPTION_VEGETATION_ANIMATION, true);
    EnableOption(QUALITY_OPTION_STENCIL_SHADOW, true);
    EnableOption(QUALITY_OPTION_WATER_DECORATIONS, false);
}

void QualitySettingsSystem::Load(const FilePath &path)
{
    Logger::FrameworkDebug("Trying to load QUALITY from: %s", path.GetAbsolutePathname().c_str());

    if (FileSystem::Instance()->Exists(path))
    {
        YamlParser *parser = YamlParser::Create(path);
        YamlNode *rootNode = parser->GetRootNode();

        if(NULL != rootNode)
        {
            textureQualities.clear();
            materialGroups.clear();
            soundQualities.clear();

            // materials
            const YamlNode *materialGroupsNode = rootNode->Get("materials");
            if(NULL != materialGroupsNode)
            {
                for(uint32 i = 0; i < materialGroupsNode->GetCount(); ++i)
                {
                    const YamlNode *groupNode = materialGroupsNode->Get(i);
                    const YamlNode *name = groupNode->Get("group");
                    const YamlNode *values = groupNode->Get("quality");
                    const YamlNode *deflt = groupNode->Get("default");

                    FastName defQualityName;
                    if(NULL != deflt && deflt->GetType() == YamlNode::TYPE_STRING)
                    {
                        defQualityName = FastName(deflt->AsString().c_str());
                    }

                    if(NULL != name && NULL != values &&
                        name->GetType() == YamlNode::TYPE_STRING &&
                        values->GetType() == YamlNode::TYPE_ARRAY)
                    {
                        const Vector<YamlNode *> &v = values->AsVector();

                        MAGrQ maGr;
                        maGr.curQuality = 0;
                        maGr.qualities.reserve(v.size());

                        for(size_t j = 0; j < v.size(); ++j)
                        {
                            if(v[j]->GetType() == YamlNode::TYPE_STRING)
                            {
                                MaterialQuality mq;
                                mq.weight = j;
                                mq.qualityName = FastName(v[j]->AsString().c_str());

                                maGr.qualities.push_back(mq);

                                if(mq.qualityName == defQualityName)
                                {
                                    maGr.curQuality = j;
                                }
                            }
                        }

                        materialGroups.insert(FastName(name->AsString().c_str()), maGr);
                    }
                }
            }

            // textures
            const YamlNode *texturesNode = rootNode->Get("textures");
            if(NULL != texturesNode)
            {
                const YamlNode *defltTx = texturesNode->Get("default");
                const YamlNode *qualitiesNode = texturesNode->Get("qualities");

                if(NULL != qualitiesNode)
                {
                    FastName defTxQualityName;
                    if(NULL != defltTx && defltTx->GetType() == YamlNode::TYPE_STRING)
                    {
                        defTxQualityName = FastName(defltTx->AsString().c_str());
                    }

                    textureQualities.reserve(qualitiesNode->GetCount());
                    for(uint32 i = 0; i < qualitiesNode->GetCount(); ++i)
                    {
                        const YamlNode *qualityNode = qualitiesNode->Get(i);
                        const YamlNode *name = qualityNode->Get("quality");
                        const YamlNode *albedoNode = qualityNode->Get("albedo");
                        const YamlNode *normalNode = qualityNode->Get("normalmap");
                        const YamlNode *sizeNode = qualityNode->Get("minsize");

                        if(NULL != name && name->GetType() == YamlNode::TYPE_STRING &&
                            NULL != albedoNode && albedoNode->GetType() == YamlNode::TYPE_STRING &&
                            NULL != normalNode && normalNode->GetType() == YamlNode::TYPE_STRING &&
                            NULL != sizeNode && sizeNode->GetType() == YamlNode::TYPE_STRING)
                        {
                            TXQ txq;

                            txq.name = FastName(name->AsString().c_str());
                            txq.quality.weight = i;
                            txq.quality.albedoBaseMipMapLevel = albedoNode->AsUInt32();
                            txq.quality.normalBaseMipMapLevel = normalNode->AsUInt32();
                            txq.quality.minSize = Vector2(sizeNode->AsFloat(), sizeNode->AsFloat());

                            textureQualities.push_back(txq);

                            if(txq.name == defTxQualityName)
                            {
                                curTextureQuality = i;
                            }
                        }
                    }

                }
            }

            // sound
            const YamlNode *soundsNode = rootNode->Get("sounds");
            if(NULL != soundsNode)
            {
                const YamlNode *defltSfx = soundsNode->Get("default");
                const YamlNode *qualitiesNode = soundsNode->Get("qualities");

                if(NULL != qualitiesNode)
                {
                    FastName defSfxQualityName;
                    if(NULL != defltSfx && defltSfx->GetType() == YamlNode::TYPE_STRING)
                    {
                        defSfxQualityName = FastName(defltSfx->AsString().c_str());
                    }

                    soundQualities.reserve(qualitiesNode->GetCount());
                    for(uint32 i = 0; i < qualitiesNode->GetCount(); ++i)
                    {
                        const YamlNode *qualityNode = qualitiesNode->Get(i);
                        const YamlNode *name = qualityNode->Get("quality");
                        const YamlNode *confgNode = qualityNode->Get("configPath");

                        if(NULL != name && name->GetType() == YamlNode::TYPE_STRING &&
                           NULL != confgNode && confgNode->GetType() == YamlNode::TYPE_STRING)
                        {
                            SFXQ sfxq;

                            sfxq.name = FastName(name->AsString().c_str());
                            sfxq.configPath = FilePath(confgNode->AsString());

                            soundQualities.push_back(sfxq);

                            if(sfxq.name == defSfxQualityName)
                            {
                                curSoundQuality = i;
                            }
                        }
                    }

                }
            }
        }

        parser->Release();
    }

    Logger::FrameworkDebug("Done. TxQualities: %u, MaGrQualities: %u", textureQualities.size(), materialGroups.size());
}

size_t QualitySettingsSystem::GetTextureQualityCount() const
{
    return textureQualities.size();
}

FastName QualitySettingsSystem::GetTextureQualityName(size_t index) const
{
    FastName ret;

    if(index < textureQualities.size())
    {
        ret = textureQualities[index].name;
    }

    return ret;
}

FastName QualitySettingsSystem::GetCurTextureQuality() const
{
    return GetTextureQualityName(curTextureQuality);
}

void QualitySettingsSystem::SetCurTextureQuality(const FastName &name)
{
    for(size_t i = 0; i < textureQualities.size(); ++i)
    {
        if(textureQualities[i].name == name)
        {
            curTextureQuality = static_cast<int32>(i);
            return;
        }
    }

    DVASSERT(0 && "No such quality");
}

const TextureQuality* QualitySettingsSystem::GetTxQuality(const FastName &name) const
{
    const TextureQuality *ret = NULL;

    for(size_t i = 0; i < textureQualities.size(); ++i)
    {
        if(textureQualities[i].name == name)
        {
            ret = &textureQualities[i].quality;
            break;
        }
    }

    //DVASSERT(NULL != ret && "No such quality");

    return ret;
}

size_t QualitySettingsSystem::GetSFXQualityCount() const
{
    return soundQualities.size();
}

FastName QualitySettingsSystem::GetSFXQualityName(size_t index) const
{
    FastName ret;

    if(index < soundQualities.size())
    {
        ret = soundQualities[index].name;
    }

    return ret;
}

FastName QualitySettingsSystem::GetCurSFXQuality() const
{
    return GetSFXQualityName(curSoundQuality);
}

void QualitySettingsSystem::SetCurSFXQuality(const FastName &name)
{
    for(size_t i = 0; i < soundQualities.size(); ++i)
    {
        if(soundQualities[i].name == name)
        {
            curSoundQuality = static_cast<int32>(i);
            return;
        }
    }
}

FilePath QualitySettingsSystem::GetSFXQualityConfigPath(const FastName &name) const
{
    FilePath ret;

    for(size_t i = 0; i < soundQualities.size(); ++i)
    {
        if(soundQualities[i].name == name)
        {
            ret = soundQualities[i].configPath;
            break;
        }
    }

    return ret;
}

FilePath QualitySettingsSystem::GetSFXQualityConfigPath(size_t index) const
{
    FilePath ret;

    if(index < soundQualities.size())
    {
        ret = soundQualities[index].configPath;
    }

    return ret;
}

bool QualitySettingsSystem::GetAllowCutUnusedVertexStreams()
{
    return cutUnusedVertexStreams;
}
void QualitySettingsSystem::SetAllowCutUnusedVertexStreams(bool cut)
{
    cutUnusedVertexStreams = cut;
}

size_t QualitySettingsSystem::GetMaterialQualityGroupCount() const
{
    return materialGroups.size();
}

FastName QualitySettingsSystem::GetMaterialQualityGroupName(size_t index) const
{
    FastName ret;

    if(index < materialGroups.size())
    {
        ret = materialGroups.keyByIndex(index);
    }

    return ret;
}

size_t QualitySettingsSystem::GetMaterialQualityCount(const FastName &group) const
{
    size_t ret = 0;

    if(materialGroups.count(group) > 0)
    {
        ret = materialGroups[group].qualities.size();
    }

    return ret;
}

FastName QualitySettingsSystem::GetMaterialQualityName(const FastName &group, size_t index) const
{
    FastName ret;

    if(materialGroups.count(group) > 0 && index < materialGroups[group].qualities.size())
    {
        ret = materialGroups[group].qualities[index].qualityName;
    }

    return ret;
}

FastName QualitySettingsSystem::GetCurMaterialQuality(const FastName &group) const
{
    FastName ret;

    if(materialGroups.count(group) > 0)
    {
        ret = GetMaterialQualityName(group, materialGroups[group].curQuality);
    }

    return ret;
}

void QualitySettingsSystem::SetCurMaterialQuality(const FastName &group, const FastName &quality)
{
    if(materialGroups.count(group) > 0)
    {
        for(size_t i = 0; i < materialGroups[group].qualities.size(); ++i)
        {
            if(materialGroups[group].qualities[i].qualityName == quality)
            {
                materialGroups[group].curQuality = i;
                return;
            }
        }
    }

    DVASSERT(0 && "Not such quality");
}

const MaterialQuality* QualitySettingsSystem::GetMaterialQuality(const FastName &group, const FastName &quality) const
{
    const MaterialQuality *ret = NULL;

    if(materialGroups.count(group) > 0)
    {
        for(size_t i = 0; i < materialGroups[group].qualities.size(); ++i)
        {
            if(materialGroups[group].qualities[i].qualityName == quality)
            {
                ret = &materialGroups[group].qualities[i];
                break;
            }
        }
    }

    //DVASSERT(NULL != ret && "No such quality");

    return ret;
}



void QualitySettingsSystem::EnableOption( const FastName & option, bool enabled )
{
	qualityOptions[option] = enabled;
}

bool QualitySettingsSystem::IsOptionEnabled( const FastName & option ) const
{
	if(qualityOptions.count(option) > 0)
	{
		return qualityOptions[option];
	}

	return false;
}

int32 QualitySettingsSystem::GetOptionsCount() const
{
    return static_cast<int32>(qualityOptions.size());
}

FastName QualitySettingsSystem::GetOptionName(int32 index) const
{
    return qualityOptions.keyByIndex(index);
}

void QualitySettingsSystem::UpdateEntityAfterLoad(Entity *entity)
{
	if(qualityOptions.empty() || (NULL == entity)) return;

	Vector<Entity *> entitiesWithQualityComponent;
	entity->GetChildEntitiesWithComponent(entitiesWithQualityComponent, Component::QUALITY_SETTINGS_COMPONENT);

    for (size_t i = 0, sz = entitiesWithQualityComponent.size(); i< sz; ++i)
    {
        if (!IsQualityVisible(entitiesWithQualityComponent[i]))
        {
            if (keepUnusedQualityEntities)
            {
                UpdateEntityVisibilityRecursively(entitiesWithQualityComponent[i], false);
            }
            else
            {
                Entity *parent = entitiesWithQualityComponent[i]->GetParent();
                parent->RemoveNode(entitiesWithQualityComponent[i]);
            }
        }
    }
}


bool QualitySettingsSystem::IsQualityVisible(const Entity *entity)
{    
    QualitySettingsComponent * comp = GetQualitySettingsComponent(entity);
    if(comp)
    {
        if (comp->filterByType)
            return (!comp->modelType.IsValid())||IsOptionEnabled(comp->GetModelType());
        else
            return (GetCurMaterialQuality(comp->requiredGroup) == comp->requiredQuality);
    }
    
    return true;
}

void QualitySettingsSystem::UpdateEntityVisibility(Entity *e)
{
    QualitySettingsComponent * comp = GetQualitySettingsComponent(e);
    if (comp)
        UpdateEntityVisibilityRecursively(e, IsQualityVisible(e));
}



void QualitySettingsSystem::UpdateEntityVisibilityRecursively(Entity *e, bool qualityVisible)
{
    RenderObject *ro = GetRenderObject(e);
    if (ro)
    {
        if (qualityVisible)
            ro->AddFlag(RenderObject::VISIBLE_QUALITY);
        else
            ro->RemoveFlag(RenderObject::VISIBLE_QUALITY);
    }

    for (int32 i = 0, sz = e->GetChildrenCount(); i < sz; ++i)
        UpdateEntityVisibilityRecursively(e->GetChild(i), qualityVisible);
}


}