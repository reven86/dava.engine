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

#ifndef __DAVAENGINE_IMAGEPACKER_H__
#define __DAVAENGINE_IMAGEPACKER_H__

#include "Base/BaseTypes.h"
#include "Math/Math2D.h"

namespace DAVA
{
enum class PackingAlgorithm
{
    ALG_BASIC,
    ALG_MAXRECTS_BOTTOM_LEFT,
    ALG_MAXRECTS_BEST_AREA_FIT,
    ALG_MAXRECTS_BEST_SHORT_SIDE_FIT,
    ALG_MAXRECTS_BEST_LONG_SIDE_FIT,
    ALG_MAXRRECT_BEST_CONTACT_POINT
};

struct SpriteBoundsRect
{
    Rect2i marginsRect;
    Rect2i spriteRect;
    uint32 leftEdgePixel = 0;
    uint32 rightEdgePixel = 0;
    uint32 topEdgePixel = 0;
    uint32 bottomEdgePixel = 0;
    uint32 rightMargin = 0;
    uint32 bottomMargin = 0;
};

struct SpritesheetLayout
{
    virtual ~SpritesheetLayout() = default;
    virtual bool AddSprite(const Size2i& spriteSize, const void* searchPtr) = 0;
    virtual const SpriteBoundsRect* GetSpriteBoundsRect(const void* searchPtr) const = 0;
    virtual const Rect2i& GetRect() const = 0;
    virtual uint32 GetWeight() const = 0;

    static std::unique_ptr<SpritesheetLayout> Create(uint32 w, uint32 h, bool duplicateEdgePixel, uint32 spritesMargin, PackingAlgorithm alg);
};
}

#endif // __DAVAENGINE_IMAGEPACKER_H__
