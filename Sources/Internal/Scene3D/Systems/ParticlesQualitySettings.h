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


#ifndef __DAVAENGINE_PARTICLES_QUALITY_SETTINGS_H__
#define __DAVAENGINE_PARTICLES_QUALITY_SETTINGS_H__

#include "Base/BaseTypes.h"
#include "Base/FastName.h"

namespace DAVA
{
class YamlNode;
class ParticlesQualitySettings
{
public:
    class QualitySheet
    {
    public:
        struct Selector
        {
            UnorderedSet<FastName> qualities;
            UnorderedSet<FastName> tags;
        };

        struct Action
        {
            bool Apply(const FilePath& originalPath, FilePath& alternativePath) const;

            FastName name;
            Vector<String> params;
        };

        QualitySheet();
        QualitySheet(const QualitySheet& src);
        QualitySheet(const Selector& selector, const Vector<Action>& actions, uint32 qualitiesCount);

        const Selector& GetSelector() const
        {
            return selector;
        }
        const Vector<Action>& GetActions() const
        {
            return actions;
        }

        bool IsMatched(const FastName& quality, const Set<FastName>& tagsCloud) const;
        bool Apply(const FilePath& originalPath, FilePath& alternativePath) const;

        int32 GetScore() const;

    private:
        void RecalculateScore();

        Selector selector;
        Vector<Action> actions;
        uint32 qualitiesCount = 0;
        uint32 score = 0;
    };

    class FilepathSelector
    {
    public:
        FilepathSelector(const ParticlesQualitySettings& settings);
        FilePath SelectFilepath(const FilePath& originalFilepath) const;

    private:
        Vector<QualitySheet> actualSheets;
    };

    ParticlesQualitySettings();
    ~ParticlesQualitySettings();

    void LoadFromYaml(const YamlNode* root);

    size_t GetQualitiesCount() const;
    FastName GetQualityName(size_t index) const;

    FastName GetCurrentQuality() const;
    void SetCurrentQuality(const FastName& name);

    const Set<FastName>& GetTagsCloud() const;
    void SetTagsCloud(const Set<FastName>& newTagsCloud);

    bool HasTag(const FastName& tag) const;
    void AddTag(const FastName& tag);
    void RemoveTag(const FastName& tag);

    const Vector<QualitySheet>& GetQualitySheets() const
    {
        return qualitySheets;
    }

    const FilepathSelector* GetOrCreateFilepathSelector();

private:
    int32 GetQualityIndex(const FastName& name) const;

    Vector<FastName> qualities;
    int32 defaultQualityIndex = -1;
    int32 currentQualityIndex = -1;
    Vector<QualitySheet> qualitySheets;
    Set<FastName> tagsCloud;

    std::unique_ptr<FilepathSelector> filepathSelector;
};
};
#endif // __DAVAENGINE_PARTICLES_QUALITY_SETTINGS_H__
