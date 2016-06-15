#ifndef __TEXTURE_DESCRIPTOR_TOOL_H__
#define __TEXTURE_DESCRIPTOR_TOOL_H__

#include "CommandLine/CommandLineTool.h"

#include "Render/TextureDescriptor.h"
#include "TextureCompression/TextureConverter.h"

class TextureDescriptorTool : public CommandLineTool
{
    enum eAction : DAVA::int32
    {
        ACTION_NONE = -1,

        ACTION_RESAVE_DESCRIPTORS,
        ACTION_CREATE_DESCRIPTORS,
        ACTION_SET_COMPRESSION,
        ACTION_SET_PRESET,
        ACTION_SAVE_PRESET
    };

public:
    TextureDescriptorTool();

private:
    void ConvertOptionsToParamsInternal() override;
    bool InitializeInternal() override;
    void ProcessInternal() override;

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


#endif // __TEXTURE_DESCRIPTOR_TOOL_H__
