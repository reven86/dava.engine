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

#include "CommandLine/TextureDescriptor/TextureDescriptorUtils.h"

#include "ImageTools/ImageTools.h"
#include "Settings/SettingsManager.h"


using namespace DAVA;

void TextureDescriptorUtils::ResaveDescriptorsForFolder(const FilePath &folderPathname)
{
    ScopedPtr<FileList> fileList(new FileList(folderPathname));
    for (int32 fi = 0; fi < fileList->GetCount(); ++fi)
	{
		const FilePath &pathname = fileList->GetPathname(fi);
		if(IsCorrectDirectory(fileList, fi))
		{
            ResaveDescriptorsForFolder(pathname);
		}
        else if(IsDescriptorPathname(pathname))
        {
			ResaveDescriptor(pathname);
        }
	}
}

void TextureDescriptorUtils::ResaveDescriptor(const FilePath & descriptorPathname) 
{
    std::unique_ptr<TextureDescriptor> descriptor(TextureDescriptor::CreateFromFile(descriptorPathname));
    descriptor->Save();
}


void TextureDescriptorUtils::CopyCompressionParamsForFolder(const FilePath &folderPathname)
{
    ScopedPtr<FileList> fileList(new FileList(folderPathname));
    for (int32 fi = 0; fi < fileList->GetCount(); ++fi)
	{
		const FilePath &pathname = fileList->GetPathname(fi);
		if(IsCorrectDirectory(fileList, fi))
		{
			CopyCompressionParamsForFolder(pathname);
		}
		else if(IsDescriptorPathname(pathname))
        {
            CopyCompressionParams(pathname);
        }
	}
}

void TextureDescriptorUtils::CopyCompressionParams(const FilePath &descriptorPathname)
{
    std::unique_ptr<TextureDescriptor> descriptor(TextureDescriptor::CreateFromFile(descriptorPathname));
    if(!descriptor) return;

    const TextureDescriptor::Compression * srcCompression = &descriptor->compression[GPU_POWERVR_IOS];
    if(srcCompression->format == FORMAT_INVALID)
    {   //source format not set
        return;
    }
    
    for(int32 gpu = GPU_POWERVR_ANDROID; gpu < GPU_DEVICE_COUNT; ++gpu)
    {
        if(descriptor->compression[gpu].format != FORMAT_INVALID)
            continue;
        
        descriptor->compression[gpu].compressToWidth = srcCompression->compressToWidth;
        descriptor->compression[gpu].compressToHeight = srcCompression->compressToHeight;
        descriptor->compression[gpu].sourceFileCrc = srcCompression->sourceFileCrc;
        
        if((srcCompression->format == FORMAT_PVR2 || srcCompression->format == FORMAT_PVR4) && (gpu != GPU_POWERVR_ANDROID))
        {
            PixelFormat newFormat = FORMAT_ETC1;
            switch (gpu)
            {
            case GPU_TEGRA:
            case GPU_DX11:
                newFormat = FORMAT_DXT1;
                break;
            case GPU_ADRENO:
                newFormat = FORMAT_ATC_RGB;
                break;
            default:
                break;
            }

            descriptor->compression[gpu].format = newFormat;
        }
        else
        {
            descriptor->compression[gpu].format = srcCompression->format;
        }
    }
    
    descriptor->Save();
}


void TextureDescriptorUtils::CreateDescriptorsForFolder(const FilePath &folderPathname)
{
    ScopedPtr<FileList> fileList(new FileList(folderPathname));
    for (int32 fi = 0; fi < fileList->GetCount(); ++fi)
	{
		const FilePath &pathname = fileList->GetPathname(fi);
		if(IsCorrectDirectory(fileList, fi))
		{
			CreateDescriptorsForFolder(pathname);
		}
        else if(DAVA::TextureDescriptor::IsSourceTextureExtension(pathname.GetExtension()))
        {
            CreateDescriptorIfNeed(pathname);
        }
	}
}


bool TextureDescriptorUtils::CreateDescriptorIfNeed(const FilePath &originalPathname)
{
	FilePath descriptorPathname = TextureDescriptor::GetDescriptorPathname(originalPathname);
    if(false == FileSystem::Instance()->IsFile(descriptorPathname))
    {
        std::unique_ptr<TextureDescriptor> descriptor(new TextureDescriptor());

        const String extension = originalPathname.GetExtension();
		const ImageFormat sourceFormat = ImageSystem::Instance()->GetImageFormatForExtension(extension);
		if (sourceFormat != IMAGE_FORMAT_UNKNOWN)
		{
			descriptor->dataSettings.sourceFileFormat = sourceFormat;
			descriptor->dataSettings.sourceFileExtension = extension;
		}
        
        descriptor->Save(descriptorPathname);
		return true;
    }

	return false;
}


void TextureDescriptorUtils::SetCompressionParamsForFolder( const FilePath &folderPathname, const DAVA::Map<DAVA::eGPUFamily, DAVA::TextureDescriptor::Compression> & compressionParams, bool convertionEnabled, bool force, DAVA::TextureConverter::eConvertQuality quality, bool generateMipMaps)
{
    ScopedPtr<FileList> fileList(new FileList(folderPathname));

    for (int32 fi = 0; fi < fileList->GetCount(); ++fi)
	{
		const FilePath &pathname = fileList->GetPathname(fi);
		if(IsCorrectDirectory(fileList, fi))
		{
			SetCompressionParamsForFolder(pathname, compressionParams, convertionEnabled, force, quality, generateMipMaps);
		}
		else if(IsDescriptorPathname(pathname))
		{
			SetCompressionParams(pathname, compressionParams, convertionEnabled, force, quality, generateMipMaps);
		}
	}
}


void TextureDescriptorUtils::SetCompressionParams( const FilePath &descriptorPathname, const DAVA::Map<DAVA::eGPUFamily, DAVA::TextureDescriptor::Compression> & compressionParams, bool convertionEnabled, bool force, DAVA::TextureConverter::eConvertQuality quality, bool generateMipMaps)
{
	TextureDescriptor *descriptor = TextureDescriptor::CreateFromFile(descriptorPathname);
	if(!descriptor) return;

	DVASSERT(descriptor->compression);

	auto endIt = compressionParams.end();
	for(auto it = compressionParams.begin(); it != endIt; ++it)
	{
		eGPUFamily gpu = it->first;

		if(force || (descriptor->compression[gpu].format == FORMAT_INVALID))
		{
			descriptor->compression[gpu] = it->second;

			if(convertionEnabled)
			{
				ImageTools::ConvertImage(descriptor, gpu, (PixelFormat)descriptor->compression[gpu].format, quality);
			}
		}
	}

	descriptor->Save();
	delete descriptor;
}

bool TextureDescriptorUtils::IsCorrectDirectory( FileList *fileList, const int32 fileIndex )
{
	if (fileList->IsDirectory(fileIndex))
	{
		String name = fileList->GetFilename(fileIndex);
		if(0 != CompareCaseInsensitive(String(".svn"), name) && !fileList->IsNavigationDirectory(fileIndex))
		{
			return true;
		}
	}

	return false;
}

bool TextureDescriptorUtils::IsDescriptorPathname( const FilePath &pathname )
{
	return pathname.IsEqualToExtension(TextureDescriptor::GetDescriptorExtension());
}


