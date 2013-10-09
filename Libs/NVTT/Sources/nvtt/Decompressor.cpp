// Copyright NVIDIA Corporation 2008 -- Ignacio Castano <icastano@nvidia.com>
// 
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use,
// copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following
// conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.

#include <nvtt/nvtt.h>

#include <nvcore/Memory.h>
#include <nvcore/Ptr.h>


#include "Decompressor.h"

#include <nvimage/Image.h>


#include <nvimage/ImageIO.h>
#include <nvcore/StrLib.h>
#include <nvmath/color.h>

#include "InputOptions.h"
#include "CompressionOptions.h"

#define DDSCAPS2_CUBEMAP_POSITIVEX 0x00000400U
#define DDSCAPS2_CUBEMAP_NEGATIVEX 0x00000800U
#define DDSCAPS2_CUBEMAP_POSITIVEY 0x00001000U
#define DDSCAPS2_CUBEMAP_NEGATIVEY 0x00002000U
#define DDSCAPS2_CUBEMAP_POSITIVEZ 0x00004000U
#define DDSCAPS2_CUBEMAP_NEGATIVEZ 0x00008000U

using namespace nv;
using namespace nvtt;

Decompressor::Decompressor() : m(*new Decompressor::Private())
{
}

Decompressor::~Decompressor()
{
	erase();
	delete &m;
}

bool Decompressor::initWithDDSFile(const char * pathToDDSFile)
{
	return m.initWithDDSFile(pathToDDSFile);
}

bool Decompressor::Private::initWithDDSFile(const char * pathToDDSFile) 
{
	if(NULL == pathToDDSFile)
	{
		return false;
	}
	m_dds = new nv::DirectDrawSurface(pathToDDSFile);
	
	if (!m_dds->isValid())
	{
		printf("The file '%s' is not a valid DDS file.\n", pathToDDSFile);
		return false;
	}
	
	return true;
}


bool Decompressor::initWithDDSFile(FILE * handler)
{
	return m.initWithDDSFile(handler);
}

bool Decompressor::Private::initWithDDSFile(FILE * handler) 
{
	if(NULL == handler)
	{
		return false;
	}
	m_dds = new nv::DirectDrawSurface(handler);
	
	if (!m_dds->isValid())
	{
		return false;
	}
	
	return true;
}

bool Decompressor::initWithDDSFile(const uint8 * mem, uint size)
{
	return m.initWithDDSFile(mem, size);
}

bool Decompressor::Private::initWithDDSFile(const uint8 * mem, uint size) 
{
	if(NULL == mem || size == 0)
	{
		return false;
	}

	m_memmoryBufferPointer = new uint8[size];
	memcpy(m_memmoryBufferPointer, mem, size);
	m_dds = new nv::DirectDrawSurface(m_memmoryBufferPointer, size);
	
	if (!m_dds->isValid())
	{
		return false;
	}
	
	return true;
}


//NVTT_API void erase();
void Decompressor::erase()
{
	m.erase();
}

void Decompressor::Private::erase()
{
	if(NULL != m_dds)
	{
		delete m_dds;
		m_dds = NULL;
	}
	if(NULL != m_memmoryBufferPointer)
	{
		delete[] m_memmoryBufferPointer;
		m_memmoryBufferPointer = NULL;
	}
}

//NVTT_API bool process(void * data, unsigned int size, unsigned int mimpmapNumber) const;
bool Decompressor::process(void * data, unsigned int size, unsigned int mipmapNumber, unsigned int face) const
{
	return m.decompress( data, size, mipmapNumber, face);
}

bool Decompressor::Private::decompress(void * data, unsigned int size, unsigned int mipmapNumber, unsigned int face) const
{
	if(NULL == m_dds || NULL == data || 0 == size)
	{
		return false;
	}

	nv::Image img;
	m_dds->mipmap(&img, face, mipmapNumber);
	
	Color32 * innerContent = img.pixels();
	uint innerSize = img.width() * img.height() ;
	
	uint rawSize = innerSize * sizeof(Color32);

	if(size != rawSize)
	{
		return false;
	}

	memcpy(data, innerContent, rawSize);

	return true;
}

//NVTT_API bool getMipMapCount(unsigned int * mipmapCount) const;
bool Decompressor::getInfo(unsigned int & mipmapCount,
						   unsigned int & width,
						   unsigned int & height,
						   unsigned int & size,
						   unsigned int & headerSize,
						   unsigned int & faceCount,
						   unsigned int & faceFlags)  const
{
	return m.getInfo(mipmapCount, width, height, size, headerSize, faceCount, faceFlags);
}

bool Decompressor::Private::getInfo(unsigned int & mipmapCount,
									unsigned int & width,
									unsigned int & height,
									unsigned int & size,
									unsigned int & headerSize,
									unsigned int & faceCount,
									unsigned int & faceFlags) const
{
	if(NULL == m_dds)
	{
		return false;
	}

	mipmapCount = m_dds->mipmapCount();
	width = m_dds->width();
	height = m_dds->height();
	size = m_dds->size();
	headerSize = m_dds->headerSize();
	faceCount = 1;
	faceFlags = 0;
	
	if(m_dds->isTextureCube())
	{
		faceCount = m_dds->getFaceCount();
		faceFlags = m_dds->getFaceFlags();
	}
	
	return true;
}

bool Decompressor::getRawData(void* buffer, unsigned int size)  const
{
	return m.getRawData(buffer, size);
}

bool Decompressor::Private::getRawData(void* buffer, unsigned int size) const
{
	if(NULL == m_dds)
	{
		return false;
	}

	return m_dds->getRawDate(buffer, size);
}

bool Decompressor::getMipmapSize(unsigned int number, unsigned int & size)  const
{
	return m.getMipmapSize(number, size);
}

bool Decompressor::Private::getMipmapSize(unsigned int number, unsigned int & size) const
{
	if(NULL == m_dds)
	{
		return false;
	}

	size = m_dds->mipmapSize(number);
	return true;
}

bool Decompressor::getCompressionFormat(Format & comprFormat) const
{
	return m.getCompressionFormat(comprFormat);
}

bool Decompressor::Private::getCompressionFormat(Format & comprFormat) const
{
	Format tmp;
	bool retValue = m_dds->getFormat(&tmp);
	comprFormat = tmp;
	return retValue;
}

unsigned int Decompressor::getHeader(void* buffer, unsigned int & bufferSize, const InputOptions & inputOptions, const CompressionOptions & compressionOptions)
{
	return  Decompressor::Private::getHeader(buffer, bufferSize, inputOptions.m, compressionOptions.m);
}

unsigned int Decompressor::Private::getHeader(void* buffer, const unsigned int bufferSize, const InputOptions::Private & inputOptions, const CompressionOptions::Private & compressionOptions)
{
	if( bufferSize < DECOMPRESSOR_MIN_HEADER_SIZE )
	{
		return 0;
	}

	DDSHeader header;

	inputOptions.computeTargetExtents();

	header.setWidth(inputOptions.targetWidth);
	header.setHeight(inputOptions.targetHeight);

	int mipmapCount = inputOptions.realMipmapCount();
	nvDebugCheck(mipmapCount > 0);

	header.setMipmapCount(mipmapCount);

	if (inputOptions.textureType == TextureType_2D) {
		header.setTexture2D();
	}
	else if (inputOptions.textureType == TextureType_Cube) {
		header.setTextureCube();
	}		

	uint p = inputOptions.targetWidth * ((compressionOptions.bitcount + 7) / 8);
		// Align to 32 bits.
	uint pitch = ((p + 3) / 4) * 4;

	if (compressionOptions.format == Format_RGBA)
	{
		
		header.setPitch(pitch);
		header.setPixelFormat(compressionOptions.bitcount, compressionOptions.rmask, compressionOptions.gmask, compressionOptions.bmask, compressionOptions.amask);
	}
	else
	{
		
		uint lineralSize = 0;
		if (compressionOptions.format == Format_RGBA)
		{
			lineralSize = inputOptions.targetDepth * inputOptions.targetHeight * pitch;
		}
		else 
		{
			uint blockSize = 0;
			if (compressionOptions.format == Format_DXT1 || compressionOptions.format == Format_DXT1a || compressionOptions.format == Format_BC4) 
			{
				blockSize = 8;
			}
			else 
			{
				blockSize = 16;
			}
			

			lineralSize = ((inputOptions.targetWidth + 3) / 4) * ((inputOptions.targetHeight + 3) / 4) * blockSize;
		}

		header.setLinearSize(lineralSize);

		if (compressionOptions.format == Format_DXT1 || compressionOptions.format == Format_DXT1a) {
			header.setFourCC('D', 'X', 'T', '1');
			if (inputOptions.isNormalMap) header.setNormalFlag(true);
		}
		else if (compressionOptions.format == Format_DXT3) {
			header.setFourCC('D', 'X', 'T', '3');
		}
		else if (compressionOptions.format == Format_DXT5) {
			header.setFourCC('D', 'X', 'T', '5');
		}
		else if (compressionOptions.format == Format_DXT5n) {
			header.setFourCC('D', 'X', 'T', '5');
			if (inputOptions.isNormalMap) header.setNormalFlag(true);
		}
		else if (compressionOptions.format == Format_BC4) {
			header.setFourCC('A', 'T', 'I', '1');
		}
		else if (compressionOptions.format == Format_BC5) {
			header.setFourCC('A', 'T', 'I', '2');
			if (inputOptions.isNormalMap) header.setNormalFlag(true);
		}
		else if (compressionOptions.format == Format_ATC_RGB) {
			header.setFourCC('A', 'T', 'C', ' ');
		}
		else if (compressionOptions.format == Format_ATC_RGBA_EXPLICIT_ALPHA) {
			header.setFourCC('A', 'T', 'C', 'I');
		}
		else if (compressionOptions.format == Format_ATC_RGBA_INTERPOLATED_ALPHA) {
			header.setFourCC('A', 'T', 'C', 'A');
		}
	}

	// Swap bytes if necessary.
	header.swapBytes();

	uint headerSize = 128;
	if (header.hasDX10Header())
	{
		nvStaticCheck(sizeof(DDSHeader) == 128 + 20);
		headerSize = 128 + 20;
	}

	
	memcpy( buffer, &header, headerSize);
	return headerSize;
}
