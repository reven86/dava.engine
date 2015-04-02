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


#ifndef __DAVAENGINE_PVR_CONVERTER_H__
#define __DAVAENGINE_PVR_CONVERTER_H__

#include "Base/StaticSingleton.h"
#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"
#include "FileSystem/FilePath.h"
#include "TextureCompression/TextureConverter.h"


namespace DAVA
{
    
class TextureDescriptor;
class PVRConverter: public StaticSingleton<PVRConverter>
{    
public:
	PVRConverter();
	virtual ~PVRConverter();

    FilePath ConvertToPvr(const TextureDescriptor &descriptor, eGPUFamily gpuFamily, TextureConverter::eConvertQuality quality, bool addCRC = true);
    FilePath ConvertNormalMapToPvr(const TextureDescriptor &descriptor, eGPUFamily gpuFamily, TextureConverter::eConvertQuality quality);

	void SetPVRTexTool(const FilePath &textToolPathname);

	FilePath GetPVRToolOutput(const TextureDescriptor &descriptor, eGPUFamily gpuFamily);
	
protected:
	
	FilePath PrepareCubeMapForPvrConvert(const TextureDescriptor& descriptor);
	void CleanupCubemapAfterConversion(const TextureDescriptor& descriptor);
	void InitFileSuffixes();
	
	void GetToolCommandLine(const TextureDescriptor &descriptor, const FilePath & fileToConvert, eGPUFamily gpuFamily, TextureConverter::eConvertQuality quality, Vector<String>& args);

	String GenerateInputName(const TextureDescriptor& descriptor, const FilePath & fileToConvert);

protected:
	
    Map<PixelFormat, String> pixelFormatToPVRFormat;

	FilePath pvrTexToolPathname;
	
	Vector<String> pvrToolSuffixes;
	Vector<String> cubemapSuffixes;

};

};


#endif // __DAVAENGINE_PVR_CONVERTER_H__