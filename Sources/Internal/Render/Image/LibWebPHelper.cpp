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


#include "Render/Image/LibWebPHelper.h"

#include "Render/Image/Image.h"
#include "Render/Image/ImageConvert.h"

#include "Render/Texture.h"
#include "Render/RenderManager.h"

#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"

#include <stdlib.h>
#include <stdio.h>

#include "libjpeg/jpeglib.h"
#include "webp/decode.h"
#include "webp/encode.h"
#include "webp/types.h"
#include <setjmp.h>

#define QUALITY 100 //0..100
#include <io.h>

namespace DAVA
{
    // Output types
    typedef enum
    {
        PNG = 0,
        PAM,
        PPM,
        PGM,
        BMP,
        TIFF,
        YUV,
        ALPHA_PLANE_ONLY  // this is for experimenting only
    } OutputFileFormat;

// 
// struct jpegErrorManager
// {
//     // "public" fields 
//     struct jpeg_error_mgr pub;
// 
//     // for return to caller 
//     jmp_buf setjmp_buffer;
// };
// 
// char jpegLastErrorMsg[JMSG_LENGTH_MAX];
// 
// void jpegErrorExit (j_common_ptr cinfo)
// {
//     // cinfo->err actually points to a jpegErrorManager struct
//     jpegErrorManager* myerr = (jpegErrorManager*) cinfo->err;
//     // note : *(cinfo->err) is now equivalent to myerr->pub
// 
//     // output_message is a method to print an error message
//     //(* (cinfo->err->output_message) ) (cinfo);
// 
//     // Create the message
//     ( *(cinfo->err->format_message) ) (cinfo, jpegLastErrorMsg);
// 
//     // Jump to the setjmp point
//     longjmp(myerr->setjmp_buffer, 1);
// }

LibWebPHelper::LibWebPHelper()
{
    supportedExtensions.push_back(".webp");
}

bool LibWebPHelper::IsMyImage(File *infile) const
{
    return GetImageInfo(infile).dataSize != 0;
}

eErrorCode LibWebPHelper::ReadFile(File *infile, Vector<Image *> &imageSet, int32 baseMipMap) const
{
    return SUCCESS;
}

eErrorCode LibWebPHelper::WriteFile(const FilePath & fileName, const Vector<Image *> &imageSet, PixelFormat compressionFormat) const
{
    return SUCCESS;
}

eErrorCode LibWebPHelper::WriteFileAsCubeMap(const FilePath &fileName, const Vector<Vector<Image *> > &imageSet, PixelFormat compressionFormat) const
{
    Logger::Error("[LibJpegHelper::WriteFileAsCubeMap] For jpeg cubeMaps are not supported");
    return ERROR_WRITE_FAIL;
}



// FILE* ExUtilSetBinaryMode(FILE* file)
// {
// #if defined(_WIN32)
//     if (_setmode(_fileno(file), _O_BINARY) == -1)
//     {
//         fprintf(stderr, "Failed to reopen file in O_BINARY mode.\n");
//         return NULL;
//     }
// #endif
//     return file;
// }
// 
// int ExUtilReadFromStdin(const uint8_t** data, size_t* data_size)
// {
//     static const size_t kBlockSize = 16384;  // default initial size
//     size_t max_size = 0;
//     size_t size = 0;
//     uint8_t* input = NULL;
// 
//     if (data == NULL || data_size == NULL) return 0;
//     *data = NULL;
//     *data_size = 0;
// 
//     //if (!ExUtilSetBinaryMode(stdin)) return 0;
// 
//     while (!feof(stdin))
//     {
//         // We double the buffer size each time and read as much as possible.
//         const size_t extra_size = (max_size == 0) ? kBlockSize : max_size;
//         void* const new_data = realloc(input, max_size + extra_size);
//         if (new_data == NULL) goto Error;
//         input = (uint8_t*)new_data;
//         max_size += extra_size;
//         size += fread(input + size, 1, extra_size, stdin);
//         if (size < max_size) break;
//     }
//     if (ferror(stdin)) goto Error;
//     *data = input;
//     *data_size = size;
//     return 1;
// 
// Error:
//     free(input);
//     fprintf(stderr, "Could not read from stdin\n");
//     return 0;
// }
// 
// int ExUtilReadFile(const char* const file_name,
//                    const uint8_t** data, size_t* data_size)
// {
//     int ok;
//     void* file_data;
//     size_t file_size;
//     FILE* in;
//     const int from_stdin = (file_name == NULL) || !strcmp(file_name, "-");
// 
//     if (from_stdin) return ExUtilReadFromStdin(data, data_size);
// 
//     if (data == NULL || data_size == NULL) return 0;
//     *data = NULL;
//     *data_size = 0;
// 
//     in = fopen(file_name, "rb");
//     if (in == NULL)
//     {
//         fprintf(stderr, "cannot open input file '%s'\n", file_name);
//         return 0;
//     }
//     fseek(in, 0, SEEK_END);
//     file_size = ftell(in);
//     fseek(in, 0, SEEK_SET);
//     file_data = malloc(file_size);
//     if (file_data == NULL) return 0;
//     ok = (fread(file_data, file_size, 1, in) == 1);
//     fclose(in);
// 
//     if (!ok)
//     {
//         fprintf(stderr, "Could not read %d bytes of data from file %s\n",
//                 (int)file_size, file_name);
//         free(file_data);
//         return 0;
//     }
//     *data = (uint8_t*)file_data;
//     *data_size = file_size;
//     return 1;
// }
// 
// static const char* const kStatusMessages[VP8_STATUS_NOT_ENOUGH_DATA + 1] = {
//     "OK", "OUT_OF_MEMORY", "INVALID_PARAM", "BITSTREAM_ERROR",
//     "UNSUPPORTED_FEATURE", "SUSPENDED", "USER_ABORT", "NOT_ENOUGH_DATA"
// };
// 
// void ExUtilPrintWebPError(const char* const in_file, int status)
// {
//     fprintf(stderr, "Decoding of %s failed.\n", in_file);
//     fprintf(stderr, "Status: %d", status);
//     if (status >= VP8_STATUS_OK && status <= VP8_STATUS_NOT_ENOUGH_DATA)
//     {
//         fprintf(stderr, "(%s)", kStatusMessages[status]);
//     }
//     fprintf(stderr, "\n");
// }
// 
// int ExUtilLoadWebP(const char* const in_file,
//                    const uint8_t** data, size_t* data_size,
//                    WebPBitstreamFeatures* bitstream)
// {
//     VP8StatusCode status;
//     WebPBitstreamFeatures local_features;
//     if (!ExUtilReadFile(in_file, data, data_size)) return 0;
// 
//     if (bitstream == NULL)
//     {
//         bitstream = &local_features;
//     }
// 
//     status = WebPGetFeatures(*data, *data_size, bitstream);
//     if (status != VP8_STATUS_OK)
//     {
//         free((void*)*data);
//         *data = NULL;
//         *data_size = 0;
//         ExUtilPrintWebPError(in_file, status);
//         return 0;
//     }
//     return 1;
// }

DAVA::ImageInfo LibWebPHelper::GetImageInfo(File *infile) const
{
    VP8StatusCode status = VP8_STATUS_OK;
    
    WebPDecoderConfig config;
    WebPDecBuffer* const output_buffer = &config.output;
    WebPBitstreamFeatures* const bitstream = &config.input;
    OutputFileFormat format = PNG;
    config.options.dithering_strength = 0;
    config.options.use_threads = 1;
    int incremental = 0;

    size_t data_size = infile->GetSize();
    uint8_t *data = new uint8_t[data_size];
    uint8 *d = new uint8[data_size];
    infile->Read(data, data_size);
    infile->Read(d, data_size);

    int *width = new int;
    int *height = new int;
    
    int output = WebPGetInfo(data, data_size, width, height);

//     if (!ExUtilLoadWebP(infile->GetFilename(), &data, &data_size, bitstream))
//     {
//         return -1;
//     }

//     const uint8_t *data = nullptr;
//     size_t *data_size = new size_t;
//     int *width = new int;
//     int *height = new int;
//     int WebPGetInfo(data, data_size, width, height);

    return ImageInfo();
}

};
