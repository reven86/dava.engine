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
struct ImageCell
{
    Rect2i rect;
    Rect2i imageRect;
    uint32 leftEdgePixel = 0;
    uint32 rightEdgePixel = 0;
    uint32 topEdgePixel = 0;
    uint32 bottomEdgePixel = 0;
    uint32 rightMargin = 0;
    uint32 bottomMargin = 0;
};

//! helper class to simplify packing of many small 2D images to one big 2D image
class TextureAtlas
{
public:
    TextureAtlas(const Rect2i& _rect, bool _useTwoSideMargin, int32 _texturesMargin);

    bool AddImage(const Size2i& imageSize, void* searchPtr);
    ImageCell* GetImageCell(void* searchPtr);

    Rect2i& GetRect()
    {
        return rootNode->cell.rect;
    };

private:
    struct AtlasNode;
    using AtlasNodePtr = std::unique_ptr<AtlasNode>;

    struct AtlasNode
    {
        AtlasNodePtr child[2];
        ImageCell cell;
        void* imagePtr = nullptr;
    };

    AtlasNode* Insert(const AtlasNodePtr& node, const Size2i& imageSize, void* imagePtr);
    AtlasNode* SearchRectForPtr(const AtlasNodePtr& node, void* imagePtr);

    const int32 edgePixel;
    const int32 texturesMargin;
    const int32 splitter;
    AtlasNodePtr rootNode;
};

using TextureAtlasPtr = std::unique_ptr<TextureAtlas>;
}

#endif // __DAVAENGINE_IMAGEPACKER_H__
