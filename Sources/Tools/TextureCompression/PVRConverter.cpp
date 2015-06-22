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

#include "Base/Platform.h"
#ifndef __DAVAENGINE_WIN_UAP__

#include "PVRConverter.h"
#include "Render/TextureDescriptor.h"
#include "FileSystem/FileSystem.h"
#include "Utils/StringFormat.h"
#include "Platform/Process.h"

#include "Render/GPUFamilyDescriptor.h"
#include "Render/Image/LibPVRHelper.h"
#include "Render/Image/ImageSystem.h"
#include "Render/Image/Image.h"
#include "Render/Image/LibTgaHelper.h"

#include "Base/GlobalEnum.h"

namespace DAVA
{
static String CUBEMAP_TMP_DIR = "~doc:/ResourceEditor_Cubemap_Tmp/";

static Array<String, Texture::CUBE_FACE_COUNT> PVRTOOL_FACE_SUFFIXES =
{
    String("1"), //pz
    String("2"), //nz
    String("3"), //px
    String("4"), //nx
    String("5"), //pz
    String("6"), //nz
};

static DAVA::String PVR_QUALITY_SETTING[] =
{
    "pvrtcfastest",
    "pvrtcfast",
    "pvrtcnormal",
    "pvrtchigh",
    "pvrtcbest"
};

static DAVA::String ETC_QUALITY_SETTING[] =
{
	"etcfast",
	"etcfast",
	"etcslow",
	"etcfastperceptual",
	"etcslowperceptual"
};


PVRConverter::PVRConverter()
{
//	PVRTC1_2, PVRTC1_4, PVRTC1_2_RGB, PVRTC1_4_RGB, PVRTC2_2, PVRTC2_4, ETC1, BC1, BC2, BC3,UYVY, YUY2, 1BPP, RGBE9995, RGBG8888, GRGB8888, ETC2_RGB, ETC2_RGBA, ETC2_RGB_A1, EAC_R11, EAC_RG11

	// pvr map
	pixelFormatToPVRFormat[FORMAT_RGBA8888] = "r8g8b8a8";//"OGL8888";
	pixelFormatToPVRFormat[FORMAT_RGBA4444] = "r4g4b4a4";//"OGL4444";
	pixelFormatToPVRFormat[FORMAT_RGBA5551] = "r5g5b5a1";//"OGL5551";
	pixelFormatToPVRFormat[FORMAT_RGB565] = "r5g6b5";//"OGL565";
	pixelFormatToPVRFormat[FORMAT_RGB888] = "r8g8b8";//"OGL888";
	pixelFormatToPVRFormat[FORMAT_PVR2] = "PVRTC1_2";
	pixelFormatToPVRFormat[FORMAT_PVR4] = "PVRTC1_4";
	pixelFormatToPVRFormat[FORMAT_A8] = "l8";//"OGL8";
	pixelFormatToPVRFormat[FORMAT_ETC1] = "ETC1";

    pixelFormatToPVRFormat[FORMAT_PVR2_2] = "PVRTC2_2";
	pixelFormatToPVRFormat[FORMAT_PVR4_2] = "PVRTC2_4";
	pixelFormatToPVRFormat[FORMAT_EAC_R11_UNSIGNED] = "EAC_R11";
	pixelFormatToPVRFormat[FORMAT_EAC_R11_SIGNED] = "EAC_R11";
	pixelFormatToPVRFormat[FORMAT_EAC_RG11_UNSIGNED] = "EAC_RG11";
	pixelFormatToPVRFormat[FORMAT_EAC_RG11_SIGNED] = "EAC_RG11";
	pixelFormatToPVRFormat[FORMAT_ETC2_RGB] = "ETC2_RGB";
	pixelFormatToPVRFormat[FORMAT_ETC2_RGBA] = "ETC2_RGBA";
	pixelFormatToPVRFormat[FORMAT_ETC2_RGB_A1] = "ETC2_RGB_A1";
}

PVRConverter::~PVRConverter()
{

}

FilePath PVRConverter::ConvertToPvr(const TextureDescriptor &descriptor, eGPUFamily gpuFamily, TextureConverter::eConvertQuality quality, bool addCRC /* = true */)
{
#ifdef __DAVAENGINE_WIN_UAP__

    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
    return FilePath();

#else
	FilePath outputName = (descriptor.IsCubeMap()) ? PrepareCubeMapForPvrConvert(descriptor) : descriptor.GetSourceTexturePathname();

	Vector<String> args;
	GetToolCommandLine(descriptor, outputName, gpuFamily, quality, args);
	Process process(pvrTexToolPathname, args);
	if(process.Run(false))
	{
		process.Wait();
			
		const String& procOutput = process.GetOutput();
		if(procOutput.size() > 0)
		{
			Logger::FrameworkDebug(procOutput.c_str());
		}
			
		outputName = GetPVRToolOutput(descriptor, gpuFamily);
	}
	else
	{
		Logger::Error("Failed to run PVR tool! %s", pvrTexToolPathname.GetAbsolutePathname().c_str());
		DVASSERT(false);
	}
		
	if(descriptor.IsCubeMap())
	{
		CleanupCubemapAfterConversion(descriptor);
	}
		
    if(addCRC)
    {
        LibPVRHelper helper;
	    helper.AddCRCIntoMetaData(outputName);
    }
	return outputName;
#endif
}

FilePath PVRConverter::ConvertNormalMapToPvr(const TextureDescriptor &descriptor, eGPUFamily gpuFamily, TextureConverter::eConvertQuality quality)
{
    FilePath filePath = descriptor.GetSourceTexturePathname();

    Vector<Image *> images;
    ImageSystem::Instance()->Load(filePath, images);

    if(!images.size())
    {
        return FilePath();
    }

    DVASSERT(images.size() == 1);

    Image * originalImage = images[0];
    bool normalized = originalImage->Normalize();
    if(!normalized)
    {
        Logger::Error("[PVRConverter::ConvertNormalMapToPvr] Cannot normalize image %s", filePath.GetStringValue().c_str());
        SafeRelease(originalImage);
        return FilePath();
    }
    
    if(descriptor.GetGenerateMipMaps())
    {
        images = originalImage->CreateMipMapsImages(true);
        SafeRelease(originalImage);
    }

    FilePath dirPath = filePath + "_mips";
    dirPath.MakeDirectoryPathname();
    FileSystem::Instance()->CreateDirectory(dirPath);

    Vector<FilePath> convertedPVRs;

    int32 imgCount = (int32)images.size();
    for(int32 i = 0; i < imgCount; ++i)
    {
        ImageFormat targetFormat = IMAGE_FORMAT_PNG;
        
        TextureDescriptor desc;
        desc.Initialize(&descriptor);
        desc.SetGenerateMipmaps(false);
        desc.dataSettings.sourceFileFormat = targetFormat;
        desc.dataSettings.sourceFileExtension = ImageSystem::Instance()->GetExtensionsFor(targetFormat)[0];
        desc.pathname = dirPath + Format("mip%d%s", i,  desc.dataSettings.sourceFileExtension.c_str());;

        ImageSystem::Instance()->Save(desc.pathname, images[i]);
        FilePath convertedImgPath = ConvertToPvr(desc, gpuFamily, quality, false);

        convertedPVRs.push_back(convertedImgPath);
    }

    FilePath outputName = GetPVRToolOutput(descriptor, gpuFamily);
    bool ret = LibPVRHelper::WriteFileFromMipMapFiles(outputName, convertedPVRs);

    FileSystem::Instance()->DeleteDirectory(dirPath, true);

    if(ret)
    {
        LibPVRHelper helper;
        helper.AddCRCIntoMetaData(outputName);
        return outputName;
    }
    else
    {
        return FilePath();
    }
}

void PVRConverter::GetToolCommandLine(const TextureDescriptor &descriptor, const FilePath & fileToConvert, eGPUFamily gpuFamily, TextureConverter::eConvertQuality quality, Vector<String>& args)
{
	DVASSERT(descriptor.compression);
	const TextureDescriptor::Compression *compression = &descriptor.compression[gpuFamily];

	String format = pixelFormatToPVRFormat[(PixelFormat) compression->format];
	FilePath outputFile = GetPVRToolOutput(descriptor, gpuFamily);
		
	// input file
	args.push_back("-i");
	String inputName = GenerateInputName(descriptor, fileToConvert);
#if defined (__DAVAENGINE_MACOS__)
	args.push_back(inputName);
#else //defined (__DAVAENGINE_WINDOWS__)
	args.push_back(String("\"") + inputName + String("\""));
#endif //MAC-WIN

	// output file
	args.push_back("-o");
#if defined (__DAVAENGINE_MACOS__)
	args.push_back(outputFile.GetAbsolutePathname());
#else //defined (__DAVAENGINE_WINDOWS__)
	args.push_back(String("\"") + outputFile.GetAbsolutePathname() + String("\""));
#endif //MAC-WIN

    // flip for some TGA files
    // PVR converter seems to have a bug:
    // if source file is a top-left TGA, then the resulting file will be flipped vertically.
    // to fix this, -flip param will be passed for such files
    if (descriptor.dataSettings.sourceFileFormat == IMAGE_FORMAT_TGA)
    {
        LibTgaHelper tgaHelper;
        LibTgaHelper::TgaInfo tgaInfo;
        auto readRes = tgaHelper.ReadTgaHeader(inputName, tgaInfo);
        if (readRes == DAVA::eErrorCode::SUCCESS)
        {
            switch (tgaInfo.origin_corner)
            {
            case LibTgaHelper::TgaInfo::BOTTOM_LEFT:
                break;
            case LibTgaHelper::TgaInfo::BOTTOM_RIGHT:
                args.push_back("-flip");
                args.push_back("x");
                break;
            case LibTgaHelper::TgaInfo::TOP_LEFT:
                args.push_back("-flip");
                args.push_back("y");
                break;
            case LibTgaHelper::TgaInfo::TOP_RIGHT:
                args.push_back("-flip");
                args.push_back("x");
                args.push_back("-flip");
                args.push_back("y");
                break;
            }
        }
        else
        {
            Logger::Error("Failed to read %s: error %d", inputName.c_str(), readRes);
        }
    }

	//quality
	args.push_back("-q");
	if(FORMAT_ETC1 == descriptor.compression[gpuFamily].format)
	{
		args.push_back(ETC_QUALITY_SETTING[quality]);
	}
	else
	{
		args.push_back(PVR_QUALITY_SETTING[quality]);
	}

	// mipmaps
	if(descriptor.GetGenerateMipMaps())
	{
		args.push_back("-m");
	}
    
	if(descriptor.IsCubeMap())
	{
		args.push_back("-cube");
	}
		
	// output format
	args.push_back("-f");
	args.push_back(format);

	// base mipmap level (base resize)
	if(0 != compression->compressToWidth && compression->compressToHeight != 0)
	{
        args.push_back("-r");
		args.push_back(Format("%d,%d", compression->compressToWidth, compression->compressToHeight));
	}
    
    //args.push_back("-l"); //Alpha Bleed: Discards any data in fully transparent areas to optimise the texture for better compression.

}

FilePath PVRConverter::GetPVRToolOutput(const TextureDescriptor &descriptor, eGPUFamily gpuFamily)
{
    return descriptor.CreatePathnameForGPU(gpuFamily);
}

void PVRConverter::SetPVRTexTool(const FilePath &textToolPathname)
{
	pvrTexToolPathname = textToolPathname;

	if(!FileSystem::Instance()->IsFile(pvrTexToolPathname))
	{
		Logger::Error("PVRTexTool doesn't found in %s\n", pvrTexToolPathname.GetAbsolutePathname().c_str());
		pvrTexToolPathname = FilePath();
	}
}

FilePath PVRConverter::PrepareCubeMapForPvrConvert(const TextureDescriptor& descriptor)
{
	DAVA::Vector<DAVA::FilePath> pvrToolFaceNames;
	DAVA::Vector<DAVA::FilePath> cubemapFaceNames;
	descriptor.GenerateFacePathnames(CUBEMAP_TMP_DIR, PVRTOOL_FACE_SUFFIXES, pvrToolFaceNames);
	descriptor.GetFacePathnames(cubemapFaceNames);
		
	if(!FileSystem::Instance()->IsDirectory(CUBEMAP_TMP_DIR))
	{
		int createResult = FileSystem::Instance()->CreateDirectory(CUBEMAP_TMP_DIR);
		if(FileSystem::DIRECTORY_CREATED != createResult)
		{
			DAVA::Logger::Error("Failed to create temp dir for cubemap generation!");
			return FilePath();
		}
	}
		
	for(size_t i = 0; i < pvrToolFaceNames.size(); ++i)
	{
		//cleanup in case previous cleanup failed
		if(FileSystem::Instance()->IsFile(pvrToolFaceNames[i]))
		{
			FileSystem::Instance()->DeleteFile(pvrToolFaceNames[i]);
		}
			
		bool result = FileSystem::Instance()->CopyFile(cubemapFaceNames[i], pvrToolFaceNames[i]);
		if(!result)
		{
			DAVA::Logger::Error("Failed to copy tmp files for cubemap generation!");
			return FilePath();
		}
	}
		
	return FilePath(pvrToolFaceNames[0]);
}

void PVRConverter::CleanupCubemapAfterConversion(const TextureDescriptor& descriptor)
{
	Vector<FilePath> pvrToolFaceNames;
	descriptor.GenerateFacePathnames(CUBEMAP_TMP_DIR, PVRTOOL_FACE_SUFFIXES, pvrToolFaceNames);
		
	for(auto& faceName : pvrToolFaceNames)
	{
		if(FileSystem::Instance()->IsFile(faceName))
		{
			FileSystem::Instance()->DeleteFile(faceName);
		}
	}
}


DAVA::String PVRConverter::GenerateInputName( const TextureDescriptor& descriptor, const FilePath & fileToConvert)
{
	if(descriptor.IsCubeMap())
	{
		Vector<FilePath> pvrToolFaceNames;
		descriptor.GenerateFacePathnames(CUBEMAP_TMP_DIR, PVRTOOL_FACE_SUFFIXES, pvrToolFaceNames);

		String retNames;
		for(size_t i = 0; i < pvrToolFaceNames.size(); ++i)
		{
			if(i)
			{
				retNames += ",";
			}

			retNames += pvrToolFaceNames[i].GetAbsolutePathname();
		}

		return retNames;
	}

	return	fileToConvert.GetAbsolutePathname();
}

};

#endif // #ifndef __DAVAENGINE_WIN_UAP__
