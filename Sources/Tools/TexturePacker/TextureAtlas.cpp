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
TextureAtlas::AtlasNode* TextureAtlas::Insert(const TextureAtlas::AtlasNodePtr& node, const Size2i& imageSize, void* imagePtr)
{
    DVASSERT(node);

    if (node->child[0])
    {
        DVASSERT(node->child[1]);

        AtlasNode* insResult = Insert(node->child[0], imageSize, imagePtr);
        if (insResult)
        {
            return insResult;
        }
        else
        {
            return Insert(node->child[1], imageSize, imagePtr);
        }
    }
    else
    {
        if (node->imagePtr)
        {
            return nullptr;
        }

        ImageCell& cell = node->cell;

        int32 occupiedWidth = imageSize.dx + cell.leftEdgePixel + cell.rightEdgePixel + cell.rightMargin;
        int32 cell0height = imageSize.dy + cell.topEdgePixel + cell.bottomEdgePixel + cell.bottomMargin;
        int32 restWidth = cell.rect.dx - occupiedWidth;
        int32 restHeight = cell.rect.dy - cell0height;

        auto SetImage = [&] {
            node->imagePtr = imagePtr;
            cell.imageRect.dx = imageSize.dx;
            cell.imageRect.dy = imageSize.dy;
            cell.imageRect.x = cell.rect.x + cell.leftEdgePixel;
            cell.imageRect.y = cell.rect.y + cell.topEdgePixel;
        };

        if (restWidth == 0 && restHeight == 0)
        {
            SetImage();
            return node.get();
        }
        else if (restWidth < 0 || restHeight < 0)
        {
            return nullptr;
        }
        else
        {
            int32 longest = (restWidth > restHeight) ? restWidth : restHeight;
            if (longest <= splitter) // it's no use to make split
            {
                // try to add edge pixel anyway
                if (longest == restWidth && cell.rightEdgePixel == 0 && restWidth >= edgePixel)
                {
                    restWidth -= edgePixel;
                    cell.rightEdgePixel = edgePixel;
                }
                else if (longest == restHeight && cell.bottomEdgePixel == 0 && restHeight >= edgePixel)
                {
                    restHeight -= edgePixel;
                    cell.bottomEdgePixel = edgePixel;
                }

                cell.rightMargin += restWidth;
                cell.bottomMargin += restHeight;
                SetImage();
                return node.get();
            }

            node->child[0].reset(new TextureAtlas::AtlasNode);
            node->child[1].reset(new TextureAtlas::AtlasNode);

            ImageCell& cell0 = node->child[0]->cell;
            ImageCell& cell1 = node->child[1]->cell;

            cell0 = cell1 = cell;

            if (longest == restWidth) // horizontal split
            {
                cell0.rightEdgePixel = edgePixel;
                cell0.rightMargin = texturesMargin;
                cell1.leftEdgePixel = edgePixel;

                int32 cell0width = imageSize.dx + cell0.leftEdgePixel + cell0.rightEdgePixel + cell0.rightMargin;

                cell0.rect = Rect2i(cell.rect.x, cell.rect.y, cell0width, cell.rect.dy);
                cell1.rect = Rect2i(cell.rect.x + cell0width, cell.rect.y, cell.rect.dx - cell0width, cell.rect.dy);
            }
            else // vertical split
            {
                cell0.bottomEdgePixel = edgePixel;
                cell0.bottomMargin = texturesMargin;
                cell1.topEdgePixel = edgePixel;

                int32 cell0height = imageSize.dy + cell0.topEdgePixel + cell0.bottomEdgePixel + cell0.bottomMargin;

                cell0.rect = Rect2i(cell.rect.x, cell.rect.y, cell.rect.dx, cell0height);
                cell1.rect = Rect2i(cell.rect.x, cell.rect.y + cell0height, cell.rect.dx, cell.rect.dy - cell0height);
            }

            return Insert(node->child[0], imageSize, imagePtr);
        }
    }
}

TextureAtlas::TextureAtlas(const Rect2i& rect, bool useTwoSideMargin, int32 _texturesMargin)
    : texturesMargin(_texturesMargin)
    , edgePixel(useTwoSideMargin ? 1 : 0)
    , splitter(texturesMargin + edgePixel + edgePixel)
{
    rootNode.reset(new AtlasNode);
    rootNode->cell.rect = rect;
}

bool TextureAtlas::AddImage(const Size2i& imageSize, void* imagePtr)
{
    AtlasNode* node = Insert(rootNode, imageSize, imagePtr);
    if (node)
    {
        Logger::FrameworkDebug("image set to (%d, %d), image size [%d x %d]", node->cell.rect.dx, node->cell.rect.dy, imageSize.dx, imageSize.dy);
        return true;
    }
    else
    {
        return false;
    }
}

ImageCell* TextureAtlas::GetImageCell(void* searchPtr)
{
    AtlasNode* res = SearchRectForPtr(rootNode, searchPtr);
    return (res ? &res->cell : nullptr);
}

TextureAtlas::AtlasNode* TextureAtlas::SearchRectForPtr(const AtlasNodePtr& node, void* imagePtr)
{
    if (imagePtr == node->imagePtr)
    {
        return node.get();
    }
    else
    {
        TextureAtlas::AtlasNode* res = nullptr;
        if (node->child[0])
        {
            res = SearchRectForPtr(node->child[0], imagePtr);
        }
        if (!res && node->child[1])
        {
            res = SearchRectForPtr(node->child[1], imagePtr);
        }
        return res;
    }
}
};
