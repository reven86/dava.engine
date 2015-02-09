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

#include "ColorChannelsExchanger.h"

namespace DAVA {

void RGBA5551_Exchanger::SwapRedAndBlue(uint8* pixel)
{
    // rrrr rggg ggbb bbba
//     uint8 red = pixel[0] & 0xF8;    // copy red bits
//     red >>= 2;                      // shift at blue bits position
//     uint8 blue = pixel[1] & 0x3E;   // copy blue bits
//     blue <<= 2;                     // shift at red bits position
//     //*((uint16*)pixel) &= 0x07C1;    // leave green and attr bits only
//     pixel[0] &= 0x07;
//     pixel[1] &= 0xC1;
//     pixel[1] |= red;                // set red bits
//     pixel[0] |= blue;               // set blue bits
    pixel[0] |= 0x01;

    // arrr rrgg gggb bbbb
//     uint8 red = (pixel[0] & 0x7C) >> 2;
//     uint8 blue = (pixel[1] & 0x1F) << 2;
//     pixel[0] &= 0x83; 
//     pixel[1] &= 0xE0;
//     pixel[0] |= blue;
//     pixel[1] |= red;
//     uint8 tmp = pixel[0]; pixel[0] = pixel[1]; pixel[1] = tmp;
}

void RGBA4444_Exchanger::SwapRedAndBlue(uint8* pixel)
{
    // rrrr gggg bbbb aaaa
    uint8 red = pixel[0] & 0xF0;    // copy red bits
    uint8 blue = pixel[1] & 0xF0;   // copy blue bits
    *((uint16*)pixel) &= 0x0F0F;    // leave green and attr bits only
    pixel[1] |= red;                // set red bits
    pixel[0] |= blue;               // set blue bits
}

void RGB888_Exchanger::SwapRedAndBlue(uint8* pixel)
{
    pixel[0] ^= pixel[2] ^= pixel[0] ^= pixel[2]; // XOR exchange trick
}

void RGB565_Exchanger::SwapRedAndBlue(uint8* pixel)
{
    // rrrr rggg gggb bbbb
    
    uint8 red = pixel[0] & 0xF8;    // copy red bits
    red >>= 3;                      // shift at blue bits position
    uint8 blue = pixel[1] & 0x1F;   // copy blue bits
    blue <<= 3;                     // shift at red bits position
    //*((uint16*)pixel) &= 0x07E0;    // leave green bits only
    pixel[0] &= 0x07;
    pixel[1] &= 0xE0;
    pixel[1] |= red;                // set red bits
    pixel[0] |= blue;               // set blue bits
}

void RGBA16161616_Exchanger::SwapRedAndBlue(uint8* pixel)
{
    uint16* channel = (uint16*)pixel;
    pixel[0] ^= pixel[2] ^= pixel[0] ^= pixel[2];
}

void RGBA32323232_Exchanger::SwapRedAndBlue(uint8* pixel)
{
    uint32* channel = (uint32*)pixel;
    pixel[0] ^= pixel[2] ^= pixel[0] ^= pixel[2];
}

ColorChannelsExchanger* CreateChannelsExchanger(PixelFormat format)
{
    switch (format)
    {
    case FORMAT_RGB888:
    case FORMAT_RGBA8888:
        return new RGB888_Exchanger;
    case FORMAT_RGBA5551:
        return new RGBA5551_Exchanger;
    case FORMAT_RGBA4444:
        return new RGBA4444_Exchanger;
    case FORMAT_RGB565:
        return new RGB565_Exchanger;
    case FORMAT_RGBA16161616:
        return new RGBA16161616_Exchanger;
    case FORMAT_RGBA32323232:
        return new RGBA32323232_Exchanger;
    default:
        return new Dummy_Exchanger;
    }
}
}