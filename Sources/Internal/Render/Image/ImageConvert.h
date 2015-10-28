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


#include "Image.h"
#include "Render/Texture.h"
#include "Render/PixelFormatDescriptor.h"

namespace DAVA
{

struct RGB888
{
    uint8 r;
    uint8 g;
    uint8 b;
};

struct BGR888
{
    uint8 b;
    uint8 g;
    uint8 r;
};

struct BGRA8888
{
    uint8 b;
    uint8 g;
    uint8 r;
    uint8 a;
};

struct RGBA16161616
{
    uint16 r;
    uint16 g;
    uint16 b;
    uint16 a;
};

struct RGBA32323232
{
    uint32 r;
    uint32 g;
    uint32 b;
    uint32 a;
};

struct ConvertBGRA8888toRGBA8888
{
    // input and output is the same memory
    inline void operator()(const BGRA8888* input, uint32* output)
    {
        static_assert(sizeof(BGRA8888) == sizeof(uint32), "remove alignment");
        //             {input->b, input->g, input->r, input->a}
        //..............r...g...b
        BGRA8888 tmp = {input->r, input->g, input->b, input->a};
        *output = *reinterpret_cast<uint32*>(&tmp);
    }
};

struct ConvertRGBA8888toRGB888
{
    inline void operator()(const uint32 * input, RGB888 *output)
    {
		uint32 pixel = *input;
        output->b = ((pixel >> 16) & 0xFF);
        output->g = ((pixel >> 8) & 0xFF);
        output->r = (pixel & 0xFF);
    }
};
    
struct ConvertRGB888toRGBA8888
{
    inline void operator()(const RGB888 *input, uint32 * output)
    {
        *output = ((0xFF) << 24) | (input->b << 16) | (input->g << 8) | input->r;
    }
};

struct ConvertBGR888toRGBA8888
{
    inline void operator()(const BGR888 *input, uint32 * output)
    {
        *output = ((0xFF) << 24) | (input->b << 16) | (input->g << 8) | input->r;
    }
};

struct ConvertRGBA16161616toRGBA8888
{
    inline void operator()(const RGBA16161616 * input, uint32 *output)
    {
        uint8 r = (input->r >> 8) & 0xFF;
        uint8 g = (input->g >> 8) & 0xFF;
        uint8 b = (input->b >> 8) & 0xFF;
        uint8 a = (input->a >> 8) & 0xFF;
        *output = (a << 24) | (b << 16) | (g << 8) | r;
    }
};

struct ConvertRGBA32323232toRGBA8888
{
    inline void operator()(const RGBA32323232 * input, uint32 *output)
    {
        uint8 r = (input->r >> 24) & 0xFF;
        uint8 g = (input->g >> 24) & 0xFF;
        uint8 b = (input->b >> 24) & 0xFF;
        uint8 a = (input->a >> 24) & 0xFF;
        *output = (a << 24) | (b << 16) | (g << 8) | r;
    }
};

struct ConvertBGR888toRGB888
{
    inline void operator()(const BGR888 *input, RGB888* output)
    {
        BGR888 tmp = *input;
        output->r = tmp.r;
        output->g = tmp.g;
        output->b = tmp.b;
    }
};
    
struct ConvertA16toA8
{
    inline void operator()(const uint16 * input, uint8 *output)
    {
        uint16 pixel = *input;
        *output = (uint8)pixel;
    }
};

struct ConvertA8toA16
{
    inline void operator()(const uint8 * input, uint16 *output)
    {
        uint8 pixel = *input;
        *output = pixel;
    }
};
    
struct ConvertRGBA8888toRGBA4444
{
	inline void operator()(const uint32 * input, uint16 *output)
	{
		uint32 pixel = *input;
		uint32 a = ((pixel >> 24) & 0xFF) >> 4;
		uint32 b = ((pixel >> 16) & 0xFF) >> 4;
		uint32 g = ((pixel >> 8) & 0xFF) >> 4;
		uint32 r = (pixel & 0xFF) >> 4;
		*output = ((r) << 12) | (g << 8) | (b << 4) | a;
	}
};

struct ConvertRGBA5551toRGBA8888
{
	inline void operator()(const uint16 * input, uint32 *output)
	{
		uint16 pixel = *input;

        uint32 a = ((pixel >> 15) & 0x01) ? 0x00FF : 0;
        uint32 b = (((pixel >> 10) & 0x01F) << 3);
        uint32 g = (((pixel >> 5) & 0x01F) << 3);
        uint32 r = (((pixel >> 0) & 0x01F) << 3);

        *output = (r) | (g << 8) | (b << 16) | (a << 24);
    }
};

struct ConvertRGBA4444toRGBA8888
{
	inline void operator()(const uint16 * input, uint32 *output)
	{
		uint16 pixel = *input;
        uint32 a = (((pixel >> 12) & 0x0F) << 4);
        uint32 b = (((pixel >> 8) & 0x0F) << 4);
        uint32 g = (((pixel >> 4) & 0x0F) << 4);
        uint32 r = (((pixel >> 0) & 0x0F) << 4);

        *output = (r) | (g << 8) | (b << 16) | (a << 24);
	}
    
};

struct ConvertRGB565toRGBA8888
{
	inline void operator()(const uint16 * input, uint32 *output)
	{
		uint16 pixel = *input;
		uint32 r = (((pixel >> 11) & 0x01F) << 3);
		uint32 g = (((pixel >> 5) & 0x03F) << 2);
		uint32 b = (((pixel >> 0) & 0x01F) << 3);
		uint32 a = 0xFF;

 		*output = (r) | (g << 8) | (b << 16) | (a << 24);
	}
};

struct ConvertA8toRGBA8888
{
	inline void operator()(const uint8 * input, uint32 *output)
	{
		uint32 pixel = *input;
		*output = (0xFF << 24) | (pixel << 16) | (pixel << 8) | pixel;
	}
};

struct ConvertA16toRGBA8888
{
    inline void operator()(const uint16 * input, uint32 *output)
    {
        uint32 pixel = *input;
        *output = (0xFF << 24) | (pixel << 16) | (pixel << 8) | pixel;
    }
};

struct ConvertBGRA4444toRGBA4444
{
    inline void operator()(const uint16 * input, uint16 *output)
    {
        //bbbb gggg rrrr aaaa --> rrrr gggg bbbb aaaa
        const uint16 in = *input;
        uint16 greenAlpha = in & 0x0F0F;
        uint16 blue = (in >> 8) & 0x00F0;
        uint16  red = (in & 0x00F0) << 8;
        *output = red | greenAlpha | blue;
    }
};

struct ConvertABGR4444toRGBA4444
{
    inline void operator()(const uint16* input, uint16* output)
    {
        const uint8* in = (const uint8*)input;
        uint8* out = (uint8*)output;

        //aaaa bbbb gggg rrrr --> rrrr gggg bbbb aaaa
        uint8 ab = in[0];
        uint8 gr = in[1];

        out[0] = ((gr & 0x0f) << 4) | ((gr & 0xf0) >> 4); //rg
        out[1] = ((ab & 0x0f) << 4) | ((ab & 0xf0) >> 4); //ba
    }
};

struct ConvertBGRA5551toRGBA5551
{
    inline void operator()(const uint16* input, uint16* output)
    {
        //bbbb bggg ggrr rrra --> rrrr rggg ggbb bbba
        const uint16 in = *input;
        uint16 r = (in & 0x7c00) >> 5;
        uint16 b = (in & 0x001f) << 5;
        uint16 ga = in & 0x83e0;

        *output = r | b | ga;
    }
};

struct ConvertABGR1555toRGBA5551
{
    inline void operator()(const uint16* input, uint16* output)
    {
        //abbb bbgg gggr rrrr --> rrrr rggg ggbb bbba
        const uint16 in = *input;
        uint16 r = (in & 0xf800) >> 11;
        uint16 g = (in & 0x07c0) >> 1;
        uint16 b = (in & 0x003e) << 9;
        uint16 a = (in & 0x0001) << 15;

        *output = r | g | b | a;
    }
};

struct ConvertBGR565toRGB565
{
    inline void operator()(const uint16* input, uint16* output)
    {
        // bbbb bggg gggr rrrr --> rrrr rggg gggb bbbb
        uint16 in = *input;
        uint16 blue = (in >> 11) & 0x1F;
        uint16  red = (in & 0x1F) << 11;
        uint16 green = in & 0x07E0;
        *output = red | green | blue;
    }
};

struct ConvertBGRA16161616toRGBA16161616
{
    inline void operator()(const RGBA16161616* input, RGBA16161616* output)
    {
        RGBA16161616 out = *input;
        uint16 tmp = out.r;
        out.r = out.b;
        out.b = tmp;
        *output = out;
    }
};

struct ConvertBGRA32323232toRGBA32323232
{
    inline void operator()(const RGBA32323232* input, RGBA32323232* output)
    {
        RGBA32323232 out = *input;
        uint32 tmp = out.r;
        out.r = out.b;
        out.b = tmp;
        *output = out;
    }
};

struct UnpackRGBA8888
{
	inline void operator()(const uint32 * input, uint32 & r, uint32 & g, uint32 & b, uint32 & a)
	{
		uint32 pixel = *input;
		a = ((pixel >> 24) & 0xFF);
		r = ((pixel >> 16) & 0xFF);
		g = ((pixel >> 8) & 0xFF);
		b = (pixel & 0xFF);
	}
};

struct PackRGBA8888
{
	inline void operator()(uint32 r, uint32 g, uint32 b, uint32 a, uint32 * output)
	{
		*output = ((a) << 24) | (r << 16) | (g << 8) | b;
	}
};

struct UnpackRGBA4444
{
	inline void operator()(const uint16 * input, uint32 & r, uint32 & g, uint32 & b, uint32 & a)
	{
		uint16 pixel = *input;
		a = ((pixel >> 12) & 0xF);
		r = ((pixel >> 8) & 0xF);
		g = ((pixel >> 4) & 0xF);
		b = (pixel & 0xF);
	}
};

struct PackRGBA4444
{
	inline void operator()(uint32 r, uint32 g, uint32 b, uint32 a, uint16 * output)
	{
		*output = ((b >> 4) << 12) | ((g >> 4) << 8) | ((r >> 4) << 4) | (a >> 4);
	}
};

struct UnpackA8
{
    inline void operator()(const uint8 * input, uint32 & r, uint32 & g, uint32 & b, uint32 & a)
    {
        r = g = b = 0;
        a = (*input);
    }
};

struct PackA8
{
    inline void operator()(uint32 r, uint32 g, uint32 b, uint32 a, uint8 * output)
    {
        *output = a;
    }
};

    
struct UnpackRGB888
{
    inline void operator()(const RGB888 * input, uint32 & r, uint32 & g, uint32 & b, uint32 & a)
    {
        r = input->r;
        g = input->g;
        b = input->b;
        a = 0xFF;
    }
};

struct PackRGB888
{
    inline void operator()(uint32 r, uint32 g, uint32 b, uint32 a, RGB888 * output)
    {
        output->r = r;
        output->g = g;
        output->b = b;
    }
};

struct UnpackRGBA5551
{
    inline void operator()(const uint16 * input, uint32 & r, uint32 & g, uint32 & b, uint32 & a)
    {
        auto & in = *input;
        
        r = (in >> 11) & 0x001F;
        g = (in >> 6) & 0x001F;
        b = (in >> 1) & 0x001F;
        a = (in) & 0x0001;
    }
};

struct PackRGBA5551
{
    inline void operator()(uint32 r, uint32 g, uint32 b, uint32 a, uint16 * output)
    {
        *output = (r << 11) | (g << 6) | (b << 1) | a;
    }
};
    
    
    
struct PackNormalizedRGBA8888
{
    void operator()(uint32 r, uint32 g, uint32 b, uint32 a, uint32 * output)
    {
        Vector3 v(r / 255.f, g / 255.f, b / 255.f);
        v *= 2.f;
        v -= Vector3(1.f, 1.f, 1.f);
        v.Normalize();
        v /= 2.f;
        v += Vector3(.5f, .5f, .5f);

        PackRGBA8888 packFunc;
        packFunc((uint32)(0xFF * v.x), (uint32)(0xFF * v.y), (uint32)(0xFF * v.z), a, output);
    }
};

struct NormalizeRGBA8888
{
    inline void operator()(const uint32 * input, uint32 *output)
    {
        UnpackRGBA8888 unpack;
        PackNormalizedRGBA8888 pack;
        uint32 r, g, b, a;

        unpack(input, r, g, b, a);
        pack(r, g, b, a, output);
    }
};

    
template<class TYPE_IN, class TYPE_OUT, typename CONVERT_FUNC>
class ConvertDirect
{
public:
    void operator()(const void * inData, uint32 width, uint32 height, uint32 pitch, void * outData)
    {
        CONVERT_FUNC func;
        const uint8 * readPtr = reinterpret_cast<const uint8*>(inData);
        uint8 * writePtr = reinterpret_cast<uint8*>(outData);

        for (uint32 y = 0; y < height; ++y)
        {
            const TYPE_IN * readPtrLine = reinterpret_cast<const TYPE_IN*>(readPtr);
            TYPE_OUT * writePtrLine = reinterpret_cast<TYPE_OUT*>(writePtr);
            for (uint32 x = 0; x < width; ++x)
            {
                func(readPtrLine, writePtrLine);
                readPtrLine++;
                writePtrLine++;
            }
            readPtr += pitch;
            writePtr += pitch;
        }
    };

    void operator()(void* data, uint32 width, uint32 height, uint32 pitch)
    {
        CONVERT_FUNC func;
        const uint8 * ptrToLine = reinterpret_cast<const uint8*>(data);

        for (uint32 y = 0; y < height; ++y)
        {
            const TYPE_IN * ptr = reinterpret_cast<const TYPE_IN*>(ptrToLine);
            for (uint32 x = 0; x < width; ++x)
            {
                func(ptr, ptr);
                ptr++;
            }
            ptrToLine += pitch;
        }
    };

    void operator()(const void * inData, uint32 inWidth, uint32 inHeight, uint32 inPitch, 
                    void * outData, uint32 outWidth, uint32 outHeight, uint32 outPitch)
    {
        CONVERT_FUNC func;
        const uint8 * readPtr = reinterpret_cast<const uint8*>(inData);
        uint8 * writePtr = reinterpret_cast<uint8*>(outData);

        for (uint32 y = 0; y < inHeight; ++y)
        {
            const TYPE_IN * readPtrLine = reinterpret_cast<const TYPE_IN*>(readPtr);
            TYPE_OUT * writePtrLine = reinterpret_cast<TYPE_OUT*>(writePtr);
            for (uint32 x = 0; x < inWidth; ++x)
            {
                func(readPtrLine, writePtrLine);
                readPtrLine++;
                writePtrLine++;
            }
            readPtr += inPitch;
            writePtr += outPitch;
        }
    };
};

    
template<class TYPE_IN, class TYPE_OUT, typename UNPACK_FUNC, typename PACK_FUNC>
class ConvertDownscaleTwiceBillinear
{
public:
	void operator()(const void * inData, uint32 inWidth, uint32 inHeight, uint32 inPitch,
		void * outData, uint32 outWidth, uint32 outHeight, uint32 outPitch)
	{
		UNPACK_FUNC unpackFunc;
		PACK_FUNC packFunc;
		const uint8 * readPtr = reinterpret_cast<const uint8*>(inData);
		uint8 * writePtr = reinterpret_cast<uint8*>(outData);

		for (uint32 y = 0; y < outHeight; ++y)
		{
			const TYPE_IN * readPtrLine = reinterpret_cast<const TYPE_IN*>(readPtr);
			TYPE_OUT * writePtrLine = reinterpret_cast<TYPE_OUT*>(writePtr);
			
			for (uint32 x = 0; x < outWidth; ++x)
			{
				uint32 r00, r01, r10, r11;
				uint32 g00, g01, g10, g11;
				uint32 b00, b01, b10, b11;
				uint32 a00, a01, a10, a11;

				unpackFunc(readPtrLine, r00, g00, b00, a00);
				unpackFunc(readPtrLine + 1, r01, g01, b01, a01);
				unpackFunc(readPtrLine + inWidth, r10, g10, b10, a10);
				unpackFunc(readPtrLine + inWidth + 1, r11, g11, b11, a11);
		
				uint32 r = (r00 + r01 + r10 + r11) >> 2;
				uint32 g = (g00 + g01 + g10 + g11) >> 2;
				uint32 b = (b00 + b01 + b10 + b11) >> 2;
				uint32 a = (a00 + a01 + a10 + a11) >> 2;
				
				packFunc(r, g, b, a, writePtrLine);

				readPtrLine += 2;
				writePtrLine++;
			}
			readPtr += inPitch * 2; 
			writePtr += outPitch;
		}
	};
};


class ImageConvert
{
public:

    static bool Normalize(PixelFormat format, const void * inData, uint32 width, uint32 height, uint32 pitch, void * outData)
    {
        if (format == FORMAT_RGBA8888)
        {
            ConvertDirect<uint32, uint32, NormalizeRGBA8888> convert;
            convert(inData, width, height, pitch, outData, width, height, pitch);
            
            return true;
        }
        
        Logger::Error("Normalize function not implemented for %s", PixelFormatDescriptor::GetPixelFormatString(format));
        return false;
    }

    static bool CanConvertFromTo(PixelFormat inFormat, PixelFormat outFormat)
    {
        return ConvertImageDirect(inFormat, outFormat, nullptr, 0, 0, 0, nullptr, 0, 0, 0);
    }

    static bool ConvertImageDirect(const Image *srcImage, Image *dstImage)
    {
        return ConvertImageDirect(srcImage->format, dstImage->format, 
                                  srcImage->data, srcImage->width, srcImage->height, 
                                  srcImage->width * PixelFormatDescriptor::GetPixelFormatSizeInBytes(srcImage->format),
                                  dstImage->data, dstImage->width, dstImage->height, 
                                  dstImage->width * PixelFormatDescriptor::GetPixelFormatSizeInBytes(dstImage->format));
    }

    static bool ConvertImageDirect(PixelFormat inFormat, PixelFormat outFormat, 
                                   const void * inData, uint32 inWidth, uint32 inHeight, uint32 inPitch,
                                   void * outData, uint32 outWidth, uint32 outHeight, uint32 outPitch)
    {
        if (inFormat == FORMAT_RGBA5551 && outFormat == FORMAT_RGBA8888)
        {
            ConvertDirect<uint16, uint32, ConvertRGBA5551toRGBA8888> convert;
            convert(inData, inWidth, inHeight, inPitch, outData, outWidth, outHeight, outPitch);
            return true;
        }
        else if (inFormat == FORMAT_RGBA4444 && outFormat == FORMAT_RGBA8888)
        {
            ConvertDirect<uint16, uint32, ConvertRGBA4444toRGBA8888> convert;
            convert(inData, inWidth, inHeight, inPitch, outData, outWidth, outHeight, outPitch);
            return true;
        }
        else if (inFormat == FORMAT_RGB888 && outFormat == FORMAT_RGBA8888)
        {
            ConvertDirect<RGB888, uint32, ConvertRGB888toRGBA8888> convert;
            convert(inData, inWidth, inHeight, inPitch, outData, outWidth, outHeight, outPitch);
            return true;
        }
        else if (inFormat == FORMAT_RGB565 && outFormat == FORMAT_RGBA8888)
        {
            ConvertDirect<uint16, uint32, ConvertRGB565toRGBA8888> convert;
            convert(inData, inWidth, inHeight, inPitch, outData, outWidth, outHeight, outPitch);
            return true;
        }
        else if (inFormat == FORMAT_A8 && outFormat == FORMAT_RGBA8888)
        {
            ConvertDirect<uint8, uint32, ConvertA8toRGBA8888> convert;
            convert(inData, inWidth, inHeight, inPitch, outData, outWidth, outHeight, outPitch);
            return true;
        }
        else if (inFormat == FORMAT_A16 && outFormat == FORMAT_RGBA8888)
        {
            ConvertDirect<uint16, uint32, ConvertA16toRGBA8888> convert;
            convert(inData, inWidth, inHeight, inPitch, outData, outWidth, outHeight, outPitch);
            return true;
        }
        else if (inFormat == FORMAT_BGR888 && outFormat == FORMAT_RGB888)
        {
            ConvertDirect<BGR888, RGB888, ConvertBGR888toRGB888> convert;
            convert(inData, inWidth, inHeight, inPitch, outData, outWidth, outHeight, outPitch);
            return true;
        }
        else if (inFormat == FORMAT_BGR888 && outFormat == FORMAT_RGBA8888)
        {
            ConvertDirect<BGR888, uint32, ConvertBGR888toRGBA8888> convert;
            convert(inData, inWidth, inHeight, inPitch, outData, outWidth, outHeight, outPitch);
            return true;
        }
        else if (inFormat == FORMAT_BGRA8888 && outFormat == FORMAT_RGBA8888)
        {
            ConvertDirect<BGRA8888, uint32, ConvertBGRA8888toRGBA8888> convert;
            convert(inData, inWidth, inHeight, inPitch, outData, outWidth, outHeight, outPitch);
            return true;
        }
        else if (inFormat == FORMAT_RGBA8888 && outFormat == FORMAT_RGB888)
        {
            ConvertDirect<uint32, RGB888, ConvertRGBA8888toRGB888> convert;
            convert(inData, inWidth, inHeight, inPitch, outData, outWidth, outHeight, outPitch);
            return true;
        }
        else if (inFormat == FORMAT_RGBA16161616 && outFormat == FORMAT_RGBA8888)
        {
            ConvertDirect<RGBA16161616, uint32, ConvertRGBA16161616toRGBA8888> convert;
            convert(inData, inWidth, inHeight, inPitch, outData, outWidth, outHeight, outPitch);
            return true;
        }
        else if (inFormat == FORMAT_RGBA32323232 && outFormat == FORMAT_RGBA8888)
        {
            ConvertDirect<RGBA32323232, uint32, ConvertRGBA32323232toRGBA8888> convert;
            convert(inData, inWidth, inHeight, inPitch, outData, outWidth, outHeight, outPitch);
            return true;
        }
        else
        {
            Logger::FrameworkDebug("Unsupported image conversion from format %d to %d", inFormat, outFormat);
            return false;
        }
    }

    static void SwapRedBlueChannels(const Image *srcImage)
    {
        DVASSERT(srcImage);
        SwapRedBlueChannels(srcImage->format, srcImage->data, srcImage->width, srcImage->height, srcImage->width * PixelFormatDescriptor::GetPixelFormatSizeInBytes(srcImage->format));
    }
    
    static void SwapRedBlueChannels(PixelFormat format, void* srcData, uint32 width, uint32 height, uint32 pitch, void* dstData = nullptr)
    {
        if (!dstData) 
            dstData = srcData;

        switch (format)
        {
        case FORMAT_RGB888:
        {
            ConvertDirect<BGR888, RGB888, ConvertBGR888toRGB888> swap;
            swap(srcData, width, height, pitch, dstData);
            return;
        }
        case FORMAT_RGBA8888:
        {
            ConvertDirect<BGRA8888, uint32, ConvertBGRA8888toRGBA8888> swap;
            swap(srcData, width, height, pitch, dstData);
            return;
        }
        case FORMAT_RGBA4444:
        {
            ConvertDirect<uint16, uint16, ConvertBGRA4444toRGBA4444> swap;
            swap(srcData, width, height, pitch, dstData);
            return;
        }
        case FORMAT_RGB565:
        {
            ConvertDirect<uint16, uint16, ConvertBGR565toRGB565> swap;
            swap(srcData, width, height, pitch, dstData);
            return;
        }
        case FORMAT_RGBA16161616:
        {
            ConvertDirect<RGBA16161616, RGBA16161616, ConvertBGRA16161616toRGBA16161616> swap;
            swap(srcData, width, height, pitch, dstData);
            return;
        }
        case FORMAT_RGBA32323232:
        {
            ConvertDirect<RGBA32323232, RGBA32323232, ConvertBGRA32323232toRGBA32323232> swap;
            swap(srcData, width, height, pitch, dstData);
            return;
        }
        case FORMAT_A8: case FORMAT_A16:
        {
            // do nothing for grayscale images
            return;
        }
        default:
        {
            Logger::Error("Image color exchanging is not supported for format %d", format);
            return;
        }
        }
    }

	static void DownscaleTwiceBillinear(	PixelFormat inFormat,
												PixelFormat outFormat,
												const void * inData, uint32 inWidth, uint32 inHeight, uint32 inPitch,
												void * outData, uint32 outWidth, uint32 outHeight, uint32 outPitch, bool normalize)
	{
		if ((inFormat == FORMAT_RGBA8888) && (outFormat == FORMAT_RGBA8888))
		{
            if(normalize)
            {
			    ConvertDownscaleTwiceBillinear<uint32, uint32, UnpackRGBA8888, PackNormalizedRGBA8888> convert;
			    convert(inData, inWidth, inHeight, inPitch, outData, outWidth, outHeight, outPitch);
            }
            else
            {
                ConvertDownscaleTwiceBillinear<uint32, uint32, UnpackRGBA8888, PackRGBA8888> convert;
                convert(inData, inWidth, inHeight, inPitch, outData, outWidth, outHeight, outPitch);
            }
		}
        else if ((inFormat == FORMAT_RGBA8888) && (outFormat == FORMAT_RGBA4444))
		{
			ConvertDownscaleTwiceBillinear<uint32, uint16, UnpackRGBA8888, PackRGBA4444> convert;
			convert(inData, inWidth, inHeight, inPitch, outData, outWidth, outHeight, outPitch);
		}
        else if ((inFormat == FORMAT_RGBA4444) && (outFormat == FORMAT_RGBA8888))
		{
			ConvertDownscaleTwiceBillinear<uint16, uint32, UnpackRGBA4444, PackRGBA8888> convert;
			convert(inData, inWidth, inHeight, inPitch, outData, outWidth, outHeight, outPitch);
        }
        else if ((inFormat == FORMAT_A8) && (outFormat == FORMAT_A8))
        {
            ConvertDownscaleTwiceBillinear<uint8, uint8, UnpackA8, PackA8> convert;
            convert(inData, inWidth, inHeight, inPitch, outData, outWidth, outHeight, outPitch);
        }
        else if ((inFormat == FORMAT_RGB888) && (outFormat == FORMAT_RGB888))
        {
            ConvertDownscaleTwiceBillinear<RGB888, RGB888, UnpackRGB888, PackRGB888> convert;
            convert(inData, inWidth, inHeight, inPitch, outData, outWidth, outHeight, outPitch);
        }
        else if ((inFormat == FORMAT_RGBA5551) && (outFormat == FORMAT_RGBA5551))
        {
            ConvertDownscaleTwiceBillinear<uint16, uint16, UnpackRGBA5551, PackRGBA5551> convert;
            convert(inData, inWidth, inHeight, inPitch, outData, outWidth, outHeight, outPitch);
        }
        else
		{
            Logger::Debug("Convert function not implemented for %s or %s", PixelFormatDescriptor::GetPixelFormatString(inFormat), PixelFormatDescriptor::GetPixelFormatString(outFormat));
		}
	}

	Image* DownscaleTwiceBillinear(const Image * source)
	{
		if (source->GetPixelFormat() == FORMAT_RGBA8888)
		{
			Image * destination = Image::Create(source->GetWidth() / 2, source->GetHeight() / 2, source->GetPixelFormat());
			if (destination)
			{
				ConvertDownscaleTwiceBillinear<uint32, uint32, UnpackRGBA8888, PackRGBA8888> convertFunc;
				convertFunc(source->GetData(), source->GetWidth(), source->GetHeight(), source->GetWidth() * PixelFormatDescriptor::GetPixelFormatSizeInBytes(source->GetPixelFormat()),
					destination->GetData(), destination->GetWidth(), destination->GetHeight(), destination->GetWidth() * PixelFormatDescriptor::GetPixelFormatSizeInBytes(destination->GetPixelFormat()));
			}
			return destination;
		}
		return 0;
	}

    static void ResizeRGBA8Billinear(const uint32* inPixels, uint32 w, uint32 h, uint32* outPixels, uint32 w2, uint32 h2)
    {
        int32 a, b, c, d, x, y, index;
        float32 x_ratio = ((float32)(w - 1)) / w2;
        float32 y_ratio = ((float32)(h - 1)) / h2;
        float32 x_diff, y_diff, blue, red, green, alpha;
        uint32 offset = 0;
        for (uint32 i = 0; i < h2; i++)
        {
            for (uint32 j = 0; j < w2; j++)
            {
                x = (int32)(x_ratio * j);
                y = (int32)(y_ratio * i);
                x_diff = (x_ratio * j) - x;
                y_diff = (y_ratio * i) - y;
                index = (y * w + x);
                a = inPixels[index];
                b = inPixels[index + 1];
                c = inPixels[index + w];
                d = inPixels[index + w + 1];

                blue = (a & 0xff) * (1 - x_diff) * (1 - y_diff) + (b & 0xff) * (x_diff) * (1 - y_diff) +
                (c & 0xff) * (y_diff) * (1 - x_diff) + (d & 0xff) * (x_diff * y_diff);

                green = ((a >> 8) & 0xff) * (1 - x_diff) * (1 - y_diff) + ((b >> 8) & 0xff) * (x_diff) * (1 - y_diff) +
                ((c >> 8) & 0xff) * (y_diff) * (1 - x_diff) + ((d >> 8) & 0xff) * (x_diff * y_diff);

                red = ((a >> 16) & 0xff) * (1 - x_diff) * (1 - y_diff) + ((b >> 16) & 0xff) * (x_diff) * (1 - y_diff) +
                ((c >> 16) & 0xff) * (y_diff) * (1 - x_diff) + ((d >> 16) & 0xff) * (x_diff * y_diff);

                alpha = ((a >> 24) & 0xff) * (1 - x_diff) * (1 - y_diff) + ((b >> 24) & 0xff) * (x_diff) * (1 - y_diff) +
                ((c >> 24) & 0xff) * (y_diff) * (1 - x_diff) + ((d >> 24) & 0xff) * (x_diff * y_diff);

                outPixels[offset++] =
                ((((uint32)alpha) << 24) & 0xff000000) |
                ((((uint32)red) << 16) & 0xff0000) |
                ((((uint32)green) << 8) & 0xff00) |
                ((uint32)blue);
            }
        }
    }
};

};
