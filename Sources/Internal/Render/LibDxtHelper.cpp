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



#include "Render/LibDxtHelper.h"

#include "Render/Image.h"
#include "Render/Texture.h"
#include "Render/RenderManager.h"

#include <libdxt/nvtt.h>
#include <libdxt/nvtt_extra.h>

#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"

#include "Utils/Utils.h"
#include "Utils/CRC32.h"

#include <libatc/TextureConverter.h>

#define DDS_HEADER_CRC_OFFSET		60			//offset  to 9th element of dwReserved1 array(dds header)
#define METADATA_CRC_TAG			0x5f435243  // equivalent of 'C''R''C''_'

using namespace nvtt;

namespace DAVA
{

class NvttHelper
{
public:
	struct DDSInfo
	{
		uint32 width;
		uint32 height;
		uint32 dataSize;
		uint32 mipmapsCount;
		uint32 headerSize;
		uint32 faceCount;
		uint32 faceFlags;
		
		DDSInfo()
		{
			width		  = 0;
			height		  = 0;
			dataSize	  = 0;
			mipmapsCount = 0;
			headerSize	  = 0;
			faceCount = 0;
			faceFlags = 0;
		}
	};

	struct PairNvttPixelGLFormat
    {
		static const int32  WRONG_GL_VALUE = -1;
        nvtt::Format nvttFormat;
		PixelFormat  davaFormat;
        //int32		 glFormat;
        
        PairNvttPixelGLFormat(nvtt::Format _nvttFormat, PixelFormat _davaFormat, uint32 _glFormat)
        {
            nvttFormat = _nvttFormat;
            davaFormat = _davaFormat;
			//glFormat   = _glFormat;
        }
    };
	
	const static PairNvttPixelGLFormat formatNamesMap[];

	static bool InitDecompressor(nvtt::Decompressor & dec, const FilePath & fileName);
	static bool InitDecompressor(nvtt::Decompressor & dec, File * file);
	static bool InitDecompressor(nvtt::Decompressor & dec, const uint8 * mem, uint32 size);
	
	static bool ReadDxtFile(nvtt::Decompressor & dec, Vector<Image*> &imageSet, bool forceSoftwareConvertation);
	
	static PixelFormat GetPixelFormat(nvtt::Decompressor & dec);
	
	static bool GetTextureSize(nvtt::Decompressor & dec, uint32 & width, uint32 & height);
	
	static uint32 GetMipMapLevelsCount(nvtt::Decompressor & dec);
	
	static uint32 GetDataSize(nvtt::Decompressor & dec);
	
	static bool GetInfo(nvtt::Decompressor & dec, DDSInfo &info);
	
	static void SwapBRChannels(uint8* data, uint32 size);

	static PixelFormat GetPixelFormatByNVTTFormat(nvtt::Format nvttFormat);

	static nvtt::Format GetNVTTFormatByPixelFormat(PixelFormat pixelFormat);
	
	static bool IsAtcFormat(nvtt::Format format);
	
	static uint32 GetCubeFaceId(uint32 nvttFaceDesc, int faceIndex);
	
private:
	static bool DecompressAtc(const nvtt::Decompressor & dec, DDSInfo info, PixelFormat format, Vector<Image*> &imageSet);
	static bool DecompressDxt(const nvtt::Decompressor & dec, DDSInfo info, Vector<Image*> &imageSet);
};

const NvttHelper::PairNvttPixelGLFormat NvttHelper::formatNamesMap[] =
{
	//FORMAT_NAMES_MAP_COUNT should be inceased in case of addition to set
#if defined(GL_COMPRESSED_RGB_S3TC_DXT1_EXT)
	NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT1,	FORMAT_DXT1,	 GL_COMPRESSED_RGB_S3TC_DXT1_EXT),
	NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT1,	FORMAT_DXT1NM,	 GL_COMPRESSED_RGB_S3TC_DXT1_EXT),
#else
	NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT1,	FORMAT_DXT1,	 PairNvttPixelGLFormat::WRONG_GL_VALUE),
	NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT1,	FORMAT_DXT1NM,	 PairNvttPixelGLFormat::WRONG_GL_VALUE),
#endif //GL_COMPRESSED_RGB_S3TC_DXT1_EXT
    

#if defined(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
	NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT1a,	FORMAT_DXT1A,	 GL_COMPRESSED_RGBA_S3TC_DXT1_EXT),
#else
	NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT1a,	FORMAT_DXT1A,	 PairNvttPixelGLFormat::WRONG_GL_VALUE),
#endif //GL_COMPRESSED_RGBA_S3TC_DXT1_EXT

#if defined(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT)
	NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT3,	FORMAT_DXT3,	 GL_COMPRESSED_RGBA_S3TC_DXT3_EXT),
#else
	NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT3,	FORMAT_DXT3,	 PairNvttPixelGLFormat::WRONG_GL_VALUE),
#endif //GL_COMPRESSED_RGBA_S3TC_DXT3_EXT

#if defined(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)
	NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT5,	FORMAT_DXT5,	 GL_COMPRESSED_RGBA_S3TC_DXT5_EXT),
	NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT5n,	FORMAT_DXT5NM,	 GL_COMPRESSED_RGBA_S3TC_DXT5_EXT),
#else
	NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT5,	FORMAT_DXT5,	 PairNvttPixelGLFormat::WRONG_GL_VALUE),
	NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT5n,	FORMAT_DXT5NM,	 PairNvttPixelGLFormat::WRONG_GL_VALUE),
#endif //GL_COMPRESSED_RGBA_S3TC_DXT5_EXT

	NvttHelper::PairNvttPixelGLFormat(nvtt::Format_ATC_RGB,	FORMAT_ATC_RGB,	 0),
	NvttHelper::PairNvttPixelGLFormat(nvtt::Format_ATC_RGBA_EXPLICIT_ALPHA,	FORMAT_ATC_RGBA_EXPLICIT_ALPHA,	 0),
	NvttHelper::PairNvttPixelGLFormat(nvtt::Format_ATC_RGBA_INTERPOLATED_ALPHA,	FORMAT_ATC_RGBA_INTERPOLATED_ALPHA,	 0),
	
	NvttHelper::PairNvttPixelGLFormat(nvtt::Format_RGBA,	FORMAT_RGBA8888, PairNvttPixelGLFormat::WRONG_GL_VALUE),
};

PixelFormat NvttHelper::GetPixelFormatByNVTTFormat(nvtt::Format nvttFormat)
{
	PixelFormat retValue = FORMAT_INVALID;
	for(uint32 i = 0; i < Format_COUNT; ++i)
	{
		if(formatNamesMap[i].nvttFormat == nvttFormat)
		{
			 retValue = formatNamesMap[i].davaFormat;
			 break;
		}
	}
	return retValue;
}

nvtt::Format NvttHelper::GetNVTTFormatByPixelFormat(PixelFormat pixelFormat)
{
	//bc5 is unsupported, used to determinate fail in search
	nvtt::Format retValue = Format_BC5;
	for(uint32 i = 0; i < Format_COUNT; ++i)
	{
		if(formatNamesMap[i].davaFormat == pixelFormat)
		{
			retValue = formatNamesMap[i].nvttFormat;
			break;
		}
	}
	return retValue;
}

class QualcommHeler
{
#define Q_FORMAT_COUNT 3
	
public:
	struct PairQualcommPixelGLFormat {
		int32 qFormat;
		PixelFormat davaFormat;
		PairQualcommPixelGLFormat(int32 qFormat, PixelFormat davaFormat)
		{
			this->qFormat = qFormat;
			this->davaFormat = davaFormat;
		}
	};
	
	const static PairQualcommPixelGLFormat formatPair[Q_FORMAT_COUNT];
	int32 static GetQualcommFormat(PixelFormat format);
	PixelFormat static GetDavaFormat(int32 qFormat);
};

const QualcommHeler::PairQualcommPixelGLFormat QualcommHeler::formatPair[] =
{
	PairQualcommPixelGLFormat(Q_FORMAT_ATC_RGB, FORMAT_ATC_RGB),
	PairQualcommPixelGLFormat(Q_FORMAT_ATC_RGBA_EXPLICIT_ALPHA, FORMAT_ATC_RGBA_EXPLICIT_ALPHA),
	PairQualcommPixelGLFormat(Q_FORMAT_ATC_RGBA_INTERPOLATED_ALPHA, FORMAT_ATC_RGBA_INTERPOLATED_ALPHA),
};

int32 QualcommHeler::GetQualcommFormat(PixelFormat format)
{
	for (int32 i = 0; i < Q_FORMAT_COUNT; ++i)
	{
		if (formatPair[i].davaFormat == format)
			return formatPair[i].qFormat;
	}
	Logger::Error("Wrong pixel format (%d).", format);
	return -1;
}

PixelFormat QualcommHeler::GetDavaFormat(int32 format)
{
	for (int32 i = 0; i < Q_FORMAT_COUNT; ++i)
	{
		if (formatPair[i].qFormat == format)
			return formatPair[i].davaFormat;
	}
	Logger::Error("Wrong qualcomm format (%d).", format);
	return FORMAT_INVALID;
}

	
bool LibDxtHelper::ReadDxtFile(const FilePath &fileName, Vector<Image*> &imageSet, bool forceSoftwareConvertation /*=false*/)
{
	nvtt::Decompressor dec;

	if(!NvttHelper::InitDecompressor(dec, fileName))
	{
		return false;
	}
	
	return NvttHelper::ReadDxtFile(dec, imageSet, forceSoftwareConvertation);
}

bool LibDxtHelper::ReadDxtFile(File * file, Vector<Image*> &imageSet, bool forceSoftwareConvertation /*=false*/)
{
	nvtt::Decompressor dec;

	if(!NvttHelper::InitDecompressor(dec, file))
	{
		return false;
	}
	return NvttHelper::ReadDxtFile(dec, imageSet, forceSoftwareConvertation);
}

bool LibDxtHelper::DecompressImageToRGBA(const Image & image, Vector<Image*> &imageSet, bool forceSoftwareConvertation)
{
	if(!(image.format >= FORMAT_DXT1 && image.format <= FORMAT_DXT5NM) )
	{
		Logger::Error("[LibDxtHelper::DecompressImageToRGBA] Wrong copression format (%d).", image.format);
		return false;
	}
	nvtt::Format innerComprFormat = NvttHelper::GetNVTTFormatByPixelFormat(image.format);
	if(nvtt::Format_BC5 == innerComprFormat)
	{ 	//bc5 is unsupported, used to determinate fail in search
		Logger::Error("[LibDxtHelper::DecompressImageToRGBA] Can't work with nvtt::Format_BC5.");
		return false;
	}
	
		
	InputOptions inputOptions;
	inputOptions.setTextureLayout(TextureType_2D, image.width, image.height);
	inputOptions.setMipmapGeneration(false);

	CompressionOptions compressionOptions;
	compressionOptions.setFormat(innerComprFormat);
	if(FORMAT_DXT1NM == image.format)
	{
		compressionOptions.setColorWeights(1, 1, 0);
	}
	else if (FORMAT_DXT5NM == image.format)
	{
		inputOptions.setNormalMap(true);
	}
	
	uint32 headerSize = DECOMPRESSOR_MIN_HEADER_SIZE;
	uint8* imageHeaderBuffer = new uint8[headerSize];

	uint32 realHeaderSize = nvtt::Decompressor::getHeader(imageHeaderBuffer, headerSize, inputOptions, compressionOptions);

	nvtt::Decompressor dec;

	uint8* compressedImageBuffer = new uint8[realHeaderSize + image.dataSize];
	memcpy(compressedImageBuffer, imageHeaderBuffer, realHeaderSize);
	memcpy(compressedImageBuffer + realHeaderSize, image.data, image.dataSize);
    SafeDeleteArray(imageHeaderBuffer);
	
	bool retValue = NvttHelper::InitDecompressor(dec, compressedImageBuffer, realHeaderSize + image.dataSize);
	if(retValue)
	{
		retValue = NvttHelper::ReadDxtFile(dec, imageSet, forceSoftwareConvertation);
	}

    SafeDeleteArray(compressedImageBuffer);
	return retValue;
}

bool NvttHelper::IsAtcFormat(nvtt::Format format)
{
	return (format == Format_ATC_RGB || format == Format_ATC_RGBA_EXPLICIT_ALPHA || format == Format_ATC_RGBA_INTERPOLATED_ALPHA);
}
	
bool NvttHelper::ReadDxtFile(nvtt::Decompressor & dec, Vector<Image*> &imageSet, bool forceSoftwareConvertation)
{
    for_each(imageSet.begin(), imageSet.end(), SafeRelease<Image>);
	imageSet.clear();
	
	
    DDSInfo info;
    if(!GetInfo(dec, info))
    {
		Logger::Error("[NvttHelper::ReadDxtFile] Error during header reading.");
		return false;
    }

	if(0 == info.width || 0 == info.height || 0 == info.mipmapsCount)
	{
		Logger::Error("[NvttHelper::ReadDxtFile] Wrong mipmapsCount/width/height value in dds header.");
		return false;
	}
	

	nvtt::Format format;
	if(!dec.getCompressionFormat(format))
	{
		Logger::Error("[NvttHelper::ReadDxtFile] Getting format information cause error.");
		return false;
	}

	//check hardware support, in case of rgb use nvtt to reorder bytes
	bool isHardwareSupport = false;
	if (IsAtcFormat(format))
		isHardwareSupport = RenderManager::Instance()->GetCaps().isATCSupported;
	else
		isHardwareSupport = RenderManager::Instance()->GetCaps().isDXTSupported;
	
	if (!forceSoftwareConvertation && isHardwareSupport)
	{
		uint8* compressedImges = new uint8[info.dataSize];
	
		if(!dec.getRawData(compressedImges, info.dataSize))
		{
			Logger::Error("[NvttHelper::ReadDxtFile] Reading compressed data cause error in nvtt lib.");

            SafeDeleteArray(compressedImges);
			return false;
		}
        
        if(format == Format_RGB)
        {
            SwapBRChannels(compressedImges, info.dataSize);
        }

        PixelFormat pixFormat = GetPixelFormat(dec);

		bool retValue = true;
        
        uint32 offset = 0;
		for(uint32 faceIndex = 0; faceIndex < info.faceCount; ++faceIndex)
		{
			uint32 faceWidth = info.width;
			uint32 faceHeight = info.height;
			
			for(uint32 i = 0; i < info.mipmapsCount; ++i)
			{
				uint32 mipSize = 0;
				if(!dec.getMipmapSize(i, mipSize))
				{
					retValue = false;
					break;
				}
				
				Image* innerImage = Image::Create(faceWidth, faceHeight, pixFormat);
				innerImage->mipmapLevel = i;
				
				if(info.faceCount > 1)
				{
					innerImage->cubeFaceID = NvttHelper::GetCubeFaceId(info.faceFlags, faceIndex);
				}
				
				memcpy(innerImage->data, compressedImges + offset, mipSize);
				imageSet.push_back(innerImage);
				
				faceWidth = Max((uint32)1, faceWidth / 2);
				faceHeight = Max((uint32)1, faceHeight / 2);
				
				offset += mipSize;
			}
		}

        SafeDeleteArray(compressedImges);
		return retValue;
	}
	else
	{
#if defined (__DAVAENGINE_ANDROID__) || defined(__DAVAENGINE_IPHONE__)
        Logger::Error("[NvttHelper::ReadDxtFile] Android should have hardware decoding of DDS. iPhone should have no support of DDS.");
		return false;
#else //#if defined (__DAVAENGINE_ANDROID__) || defined(__DAVAENGINE_IPHONE__)
        
		if (IsAtcFormat(format))
		{
			return DecompressAtc(dec, info, GetPixelFormatByNVTTFormat(format), imageSet);
		}
		else
		{
			return DecompressDxt(dec, info, imageSet);
		}
		
#endif //#if defined (__DAVAENGINE_ANDROID__) || defined(__DAVAENGINE_IPHONE__)

	}

	return false;
}
	
bool NvttHelper::DecompressDxt(const nvtt::Decompressor & dec, DDSInfo info, Vector<Image*> &imageSet)
{
	for(uint32 faceIndex = 0; faceIndex < info.faceCount; ++faceIndex)
	{
		uint32 faceWidth = info.width;
		uint32 faceHeight = info.height;

		for(uint32 i = 0; i < info.mipmapsCount; ++i)
		{
			Image* innerImage = Image::Create(faceWidth, faceHeight, FORMAT_RGBA8888);
			if(dec.process(innerImage->data, innerImage->dataSize, i, faceIndex))
			{
				SwapBRChannels(innerImage->data, innerImage->dataSize);
				
				innerImage->mipmapLevel = i;
				
				if(info.faceCount > 1)
				{
					innerImage->cubeFaceID = NvttHelper::GetCubeFaceId(info.faceFlags, faceIndex);
				}
				
				imageSet.push_back(innerImage);
			}
			else
			{
				Logger::Error("nvtt lib compression fail.");
				SafeRelease(innerImage);
				return false;
			}
			
			faceWidth = Max((uint32)1, faceWidth / 2);
			faceHeight = Max((uint32)1, faceHeight / 2);
		}
	}
	return true;
}

bool NvttHelper::DecompressAtc(const nvtt::Decompressor & dec, DDSInfo info, PixelFormat format, Vector<Image*> &imageSet)
{
#if defined(__DAVAENGINE_MACOS__)
	if (format == FORMAT_ATC_RGBA_INTERPOLATED_ALPHA)
	{
		Logger::Error("Decompressing FORMAT_ATC_RGBA_INTERPOLATED_ALPHA disabled on OSX platform, because of bug in qualcomm library");
		return false;
	}
#endif
	uint8* compressedImges = new uint8[info.dataSize];
	
	if(!dec.getRawData(compressedImges, info.dataSize))
	{
		Logger::Error("[NvttHelper::DecompressAtc] Reading compressed data cause error in nvtt lib.");
		
		SafeDeleteArray(compressedImges);
		return false;
	}

	bool res = true;
	unsigned char* buffer = compressedImges;
	
	for(uint32 faceIndex = 0; faceIndex < info.faceCount; ++faceIndex)
	{
		uint32 faceWidth = info.width;
		uint32 faceHeight = info.height;

		for(uint32 i = 0; i < info.mipmapsCount; ++i)
		{
			TQonvertImage srcImg = {0};
			TQonvertImage dstImg = {0};
			
			srcImg.nWidth = faceWidth;
			srcImg.nHeight = faceHeight;
			srcImg.nFormat = QualcommHeler::GetQualcommFormat(format);
			dec.getMipmapSize(i, srcImg.nDataSize);
			srcImg.pData = buffer;
			buffer += srcImg.nDataSize;
			
			dstImg.nWidth = faceWidth;
			dstImg.nHeight = faceHeight;
			dstImg.nFormat = Q_FORMAT_RGBA_8888;
			dstImg.nDataSize = 0;
			dstImg.pData = NULL;
			
			if (Qonvert(&srcImg, &dstImg) != Q_SUCCESS ||
				dstImg.nDataSize == 0)
			{
				Logger::Error("[NvttHelper::DecompressAtc] Reading decompress atc data.");
				res = false;
				break;
			}
			
			dstImg.pData = new unsigned char[dstImg.nDataSize];
			if (Qonvert(&srcImg, &dstImg) != Q_SUCCESS)
			{
				Logger::Error("[NvttHelper::DecompressAtc] Reading decompress atc data.");
				SafeDeleteArray(dstImg.pData);
				
				res = false;
				break;
			}
			
			Image* innerImage = Image::CreateFromData(faceWidth, faceHeight, FORMAT_RGBA8888, dstImg.pData);
			innerImage->mipmapLevel = i;
			
			if(info.faceCount > 1)
			{
				innerImage->cubeFaceID = NvttHelper::GetCubeFaceId(info.faceFlags, faceIndex);
			}

			//SwapBRChannels(innerImage->data, innerImage->dataSize);
			imageSet.push_back(innerImage);
			
			faceWidth = Max((uint32)1, faceWidth / 2);
			faceHeight = Max((uint32)1, faceHeight / 2);

			SafeDeleteArray(dstImg.pData);
		}
	}
	
	SafeDeleteArray(compressedImges);
	return res;
}

bool LibDxtHelper::WriteDdsFile(const FilePath & fileNameOriginal, int32 width, int32 height, uint8 ** data, uint32 dataCount, PixelFormat compressionFormat, bool generateMipmaps)
{
	//creating tmp dds file, nvtt accept only filename.dds as input, because of this the last letter befor "." should be changed to "_".
	if(!fileNameOriginal.IsEqualToExtension(".dds"))
    {
		Logger::Error("[LibDxtHelper::WriteDxtFile] Wrong input file name (%s).", fileNameOriginal.GetAbsolutePathname().c_str());
        return false;
    }
	
	if (compressionFormat == FORMAT_ATC_RGB ||
		compressionFormat == FORMAT_ATC_RGBA_EXPLICIT_ALPHA ||
		compressionFormat == FORMAT_ATC_RGBA_INTERPOLATED_ALPHA)
	{
		return WriteAtcFile(fileNameOriginal, width, height, data, dataCount, compressionFormat, generateMipmaps);
	}
	return WriteDxtFile(fileNameOriginal, width, height, data, dataCount, compressionFormat, generateMipmaps);
}

bool LibDxtHelper::WriteDxtFile(const FilePath & fileNameOriginal, int32 width, int32 height, uint8 ** data, uint32 dataCount, PixelFormat compressionFormat, bool generateMipmaps)
{
	if(!( (compressionFormat >= FORMAT_DXT1 && compressionFormat <= FORMAT_DXT5NM)|| (compressionFormat == FORMAT_RGBA8888)) )
	{
		Logger::Error("[LibDxtHelper::WriteDxtFile] Wrong copression format (%d).", compressionFormat);
		return false;
	}

    nvtt::Format innerComprFormat = NvttHelper::GetNVTTFormatByPixelFormat(compressionFormat);
    if(nvtt::Format_BC5 == innerComprFormat)
	{ 	//bc5 is unsupported, used to determinate fail in search
		Logger::Error("[LibDxtHelper::WriteDxtFile] Can't work with nvtt::Format_BC5.");
		return false;
	}
    
	uint32 imgDataSize = width * height * Texture::GetPixelFormatSizeInBytes(FORMAT_RGBA8888);
	for(uint32 i = 0; i < dataCount; ++i)
	{
		NvttHelper::SwapBRChannels(data[i], imgDataSize);
	}

	nvtt::TextureType textureType = (dataCount > 1) ? nvtt::TextureType_Cube : nvtt::TextureType_2D;
	InputOptions inputOptions;
	inputOptions.setTextureLayout(textureType, width, height);
    inputOptions.setMipmapGeneration(generateMipmaps);
	
	for(uint32 i = 0; i < dataCount; ++i)
	{
		inputOptions.setMipmapData(data[i], width, height, 1, i);
	}

	CompressionOptions compressionOptions;
	compressionOptions.setFormat(innerComprFormat);
	if(FORMAT_DXT1NM == compressionFormat)
	{
		compressionOptions.setColorWeights(1, 1, 0);
	}
	else if (FORMAT_DXT5NM == compressionFormat)
	{
		inputOptions.setNormalMap(true);
	}
	
    
	OutputOptions outputOptions;
	FilePath fileName = FilePath::CreateWithNewExtension(fileNameOriginal, "_dds");
	outputOptions.setFileName(fileName.GetAbsolutePathname().c_str());
	
	Compressor compressor;
	bool ret = compressor.process(inputOptions, compressionOptions, outputOptions);
    if(ret)
    {
        FileSystem::Instance()->DeleteFile(fileNameOriginal);
        if(!FileSystem::Instance()->MoveFile(fileName, fileNameOriginal, true))
        {
            Logger::Error("[LibDxtHelper::WriteDxtFile] Temporary dds file renamig failed.");
            ret = false;
        }
    }
    else
    {
		Logger::Error("[LibDxtHelper::WriteDxtFile] Error during writing DDS file (%s).", fileName.GetAbsolutePathname().c_str());
    }
    
	return ret;
}

bool LibDxtHelper::WriteAtcFile(const FilePath & fileNameOriginal, int32 width, int32 height, uint8 ** data, uint32 dataCount, PixelFormat compressionFormat, bool generateMipmaps)
{
	const int32 minSize = 0;
	
	if (compressionFormat != FORMAT_ATC_RGB &&
		compressionFormat != FORMAT_ATC_RGBA_EXPLICIT_ALPHA &&
		compressionFormat != FORMAT_ATC_RGBA_INTERPOLATED_ALPHA)
	{
		Logger::Error("[LibDxtHelper::WriteAtcFile] Wrong copression format (%d).", compressionFormat);
		return false;
	}
	
	//VI: calculate image buffer size
	
	int32 bufSize = 0;
	std::map<int32, int32>* mipSize = new std::map<int32, int32>[dataCount];
	int32 baseWidth = 0;
	int32 baseHeight = 0;
	
	for(uint32 i = 0; i < dataCount; ++i)
	{
		TQonvertImage srcImg = {0};
		
		srcImg.nWidth = width;
		srcImg.nHeight = height;
		srcImg.nFormat = Q_FORMAT_RGBA_8888;
		srcImg.nDataSize = width * height * 4;
		srcImg.pData = data[i];
		
		baseWidth = width;
		baseHeight = height;
		
		do
		{
			TQonvertImage dstImg = {0};
			dstImg.nWidth = baseWidth;
			dstImg.nHeight = baseHeight;
			dstImg.nFormat = QualcommHeler::GetQualcommFormat(compressionFormat);
			dstImg.nDataSize = 0;
			dstImg.pData = NULL;
			
			if (Qonvert(&srcImg, &dstImg) != Q_SUCCESS || dstImg.nDataSize == 0)
			{
				Logger::Error("[LibDxtHelper::WriteAtcFile] Error converting (%s).", fileNameOriginal.GetAbsolutePathname().c_str());
				SafeDeleteArray(mipSize);
				return false;
			}
			bufSize += dstImg.nDataSize;
			mipSize[i][baseWidth] = dstImg.nDataSize;
			
			baseWidth = baseWidth >> 1;
			baseHeight = baseHeight >> 1;
		}
		while(generateMipmaps && baseWidth > minSize && baseHeight > minSize);
	}
	
	//VI: convert faces
	unsigned char* buffer = new unsigned char[bufSize];
	unsigned char* tmpBuffer = buffer;

	for(uint32 i = 0; i < dataCount; ++i)
	{
		baseWidth = width;
		baseHeight = height;
		
		TQonvertImage srcImg = {0};
		
		srcImg.nWidth = width;
		srcImg.nHeight = height;
		srcImg.nFormat = Q_FORMAT_RGBA_8888;
		srcImg.nDataSize = width * height * 4;
		srcImg.pData = data[i];

		do
		{
			TQonvertImage dstImg = {0};
			dstImg.nWidth = baseWidth;
			dstImg.nHeight = baseHeight;
			dstImg.nFormat = QualcommHeler::GetQualcommFormat(compressionFormat);
			dstImg.nDataSize = mipSize[i][baseWidth];
			dstImg.pData = tmpBuffer;
			tmpBuffer += dstImg.nDataSize;
			
			if (Qonvert(&srcImg, &dstImg) != Q_SUCCESS || dstImg.nDataSize == 0)
			{
				Logger::Error("[LibDxtHelper::WriteAtcFile] Error converting (%s).", fileNameOriginal.GetAbsolutePathname().c_str());
				SafeDeleteArray(buffer);
				SafeDeleteArray(mipSize);
				return false;
			}
			
			baseWidth = baseWidth >> 1;
			baseHeight = baseHeight >> 1;
		}
		while(generateMipmaps && baseWidth > minSize && baseHeight > minSize);
	}
	
	nvtt::Format innerComprFormat = NvttHelper::GetNVTTFormatByPixelFormat(compressionFormat);
	
	nvtt::TextureType textureType = (dataCount > 1) ? nvtt::TextureType_Cube : nvtt::TextureType_2D;
	InputOptions inputOptions;
	inputOptions.setTextureLayout(textureType, width, height);
    inputOptions.setMipmapGeneration(generateMipmaps, mipSize[0].size() - 1);
	
	CompressionOptions compressionOptions;
	compressionOptions.setFormat(innerComprFormat);
	
	nvtt::Decompressor decompress;
	unsigned int headerSize = DECOMPRESSOR_MIN_HEADER_SIZE;
	unsigned char header[DECOMPRESSOR_MIN_HEADER_SIZE];
	headerSize = decompress.getHeader(header, headerSize, inputOptions, compressionOptions);
	
	FilePath fileName =	FilePath::CreateWithNewExtension(fileNameOriginal, "_dds");
	bool res = false;
	File *file = File::Create(fileName, File::CREATE | File::WRITE);
	if (file)
	{
		file->Write(header, headerSize);
		file->Write(buffer, bufSize);
		SafeRelease(file);
		FileSystem::Instance()->DeleteFile(fileNameOriginal);
		res = FileSystem::Instance()->MoveFile(fileName, fileNameOriginal, true);
		if (!res)
			Logger::Error("[LibDxtHelper::WriteDxtFile] Temporary dds file renamig failed.");
	}
	else
	{
		Logger::Error("[LibDxtHelper::WriteDxtFile] Temporary dds file renamig failed.");
	}

	SafeDeleteArray(mipSize);
	SafeDeleteArray(buffer);
	return res;
}
	
bool LibDxtHelper::IsDxtFile(const FilePath & filePathname)
{
	nvtt::Decompressor dec;
	return dec.initWithDDSFile(filePathname.GetAbsolutePathname().c_str());
}

bool LibDxtHelper::IsDxtFile(File * file)
{
	nvtt::Decompressor dec;
	return NvttHelper::InitDecompressor(dec,file);
}

PixelFormat LibDxtHelper::GetPixelFormat(const FilePath & fileName)
{
	nvtt::Decompressor dec;

	if(!NvttHelper::InitDecompressor(dec, fileName))
	{
		return FORMAT_INVALID;
	}

	return NvttHelper::GetPixelFormat(dec);
}

PixelFormat LibDxtHelper::GetPixelFormat(File * file)
{
	nvtt::Decompressor dec;

	if(!NvttHelper::InitDecompressor(dec, file))
	{
		return FORMAT_INVALID;
	}

	return NvttHelper::GetPixelFormat(dec);
}

uint32 LibDxtHelper::GetMipMapLevelsCount(const FilePath & fileName)
{
	nvtt::Decompressor dec;

	if(!NvttHelper::InitDecompressor(dec, fileName))
	{
		return 0;
	}

	return NvttHelper::GetMipMapLevelsCount(dec);
}

uint32 LibDxtHelper::GetMipMapLevelsCount(File * file)
{
	nvtt::Decompressor dec;

	if(!NvttHelper::InitDecompressor(dec, file))
	{
		return 0;
	}

	return NvttHelper::GetMipMapLevelsCount(dec);
}

uint32 LibDxtHelper::GetDataSize(const FilePath & fileName)
{
	nvtt::Decompressor dec;

	if(!NvttHelper::InitDecompressor(dec, fileName))
	{
		return 0;
	}

	return NvttHelper::GetDataSize(dec);
}

uint32 LibDxtHelper::GetDataSize(File * file)
{
    nvtt::Decompressor dec ;

	if(!NvttHelper::InitDecompressor(dec, file))
	{
		return 0;
	}

	return NvttHelper::GetDataSize(dec);
}


bool LibDxtHelper::GetTextureSize(const FilePath & fileName, uint32 & width, uint32 & height)
{
	nvtt::Decompressor dec;

	if(!NvttHelper::InitDecompressor(dec, fileName))
	{
		return false;
	}

	return NvttHelper::GetTextureSize(dec, width, height);
}

bool LibDxtHelper::GetTextureSize(File * file, uint32 & width, uint32 & height)
{
	nvtt::Decompressor dec ;

	if(!NvttHelper::InitDecompressor(dec, file))
	{
		return false;
	}

	return NvttHelper::GetTextureSize(dec, width, height);
}
    

bool LibDxtHelper::AddCRCIntoMetaData(const FilePath &filePathname)
{
	String fileNameStr = filePathname.GetAbsolutePathname();

	uint32 tag = 0, crc = 0;
    bool haveCRCTag = GetCRCFromDDSHeader(filePathname, &tag, &crc);
	if(haveCRCTag)
	{
		Logger::Error("[LibDxtHelper::AddCRCIntoMetaData] CRC is already added into %s", fileNameStr.c_str());
		return false;
	}
	else if(crc != 0 || tag != 0)
	{
		Logger::Error("[LibDxtHelper::AddCRCIntoMetaData] reserved for CRC place is used in %s", fileNameStr.c_str());
		return false;
	}

	File *fileRead = File::Create(filePathname, File::READ | File::OPEN);
	if(!fileRead)
	{
		Logger::Error("[LibDxtHelper::AddCRCIntoMetaData] cannot open file %s", fileNameStr.c_str());
		return false;
	}
    uint32 fileSize = fileRead->GetSize();
    char *fileBuffer = new char[fileSize];
	if(!fileBuffer)
	{
		Logger::Error("[LibDxtHelper::AddCRCIntoMetaData]: cannot allocate buffer for file data");
		SafeRelease(fileRead);
		return false;
	}
    if(fileRead->Read(fileBuffer, fileSize) != fileSize)
	{
		Logger::Error("[LibDxtHelper::AddCRCIntoMetaData]: cannot read from file %s", fileNameStr.c_str());
		SafeDeleteArray(fileBuffer);
		SafeRelease(fileRead);
		return false;
	}
    
    SafeRelease(fileRead);
    
    crc = CRC32::ForBuffer(fileBuffer, fileSize);
    uint32* modificationMetaDataPointer = (uint32*)(fileBuffer + DDS_HEADER_CRC_OFFSET);
    *modificationMetaDataPointer = METADATA_CRC_TAG;
    modificationMetaDataPointer++;
    *modificationMetaDataPointer = crc;
	
    FilePath tempFile(filePathname.GetAbsolutePathname() + "_");
	
	File *fileWrite = File::Create(tempFile, File::WRITE | File::CREATE);
	if(!fileWrite)
	{
		Logger::Error("[LibDxtHelper::AddCRCIntoMetaData]: cannot create file %s",
					  tempFile.GetAbsolutePathname().c_str());
		return false;
	}
	
    bool writeSucces = fileWrite->Write(fileBuffer, fileSize) == fileSize;
    SafeDeleteArray(fileBuffer);
    SafeRelease(fileWrite);
	if(writeSucces)
	{
		FileSystem::Instance()->DeleteFile(filePathname);
		FileSystem::Instance()->MoveFile(tempFile, filePathname, true);
		return true;
	}
    else
    {
        Logger::Error("[LibDxtHelper::AddCRCIntoMetaData]: cannot write to file %s",
                      tempFile.GetAbsolutePathname().c_str());
        FileSystem::Instance()->DeleteFile(tempFile);
        return false;
    }
}

bool LibDxtHelper::GetCRCFromDDSHeader(const FilePath &filePathname, uint32* outputTag, uint32* outputCRC)
{
	String fileNameStr = filePathname.GetAbsolutePathname();
	
	File *fileRead = File::Create(filePathname, File::READ | File::OPEN);
	if(!fileRead)
	{
		Logger::Error("[LibDxtHelper::GetCRCFromDDSHeader] cannot open file %s", fileNameStr.c_str());
		return false;
	}

	if(!IsDxtFile(fileRead))
	{
		Logger::Error("[LibDxtHelper::GetCRCFromDDSHeader] file %s isn't a dds one", fileNameStr.c_str());
		SafeRelease(fileRead);
		return false;
	}

	fileRead->Seek(DDS_HEADER_CRC_OFFSET, File::SEEK_FROM_START);
	uint32 tag = 0;
	if(fileRead->Read(&tag,sizeof(tag)) != sizeof(tag))
	{
		Logger::Error("[LibDxtHelper::GetCRCFromDDSHeader]  cannot read file %s", fileNameStr.c_str());
		SafeRelease(fileRead);
		return false;
	}

	uint32 crc = 0;
	if(fileRead->Read(&crc,sizeof(crc)) != sizeof(crc))
	{
		Logger::Error("[LibDxtHelper::GetCRCFromDDSHeader]  cannot read file %s", fileNameStr.c_str());
		SafeRelease(fileRead);
		return false;
	}
	
	*outputCRC = crc;
	*outputTag = tag;
	
	SafeRelease(fileRead);
	return tag == METADATA_CRC_TAG;
}

uint32 LibDxtHelper::GetCRCFromFile(const FilePath &filePathname)
{
	uint32 tag = 0, crc = 0;
	bool success = GetCRCFromDDSHeader(filePathname, &tag, &crc);
	return success ? crc : CRC32::ForFile(filePathname);
}

PixelFormat NvttHelper::GetPixelFormat(nvtt::Decompressor & dec)
{
    nvtt::Format innerFormat;
    
    if(!dec.getCompressionFormat(innerFormat))
    {
        Logger::Error("[NvttHelper::GetPixelFormat] Wrong dds file compression format.");
        return FORMAT_INVALID;
    }
    
    return NvttHelper::GetPixelFormatByNVTTFormat(innerFormat);
}

uint32 NvttHelper::GetMipMapLevelsCount(nvtt::Decompressor & dec)
{
    DDSInfo info;
    GetInfo(dec, info);
    return info.mipmapsCount;
}
    
uint32 NvttHelper::GetDataSize(nvtt::Decompressor & dec)
{
    DDSInfo info;
    GetInfo(dec, info);
    return info.dataSize;
}


bool NvttHelper::GetTextureSize(nvtt::Decompressor & dec, uint32 & width, uint32 & height)
{
	DDSInfo info;
	
	bool ret = GetInfo(dec, info);
	width = info.width;
	height = info.height;
	return ret;
}

bool NvttHelper::GetInfo(nvtt::Decompressor & dec, DDSInfo &info)
{
	bool retVal = dec.getInfo(info.mipmapsCount, info.width, info.height, info.dataSize, info.headerSize, info.faceCount, info.faceFlags);
	if(!retVal)
	{
		Logger::Error("[NvttHelper::GetInfo] Error: can't read info from DDS file.");
	}
		
	return retVal;
}

void NvttHelper::SwapBRChannels(uint8* data, uint32 size)
{
	for(uint32 i = 0; i < size; i+=4)
	{
		//RGBA <-> BGRA
	
		uint8* rComponent = data + i;
		
		uint8* bComponent = data + i + 2;
		uint8 tmp = *rComponent;
		*rComponent = *bComponent;
		*bComponent = tmp;
	}
}

bool NvttHelper::InitDecompressor(nvtt::Decompressor & dec, const FilePath & fileName)
{
	if(fileName.IsEmpty())
	{
		Logger::Error("[NvttHelper::InitDecompressor] try init with empty name");
		return false;
	}

	if(!dec.initWithDDSFile(fileName.GetAbsolutePathname().c_str()))
	{
		Logger::Error("[NvttHelper::InitDecompressor] Wrong dds file (%s).", fileName.GetAbsolutePathname().c_str());
		return false;
	}
    
	return true;
}

bool NvttHelper::InitDecompressor(nvtt::Decompressor & dec, File * file)
{
	if(NULL == file)
	{
		Logger::Error("[NvttHelper::InitDecompressor] Wrong handler.");
		return false;
	}
    
	file->Seek(0, File::SEEK_FROM_START);
	uint32 fileSize = file->GetSize();
	uint8* fileBuffer= new uint8[fileSize];
	file->Read(fileBuffer, fileSize);
    bool initied = dec.initWithDDSFile(fileBuffer, fileSize);
	file->Seek(0, File::SEEK_FROM_START);
	SafeDeleteArray(fileBuffer);
    return initied;
}

bool NvttHelper::InitDecompressor(nvtt::Decompressor & dec, const uint8 * mem, uint32 size)
{
	if(NULL == mem || size == 0 )
	{
		Logger::Error("[NvttHelper::InitDecompressor] Wrong buffer params.");
		return false;
	}

	if(!dec.initWithDDSFile(mem, size))
	{
		Logger::Error("[NvttHelper::InitDecompressor] Wrong buffer.");
		return false;
	}
	
	return true;
}
	
uint32 NvttHelper::GetCubeFaceId(uint32 nvttFaceDesc, int faceIndex)
{
	uint32 faceId = Texture::CUBE_FACE_INVALID;
	
	if(faceIndex >= 0 && faceIndex < 6)
	{
		/*uint32 preparedMask = nvttFaceDesc >> faceIndex;
		for(int i = 0; i < 6; ++i)
		{
			if(0 == (preparedMask & 1))
			{
				faceIndex++;
				preparedMask = preparedMask >> 1;
			}
		}*/
		int bitIndex = 0;
		int faceIdIndex = 0;
		for(int i = 0; i < 6; ++i)
		{
			if((nvttFaceDesc >> i) & 1)
			{
				bitIndex++;
				
				if(bitIndex >= (faceIndex + 1))
				{
					faceIdIndex = i;
					break;
				}
			}
		}

		static uint32 faceIndexMap[] = {
			Texture::CUBE_FACE_POSITIVE_X,
			Texture::CUBE_FACE_NEGATIVE_X,
			Texture::CUBE_FACE_POSITIVE_Y,
			Texture::CUBE_FACE_NEGATIVE_Y,
			Texture::CUBE_FACE_POSITIVE_Z,
			Texture::CUBE_FACE_NEGATIVE_Z
		};
		
		faceId = faceIndexMap[faceIdIndex];
	}
	
	return faceId;
}
	
};
