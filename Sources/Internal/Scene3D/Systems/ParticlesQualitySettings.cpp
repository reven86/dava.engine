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


#include "ParticlesQualitySettings.h"
#include "Utils/Utils.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/YamlNode.h"

namespace DAVA
{
ParticlesQualitySettings::QualitySheet::QualitySheet()
{
}

ParticlesQualitySettings::QualitySheet::QualitySheet(const QualitySheet& src)
    : selector(src.selector)
    , folder(src.folder)
    , qualitiesCount(src.qualitiesCount)
    , score(src.score)
{
}

ParticlesQualitySettings::QualitySheet::QualitySheet(const Selector& selector_, const Folder& folder_, uint32 qualitiesCount_)
    : selector(selector_)
    , folder(folder_)
    , qualitiesCount(qualitiesCount_)
{
    RecalculateScore();
}

bool ParticlesQualitySettings::QualitySheet::IsMatched(const FastName& quality, const Set<FastName>& tagsCloud) const
{
    if (!selector.qualities.empty() && selector.qualities.count(quality) == 0)
    {
        return false;
    }

    if (tagsCloud.size() < selector.tags.size())
    {
        return false;
    }

    for (const FastName& tag : selector.tags)
    {
        if (tagsCloud.count(tag) == 0)
        {
            return false;
        }
    }

    return true;
}

int32 ParticlesQualitySettings::QualitySheet::GetScore() const
{
    return score;
}

void ParticlesQualitySettings::QualitySheet::RecalculateScore()
{
    int32 qualitiesScore = selector.qualities.empty() ? 0 : (qualitiesCount - (int32)selector.qualities.size());
    int32 tagsScore = (int32)selector.tags.size() * 100;
    score = qualitiesScore + tagsScore;
}

DAVA::FilePath ParticlesQualitySettings::QualitySheet::Folder::MakeFilepath(const FilePath& originalFilepath) const
{
    String pathString = originalFilepath.GetStringValue();
    StringReplace(pathString, folderToReplace, replaceWithFolder);
    FilePath newFilepath = pathString;
    return newFilepath;
}

ParticlesQualitySettings::ParticlesQualitySettings()
{
}

ParticlesQualitySettings::~ParticlesQualitySettings()
{
}

void ParticlesQualitySettings::LoadFromYaml(const YamlNode* rootNode)
{
    const YamlNode* particlesNode = rootNode->Get("particles");
    if (nullptr == particlesNode)
    {
        return;
    }
    qualities.clear();
    const YamlNode* qualitiesNode = particlesNode->Get("qualities");
    if (qualitiesNode != nullptr && qualitiesNode->GetType() == YamlNode::TYPE_ARRAY)
    {
        for (uint32 i = 0; i < qualitiesNode->GetCount(); ++i)
        {
            const YamlNode* qualityNode = qualitiesNode->Get(i);
            if (qualityNode != nullptr && qualityNode->GetType() == YamlNode::TYPE_STRING)
            {
                qualities.push_back(qualityNode->AsFastName());
            }
        }
        DVASSERT(qualities.size() == qualitiesNode->GetCount());
    }

    defaultQualityIndex = -1;
    const YamlNode* defaultNode = particlesNode->Get("default");
    if (defaultNode != nullptr && defaultNode->GetType() == YamlNode::TYPE_STRING)
    {
        defaultQualityIndex = GetQualityIndex(defaultNode->AsFastName());
        DVASSERT(defaultQualityIndex != -1);
    }

    currentQualityIndex = -1;
    const YamlNode* currentNode = particlesNode->Get("current");
    if (currentNode != nullptr && currentNode->GetType() == YamlNode::TYPE_STRING)
    {
        currentQualityIndex = GetQualityIndex(currentNode->AsFastName());
        DVASSERT(currentQualityIndex != -1);
    }

    qualitySheets.clear();
    const YamlNode* qualitySheetsNode = particlesNode->Get("qualitySheets");
    if (qualitySheetsNode != nullptr && qualitySheetsNode->GetType() == YamlNode::TYPE_ARRAY)
    {
        for (uint32 i = 0; i < qualitySheetsNode->GetCount(); ++i)
        {
            const YamlNode* qualitySheetNode = qualitySheetsNode->Get(i);
            if (qualitySheetNode != nullptr && qualitySheetNode->GetType() == YamlNode::TYPE_MAP)
            {
                QualitySheet::Selector selector;
                const YamlNode* selectorNode = qualitySheetNode->Get("selector");
                if (selectorNode != nullptr && selectorNode->GetType() == YamlNode::TYPE_MAP)
                {
                    const YamlNode* selectorQualitiesNode = selectorNode->Get("qualities");
                    if (selectorQualitiesNode != nullptr && selectorQualitiesNode->GetType() == YamlNode::TYPE_ARRAY)
                    {
                        for (uint32 i = 0; i < selectorQualitiesNode->GetCount(); ++i)
                        {
                            const YamlNode* selectorQualityNode = selectorQualitiesNode->Get(i);
                            if (selectorQualityNode != nullptr && selectorQualityNode->GetType() == YamlNode::TYPE_STRING)
                            {
                                selector.qualities.insert(selectorQualityNode->AsFastName());
                            }
                        }
                        DVASSERT(selector.qualities.size() == selectorQualitiesNode->GetCount());
                    }

                    const YamlNode* selectorTagsNode = selectorNode->Get("tags");
                    if (selectorTagsNode != nullptr && selectorTagsNode->GetType() == YamlNode::TYPE_ARRAY)
                    {
                        for (uint32 i = 0; i < selectorTagsNode->GetCount(); ++i)
                        {
                            const YamlNode* selectorTagNode = selectorTagsNode->Get(i);
                            if (selectorTagNode != nullptr && selectorTagNode->GetType() == YamlNode::TYPE_STRING)
                            {
                                selector.tags.insert(selectorTagNode->AsFastName());
                            }
                        }
                        DVASSERT(selector.tags.size() == selectorTagsNode->GetCount());
                    }
                }

                QualitySheet::Folder folder;
                const YamlNode* replaceFolderNode = qualitySheetNode->Get("replaceFolder");
                if (replaceFolderNode != nullptr && replaceFolderNode->GetType() == YamlNode::TYPE_ARRAY && replaceFolderNode->GetCount() == 2)
                {
                    folder.folderToReplace = replaceFolderNode->Get(0)->AsString();
                    folder.replaceWithFolder = replaceFolderNode->Get(1)->AsString();
                }

                qualitySheets.push_back(QualitySheet(selector, folder, (uint32)qualities.size()));
            }
        }
        DVASSERT(qualitySheets.size() == qualitySheetsNode->GetCount());
    }

    auto sortPred = [](const QualitySheet& fSheet, const QualitySheet& sSheet) {
        return fSheet.GetScore() > sSheet.GetScore();
    };
    std::sort(qualitySheets.begin(), qualitySheets.end(), sortPred);

    tagsCloud.clear();
    const YamlNode* tagsCloudNode = particlesNode->Get("tagsCloud");
    if (tagsCloudNode != nullptr && tagsCloudNode->GetType() == YamlNode::TYPE_ARRAY)
    {
        for (uint32 i = 0; i < tagsCloudNode->GetCount(); ++i)
        {
            const YamlNode* tagNode = tagsCloudNode->Get(i);
            if (tagNode != nullptr && tagNode->GetType() == YamlNode::TYPE_STRING)
            {
                tagsCloud.insert(tagNode->AsFastName());
            }
        }
    }
}

size_t ParticlesQualitySettings::GetQualitiesCount() const
{
    return qualities.size();
}

DAVA::FastName ParticlesQualitySettings::GetQualityName(size_t index) const
{
    FastName ret;

    if (index < qualities.size())
    {
        ret = qualities[index];
    }

    return ret;
}

DAVA::FastName ParticlesQualitySettings::GetCurrentQuality() const
{
    return GetQualityName(currentQualityIndex);
}

void ParticlesQualitySettings::SetCurrentQuality(const FastName& name)
{
    int32 index = GetQualityIndex(name);
    if (index == -1)
    {
        DVASSERT_MSG(0, Format("No %s particles quality", name.c_str()).c_str());
        return;
    }

    if (currentQualityIndex != index)
    {
        currentQualityIndex = index;
        filepathSelector.reset();
    }
}

const Set<FastName>& ParticlesQualitySettings::GetTagsCloud() const
{
    return tagsCloud;
}

void ParticlesQualitySettings::SetTagsCloud(const Set<FastName>& newTagsCloud)
{
    if (tagsCloud != newTagsCloud)
    {
        tagsCloud = newTagsCloud;
        filepathSelector.reset();
    }
}

bool ParticlesQualitySettings::HasTag(const FastName& tag) const
{
    return (tagsCloud.count(tag) != 0);
}

void ParticlesQualitySettings::AddTag(const FastName& tag)
{
    auto result = tagsCloud.insert(tag);
    if (result.second)
    {
        filepathSelector.reset();
    }
}

void ParticlesQualitySettings::RemoveTag(const FastName& tag)
{
    if (tagsCloud.count(tag) != 0)
    {
        tagsCloud.erase(tag);
        filepathSelector.reset();
    }
}

const ParticlesQualitySettings::FilepathSelector* ParticlesQualitySettings::GetOrCreateFilepathSelector()
{
    if (nullptr == filepathSelector)
    {
        filepathSelector.reset(new FilepathSelector(*this));
    }
    return filepathSelector.get();
}

int32 ParticlesQualitySettings::GetQualityIndex(const FastName& name) const
{
    for (size_t i = 0; i < qualities.size(); ++i)
    {
        if (qualities[i] == name)
        {
            return static_cast<int32>(i);
        }
    }

    return -1;
}

ParticlesQualitySettings::FilepathSelector::FilepathSelector(const ParticlesQualitySettings& settings)
{
    const auto& qualities = settings.qualities;

    if (!qualities.empty() &&
        settings.defaultQualityIndex != -1 &&
        settings.currentQualityIndex != -1)
    {
        int32 defaultIndex = settings.defaultQualityIndex;
        int32 currentIndex = settings.currentQualityIndex;

        int32 step = (currentIndex < defaultIndex) ? 1 : -1;

        Set<size_t> actualSheetsIndexes;
        while (true)
        {
            const FastName& quality = qualities[currentIndex];
            for (size_t index = 0, size = settings.qualitySheets.size(); index < size; ++index)
            {
                const ParticlesQualitySettings::QualitySheet& qualitySheet = settings.qualitySheets[index];
                if (qualitySheet.IsMatched(quality, settings.tagsCloud) &&
                    actualSheetsIndexes.count(index) == 0)
                {
                    actualSheetsIndexes.insert(index);
                    actualSheets.push_back(qualitySheet);
                }
            }

            if (currentIndex == defaultIndex)
            {
                break;
            }

            currentIndex += step;
            if (currentIndex >= (int32)qualities.size() || currentIndex < 0)
            {
                break;
            }
        }
    }
}

FilePath ParticlesQualitySettings::FilepathSelector::SelectFilepath(const FilePath& originalFilepath) const
{
    for (const QualitySheet& qualitySheet : actualSheets)
    {
        FilePath alternativeFilepath = qualitySheet.GetFolder().MakeFilepath(originalFilepath);

        if (alternativeFilepath == originalFilepath)
        {
            return originalFilepath;
        }

        if (FileSystem::Instance()->Exists(alternativeFilepath))
        {
            return alternativeFilepath;
        }
    }

    return originalFilepath;
}
}