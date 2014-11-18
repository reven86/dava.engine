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


//
//  UIScrollViewMetadata.cpp
//  UIEditor
//
//  Created by Denis Bespalov on 4/23/13.
//
//

#include "UIScrollViewMetadata.h"

namespace DAVA {

UIScrollViewMetadata::UIScrollViewMetadata(QObject* parent) :
	UIControlMetadata(parent)
{
}

UIScrollView* UIScrollViewMetadata::GetActiveUIScrollView() const
{
	return static_cast<UIScrollView*>(GetActiveUIControl());
}

void UIScrollViewMetadata::InitializeControl(const String& controlName, const Vector2& position)
{
	UIControlMetadata::InitializeControl(controlName, position);
	
	int paramsCount = this->GetParamsCount();
    for (BaseMetadataParams::METADATAPARAMID i = 0; i < paramsCount; i ++)
    {
		// Initialize UIList
        UIScrollView* scrollView = static_cast<UIScrollView*>(this->treeNodeParams[i].GetUIControl());
        scrollView->GetBackground()->SetDrawType(UIControlBackground::DRAW_SCALE_TO_RECT);
    }	
}

float UIScrollViewMetadata::GetHorizontalScrollPosition() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0f;
    }
    
	Vector2 padding = GetActiveUIScrollView()->GetPadding();
	
	return (-1) * qRound(padding.x);
}

void UIScrollViewMetadata::SetHorizontalScrollPosition(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	Vector2 padding = GetActiveUIScrollView()->GetPadding();
	padding.x = (-1) * value;	

	GetActiveUIScrollView()->SetPadding(padding);
}

float UIScrollViewMetadata::GetVerticalScrollPosition() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0f;
    }
	
	Vector2 padding = GetActiveUIScrollView()->GetPadding();
	
	return (-1) * qRound(padding.y);
}

void UIScrollViewMetadata::SetVerticalScrollPosition(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

	Vector2 padding = GetActiveUIScrollView()->GetPadding();
	padding.y = (-1) * value;

	GetActiveUIScrollView()->SetPadding(padding);
}

float UIScrollViewMetadata::GetContentSizeX() const
{
	if (!VerifyActiveParamID())
    {
        return -1.0f;
    }
	
	Vector2 contentSize = GetActiveUIScrollView()->GetContentSize();
		
	return contentSize.x;
}

void UIScrollViewMetadata::SetContentSizeX(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	Vector2 contentSize = GetActiveUIScrollView()->GetContentSize();
	contentSize.x = value;
}

float UIScrollViewMetadata::GetContentSizeY() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0f;
    }
	
	Vector2 contentSize = GetActiveUIScrollView()->GetContentSize();
		
	return contentSize.y;
}

void UIScrollViewMetadata::SetContentSizeY(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	Vector2 contentSize = GetActiveUIScrollView()->GetContentSize();
	contentSize.y = value;
}

}