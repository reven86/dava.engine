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


#ifndef __DAVAENGINE_SCENEVERSION_H__
#define __DAVAENGINE_SCENEVERSION_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Base/FastName.h"
#include "Base/Singleton.h"
#include "FileSystem/FilePath.h"


//#define USER_VERSIONING_DEBUG_FEATURES

namespace DAVA
{
static const int32 SCENE_FILE_CURRENT_VERSION = 18;
static const int32 SCENE_FILE_MINIMAL_SUPPORTED_VERSION = 9;

static const int32 CUSTOM_PROPERTIES_COMPONENT_SAVE_SCENE_VERSION = 8;
static const int32 COMPONENTS_BY_NAME_SAVE_SCENE_VERSION = 10;
static const int32 OLD_LODS_SCENE_VERSION = 11;
static const int32 TREE_ANIMATION_SCENE_VERSION = 12;
static const int32 PREREQUIRED_BINORMAL_SCENE_VERSION = 13;
static const int32 SHADOW_VOLUME_SCENE_VERSION = 15;
static const int32 DEPRECATED_MATERIAL_FLAGS_SCENE_VERSION = 16;
static const int32 ALPHATEST_VALUE_FLAG_SCENE_VERSION = 17;
static const int32 RHI_SCENE_VERSION = 18;

class VersionInfo
    : public Singleton<VersionInfo>
{
public:
    using TagsMap = Map<String, uint32>;
    struct SceneVersion
    {
        uint32 version;
        TagsMap tags;

        SceneVersion()
            : version(0){}
        bool IsValid() const{ return version > 0; }
    };
    using VersionMap = Map<uint32, SceneVersion>;

    enum eStatus
    {
        VALID,
        COMPATIBLE,
        INVALID,
    };

public:
    VersionInfo();
    ~VersionInfo();

    const SceneVersion& GetCurrentVersion() const;
    eStatus TestVersion(const SceneVersion& version) const;
    String UnsupportedTagsMessage(const SceneVersion& version) const;
    String NoncompatibleTagsMessage(const SceneVersion& version) const;

#ifdef USER_VERSIONING_DEBUG_FEATURES
    VersionMap& Versions();
    VersionMap GetDefaultVersionHistory();
#else
    const VersionMap& Versions() const;
#endif

    static void AddVersion(VersionMap& versions, const SceneVersion& version);

private:
    VersionMap GetVersionHistory();
    void SetCurrentBranch();
    TagsMap GetTags(uint32 minVersion = 0) const;

    VersionMap versionMap;

    static TagsMap GetTagsDiff(const TagsMap& from, const TagsMap& what);
    static String FormatTagsString(const TagsMap& tags);
};

};

#endif
