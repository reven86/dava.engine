#pragma once

#include <Tools/TextureCompression/TextureConverter.h>
#include <Tools/AssetCache/AssetCacheClient.h>

#include "Utils/SceneExporter/SceneExporter.h"

#include "CommandLine/CommandLineModule.h"
#include "Reflection/ReflectionRegistrator.h"

class SceneExporterTool : public CommandLineModule
{
public:
    SceneExporterTool(const DAVA::Vector<DAVA::String>& commandLine);

private:
    bool PostInitInternal() override;
    eFrameResult OnFrameInternal() override;
    void BeforeDestroyedInternal() override;
    void ShowHelpInternal() override;

    enum eAction : DAVA::int8
    {
        ACTION_NONE = -1,

        ACTION_EXPORT_FILE,
        ACTION_EXPORT_FOLDER,
        ACTION_EXPORT_FILELIST
    };

    SceneExporter::ExportedObjectCollection exportedObjects;
    DAVA::AssetCacheClient::ConnectionParams connectionsParams;

    DAVA::FilePath inFolder;
    DAVA::FilePath outFolder;

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
    bool forceCompressTextures = false;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(SceneExporterTool, CommandLineModule)
    {
        DAVA::ReflectionRegistrator<SceneExporterTool>::Begin()
        .ConstructorByPointer<DAVA::Vector<DAVA::String>>()
        .End();
    }
};
