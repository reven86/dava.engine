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

#include "CommandLine/ImageSplitter/ImageSplitterTool.h"
#include "ImageTools/ImageTools.h"

using namespace DAVA;

namespace OptionName
{
static const String Split = "-split";
static const String Merge = "-merge";
static const String File = "-file";
static const String Folder = "-folder";
}

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
            AddError("Pathname of image file was not selected");
            return false;
        }
    }
    else if (commandAction == ACTION_MERGE)
    {
        if (foldername.IsEmpty())
        {
            AddError("Input folder was not selected");
            return false;
        }
        foldername.MakeDirectoryPathname();
    }
    else
    {
        AddError("Wrong action was selected");
        return false;
    }

    return true;
}

void ImageSplitterTool::ProcessInternal()
{
    if(commandAction == ACTION_SPLIT)
    {
        ImageTools::SplitImage(filename, errors);
    }
    else if(commandAction == ACTION_MERGE)
    {
        ImageTools::MergeImages(foldername, errors);
    }
}

