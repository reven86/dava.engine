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

using namespace DAVA;

PVRConverter::PVRConverter()
{
	// pvr map
	pixelFormatToPVRFormat[DAVA::FORMAT_RGBA8888] = "OGL8888";
	pixelFormatToPVRFormat[DAVA::FORMAT_RGBA4444] = "OGL4444";
	pixelFormatToPVRFormat[DAVA::FORMAT_RGBA5551] = "OGL5551";
	pixelFormatToPVRFormat[DAVA::FORMAT_RGB565] = "OGL565";
	pixelFormatToPVRFormat[DAVA::FORMAT_RGB888] = "OGL888";
	pixelFormatToPVRFormat[DAVA::FORMAT_PVR2] = "OGLPVRTC2";
	pixelFormatToPVRFormat[DAVA::FORMAT_PVR4] = "OGLPVRTC4";
	pixelFormatToPVRFormat[DAVA::FORMAT_A8] = "OGL8";
	pixelFormatToPVRFormat[DAVA::FORMAT_ETC1] = "ETC";
}

PVRConverter::~PVRConverter()
{

}

String PVRConverter::ConvertPngToPvr(const String & fileToConvert, const DAVA::TextureDescriptor &descriptor)
{
	String outputName;
	String command = GetCommandLinePVR(fileToConvert, descriptor);
    Logger::Info("[PVRConverter::ConvertPngToPvr] (%s)", command.c_str());
    
	if(!command.empty())
	{
		FileSystem::Instance()->Spawn(command);
		outputName = GetPVRToolOutput(fileToConvert);
	}

	return outputName;
}

String PVRConverter::GetCommandLinePVR(const DAVA::String & fileToConvert, const DAVA::TextureDescriptor &descriptor)
{
	String command = Format("\"%s\"", pvrTexToolPathname.c_str());
	String format = pixelFormatToPVRFormat[descriptor.pvrCompression.format];

	if(command != "" && format != "")
	{
		String outputFile = GetPVRToolOutput(fileToConvert);

		// assemble command

		// input file
		command += Format(" -i \"%s\"", fileToConvert.c_str());

		// output format
		command += Format(" -f%s", format.c_str());

		// pvr should be always flipped-y
		command += " -yflip0";

		// mipmaps
		if(descriptor.generateMipMaps)
		{
			command += " -m";
		}

		// base mipmap level (base resize)
		if(0 != descriptor.pvrCompression.compressToWidth && descriptor.pvrCompression.compressToHeight != 0)
		{
			command += Format(" -x %d -y %d", descriptor.pvrCompression.compressToWidth, descriptor.pvrCompression.compressToHeight);
		}

		// output file
		command += Format(" -o \"%s\"", outputFile.c_str());
	}
    else
    {
        Logger::Error("[PVRConverter::GetCommandLinePVR] Can't create command line for file (%s)", fileToConvert.c_str());
        command = "";
    }

	return command;
}

String PVRConverter::GetPVRToolOutput(const DAVA::String &inputPVR)
{
	return FileSystem::ReplaceExtension(inputPVR, ".pvr");
}

void PVRConverter::SetPVRTexTool(const DAVA::String &textToolPathname)
{
	pvrTexToolPathname = FileSystem::Instance()->SystemPathForFrameworkPath(textToolPathname);
	pvrTexToolPathname = FileSystem::Instance()->GetCanonicalPath(pvrTexToolPathname);

	if(!FileSystem::Instance()->IsFile(pvrTexToolPathname))
	{
		Logger::Error("PVRTexTool doesn't found in %s\n", pvrTexToolPathname.c_str());
		pvrTexToolPathname = "";
	}
}

