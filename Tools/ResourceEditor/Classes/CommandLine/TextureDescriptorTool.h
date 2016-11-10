#pragma once

#include "Render/TextureDescriptor.h"
#include "TextureCompression/TextureConverter.h"
#include "CommandLine/CommandLineModule.h"

class TextureDescriptorTool : public CommandLineModule
{
public:
    TextureDescriptorTool(const DAVA::Vector<DAVA::String>& commandLine);

    static const DAVA::String Key;

private:
    bool PostInitInternal() override;
    eFrameResult OnFrameInternal() override;
    void ShowHelpInternal() override;

    void ReadCommandLine();
    bool ValidateCommandLine();

    enum eAction : DAVA::int32
    {
        ACTION_NONE = -1,

        ACTION_RESAVE_DESCRIPTORS,
        ACTION_CREATE_DESCRIPTORS,
        ACTION_SET_COMPRESSION,
        ACTION_SET_PRESET,
        ACTION_SAVE_PRESET
    };
    eAction commandAction = ACTION_NONE;

    DAVA::FilePath folderPathname;
    DAVA::FilePath filePathname;
    DAVA::FilePath presetPath;

    DAVA::FilePath filesList;
    DAVA::FilePath presetsList;

    bool forceModeEnabled = false;
    bool convertEnabled = false;
    bool generateMipMaps = false;

    DAVA::TextureConverter::eConvertQuality quality = DAVA::TextureConverter::ECQ_DEFAULT;
    DAVA::Map<DAVA::eGPUFamily, DAVA::TextureDescriptor::Compression> compressionParams;
};
