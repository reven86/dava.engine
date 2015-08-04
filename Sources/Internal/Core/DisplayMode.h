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


#ifndef __DAVAENGINE_DISPLAYMODE_H__
#define __DAVAENGINE_DISPLAYMODE_H__


namespace DAVA
{
/**
	\ingroup core
	\brief Class that describe display mode supported by device
*/
class DisplayMode
{
public:
    static const int32 DEFAULT_WIDTH = 800;
    static const int32 DEFAULT_HEIGHT = 600;
    static const int32 DEFAULT_BITS_PER_PIXEL = 16;
    static const int32 DEFAULT_DISPLAYFREQUENCY = 0;

public:
    DisplayMode() = default;

    DisplayMode(int32 width_, int32 height_, int32 bpp_, int32 refreshRate_)
        : width(width_)
        , height(height_)
        , bpp(bpp_)
        , refreshRate(refreshRate_)
    {}

    bool IsValid()
    {
        return (width > 0 && height > 0 && refreshRate != -1);
    }

    int32 width = 0;        //! width of the display mode
    int32 height = 0;       //! height of the display mode
    int32 bpp = 0;          //! bits per pixel 
    int32 refreshRate = -1; //! refresh rate of the display mode, 0 if default
};

};

#endif // __DAVAENGINE_DISPLAYMODE_H__
