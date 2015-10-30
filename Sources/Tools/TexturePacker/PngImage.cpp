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


#include "TexturePacker/PngImage.h"
#include "CommandLine/CommandLineParser.h"
#include "Render/Image/LibPngHelper.h"
#include "Render/Image/ImageSystem.h"
#include "Render/Image/ImageConvert.h"
#include "Render/Texture.h"
#include "Render/PixelFormatDescriptor.h"

namespace DAVA
{

PngImageExt::PngImageExt()
    : internalData(nullptr)
{
}

PngImageExt::~PngImageExt()
{
}

bool PngImageExt::Read(const FilePath &filename)
{
    internalData.reset();

    ScopedPtr<File> fileRead(File::Create(filename, File::READ | File::OPEN));
    if (!fileRead)
    {
        Logger::Error("[PngImageExt::Read] failed to open png file: %s", filename.GetAbsolutePathname().c_str());
        return false;
    }

    internalData = new Image();
    eErrorCode innerRetCode = LibPngHelper::ReadPngFile(fileRead, internalData, FORMAT_RGBA8888);
    if (innerRetCode != eErrorCode::SUCCESS)
    {
        internalData.reset();
        Logger::Error("[PngImageExt::Read] failed to read png file: %s", filename.GetAbsolutePathname().c_str());
    }

    return internalData.get() != nullptr;
}

void PngImageExt::Write(const FilePath &filename, ImageQuality quality)
{
    DVASSERT(internalData);
    ImageSystem::Instance()->Save(filename, internalData, internalData->format, quality);
}

bool PngImageExt::Create(uint32 width, uint32 height)
{
    internalData = Image::Create(width, height, FORMAT_RGBA8888);
    if (internalData)
    {
        memset( GetData(), 0, width * height * PixelFormatDescriptor::GetPixelFormatSizeInBytes( FORMAT_RGBA8888 ) );
        return true;
    }

	return false;
}

bool PngImageExt::ConvertToFormat(PixelFormat newFormat)
{
    DVASSERT(internalData);
    if (internalData->format == newFormat)
    {
        return true;
    }

    ScopedPtr<Image> newImage(Image::Create(GetWidth(), GetHeight(), newFormat));
    bool convertResult = ImageConvert::ConvertImageDirect(internalData, newImage);

    if (convertResult == true)
    {
        internalData = newImage;
    }

    return convertResult;
}

void PngImageExt::DrawImage(int32 sx, int32 sy, PngImageExt * image, const Rect2i & srcRect)
{
    uint32 * destData32 = (uint32*)GetData();
	uint32 * srcData32 = (uint32*)image->GetData();

	int32 rx, ry;
	ry = sy;
	for (int32 y = srcRect.y; y < srcRect.y + srcRect.dy; ++y)
	{
		rx = sx;
		for (int32 x = srcRect.x; x < srcRect.x + srcRect.dx; ++x)
		{
            if (rx < 0 ||
                rx >= static_cast<int32>(GetWidth()) ||
                ry < 0 ||
                ry >= static_cast<int32>(GetHeight()) ||
                x < 0 ||
                x >= static_cast<int32>(image->GetWidth()) ||
                y < 0 ||
                y >= static_cast<int32>(image->GetHeight()))
            {
                continue;
            }

            destData32[(rx)+(ry)* GetWidth()] = srcData32[x + y * image->GetWidth()];
			rx++;
		}
		ry++;
	}
}

void PngImageExt::DrawImage(const ImageCell& packedCell, const Rect2i& alphaOffsetRect, PngImageExt* image)
{
    uint32* destData32 = (uint32*)GetData();
    uint32* srcData32 = (uint32*)image->GetData();
    const Rect2i& img = packedCell.imageRect;

    bool withAlpha = CommandLineParser::Instance()->IsFlagSet("--disableCropAlpha");

    int32 sx = img.x;
    int32 sy = img.y;

    if (withAlpha)
	{
		sx += alphaOffsetRect.x;
		sy += alphaOffsetRect.y;
	}

	// add image
    int32 srcPos = 0;
    int32 destPos = sx + sy * GetWidth();
    int32 destPosInc = GetWidth() - img.dx;
    for (int32 y = 0; y < image->GetHeight(); ++y, destPos += destPosInc)
    {
        for (int32 x = 0; x < image->GetWidth(); ++x, ++srcPos, ++destPos)
        {
            if ((sx + x) < 0)
                continue;
            if ((sx + x) >= static_cast<int32>(GetWidth()))
                continue;
            if ((sy + y) < 0)
                continue;
            if ((sy + y) >= static_cast<int32>(GetHeight()))
                continue;

            destData32[destPos] = srcData32[srcPos];
        }
    }

    uint32 x0 = img.x;
    uint32 xn = x0 + img.dx - 1;
    uint32 y0 = img.y;
    uint32 yn = y0 + img.dy - 1;
    uint32 yStride = GetWidth();

    if (packedCell.leftEdgePixel)
    {
        DVASSERT(x0 > 0);
        uint32 leftBorderPix = (x0 - 1) + (y0)*yStride;
        uint32 leftBorderLastPix = (x0 - 1) + (yn)*yStride;
        for (; leftBorderPix <= leftBorderLastPix; leftBorderPix += yStride)
        {
            destData32[leftBorderPix] = destData32[leftBorderPix + 1];
        }
    }

    if (packedCell.rightEdgePixel)
    {
        uint32 rightBorderPix = (xn + 1) + (y0)*yStride;
        uint32 rightBorderLastPix = (xn + 1) + (yn)*yStride;
        for (; rightBorderPix <= rightBorderLastPix; rightBorderPix += yStride)
        {
            destData32[rightBorderPix] = destData32[rightBorderPix - 1];
        }
    }

    if (packedCell.topEdgePixel)
    {
        DVASSERT(y0 > 0);
        uint32 topBorderPix = (x0) + (y0 - 1) * yStride;
        uint32 topImagePix = (x0) + (y0)*yStride;
        uint32 topBorderLastPix = (xn) + (y0 - 1) * yStride;
        for (; topBorderPix <= topBorderLastPix; ++topBorderPix, ++topImagePix)
        {
            destData32[topBorderPix] = destData32[topImagePix];
        }
    }

    if (packedCell.bottomEdgePixel)
    {
        uint32 bottomBorderPix = (x0) + (yn + 1) * yStride;
        uint32 bottomImagePix = (x0) + (yn)*yStride;
        uint32 bottomBorderLastPix = (xn) + (yn + 1) * yStride;
        for (; bottomBorderPix <= bottomBorderLastPix; ++bottomBorderPix, ++bottomImagePix)
        {
            destData32[bottomBorderPix] = destData32[bottomImagePix];
        }
    }

    if (packedCell.leftEdgePixel && packedCell.topEdgePixel)
        destData32[(x0 - 1) + (y0 - 1) * yStride] = destData32[(x0) + (y0)*yStride];

    if (packedCell.leftEdgePixel && packedCell.bottomEdgePixel)
        destData32[(x0 - 1) + (yn + 1) * yStride] = destData32[(x0) + (yn)*yStride];

    if (packedCell.rightEdgePixel && packedCell.topEdgePixel)
        destData32[(xn + 1) + (y0 - 1) * yStride] = destData32[(xn) + (y0)*yStride];

    if (packedCell.rightEdgePixel && packedCell.bottomEdgePixel)
        destData32[(xn + 1) + (yn + 1) * yStride] = destData32[(xn) + (yn)*yStride];
}

bool PngImageExt::IsHorzLineOpaque(int32 y)
{
	uint8 * line = GetData() + y * GetWidth() * 4;
	for (uint32 x = 0; x < GetWidth(); ++x)
    {
		if (line[x * 4 + 3] != 0)
        {
			return false;
        }
    }
	return true;
}

bool PngImageExt::IsVertLineOpaque(int32 x)
{
	uint8 * vertLine = GetData() + x * 4;
	for (uint32 i = 0; i < GetHeight(); ++i)
	{
		if (vertLine[3] != 0)
        {
			return false;
        }

		vertLine += GetWidth() * 4;
	}
	return true;
}

void PngImageExt::FindNonOpaqueRect(Rect2i &rect)
{
	rect = Rect2i(0, 0, GetWidth(), GetHeight());
	for (uint32 y = 0; y < GetHeight(); ++y)
    {
		if (IsHorzLineOpaque(y))
		{
			rect.y++;
			rect.dy--;
        }
        else
        {
            break;
        }
    }

	for (uint32 x = 0; x < GetWidth(); ++x)
    {
		if (IsVertLineOpaque(x))
		{
			rect.x++;
			rect.dx--;
        }
        else break;
    }

	if ((rect.dx == 0) && (rect.dy == 0))
	{
        rect.x = rect.y = 0;
		rect.dx = rect.dy = 1;
		return;
	}

	for (int32 y = GetHeight() - 1; y >= 0; --y)
    {
        if (IsHorzLineOpaque(y))
        {
            rect.dy--;
        }
        else
        {
            break;
        }
    }

	for (int32 x = GetWidth() - 1; x >= 0; --x)
    {
        if (IsVertLineOpaque(x))
        {
            rect.dx--;
        }
        else
        {
            break;
        }
    }
}

void PngImageExt::DrawRect(const Rect2i &rect, uint32 color)
{
    uint32 *destData32 = (uint32*)GetData();

	for (int32 i = 0; i < rect.dx; ++i)
	{
		destData32[rect.y * GetWidth() + rect.x + i] = color;
		destData32[(rect.y + rect.dy - 1) * GetWidth() + rect.x + i] = color;
	}
	for (int32 i = 0; i < rect.dy; ++i)
	{
		destData32[(rect.y + i) * GetWidth() + rect.x] = color;
		destData32[(rect.y + i) * GetWidth() + rect.x + rect.dx - 1] = color;
	}
}

void PngImageExt::DitherAlpha()
{
    DVASSERT(internalData);

    if (internalData->format == FORMAT_RGBA8888)
    {
        ScopedPtr<Image> image(Image::Create(GetWidth(), GetHeight(), FORMAT_RGBA8888));

        uint8 *ditheredPtr = image->GetData();
        uint8 *dataPtr = GetData();

        for (uint32 y = 0; y < GetHeight(); ++y)
        {
            for (uint32 x = 0; x < GetWidth(); ++x)
            {
                if (dataPtr[3])
                {
                    Memcpy(ditheredPtr, dataPtr, 4);
                }
                else
                {
                    Color color = GetDitheredColorForPoint(x, y);

                    ditheredPtr[0] = (uint8)color.r;
                    ditheredPtr[1] = (uint8)color.g;
                    ditheredPtr[2] = (uint8)color.b;
                    ditheredPtr[3] = 0;
                }

                ditheredPtr += 4;
                dataPtr += 4;
            }
        }

        internalData = image;
    }
}

Color PngImageExt::GetDitheredColorForPoint(int32 x, int32 y)
{
    int32 count = 0;
    Color newColor(0, 0, 0, 0);

    int32 startY = Max(y - 1, 0);
    int32 endY = Min(y + 1, (int32)GetHeight());
    int32 startX = Max(x - 1, 0);
    int32 endX = Min(x + 1, (int32)GetWidth());

    for (int32 alphaY = startY; alphaY < endY; ++alphaY)
    {
        for (int32 alphaX = startX; alphaX < endX; ++alphaX)
        {
            int32 offset = (y * GetWidth() + x) * 4;
            if (GetData()[offset + 3])
            {
                ++count;
                newColor.r += (float32)(GetData()[offset]);
                newColor.g += (float32)(GetData()[offset + 1]);
                newColor.b += (float32)(GetData()[offset + 2]);
            }
        }
    }

    if (count)
    {
        newColor /= (float32)count;
    }

    return newColor;
}

};

