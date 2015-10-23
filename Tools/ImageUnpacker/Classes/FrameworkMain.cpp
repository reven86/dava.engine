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


#include "DAVAEngine.h"
#include "GameCore.h"
#include "CommandLine/CommandLineParser.h"

#include "Render/Image/ImageConvert.h"

using namespace DAVA;
 
void PrintUsage()
{
    printf("Usage:\n");

    printf("\t-usage or --help to display this help\n");
	printf("\t-file - pvr or dds file to unpack as png\n");
	printf("\t-folder - folder with pvr or dds files to unpack as png\n");
    printf("\t-saveas -ext -folder - will open png files from folder and save as ext parameter mean\n");
    
    printf("Example:\n");
    printf("\t-saveas -ext .tga -folder /Users/nickname/test/");

}


void SaveSingleImage(const FilePath & newImagePath, Image *image)
{
    if((FORMAT_RGBA8888 == image->format) || (FORMAT_A8 == image->format) || (FORMAT_A16 == image->format))
    {
        ImageSystem::Instance()->Save(newImagePath, image, image->format);
    }
    else
    {
        Image *savedImage = Image::Create(image->width, image->height, FORMAT_RGBA8888);
        
        ImageConvert::ConvertImageDirect(image->format, savedImage->format, image->data, image->width, image->height, image->width * PixelFormatDescriptor::GetPixelFormatSizeInBytes(image->format),
                                         savedImage->data, savedImage->width, savedImage->height, savedImage->width * PixelFormatDescriptor::GetPixelFormatSizeInBytes(savedImage->format));
        
        ImageSystem::Instance()->Save(newImagePath, savedImage);
        savedImage->Release();
    }
}

void SaveCubemap(const FilePath & newImagePath, const Vector<Image *> &images)
{
    Vector<FilePath> faceNames;
    TextureDescriptor::GenerateFacePathnames(newImagePath, faceNames, ".png");

    for (auto image: images)
    {
        if(0 == image->mipmapLevel)
        {
            SaveSingleImage(faceNames[image->cubeFaceID], image);
        }
    }
}

void UnpackFile(const FilePath & sourceImagePath)
{
    Vector<Image *> images;
    ImageSystem::Instance()->Load(sourceImagePath, images);
    
    if(images.size() != 0)
    {
        FilePath imagePathname = FilePath::CreateWithNewExtension(sourceImagePath,".png");
        
        Image *image = images[0];

        if (image->cubeFaceID == Texture::INVALID_CUBEMAP_FACE)
        {
            SaveSingleImage(imagePathname, image);
        }
        else
        {
            SaveCubemap(imagePathname, images);
        }

        for_each(images.begin(), images.end(), SafeRelease<Image>);
    }
    else
    {
        Logger::Error("Cannot load file: ", sourceImagePath.GetStringValue().c_str());
    }
}

void UnpackFolder(const FilePath & folderPath)
{
    ScopedPtr<FileList> fileList(new FileList(folderPath));
	for (int fi = 0; fi < fileList->GetCount(); ++fi)
	{
        const FilePath & pathname = fileList->GetPathname(fi);
		if (fileList->IsDirectory(fi) && !fileList->IsNavigationDirectory(fi))
		{
            UnpackFolder(pathname);
        }
        else
        {
            if(pathname.IsEqualToExtension(".pvr") || pathname.IsEqualToExtension(".dds"))
            {
                UnpackFile(pathname);
            }
        }
	}
}

void ResavePNG(const FilePath & folderPath, const String & extension)
{
    ScopedPtr<FileList> fileList(new FileList(folderPath));
    
    for (int fi = 0; fi < fileList->GetCount(); ++fi)
    {
        const FilePath & pathname = fileList->GetPathname(fi);
        if (fileList->IsDirectory(fi) && !fileList->IsNavigationDirectory(fi))
        {
            ResavePNG(pathname, extension);
        }
        else
        {
            if(pathname.IsEqualToExtension(".png"))
            {
                Vector<Image *> images;
                ImageSystem::Instance()->Load(pathname, images);
                
                FilePath tgaPathname = FilePath::CreateWithNewExtension(pathname, extension);
                ImageSystem::Instance()->Save(tgaPathname, images);

                for_each(images.begin(), images.end(), SafeRelease<Image>);
            }
        }
    }
}


void ProcessImageUnpacker()
{
#if RHI_COMPLETE
    RenderManager::Create(Core::RENDERER_OPENGL);
#endif //#if RHI_COMPLETE

    PixelFormatDescriptor::InitializePixelFormatDescriptors();
    
    FilePath sourceFolderPath = CommandLineParser::GetCommandParam(String("-folder"));
    FilePath sourceFilePath = CommandLineParser::GetCommandParam(String("-file"));
    
    bool needShowUsage = true;
    if(CommandLineParser::CommandIsFound("-saveas") && sourceFolderPath.IsEmpty() == false)
    {
        String ext = CommandLineParser::GetCommandParam(String("-ext"));
        if(!ext.empty())
        {
            if(ext[0] != '.')
            {
                ext = "." + ext;
            }
            
            sourceFolderPath.MakeDirectoryPathname();
            ResavePNG(sourceFolderPath, ext);
            
            needShowUsage = false;
        }
    }
    else if(sourceFolderPath.IsEmpty() == false)
    {
        sourceFolderPath.MakeDirectoryPathname();
        UnpackFolder(sourceFolderPath);
        needShowUsage = false;
    }
    else if (sourceFilePath.IsEmpty() == false)
    {
        UnpackFile(sourceFilePath);
        needShowUsage = false;
    }

    if(needShowUsage)
    {
        PrintUsage();
    }
    
#if RHI_COMPLETE
    RenderManager::Instance()->Release();
#endif //#if RHI_COMPLETE
}

void FrameworkDidLaunched()
{
    Logger::Instance()->SetLogLevel(Logger::LEVEL_INFO);
	if (Core::Instance()->IsConsoleMode())
	{
        if(     CommandLineParser::GetCommandsCount() < 2
           ||   (CommandLineParser::CommandIsFound(String("-usage")))
           ||   (CommandLineParser::CommandIsFound(String("-help")))
           )
        {
            PrintUsage();
			return;
        }
	}

    ProcessImageUnpacker();
}


void FrameworkWillTerminate()
{
}
