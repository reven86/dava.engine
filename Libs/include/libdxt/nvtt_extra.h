// Copyright NVIDIA Corporation 2007 -- Ignacio Castano <icastano@nvidia.com>
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

#ifndef NV_TT_EXTR_H
#define NV_TT_EXTR_H

#include "nvtt.h"

// Public interface.
namespace nvtt
{
#define DECOMPRESSOR_MIN_HEADER_SIZE 148
		
	/// Texture decompressor.
	struct Decompressor
	{
		NVTT_DECLARE_PIMPL(Decompressor);

		NVTT_API Decompressor();
		NVTT_API ~Decompressor();
		
		NVTT_API bool initWithDDSFile(const char * pathToDDSFile);
		
		NVTT_API bool initWithDDSFile(FILE *file);

		NVTT_API bool initWithDDSFile(const unsigned char * mem, unsigned int size);
		
		NVTT_API void erase();

		NVTT_API bool process(void * data, unsigned int size, unsigned int mipmapNumber, unsigned int face = 0) const;

		NVTT_API bool getInfo(unsigned int & mipmapCount,
							  unsigned int & width,
							  unsigned int & heigth,
							  unsigned int & size,
							  unsigned int & headerSize,
							  unsigned int & faceCount,
							  unsigned int & faceFlags) const;
		
		NVTT_API bool getCompressionFormat(Format & comprFormat) const;
		
		NVTT_API bool getRawData(void* buffer, unsigned int size) const;

		NVTT_API bool getMipmapSize(unsigned int number, unsigned int & size) const;

		static NVTT_API unsigned int getHeader(void* buffer, unsigned int & bufferSize, const InputOptions & inputOptions, const CompressionOptions & compressionOptions);
	};
	

} // nvtt namespace

#endif // NV_TT_EXTR_H
