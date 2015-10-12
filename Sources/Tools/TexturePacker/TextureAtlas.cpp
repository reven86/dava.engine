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

#include "TexturePacker/TextureAtlas.h"
#include "TexturePacker/TexturePacker.h"
#include "Base/BaseTypes.h"

namespace DAVA
{
TextureAtlas::PackNode* TextureAtlas::PackNode::Insert(const Size2i& imageSize)
{
    if (!isLeaf)
    {
        TextureAtlas::PackNode* imageNode = child[0]->Insert(imageSize);
        if (imageNode)
            return imageNode;
        else
            return child[1]->Insert(imageSize);
    }
    else
    {
        if (isImageSet)
            return nullptr;

        if (atlas.useTwoSideMargin)
        {
            int32 dw = packCell.rect.dx - packCell.leftMargin - packCell.rightMargin - imageSize.dx;
            int32 dh = packCell.rect.dy - packCell.topMargin - packCell.bottomMargin - imageSize.dy;

            if (dw < 0 || dh < 0)
                return nullptr;

            if (dw == 0 && dh == 0)
            {
                isImageSet = true;
                return this;
            }

            isLeaf = false;

            child[0] = new TextureAtlas::PackNode(atlas);
            child[1] = new TextureAtlas::PackNode(atlas);

            child[0]->packCell = child[1]->packCell = packCell;

            if (dw > dh) // horizontal split
            {
                child[0]->packCell.rightMargin = 1;
                child[1]->packCell.leftMargin = 1;
                auto dxLeft = imageSize.dx + child[0]->packCell.rightMargin + child[0]->packCell.leftMargin;
                child[0]->packCell.rect = Rect2i(packCell.rect.x, packCell.rect.y, dxLeft, packCell.rect.dy);
                child[1]->packCell.rect = Rect2i(packCell.rect.x + dxLeft, packCell.rect.y, packCell.rect.dx - dxLeft, packCell.rect.dy);
            }
            else // vertical split
            {
                child[0]->packCell.bottomMargin = 1;
                child[1]->packCell.topMargin = 1;
                auto dyTop = imageSize.dy + child[0]->packCell.topMargin + child[0]->packCell.bottomMargin;
                child[0]->packCell.rect = Rect2i(packCell.rect.x, packCell.rect.y, packCell.rect.dx, dyTop);
                child[1]->packCell.rect = Rect2i(packCell.rect.x, packCell.rect.y + dyTop, packCell.rect.dx, packCell.rect.dy - dyTop);
            }
        }
        else
        {
            int32 dw = packCell.rect.dx - imageSize.dx;
            int32 dh = packCell.rect.dy - imageSize.dy;

            int32 rightMargin = atlas.texturesMargin;
            int32 bottomMargin = atlas.texturesMargin;

            if (dw < 0)
            {
                if (touchesRightBorder && (dw + rightMargin) >= 0)
                {
                    rightMargin += dw; // actually rightMargin is reduced as dw is negative there
                    dw = 0;
                }
                else
                    return nullptr;
            }

            if (dh < 0)
            {
                if (touchesBottomBorder && (dh + bottomMargin) >= 0)
                {
                    bottomMargin += dh; // actually bottomMargin is reduced as dh is negative there
                    dh = 0;
                }
                else
                    return nullptr;
            }

            if (dw == 0 && dh == 0)
            {
                isImageSet = true;
                this->packCell.rightMargin = rightMargin;
                this->packCell.bottomMargin = bottomMargin;
                return this;
            }

            isLeaf = false;

            child[0] = new TextureAtlas::PackNode(atlas);
            child[1] = new TextureAtlas::PackNode(atlas);

            child[0]->packCell.isTwoSideMargin = child[1]->packCell.isTwoSideMargin = false;
            child[0]->touchesBottomBorder = child[1]->touchesBottomBorder = this->touchesBottomBorder;
            child[0]->touchesRightBorder = child[1]->touchesRightBorder = this->touchesRightBorder;

            if (dw > dh) // horizontal split
            {
                child[0]->packCell.rect = Rect2i(packCell.rect.x, packCell.rect.y, imageSize.dx, packCell.rect.dy);
                child[1]->packCell.rect = Rect2i(packCell.rect.x + imageSize.dx, packCell.rect.y, packCell.rect.dx - imageSize.dx, packCell.rect.dy);
                child[0]->touchesRightBorder = false;
            }
            else // vertical split
            {
                child[0]->packCell.rect = Rect2i(packCell.rect.x, packCell.rect.y, packCell.rect.dx, imageSize.dy);
                child[1]->packCell.rect = Rect2i(packCell.rect.x, packCell.rect.y + imageSize.dy, packCell.rect.dx, packCell.rect.dy - imageSize.dy);
                child[0]->touchesBottomBorder = false;
            }
        }

        return child[0]->Insert(imageSize);
    }
}

void TextureAtlas::PackNode::Release()
{
    if (child[0])
        child[0]->Release();

    if (child[1])
        child[1]->Release();

    delete this;
}

TextureAtlas::TextureAtlas(const Rect2i& _rect, bool _useTwoSideMargin, int32 _texturesMargin)
    : useTwoSideMargin(_useTwoSideMargin)
    , texturesMargin(_texturesMargin)
{
    root = new PackNode(*this);
    root->touchesRightBorder = true;
    root->touchesBottomBorder = true;
    root->packCell.rect = rect = _rect;
    root->packCell.isTwoSideMargin = useTwoSideMargin;
}

TextureAtlas::~TextureAtlas()
{
    Release();
}

void TextureAtlas::Release()
{
    if (root)
    {
        root->Release();
        root = 0;
    }
}

bool TextureAtlas::AddImage(const Size2i& imageSize, void* searchPtr)
{
    PackNode* node = root->Insert(imageSize);
    if (node)
    {
        node->searchPtr = searchPtr;
        Logger::FrameworkDebug("set search ptr to rect:(%d, %d) ims: (%d, %d)", node->packCell.rect.dx, node->packCell.rect.dy, imageSize.dx, imageSize.dy);
    }
    return (node != 0);
}

PackedInfo* TextureAtlas::SearchRectForPtr(void* searchPtr)
{
    TextureAtlas::PackNode* res = root->SearchRectForPtr(searchPtr);
    return (res ? &res->packCell : 0);
}

TextureAtlas::PackNode* TextureAtlas::PackNode::SearchRectForPtr(void* searchPtr)
{
    if (searchPtr == this->searchPtr)
    {
        return this;
    }
    else
    {
        TextureAtlas::PackNode* res = 0;
        if (child[0])
            res = child[0]->SearchRectForPtr(searchPtr);
        if (!res && child[1])
            res = child[1]->SearchRectForPtr(searchPtr);
        return res;
    }
}
};
