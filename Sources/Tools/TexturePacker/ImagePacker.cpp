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


#include "TexturePacker/ImagePacker.h"
#include "Base/BaseTypes.h"
#include "TexturePacker/CommandLineParser.h"

namespace DAVA
{

ImagePacker::PackNode * ImagePacker::PackNode::Insert(const Size2i & imageSize)
{
	if (!isLeaf)
	{
		ImagePacker::PackNode *newNode = child[0]->Insert(imageSize);
		if (newNode)return newNode;

		return child[1]->Insert(imageSize);
	}else
	{
		if (isImageSet)return 0;

		int32 dw = rect.dx - imageSize.dx;
		int32 dh = rect.dy - imageSize.dy;

		int32 rightMargin = packer->texturesMargin;
		int32 bottomMargin = packer->texturesMargin;

		if ( dw < 0 )
		{
			if (touchesRightBorder && !packer->useTwoSideMargin && (dw + rightMargin) >= 0)
			{
				rightMargin += dw; // actually rightMargin is reduced as dw is negative there
				dw = 0;
			}
			else
				return 0;
		}

		if ( dh < 0 )
		{
			if (touchesBottomBorder && !packer->useTwoSideMargin && (dh + bottomMargin) >= 0)
			{
				bottomMargin += dh; // actually bottomMargin is reduced as dh is negative there
				dh = 0;
			}
			else
				return 0;
		}

		if (dw==0 && dh==0)
		{
			isImageSet = true;
			this->rightMargin = rightMargin;
			this->bottomMargin = bottomMargin;
			return this;
		}

		isLeaf = false;

		child[0] = new ImagePacker::PackNode(packer);
		child[1] = new ImagePacker::PackNode(packer);

		child[1]->touchesRightBorder &= 1;
		child[1]->touchesBottomBorder &= 1;

		if (dw > dh)
		{
			child[0]->rect = Rect2i(	rect.x, rect.y, imageSize.dx, rect.dy);
			child[1]->rect = Rect2i(	rect.x + imageSize.dx, rect.y, rect.dx - imageSize.dx, rect.dy);
			child[0]->touchesBottomBorder &= 1;
			child[0]->touchesRightBorder = 0;
		}else
		{
			child[0]->rect = Rect2i(	rect.x, rect.y, rect.dx, imageSize.dy);
			child[1]->rect = Rect2i(	rect.x, rect.y + imageSize.dy, rect.dx, rect.dy - imageSize.dy);
			child[0]->touchesRightBorder &= 1;
			child[0]->touchesBottomBorder = 0;
		}
		return child[0]->Insert(imageSize);
	}
}

void ImagePacker::PackNode::Release()
{
	if (child[0])
		child[0]->Release();

	if (child[1])
		child[1]->Release();

	delete this;
}


ImagePacker::ImagePacker(const Rect2i & _rect, bool _useTwoSideMargin, int32 _texturesMargin) :
	useTwoSideMargin(_useTwoSideMargin),
	texturesMargin(_texturesMargin)
{
	root = new PackNode(this);
	root->rect = _rect;
	rect = _rect;
}

ImagePacker::~ImagePacker()
{
	Release();
}

void ImagePacker::Release()
{
	if (root)
	{
		root->Release();
		root = 0;
	}
}

bool ImagePacker::AddImage(const Size2i & imageSize, void * searchPtr)
{
	PackNode * node = root->Insert(imageSize);
	if (node)
	{
		node->searchPtr = searchPtr;
        Logger::FrameworkDebug("set search ptr to rect:(%d, %d) ims: (%d, %d)", node->rect.dx, node->rect.dy, imageSize.dx, imageSize.dy);
	}
	return (node != 0);
}
	
Rect2i * ImagePacker::SearchRectForPtr(void * searchPtr)
{
	ImagePacker::PackNode * res = root->SearchRectForPtr(searchPtr);
	return (res ? &res->rect : 0);
}

Rect2i * ImagePacker::SearchRectForPtr(void * searchPtr, uint32& rmargin, uint32& bmargin)
{
	ImagePacker::PackNode * res = root->SearchRectForPtr(searchPtr);
	if ( res )
	{
		rmargin = res->rightMargin;
		bmargin = res->bottomMargin;
		return &res->rect;
	}
	else
		return 0;
}

ImagePacker::PackNode * ImagePacker::PackNode::SearchRectForPtr(void * searchPtr)
{
	if (searchPtr == this->searchPtr)
	{
		return this;
	}
	else
	{
		ImagePacker::PackNode * res = 0;
		if (child[0]) res = child[0]->SearchRectForPtr(searchPtr);
		if (!res && child[1]) res = child[1]->SearchRectForPtr(searchPtr);
		return res;
	}
}
	
};
