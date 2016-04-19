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


#ifndef __SCENE_EXPORTER_H__
#define __SCENE_EXPORTER_H__

#include "Utils/StringFormat.h"

#include "CommandLine/SceneUtils/SceneUtils.h"
#include "TextureCompression/TextureConverter.h"
#include "AssetCache/AssetCache.h"

namespace DAVA
{
class TextureDescriptor;
class Scene;
class AssetCacheClient;
}

class SceneExporter final
{
public:
    enum eExportedObjectType : DAVA::int32
    {
        OBJECT_NONE = -1,

        OBJECT_SCENE = 0,
        OBJECT_TEXTURE,
        OBJECT_HEIGHTMAP,

        OBJECT_COUNT
    };

    struct ExportedObject
    {
        ExportedObject(eExportedObjectType type_, DAVA::String path)
            : type(type_)
            , relativePathname(std::move(path))
        {
        }

        eExportedObjectType type = OBJECT_NONE;
        DAVA::String relativePathname;
    };

    struct Params
    {
        DAVA::FilePath dataFolder;
        DAVA::FilePath dataSourceFolder;

        DAVA::Vector<DAVA::eGPUFamily> exportForGPUs;
        DAVA::TextureConverter::eConvertQuality quality = DAVA::TextureConverter::eConvertQuality::ECQ_DEFAULT;

        bool optimizeOnExport = false;
        bool useHDTextures = false;
    };

    SceneExporter() = default;
    ~SceneExporter();

    using ExportedObjectCollection = DAVA::Vector<ExportedObject>;

    void SetExportingParams(const SceneExporter::Params& exportingParams);

    bool ExportScene(DAVA::Scene* scene, const DAVA::FilePath& scenePathname, ExportedObjectCollection& exportedObjects);
    void ExportObjects(const ExportedObjectCollection& exportedObjects);

    void SetCacheClient(DAVA::AssetCacheClient* cacheClient, DAVA::String machineName, DAVA::String runDate, DAVA::String comment);

private:
    void ExportSceneFile(const DAVA::FilePath& scenePathname, const DAVA::String& sceneLink); //with cache
    void ExportTextureFile(const DAVA::FilePath& descriptorPathname, const DAVA::String& descriptorLink);
    void ExportHeightmapFile(const DAVA::FilePath& heightmapPathname, const DAVA::String& heightmapLink);

    void ExportSceneFileInternal(const DAVA::FilePath& scenePathname, ExportedObjectCollection& exportedObjects); //without cache

    bool ExportTextures(const DAVA::Vector<DAVA::eGPUFamily>& gpus, DAVA::TextureConverter::eConvertQuality quality, DAVA::TextureDescriptor& descriptor);

    bool CopyFile(const DAVA::FilePath& filePath) const;
    bool CopyFile(const DAVA::FilePath& filePath, const DAVA::String& fileLink) const;

    DAVA::AssetCacheClient* cacheClient = nullptr;
    DAVA::AssetCache::CachedItemValue::Description cacheItemDescription;

    SceneExporter::Params exportingParams;
};


#endif // __SCENE_EXPORTER_H__
