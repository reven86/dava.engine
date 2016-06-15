#include "CommandLine/TextureDescriptor/TextureDescriptorTool.h"
#include "CommandLine/TextureDescriptor/TextureDescriptorUtils.h"
#include "CommandLine/OptionName.h"

#include "Render/PixelFormatDescriptor.h"
#include "Render/GPUFamilyDescriptor.h"

using namespace DAVA;

namespace TextureDescriptorToolLocal
{
Vector<FilePath> LoadPathesFromFile(const FilePath& filePath)
{
    ScopedPtr<File> fileWithPathes(File::Create(filePath, File::OPEN | File::READ));
    if (!fileWithPathes)
    {
        Logger::Error("Can't open file %s", filePath.GetStringValue().c_str());
        return Vector<FilePath>();
    }

    Vector<FilePath> pathes;
    do
    {
        String path = fileWithPathes->ReadLine();
        if (path.empty())
        {
            Logger::Warning("Found empty string in file %s", filePath.GetStringValue().c_str());
            break;
        }
        pathes.emplace_back(path);
    } while (!fileWithPathes->IsEof());

    return pathes;
}
}

TextureDescriptorTool::TextureDescriptorTool()
    : CommandLineTool("-texdescriptor")
{
    options.AddOption(OptionName::Folder, VariantType(String("")), "Path to folder for operation on descriptors");
    options.AddOption(OptionName::File, VariantType(String("")), "Pathname of descriptor");

    options.AddOption(OptionName::ProcessFileList, VariantType(String("")), "Pathname to file with descriptor pathes");
    options.AddOption(OptionName::PresetsList, VariantType(String("")), "Pathname to file with yaml pathes");

    options.AddOption(OptionName::Resave, VariantType(false), "Resave descriptor files in target folder");
    options.AddOption(OptionName::Create, VariantType(false), "Create descriptors for image files");
    options.AddOption(OptionName::SetCompression, VariantType(false), "Set compression parameters for descriptor or for all descriptors in folder");
    options.AddOption(OptionName::SetPreset, VariantType(false), "Update descriptor(s) with given preset data");
    options.AddOption(OptionName::SavePreset, VariantType(false), "Save preset of descriptor(s)");

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
    filesList = options.GetOption(OptionName::ProcessFileList).AsString();
    presetPath = options.GetOption(OptionName::PresetOpt).AsString();
    presetsList = options.GetOption(OptionName::PresetsList).AsString();

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
    else if (options.GetOption(OptionName::SavePreset).AsBool())
    {
        commandAction = ACTION_SAVE_PRESET;
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
            if (options.GetOptionValuesCount(optionName) > 2)
            {
                const String widthStr = options.GetOption(optionName, 1).AsString();
                const String heightStr = options.GetOption(optionName, 2).AsString();
                if (!widthStr.empty() && !heightStr.empty())
                {
                    compression.compressToWidth = atoi(widthStr.c_str());
                    compression.compressToHeight = atoi(heightStr.c_str());

                    if (compression.compressToWidth < 0 || compression.compressToHeight < 0)
                    {
                        Logger::Error("Wrong size parameters for gpu: %s", optionName.c_str());
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
    if (commandAction == TextureDescriptorTool::ACTION_NONE)
    {
        Logger::Error("Action was not specified");
        return false;
    }

    if (commandAction == TextureDescriptorTool::ACTION_SAVE_PRESET)
    {
        if ((!filePathname.IsEmpty() && presetPath.IsEmpty()) || (filePathname.IsEmpty() && !presetPath.IsEmpty()))
        {
            Logger::Error("File or preset parameter was not specified");
            return false;
        }
        else if ((!filesList.IsEmpty() && presetsList.IsEmpty()) || (filesList.IsEmpty() && !presetsList.IsEmpty()))
        {
            Logger::Error("FilesList or presetsList parameter was not specified");
            return false;
        }
    }
    else
    {
        if (filePathname.IsEmpty() && folderPathname.IsEmpty())
        {
            Logger::Error("File or folder parameter was not specified");
            return false;
        }

        if (!folderPathname.IsEmpty())
        {
            folderPathname.MakeDirectoryPathname();
        }

        if ((commandAction == TextureDescriptorTool::ACTION_SET_COMPRESSION) && compressionParams.empty())
        {
            Logger::Error("GPU params were not specified");
            return false;
        }
        else if ((commandAction == TextureDescriptorTool::ACTION_SET_PRESET) && presetPath.IsEmpty())
        {
            Logger::Error("Preset was not specified");
            return false;
        }
    }

    return true;
}

void TextureDescriptorTool::ProcessInternal()
{
    switch (commandAction)
    {
    case ACTION_RESAVE_DESCRIPTORS:
    {
        if (folderPathname.IsEmpty())
        {
            TextureDescriptorUtils::ResaveDescriptor(filePathname);
        }
        else
        {
            TextureDescriptorUtils::ResaveDescriptorsForFolder(folderPathname);
        }

        break;
    }
    case ACTION_CREATE_DESCRIPTORS:
    {
        if (folderPathname.IsEmpty())
        {
            TextureDescriptorUtils::CreateOrUpdateDescriptor(filePathname, presetPath);
        }
        else
        {
            TextureDescriptorUtils::CreateDescriptorsForFolder(folderPathname, presetPath);
        }
        break;
    }
    case ACTION_SET_COMPRESSION:
    {
        if (folderPathname.IsEmpty())
        {
            TextureDescriptorUtils::SetCompressionParams(filePathname, compressionParams, convertEnabled, forceModeEnabled, quality, generateMipMaps);
        }
        else
        {
            TextureDescriptorUtils::SetCompressionParamsForFolder(folderPathname, compressionParams, convertEnabled, forceModeEnabled, quality, generateMipMaps);
        }
        break;
    }
    case ACTION_SET_PRESET:
    {
        if (folderPathname.IsEmpty())
        {
            TextureDescriptorUtils::SetPreset(filePathname, presetPath, convertEnabled, quality);
        }
        else
        {
            TextureDescriptorUtils::SetPresetForFolder(folderPathname, presetPath, convertEnabled, quality);
        }
        break;
    }
    case ACTION_SAVE_PRESET:
    {
        if (!filesList.IsEmpty())
        {
            TextureDescriptorUtils::SavePreset(TextureDescriptorToolLocal::LoadPathesFromFile(filesList), TextureDescriptorToolLocal::LoadPathesFromFile(presetsList));
        }
        else
        {
            TextureDescriptorUtils::SavePreset({ filePathname }, { presetPath });
        }
        break;
    }
    default:
    {
        Logger::Error("[TextureDescriptorTool::Process] Unhandled action!");
        break;
    }
    }
}
