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


#include "UIJoypadMetadata.h"
#include "StringConstants.h"

namespace DAVA {
    
UIJoypadMetadata::UIJoypadMetadata(QObject* parent) :
    UIControlMetadata(parent)
{
}

UIJoypad* UIJoypadMetadata::GetActiveJoypad() const
{
    return static_cast<UIJoypad*>(GetActiveUIControl());
}

void UIJoypadMetadata::SetStickSprite(const QString& value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

    if (value.isEmpty())
    {
        GetActiveJoypad()->SetStickSprite(NULL, 0);
    }
    else
    {
        GetActiveJoypad()->SetStickSprite(value.toStdString(), 0);
    }
}
    
QString UIJoypadMetadata::GetStickSprite() const
{
    if (!VerifyActiveParamID())
    {
        return QString();
    }

    Sprite* sprite = GetActiveJoypad()->GetStickSprite();
    if (sprite == NULL)
    {
        return StringConstants::NO_SPRITE_IS_SET;
    }
        
    return QString::fromStdString(sprite->GetRelativePathname().GetAbsolutePathname());
}
    
void UIJoypadMetadata::SetStickSpriteFrame(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
        
    Sprite* sprite = GetActiveJoypad()->GetStickSprite();
    if (!sprite || sprite->GetFrameCount() <= value)
    {
        return;
    }

    GetActiveJoypad()->SetStickSpriteFrame(value);
}

int UIJoypadMetadata::GetStickSpriteFrame() const
{
    if (!VerifyActiveParamID())
    {
        return -1;
    }

    Sprite* sprite = GetActiveJoypad()->GetStickSprite();
    if (!sprite)
    {
        return 0;
    }
    
    return GetActiveJoypad()->GetStickSpriteFrame();
}

int UIJoypadMetadata::GetDeadArea() const
{
    if (!VerifyActiveParamID())
    {
        return  -1;
    }

    return GetActiveJoypad()->GetDeadAreaSize();
}
    
void UIJoypadMetadata::SetDeadArea(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    GetActiveJoypad()->SetDeadAreaSize(value);
}
    
float UIJoypadMetadata::GetDigitalSense() const
{
    if (!VerifyActiveParamID())
    {
        return  -1.0f;
    }
        
    return GetActiveJoypad()->GetDigitalSense();
}

void UIJoypadMetadata::SetDigitalSense(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    GetActiveJoypad()->SetDeadAreaSize(value);
}

};
