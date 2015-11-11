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

#include "CommandLine/TextureDescriptor/TextureDescriptorTool.h"
#include "CommandLine/TextureDescriptor/TextureDescriptorUtils.h"

#include "Render/PixelFormatDescriptor.h"

using namespace DAVA;

namespace OptionName
{
static const String Folder = "-folder";
static const String File = "-file";
static const String Resave = "-resave";
static const String Create = "-create";
static const String SetCompression = "-setcompression";
static const String SetPreset = "-setpreset";
static const String Force = "-f";
static const String Quality = "-quality";
static const String Mipmaps = "-m";
static const String Convert = "-convert";
static const String PresetOpt = "-preset";

const String MakeNameForGPU(eGPUFamily gpuFamily)
{
    return ("-" + GPUFamilyDescriptor::GetGPUName(gpuFamily));
}
}

TextureDescriptorTool::TextureDescriptorTool()
    : CommandLineTool("-texdescriptor")
{
    options.AddOption(OptionName::Folder, VariantType(String("")), "Path to folder for operation on descriptors");
    options.AddOption(OptionName::File, VariantType(String("")), "Pathname of descriptor");

    options.AddOption(OptionName::Resave, VariantType(false), "Resave descriptor files in target folder");
    options.AddOption(OptionName::Create, VariantType(false), "Create descriptors for image files");
    options.AddOption(OptionName::SetCompression, VariantType(false), "Set compression parameters for descriptor or for all descriptors in folder");
    options.AddOption(OptionName::SetPreset, VariantType(false), "Update descriptor(s) with given preset data");

    options.AddOption(OptionName::Force, VariantType(false), "Enables force running of selected operation");
    options.AddOption(OptionName::Mipmaps, VariantType(false), "Enables generation of mipmaps");
    options.AddOption(OptionName::Convert, VariantType(false), "Runs compression of texture after setting of compression parameters");
    options.AddOption(OptionName::Quality, VariantType(static_cast<uint32>(TextureConverter::ECQ_DEFAULT)), "Quality of pvr/etc compression. Default is 4 - the best quality. Available values [0-4]");
    options.AddOption(OptionName::PresetOpt, VariantType(String("")), "Uses preset for an operation");

    //GPU
    for (uint8 gpu = GPU_POWERVR_IOS; gpu < GPU_DEVICE_COUNT; ++gpu)
    {
        eGPUFamily gpuFamily = static_cast<eGPUFamily>(gpu);
        options.AddOption(OptionName::MakeNameForGPU(gpuFamily), VariantType(String("")), Format("Pixel format for %s gpu", GPUFamilyDescriptor::GetGPUName(gpuFamily).c_str()), true);
    }
}

void TextureDescriptorTool::ConvertOptionsToParamsInternal()
{
    folderPathname = options.GetOption(OptionName::Folder).AsString();
    filePathname = options.GetOption(OptionName::File).AsString();
    presetPath = options.GetOption(OptionName::PresetOpt).AsString();

    const uint32 qualityValue = options.GetOption(OptionName::Quality).AsUInt32();
    quality = Clamp(static_cast<TextureConverter::eConvertQuality>(qualityValue), TextureConverter::ECQ_FASTEST, TextureConverter::ECQ_VERY_HIGH);

    if (options.GetOption(OptionName::Resave).AsBool())
    {
        commandAction = ACTION_RESAVE_DESCRIPTORS;
    }
    else if (options.GetOption(OptionName::Create).AsBool())
    {
        commandAction = ACTION_CREATE_DESCRIPTORS;
    }
    else if (options.GetOption(OptionName::SetCompression).AsBool())
    {
        commandAction = ACTION_SET_COMPRESSION;
    }
    else if (options.GetOption(OptionName::SetPreset).AsBool())
    {
        commandAction = ACTION_SET_PRESET;
    }

    forceModeEnabled = options.GetOption(OptionName::Force).AsBool();
    convertEnabled = options.GetOption(OptionName::Convert).AsBool();
    generateMipMaps = options.GetOption(OptionName::Mipmaps).AsBool();
    for (uint8 gpu = GPU_POWERVR_IOS; gpu < GPU_DEVICE_COUNT; ++gpu)
    {
        const eGPUFamily gpuFamily = static_cast<eGPUFamily>(gpu);
        const String optionName = OptionName::MakeNameForGPU(gpuFamily);

        DAVA::VariantType gpuOption = options.GetOption(optionName);

        const FastName formatName(options.GetOption(optionName).AsString().c_str());
        const PixelFormat pixelFormat = PixelFormatDescriptor::GetPixelFormatByName(formatName);

        if (pixelFormat != FORMAT_INVALID)
        {
            TextureDescriptor::Compression compression;
            compression.format = pixelFormat;
            compression.compressToWidth = compression.compressToHeight = 0;
            if (options.GetOptionVaulesCount(optionName) > 2)
            {
                const String widthStr = options.GetOption(optionName, 1).AsString();
                const String heightStr = options.GetOption(optionName, 2).AsString();
                if (!widthStr.empty() && !heightStr.empty())
                {
                    compression.compressToWidth = atoi(widthStr.c_str());
                    compression.compressToHeight = atoi(heightStr.c_str());

                    if (compression.compressToWidth < 0 || compression.compressToHeight < 0)
                    {
                        AddError(Format("Wrong size parameters for gpu: %s", optionName.c_str()));
                        compression.compressToWidth = compression.compressToHeight = 0;
                    }
                }
            }

            compressionParams[gpuFamily] = compression;
        }
    }
}

bool TextureDescriptorTool::InitializeInternal()
{
    if (commandAction == ACTION_NONE)
    {
        AddError("Action was specified");
        return false;
    }

    if (filePathname.IsEmpty() && folderPathname.IsEmpty())
    {
        AddError("File or folder parameter was not specified");
        return false;
    }

    if (!folderPathname.IsEmpty())
    {
        folderPathname.MakeDirectoryPathname();
    }

    switch (commandAction)
    {
    case TextureDescriptorTool::ACTION_SET_COMPRESSION:
    {
        if (compressionParams.empty())
        {
            AddError("GPU params were not specified");
            return false;
        }
        break;
    }
    case TextureDescriptorTool::ACTION_SET_PRESET:
    {
        if (presetPath.IsEmpty())
        {
            AddError("Preset was not specified");
            return false;
        }
        break;
    }
    }

    return true;
}

void TextureDescriptorTool::ProcessInternal()
{
    switch(commandAction)
    {
    case ACTION_RESAVE_DESCRIPTORS:
    {
        if (!folderPathname.IsEmpty())
        {
            TextureDescriptorUtils::ResaveDescriptorsForFolder(folderPathname);
        }
        else
        {
            TextureDescriptorUtils::ResaveDescriptor(filePathname);
        }
        break;
    }
    case ACTION_CREATE_DESCRIPTORS:
    {
        if (!folderPathname.IsEmpty())
        {
            TextureDescriptorUtils::CreateDescriptorsForFolder(folderPathname, presetPath);
        }
        else
        {
            TextureDescriptorUtils::CreateDescriptorIfNeed(filePathname, presetPath);
        }
        break;
    }
    case ACTION_SET_COMPRESSION:
    {
        if (!folderPathname.IsEmpty())
        {
            TextureDescriptorUtils::SetCompressionParamsForFolder(folderPathname, compressionParams, convertEnabled, forceModeEnabled, quality, generateMipMaps);
        }
        else
        {
            TextureDescriptorUtils::SetCompressionParams(filePathname, compressionParams, convertEnabled, forceModeEnabled, quality, generateMipMaps);
        }
        break;
    }
    case ACTION_SET_PRESET:
    {
        if (!folderPathname.IsEmpty())
        {
            TextureDescriptorUtils::SetPresetForFolder(folderPathname, presetPath, convertEnabled, quality);
        }
        else
        {
            TextureDescriptorUtils::SetPreset(filePathname, presetPath, convertEnabled, quality);
        }
    }
    default:
    {
        Logger::Error("[TextureDescriptorTool::Process] Unhandled action!");
        break;
    }
    }
}
