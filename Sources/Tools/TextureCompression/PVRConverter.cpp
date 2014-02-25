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


#include "PVRConverter.h"
#include "Render/TextureDescriptor.h"
#include "FileSystem/FileSystem.h"
#include "Utils/StringFormat.h"
#include "Platform/Process.h"

#include "Render/GPUFamilyDescriptor.h"
#include "Render/LibPVRHelper.h"

namespace DAVA
{

static String CUBEMAP_TMP_DIR = "~doc:/ResourceEditor_Cubemap_Tmp/";
	
static String PVRTOOL_INPUT_NAMES[] =
{
	String("1"), //pz
	String("2"), //nz
	String("3"), //px
	String("4"), //nx
	String("5"), //pz
	String("6"), //nz
};
	
static DAVA::String PVRTOOL_MAP_NAMES[] =
{
	String("_pz"), //1
	String("_nz"), //2
	String("_px"), //3
	String("_nx"), //4
	String("_ny"), //5
	String("_py"), //6
};

PVRConverter::PVRConverter()
{
	// pvr map
	pixelFormatToPVRFormat[FORMAT_RGBA8888] = "OGL8888";
	pixelFormatToPVRFormat[FORMAT_RGBA4444] = "OGL4444";
	pixelFormatToPVRFormat[FORMAT_RGBA5551] = "OGL5551";
	pixelFormatToPVRFormat[FORMAT_RGB565] = "OGL565";
	pixelFormatToPVRFormat[FORMAT_RGB888] = "OGL888";
	pixelFormatToPVRFormat[FORMAT_PVR2] = "OGLPVRTC2";
	pixelFormatToPVRFormat[FORMAT_PVR4] = "OGLPVRTC4";
	pixelFormatToPVRFormat[FORMAT_A8] = "OGL8";
	pixelFormatToPVRFormat[FORMAT_ETC1] = "ETC";
	
	InitFileSuffixes();
}

PVRConverter::~PVRConverter()
{

}

FilePath PVRConverter::ConvertPngToPvr(const TextureDescriptor &descriptor, eGPUFamily gpuFamily)
{
	FilePath outputName = (descriptor.IsCubeMap()) ? PrepareCubeMapForPvrConvert(descriptor) : FilePath::CreateWithNewExtension(descriptor.pathname, ".png");

	Vector<String> args;
	GetToolCommandLine(descriptor, outputName, gpuFamily, args);
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
		
	LibPVRHelper::AddCRCIntoMetaData(outputName);
	return outputName;
}


void PVRConverter::GetToolCommandLine(const TextureDescriptor &descriptor,
										FilePath fileToConvert,
										eGPUFamily gpuFamily,
										Vector<String>& args)
{
	String format = pixelFormatToPVRFormat[(PixelFormat) descriptor.compression[gpuFamily].format];
	FilePath outputFile = GetPVRToolOutput(descriptor, gpuFamily);
		
	// assemble command
	args.push_back("-i");
    
#if defined (__DAVAENGINE_MACOS__)
	args.push_back(fileToConvert.GetAbsolutePathname());
#else //defined (__DAVAENGINE_WIN32__)
	args.push_back(String("\"") + fileToConvert.GetAbsolutePathname() + String("\""));
#endif //MAC-WIN

	args.push_back("-pvrtcbest");

	if(descriptor.IsCubeMap())
	{
		args.push_back("-s");
	}
		
	// output format
	args.push_back(Format("-f%s", format.c_str()));
		
	// pvr should be always flipped-y
	args.push_back("-yflip0");
		
	// mipmaps
	if(descriptor.settings.generateMipMaps)
	{
		args.push_back("-m");
	}
		
	// base mipmap level (base resize)
	if(0 != descriptor.compression[gpuFamily].compressToWidth && descriptor.compression[gpuFamily].compressToHeight != 0)
	{
		args.push_back("-x");
		args.push_back(Format("%d", descriptor.compression[gpuFamily].compressToWidth));
		args.push_back("-y");
		args.push_back(Format("%d", descriptor.compression[gpuFamily].compressToHeight));
	}
		
	// output file
	args.push_back("-o");
#if defined (__DAVAENGINE_MACOS__)
	args.push_back(outputFile.GetAbsolutePathname());
#else //defined (__DAVAENGINE_WIN32__)
	args.push_back(String("\"") + outputFile.GetAbsolutePathname() + String("\""));
#endif //MAC-WIN
}

FilePath PVRConverter::GetPVRToolOutput(const TextureDescriptor &descriptor, eGPUFamily gpuFamily)
{
    return GPUFamilyDescriptor::CreatePathnameForGPU(&descriptor, gpuFamily);
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
	DAVA::Texture::GenerateCubeFaceNames(CUBEMAP_TMP_DIR, pvrToolSuffixes, pvrToolFaceNames);
	DAVA::Texture::GenerateCubeFaceNames(descriptor.pathname, cubemapSuffixes, cubemapFaceNames);
		
	DVASSERT(pvrToolSuffixes.size() == cubemapSuffixes.size());
		
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
	Texture::GenerateCubeFaceNames(CUBEMAP_TMP_DIR, pvrToolSuffixes, pvrToolFaceNames);
		
	for(size_t i = 0; i < pvrToolFaceNames.size(); ++i)
	{
		if(FileSystem::Instance()->IsFile(pvrToolFaceNames[i]))
		{
			FileSystem::Instance()->DeleteFile(pvrToolFaceNames[i]);
		}
	}
}
	
void PVRConverter::InitFileSuffixes()
{
	if(pvrToolSuffixes.empty())
	{
		for(int i = 0; i < DAVA::Texture::CUBE_FACE_MAX_COUNT; ++i)
		{
			pvrToolSuffixes.push_back(PVRTOOL_INPUT_NAMES[i]);
			cubemapSuffixes.push_back(PVRTOOL_MAP_NAMES[i]);
		}
	}
}	
};

