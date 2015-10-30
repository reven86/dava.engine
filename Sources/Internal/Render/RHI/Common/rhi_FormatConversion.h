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

#ifndef __RHI_FORMAT_CONVERT_H__
#define __RHI_FORMAT_CONVERT_H__

namespace rhi
{
inline void
_FlipRGBA4_ABGR4(void* data, uint32 size)
{
    // flip RGBA-ABGR order
    for (uint8 *d = (uint8 *)data, *d_end = (uint8 *)data + size; d != d_end; d += 2)
    {
        uint8 t0 = d[0];
        uint8 t1 = d[1];

        t0 = ((t0 & 0x0F) << 4) | ((t0 & 0xF0) >> 4);
        t1 = ((t1 & 0x0F) << 4) | ((t1 & 0xF0) >> 4);

        d[0] = t1;
        d[1] = t0;
    }
}

//------------------------------------------------------------------------------

inline void
_ABGR1555toRGBA5551(void* data, uint32 size)
{
    for (uint16 *d = (uint16 *)data, *d_end = (uint16 *)data + size / sizeof(uint16); d != d_end; ++d)
    {
        const uint16 in = *d;
        uint16 r = (in & 0xF800) >> 11;
        uint16 g = (in & 0x07C0) >> 1;
        uint16 b = (in & 0x003E) << 9;
        uint16 a = (in & 0x0001) << 15;

        *d = r | g | b | a;
    }
}

//------------------------------------------------------------------------------

inline void
_RGBA5551toABGR1555(void* data, uint32 size)
{
    for (uint16 *d = (uint16 *)data, *d_end = (uint16 *)data + size / sizeof(uint16); d != d_end; ++d)
    {
        const uint16 in = *d;
        uint16 r = (in & 0x001F) << 11;
        uint16 g = (in & 0x03E0) << 1;
        uint16 b = (in & 0x7C00) >> 9;
        uint16 a = (in & 0x8000) >> 15;

        *d = r | g | b | a;
    }
}

//------------------------------------------------------------------------------

inline void
_SwapRB8(void* data, uint32 size)
{
    for (uint8 *d = (uint8 *)data, *d_end = (uint8 *)data + size; d != d_end; d += 4)
    {
        uint8 t = d[0];

        d[0] = d[2];
        d[2] = t;
    }
}

//------------------------------------------------------------------------------

inline void
_SwapRB4(void* data, uint32 size)
{
    for (uint8 *d = (uint8 *)data, *d_end = (uint8 *)data + size; d != d_end; d += 2)
    {
        uint8 t0 = d[0];
        uint8 t1 = d[1];

        d[0] = (t0 & 0xF0) | (t1 & 0x0F);
        d[1] = (t1 & 0xF0) | (t0 & 0x0F);
    }
}

//------------------------------------------------------------------------------

inline void
_SwapRB5551(void* data, uint32 size)
{
    for (uint8 *d = (uint8 *)data, *d_end = (uint8 *)data + size; d != d_end; d += 2)
    {
        uint8 t0 = d[0];
        uint8 t1 = d[1];

        d[0] = ((t1 & 0x7C) >> 2) | (t0 & 0xE0);
        d[1] = ((t0 & 0x1F) << 2) | (t1 & 0x83);
    }
}

//------------------------------------------------------------------------------

#endif //__RHI_FORMAT_CONVERT_H__

} //namespace rhi
