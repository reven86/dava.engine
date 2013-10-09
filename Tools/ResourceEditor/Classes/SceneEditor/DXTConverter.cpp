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



#include "DXTConverter.h"

using namespace DAVA;

String DXTConverter::ConvertPngToDxt(const String & fileToConvert, const DAVA::TextureDescriptor &descriptor)
{
	Vector<Image*> inputImages = ImageLoader::CreateFromFile(fileToConvert);
	if(inputImages.size() == 1)
	{
		Image* image = inputImages[0];
		String outputName = FileSystem::ReplaceExtension(fileToConvert, ".dds");

		if((descriptor.dxtCompression.compressToWidth != 0) && (descriptor.dxtCompression.compressToHeight != 0))
		{
            Logger::Warning("[DXTConverter::ConvertPngToDxt] convert to compression size");
			image->ResizeImage(descriptor.dxtCompression.compressToWidth, descriptor.dxtCompression.compressToHeight);
		}
		
		if(LibDxtHelper::WriteDdsFile(outputName,
                                      image->width, image->height, image->data,
                                      descriptor.dxtCompression.format,
                                      (descriptor.generateMipMaps == TextureDescriptor::OPTION_ENABLED)))
		{
            for_each(inputImages.begin(), inputImages.end(), SafeRelease<Image>);
            return outputName;
		}
	}
    
    Logger::Error("[DXTConverter::ConvertPngToDxt] can't convert %s to DXT", fileToConvert.c_str());

    for_each(inputImages.begin(), inputImages.end(), SafeRelease<Image>);
    return String("");
}

String DXTConverter::GetDXTOutput(const DAVA::String &inputDXT)
{
	return FileSystem::ReplaceExtension(inputDXT, ".dds");
}
