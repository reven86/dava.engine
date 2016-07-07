#ifndef __SCENE_EXPORTER_TOOL_H__
#define __SCENE_EXPORTER_TOOL_H__

#include "CommandLine/CommandLineTool.h"
#include "TextureCompression/TextureConverter.h"
#include "AssetCache/AssetCache.h"
#include "AssetCache/AssetCacheClient.h"

#include "CommandLine/SceneExporter/SceneExporter.h"

class SceneExporterTool : public CommandLineTool
{
    enum eAction : DAVA::int8
    {
        ACTION_NONE = -1,

        ACTION_EXPORT_FILE,
        ACTION_EXPORT_FOLDER,
        ACTION_EXPORT_FILELIST
    };

public:
    SceneExporterTool();

private:
    void ConvertOptionsToParamsInternal() override;
    bool InitializeInternal() override;

    void ProcessInternal() override;
    DAVA::FilePath GetQualityConfigPath() const override;

    SceneExporter::ExportedObjectCollection exportedObjects;
    DAVA::AssetCacheClient::ConnectionParams connectionsParams;

    DAVA::FilePath inFolder;
    DAVA::FilePath outFolder;
    DAVA::FilePath qualityConfigPath;

    DAVA::String filename;
    DAVA::String foldername;
    DAVA::FilePath fileListPath;

    eAction commandAction = ACTION_NONE;
    SceneExporter::eExportedObjectType commandObject = SceneExporter::OBJECT_NONE;

    DAVA::Vector<DAVA::eGPUFamily> requestedGPUs;
    DAVA::TextureConverter::eConvertQuality quality = DAVA::TextureConverter::ECQ_DEFAULT;

    bool optimizeOnExport = true;
    bool useAssetCache = false;
    bool useHDTextures = false;
};


#endif // __SCENE_EXPORTER_TOOL_H__
