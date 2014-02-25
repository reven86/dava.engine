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


#include "Render/LibPngHelpers.h"

#if !defined(__DAVAENGINE_WIN32__)
#include <unistd.h>
#endif //#if !defined(__DAVAENGINE_WIN32__)


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <iostream>

#include <libpng/png.h>
#include "Render/RenderManager.h"
#include "Render/2D/Sprite.h"
#include "Render/Texture.h"
#include "Render/Image.h"
#include "FileSystem/FileSystem.h"

using namespace DAVA;


#define PNG_DEBUG 3

void abort_(const char * s, ...)
{
	va_list args;
	va_start(args, s);
	vfprintf(stderr, s, args);
	fprintf(stderr, "\n");
	va_end(args);
	abort();
}

//int x, y;

void convert_rawpp_to_bytestream(int32 width, int32 height, png_bytepp row_pointers, uint8 * data)
{
	for (int y = 0; y < height; y++) 
	{
		png_byte* row = row_pointers[y];
		memcpy(data, row, width * 4);
		data += width * 4;
	}
}

void convert_bytestream_to_rawpp(int32 width, int32 height, uint8 * data, png_bytepp row_pointers)
{
	for (int y = 0; y < height; y++)
	{
		png_byte* row = row_pointers[y];
		memcpy(row, data, width * 4);
		data += width * 4;
	}
}

struct	PngImageRawData
{
	File * file;
};

static void	PngImageRead(png_structp pngPtr, png_bytep data, png_size_t size)
{
	PngImageRawData * self = (PngImageRawData*)png_get_io_ptr(pngPtr);
	self->file->Read(data, (uint32)size);
}

int LibPngWrapper::ReadPngFile(const FilePath & file, Image * image, PixelFormat targetFormat/* = FORMAT_INVALID*/)
{
	File * infile = File::Create(file, File::OPEN | File::READ);
	if (!infile)
	{
		return 0;
	}

    int retCode = ReadPngFile(infile, image, targetFormat);
    SafeRelease(infile);
    
    return retCode;
}

int LibPngWrapper::ReadPngFile(File *infile, Image * image, PixelFormat targetFormat/* = FORMAT_INVALID*/)
{
    DVASSERT(targetFormat == FORMAT_INVALID || targetFormat == FORMAT_RGBA8888);
    
	png_structp png_ptr;
	png_infop info_ptr;
	
	uint8 *image_data = NULL;
	char sig[8];
	
	int bit_depth;
	int color_type;
	
	png_uint_32 width;
	png_uint_32 height;
	unsigned int rowbytes;
	
	int i;
	png_bytepp row_pointers = NULL;
	
    
	infile->Read(sig, 8);
	
	if (!png_check_sig((unsigned char *) sig, 8))
	{
		return 0;
	}
    
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
	{
		return 4;    /* out of memory */
	}
	
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);
		return 4;    /* out of memory */
	}
	
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return 0;
	}
	
	PngImageRawData	raw;
	raw.file = infile;
	png_set_read_fn (png_ptr, &raw, PngImageRead);
	
	png_set_sig_bytes(png_ptr, 8);
	
	png_read_info(png_ptr, info_ptr);
	
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth,
				 &color_type, NULL, NULL, NULL);
	
	image->width = width;
	image->height = height;
    
	//1 bit images -> 8 bit
	if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
		png_set_expand_gray_1_2_4_to_8(png_ptr);
    
	image->format = FORMAT_RGBA8888;
	if (color_type == PNG_COLOR_TYPE_GRAY ||
		color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    {
        if(targetFormat == FORMAT_RGBA8888)
        {
            png_set_gray_to_rgb(png_ptr);
            png_set_filler(png_ptr, 0xFFFF, PNG_FILLER_AFTER);
        }
        else
        {
            image->format = (bit_depth == 16) ? FORMAT_A16 : FORMAT_A8;
            if(color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
            {
                png_set_strip_alpha(png_ptr);
            }
        }
	}
	else if(color_type == PNG_COLOR_TYPE_PALETTE)
	{
		png_set_palette_to_rgb(png_ptr);
		png_set_filler(png_ptr, 0xFFFF, PNG_FILLER_AFTER);
	}
	else if(color_type == PNG_COLOR_TYPE_RGB)
	{
		png_set_filler(png_ptr, 0xFFFF, PNG_FILLER_AFTER);
	}
    
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
	{
		png_set_tRNS_to_alpha(png_ptr);
	}
    
	png_read_update_info(png_ptr, info_ptr);
	
	rowbytes = png_get_rowbytes(png_ptr, info_ptr);
	
	image_data = new uint8 [rowbytes * height];
	if (image_data == 0)
	{
		memset(image_data, 0, rowbytes * height);
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return 4;
    }
	
	if ((row_pointers = (png_bytepp)malloc(height*sizeof(png_bytep))) == NULL)
	{
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        delete [] (image_data);
        image_data = NULL;
        return 4;
    }
	
	
    /* set the individual row_pointers to point at the correct offsets */
	
    for (i = 0;  i < (int)height;  ++i)
        row_pointers[i] = image_data + i*rowbytes;
	
	
    /* now we can go ahead and just read the whole image */
    png_read_image(png_ptr, row_pointers);
	
    /* and we're done!  (png_read_end() can be omitted if no processing of
     * post-IDAT text/time/etc. is desired) */
	
	/* Clean up. */
	free(row_pointers);
	
	/* Clean up. */
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	
	image->data = image_data;
	
	return 1;
}

bool LibPngWrapper::IsPngFile(File *file)
{
    char sig[8];
    file->Read(sig, 8);
	
    return (0 != png_check_sig((unsigned char *) sig, 8));
}

uint32 LibPngWrapper::GetDataSize(const FilePath &filePathname)
{
    File * infile = File::Create(filePathname, File::OPEN | File::READ);
	if (!infile)
	{
		return 0;
	}
    
	char sig[8];
	infile->Read(sig, 8);
	if (!png_check_sig((unsigned char *) sig, 8))
	{
        SafeRelease(infile);
		return 0;
	}

    
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
	{
        SafeRelease(infile);
		return 0;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);
        
        SafeRelease(infile);
		return 0;
	}
	
    if (setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        
        SafeRelease(infile);
		return 0;
	}

    PngImageRawData	raw;
	raw.file = infile;
	png_set_read_fn (png_ptr, &raw, PngImageRead);


    png_set_sig_bytes(png_ptr, 8);
	png_read_info(png_ptr, info_ptr);


    int bit_depth;
	int color_type;
	
	png_uint_32 width;
	png_uint_32 height;
	
	
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);

	PixelFormat format = FORMAT_RGBA8888;
	if (color_type == PNG_COLOR_TYPE_GRAY ||
		color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    {
        format = (bit_depth == 16) ? FORMAT_A16 : FORMAT_A8;
	}

    uint32 imageSize = width * height * Texture::GetPixelFormatSizeInBytes(format);
    
    
	/* Clean up. */
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	
    SafeRelease(infile);

	return imageSize;
}



void LibPngWrapper::WritePngFile(const FilePath & file_name, int32 width, int32 height, uint8 * data, PixelFormat format)
{
//	printf("* Writing PNG file (%d x %d): %s\n", width, height, file_name);
	png_color_8 sig_bit;
	
	png_structp png_ptr;
	png_infop info_ptr;
    
    
	png_byte color_type = PNG_COLOR_TYPE_RGBA;
	png_byte bit_depth = 8;
    
    int32 bytes_for_color = 4;
    if(FORMAT_A8 == format)
    {
        color_type = PNG_COLOR_TYPE_GRAY;
        bytes_for_color = 1;
    }
    else if(FORMAT_A16 == format)
    {
        bit_depth = 16;
        color_type = PNG_COLOR_TYPE_GRAY;
        bytes_for_color = 2;
    }
    
	png_bytep * row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
    
    
	for (int y = 0; y < height; y++)
	{
//		row_pointers[y] = (png_byte*) &data[y * width * 4];
		row_pointers[y] = (png_byte*) &data[y * width * bytes_for_color];
	}
	
	
	/* create file */
	FILE *fp = fopen(file_name.GetAbsolutePathname().c_str(), "wb");
	if (!fp)
	{
		Logger::Error("[LibPngWrapper::WritePngFile] File %s could not be opened for writing", file_name.GetAbsolutePathname().c_str());
		//abort_("[write_png_file] File %s could not be opened for writing", file_name);
        free(row_pointers);
		return;
	}
	
	
	/* initialize stuff */
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	
	if (!png_ptr)
		abort_("[write_png_file] png_create_write_struct failed");
	
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		Logger::Error("[LibPngWrapper::WritePngFile] png_create_info_struct failed");

		//abort_("[write_png_file] png_create_info_struct failed");
        free(row_pointers);
        fclose(fp);
		return;
	}
	
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		Logger::Error("[LibPngWrapper::WritePngFile] Error during init_io");
		//abort_("[write_png_file] Error during init_io");
        free(row_pointers);
        fclose(fp);
		return;
	}
	
	png_init_io(png_ptr, fp);
	
	
	/* write header */
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		Logger::Error("[LibPngWrapper::WritePngFile] Error during writing header");
		//abort_("[write_png_file] Error during writing header");
        free(row_pointers);
        fclose(fp);
		return;
	}
    
	
	png_set_IHDR(png_ptr, info_ptr, width, height,
				 bit_depth, color_type, PNG_INTERLACE_NONE,
				 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	
	
    if(FORMAT_A8 == format)
    {
        sig_bit.red = 0;
        sig_bit.green = 0;
        sig_bit.blue = 0;
        
        sig_bit.gray = 8;
        sig_bit.alpha = 0;
    }
    else if(FORMAT_A16 == format)
    {
        sig_bit.red = 0;
        sig_bit.green = 0;
        sig_bit.blue = 0;
        
        sig_bit.gray = 16;
        sig_bit.alpha = 0;
    }
    else 
    {
        sig_bit.red = 8;
        sig_bit.green = 8;
        sig_bit.blue = 8;
        sig_bit.alpha = 8;
    }

	png_set_sBIT(png_ptr, info_ptr, &sig_bit);

    
	png_write_info(png_ptr, info_ptr);
	png_set_shift(png_ptr, &sig_bit);
	png_set_packing(png_ptr);
	
	/* write bytes */
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		Logger::Error("[LibPngWrapper::WritePngFile] Error during writing bytes");
		//abort_("[write_png_file] Error during writing bytes");
        free(row_pointers);
        fclose(fp);
		return;
	}
	
	png_write_image(png_ptr, row_pointers);
	
	
	/* end write */
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		Logger::Error("[LibPngWrapper::WritePngFile] Error during end of write");
		//abort_("[write_png_file] Error during end of write");
        free(row_pointers);
        fclose(fp);
		return;
	}
	
	png_write_end(png_ptr, NULL);
	
	free(row_pointers);
	fclose(fp);
}

PngImage::PngImage()
:	width(0)
,	height(0)
,	data(0)
,   format(FORMAT_INVALID)
{
		
}

PngImage::~PngImage()
{
	SafeDeleteArray(data);
	width = 0;
	height = 0;
    format = FORMAT_INVALID;
}

bool PngImage::Load(const FilePath & filename)
{
	//LibPngWrapper::ReadPngFile(filename, &width, &height, &data);
	return true;
}

bool PngImage::Save(const FilePath & filename)
{
	LibPngWrapper::WritePngFile(filename, width, height, data, format);
	return true;
}

bool PngImage::Create(int _width, int _height)
{
	width = _width;
	height = _height;
	data = new uint8[width * 4 * height];
	memset(data, 0, width * 4 * height);
	if (!data)return false;
	return true; 
}

void PngImage::DrawImage(int sx, int sy, PngImage * image)
{
	// printf("0x%08x 0x%08x %d %d\n", data, image->data, sx, sy);
	
	uint32 * destData32 = (uint32*)data;
	uint32 * srcData32 = (uint32*)image->data;
	
	for (int y = 0; y < image->height; ++y)
		for (int x = 0; x < image->width; ++x)
		{
			destData32[(sx + x) + (sy + y) * width] = srcData32[x + y * image->width];
			//printf("%04x ", srcData32[x + y * image->width]);
		}
}

void PngImage::DrawRect(const Rect2i & rect, uint32 color)
{
	uint32 * destData32 = (uint32*)data;
	
	for (int i = 0; i < rect.dx; ++i)
	{
		destData32[rect.y * width + rect.x + i] = color; 
		destData32[(rect.y + rect.dy - 1) * width + rect.x + i] = color; 
	}
	for (int i = 0; i < rect.dy; ++i)
	{
		destData32[(rect.y + i) * width + rect.x] = color; 
		destData32[(rect.y + i) * width + rect.x + rect.dx - 1] = color; 
	}
}

bool PngImage::CreateFromFBOSprite(Sprite * fboSprite)
{
	if (!fboSprite)return false;
	
	width = fboSprite->GetTexture()->GetWidth();
	height = fboSprite->GetTexture()->GetHeight();
	data = new uint8[width * 4 * height];

	Texture * texture = fboSprite->GetTexture();

	format = texture->GetFormat();    
	if (format == FORMAT_RGBA8888)
	{
		RenderManager::Instance()->SetRenderTarget(fboSprite);
		glReadPixels(0, 0, texture->width, texture->height, GL_RGBA, GL_UNSIGNED_BYTE, data);
		RenderManager::Instance()->RestoreRenderTarget();
	}
	return true;
}

/*void process_file(void)
{
	if (info_ptr->color_type != PNG_COLOR_TYPE_RGBA)
		abort_("[process_file] color_type of input file must be PNG_COLOR_TYPE_RGBA (is %d)", info_ptr->color_type);
	
	for (y=0; y<height; y++) {
		png_byte* row = row_pointers[y];
		for (x=0; x<width; x++) {
			png_byte* ptr = &(row[x*4]);
			printf("Pixel at position [ %d - %d ] has the following RGBA values: %d - %d - %d - %d\n",
			       x, y, ptr[0], ptr[1], ptr[2], ptr[3]);
			
			
			ptr[0] = 0;
			ptr[1] = ptr[2];
		}
	}
	
}*/
