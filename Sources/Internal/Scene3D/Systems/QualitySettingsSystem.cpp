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
#include "FileSystem/YamlParser.h"

namespace DAVA
{

QualitySettingsSystem::QualitySettingsSystem()
    : curTextureQuality(0)
{

}

void QualitySettingsSystem::Load(const FilePath &path)
{
    if(path.Exists())
    {
        YamlParser *parser = YamlParser::Create(path);
        YamlNode *rootNode = parser->GetRootNode();

        if(NULL != rootNode)
        {
            textureQualities.clear();
            materialGroups.clear();

            // materials
            const YamlNode *materialGroupsNode = rootNode->Get("materials");
            if(NULL != materialGroupsNode)
            {
                for(int i = 0; i < materialGroupsNode->GetCount(); ++i)
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
                    for(int i = 0; i < qualitiesNode->GetCount(); ++i)
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
        }

        parser->Release();
    }
}

size_t QualitySettingsSystem::GetTxQualityCount() const
{
    return textureQualities.size();
}

FastName QualitySettingsSystem::GetTxQualityName(size_t index) const
{
    FastName ret;

    if(index < textureQualities.size())
    {
        ret = textureQualities[index].name;
    }

    return ret;
}

FastName QualitySettingsSystem::GetCurTxQuality() const
{
    return GetTxQualityName(curTextureQuality);
}

void QualitySettingsSystem::SetCurTxQuality(const FastName &name)
{
    for(size_t i = 0; i < textureQualities.size(); ++i)
    {
        if(textureQualities[i].name == name)
        {
            curTextureQuality = i;
            return;
        }
    }

    DVASSERT(0 && "Not such quality");
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

    DVASSERT(NULL != ret && "No such quality");

    return ret;
}

size_t QualitySettingsSystem::GetMaQualityGroupCount() const
{
    return materialGroups.size();
}

FastName QualitySettingsSystem::GetMaQualityGroupName(size_t index) const
{
    FastName ret;

    if(index < materialGroups.size())
    {
        ret = materialGroups.keyByIndex(index);
    }

    return ret;
}

size_t QualitySettingsSystem::GetMaQualityCount(const FastName &group) const
{
    size_t ret = 0;

    if(materialGroups.count(group) > 0)
    {
        ret = materialGroups[group].qualities.size();
    }

    return ret;
}

FastName QualitySettingsSystem::GetMaQualityName(const FastName &group, size_t index) const
{
    FastName ret;

    if(materialGroups.count(group) > 0 && index < materialGroups[group].qualities.size())
    {
        ret = materialGroups[group].qualities[index].qualityName;
    }

    return ret;
}

FastName QualitySettingsSystem::GetCurMaQuality(const FastName &group) const
{
    FastName ret;

    if(materialGroups.count(group) > 0)
    {
        ret = GetMaQualityName(group, materialGroups[group].curQuality);
    }

    return ret;
}

void QualitySettingsSystem::SetCurMaQuality(const FastName &group, const FastName &quality)
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

const MaterialQuality* QualitySettingsSystem::GetMaQuality(const FastName &group, const FastName &quality) const
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

    DVASSERT(NULL != ret && "No such quality");

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

void QualitySettingsSystem::UpdateEntityAfterLoad(Entity *entity)
{
	if(qualityOptions.empty() || (NULL == entity)) return;

	Vector<Entity *> entitiesWithQualityComponent;
	entity->GetChildEntitiesWithComponent(entitiesWithQualityComponent, Component::QUALITY_SETTINGS_COMPONENT);

	if(entitiesWithQualityComponent.empty()) return;

	RemoveModelsByType(entitiesWithQualityComponent);
}

void QualitySettingsSystem::RemoveModelsByType( const Vector<Entity *> & models )
{
	uint32 count = (uint32)models.size();
	for(uint32 m = 0; m < count; ++m)
	{
		QualitySettingsComponent * comp = GetQualitySettingsComponent(models[m]);

		if(IsOptionEnabled(comp->GetModelType()) == false)
		{
			Entity *parent = models[m]->GetParent();
			parent->RemoveNode(models[m]);
		}
	}
}

bool QualitySettingsSystem::NeedLoadEntity(const Entity *entity)
{
    QualitySettingsComponent * comp = GetQualitySettingsComponent(entity);
    if(comp)
    {
        return IsOptionEnabled(comp->GetModelType());
    }
    
    return true;
}


}