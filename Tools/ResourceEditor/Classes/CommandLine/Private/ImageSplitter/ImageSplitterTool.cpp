#include "CommandLine/ImageSplitterTool.h"
#include "ImageTools/ImageTools.h"
#include "CommandLine/Private/OptionName.h"

ImageSplitterTool::ImageSplitterTool(const DAVA::Vector<DAVA::String>& commandLine)
    : REConsoleModuleCommon(commandLine, "-imagesplitter")
{
    options.AddOption(OptionName::Split, DAVA::VariantType(false), "Action is splitting image file on channels");
    options.AddOption(OptionName::Merge, DAVA::VariantType(false), "Action is merging channels into one file");
    options.AddOption(OptionName::File, DAVA::VariantType(DAVA::String("")), "Full pathname of the image file");
    options.AddOption(OptionName::Folder, DAVA::VariantType(DAVA::String("")), "full pathname of the folder with channels");
}

bool ImageSplitterTool::PostInitInternal()
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

DAVA::TArc::ConsoleModule::eFrameResult ImageSplitterTool::OnFrameInternal()
{
    if (commandAction == ACTION_SPLIT)
    {
        ImageTools::SplitImage(filename);
    }
    else if (commandAction == ACTION_MERGE)
    {
        ImageTools::MergeImages(foldername);
    }

    return DAVA::TArc::ConsoleModule::eFrameResult::FINISHED;
}

void ImageSplitterTool::ShowHelpInternal()
{
    REConsoleModuleCommon::ShowHelpInternal();

    DAVA::Logger::Info("Examples:");
    DAVA::Logger::Info("\t-imagesplitter -split -file /Users/SmokeTest/images/test.png");
    DAVA::Logger::Info("\t-imagesplitter -merge -folder /Users/SmokeTest/images/");
}
