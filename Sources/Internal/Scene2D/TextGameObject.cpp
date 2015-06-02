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


#include "TextGameObject.h"
#include "Render/2D/TextBlock.h"
#include "Render/2D/FTFont.h"

namespace DAVA
{
TextGameObject::TextGameObject(const Rect &rect)
{
    textBlock = TextBlock::Create(Vector2(rect.dx, rect.dy));
    SetPosition(rect.x, rect.y);
}

TextGameObject::TextGameObject(const Rect &rect, Font *font, const WideString &string)
{
    const Vector2 requestedRect(rect.dx, rect.dy);
    textBlock = TextBlock::Create(requestedRect);
    SetPosition(rect.x, rect.y);
    SetFont(font, false);
    SetText(string, requestedRect);
}

void TextGameObject::PrepareSprite()
{
    if(textBlock->IsSpriteReady())
    {
        if(GetSprite() != textBlock->GetSprite()) 
        {
            SetSprite(textBlock->GetSprite());
        }
    }
    else 
    {
        SetSprite(NULL);
    }
}

void TextGameObject::SetText( const WideString &string, const Vector2 &requestedTextRectSize /*= Vector2(0,0)*/ )
{
    textBlock->SetText(string, requestedTextRectSize);
    PrepareSprite();
}

void TextGameObject::SetFittingOption(int32 fittingType)
{
    textBlock->SetFittingOption(fittingType);
    PrepareSprite();
}

void TextGameObject::SetFont( Font *font, bool prepareSprite /*= true*/ )
{
    textBlock->SetFont(font);
    if(prepareSprite) 
        PrepareSprite();
}

void TextGameObject::SetMultiline(bool isMultilineEnabled, bool bySymbol)
{
    textBlock->SetMultiline(isMultilineEnabled, bySymbol);
    PrepareSprite();
}

void TextGameObject::SetAlign(int32 alignment)
{
    textBlock->SetAlign(alignment);
}
};