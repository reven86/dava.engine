#include "CommandLine/ImageSplitter/ImageSplitterTool.h"
#include "ImageTools/ImageTools.h"
#include "CommandLine/OptionName.h"

using namespace DAVA;

ImageSplitterTool::ImageSplitterTool()
    : CommandLineTool("-imagesplitter")
{
    options.AddOption(OptionName::Split, VariantType(false), "Action is splitting image file on channels");
    options.AddOption(OptionName::Merge, VariantType(false), "Action is merging channels into one file");
    options.AddOption(OptionName::File, VariantType(String("")), "Full pathname of the image file");
    options.AddOption(OptionName::Folder, VariantType(String("")), "full pathname of the folder with channels");
}

void ImageSplitterTool::ConvertOptionsToParamsInternal()
{
    filename = options.GetOption(OptionName::File).AsString();
    foldername = options.GetOption(OptionName::Folder).AsString();

    if (options.GetOption(OptionName::Split).AsBool())
    {
        commandAction = ACTION_SPLIT;
    }
    else if (options.GetOption(OptionName::Merge).AsBool())
    {
        commandAction = ACTION_MERGE;
    }
}

bool ImageSplitterTool::InitializeInternal()
{
    if (commandAction == ACTION_SPLIT)
    {
        if (filename.IsEmpty())
        {
            Logger::Error("Pathname of image file was not selected");
            return false;
        }
    }
    else if (commandAction == ACTION_MERGE)
    {
        if (foldername.IsEmpty())
        {
            Logger::Error("Input folder was not selected");
            return false;
        }
        foldername.MakeDirectoryPathname();
    }
    else
    {
        Logger::Error("Wrong action was selected");
        return false;
    }

    return true;
}

void ImageSplitterTool::ProcessInternal()
{
    if (commandAction == ACTION_SPLIT)
    {
        ImageTools::SplitImage(filename);
    }
    else if (commandAction == ACTION_MERGE)
    {
        ImageTools::MergeImages(foldername);
    }
}
