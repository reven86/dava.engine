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

#ifndef __DAVA_UITEXTFIELDSTBBRIDGE_H__
#define __DAVA_UITEXTFIELDSTBBRIDGE_H__

#include "Base/BaseTypes.h"
#include "Render/2D/TextBlock.h"

namespace DAVA
{
struct StbState;

class StbTextEditBridge
{
public:
    StbTextEditBridge();
    StbTextEditBridge(const StbTextEditBridge& c);
    ~StbTextEditBridge();
    virtual void CopyStbStateFrom(const StbTextEditBridge& c);

    virtual void SendKey(uint32 codePoint);
    virtual void Cut();
    virtual void Paste(const WideString& str);
    virtual void Click(const Vector2& point);
    virtual void Drag(const Vector2& point);

    uint32 GetSelectionStart() const;
    uint32 GetSelectionEnd() const;
    uint32 GetCursor() const;
    bool IsInsertMode() const;

    virtual void InsertText(uint32 position, const WideString::value_type* str, uint32 length) = 0;
    virtual void DeleteText(uint32 position, uint32 length) = 0;
    virtual const Vector<TextBlock::Line>& GetMultilineInfo() = 0;
    virtual const Vector<float32>& GetCharactersSizes() = 0;
    virtual uint32 GetLength() = 0;
    virtual WideString::value_type GetChar(uint32 i) = 0;

private:
    StbState* stb_state = nullptr;
};
}

#define STB_TEXTEDIT_K_SHIFT 0x80000000 //SHIFT MODIFICATOR
#define STB_TEXTEDIT_K_LEFT 0x00010000 //KEY_DOWN(VK_LEFT)
#define STB_TEXTEDIT_K_RIGHT 0x00010001 //KEY_DOWN(VK_RIGHT)
#define STB_TEXTEDIT_K_UP 0x00010002 //KEY_DOWN(VK_UP)
#define STB_TEXTEDIT_K_DOWN 0x00010003 //KEY_DOWN(VK_DOWN)
#define STB_TEXTEDIT_K_LINESTART 0x00010004 //KEY_DOWN(VK_HOME)
#define STB_TEXTEDIT_K_LINEEND 0x00010005 //KEY_DOWN(VK_END)
#define STB_TEXTEDIT_K_TEXTSTART 0x00010006 //KEY_DOWN(VK_HOME + VK_CTRL)
#define STB_TEXTEDIT_K_TEXTEND 0x00010007 //KEY_DOWN(VK_END + VK_CTRL)
#define STB_TEXTEDIT_K_DELETE 0x00010008 //KEY_DOWN(VK_DELETE)
#define STB_TEXTEDIT_K_BACKSPACE 8 //CHAR(8) or KEY_DOWN(VK_BACKSPACE)
#define STB_TEXTEDIT_K_UNDO 26 //CHAR(26) or KEY_DOWN(VK_Z + VK_CTRL)
#define STB_TEXTEDIT_K_REDO 25 //CHAR(25) or KEY_DOWN(VK_Y + VK_CTRL)
#define STB_TEXTEDIT_K_INSERT 0x00010009 //KEY_DOWN(VK_INSERT)
#define STB_TEXTEDIT_K_WORDLEFT 0x00010010 //KEY_DOWN(VK_LEFT + VK_CTRL)
#define STB_TEXTEDIT_K_WORDRIGHT 0x00010011 //KEY_DOWN(VK_RIGHT + VK_CTRL)
#define STB_TEXTEDIT_K_PGUP 0x00010012 //KEY_DOWN(VK_PGUP)
#define STB_TEXTEDIT_K_PGDOWN 0x00010013 //KEY_DOWN(VK_PGDN)

#endif //__DAVA_UITEXTFIELDSTBBRIDGE_H__