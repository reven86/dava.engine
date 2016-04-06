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

#include "StbTextEditBridge.h"

#define STB_TEXTEDIT_CHARTYPE DAVA::WideString::value_type
#define STB_TEXTEDIT_STRING DAVA::StbTextEditBridge
//#define STB_TEXTEDIT_UNDOSTATECOUNT   99 // Use by default
//#define STB_TEXTEDIT_UNDOCHARCOUNT   999 // Use by default
//#define STB_TEXTEDIT_POSITIONTYPE    int // Use by default
#define STB_TEXTEDIT_NEWLINE L'\n'

#if __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#endif

#include <stb/stb_textedit.h>

inline void STB_TEXTEDIT_LAYOUTROW(StbTexteditRow* row, STB_TEXTEDIT_STRING* str, int start_i)
{
    auto start = DAVA::uint32(start_i);
    if (start >= str->GetLength())
        return;

    auto linesInfo = str->GetMultilineInfo();
    auto lineInfoIt = std::find_if(linesInfo.begin(), linesInfo.end(), [start](const DAVA::TextBlock::Line& l)
                                   {
                                       return l.offset == static_cast<DAVA::uint32>(start);
                                   });
    DVASSERT(lineInfoIt != linesInfo.end());
    auto line = *lineInfoIt;
    row->num_chars = line.length;
    row->x0 = line.xoffset;
    row->x1 = line.xoffset + line.xadvance;
    row->baseline_y_delta = 0.f;
    row->ymin = line.yoffset;
    row->ymax = line.yoffset + line.yadvance;
}

inline int STB_TEXTEDIT_INSERTCHARS(STB_TEXTEDIT_STRING* str, int pos, STB_TEXTEDIT_CHARTYPE* newtext, int num)
{
    return int(str->InsertText(DAVA::uint32(pos), newtext, DAVA::uint32(num)));
}

inline int STB_TEXTEDIT_DELETECHARS(STB_TEXTEDIT_STRING* str, int pos, int num)
{
    return int(str->DeleteText(DAVA::uint32(pos), DAVA::uint32(num)));
}

inline int STB_TEXTEDIT_STRINGLEN(STB_TEXTEDIT_STRING* str)
{
    return int(str->GetLength());
}

inline float STB_TEXTEDIT_GETWIDTH(STB_TEXTEDIT_STRING* str, int n, int i)
{
    auto charsSizes = str->GetCharactersSizes();
    if (DAVA::uint32(charsSizes.size()) > DAVA::uint32(n + i))
    {
        return charsSizes[n + i];
    }
    return 0.f;
}

inline int STB_TEXTEDIT_KEYTOTEXT(int key)
{
    return key;
}

inline STB_TEXTEDIT_CHARTYPE STB_TEXTEDIT_GETCHAR(STB_TEXTEDIT_STRING* str, int i)
{
    return str->GetChar(DAVA::uint32(i));
}

inline int STB_TEXTEDIT_IS_SPACE(STB_TEXTEDIT_CHARTYPE ch)
{
    return isspace(ch) || ch == ',' || ch == ';' || ch == '(' || ch == ')' || ch == '{' || ch == '}' || ch == '[' || ch == ']' || ch == '|';
}

#define STB_TEXTEDIT_IMPLEMENTATION
#include <stb/stb_textedit.h>

#if __clang__
#pragma clang diagnostic pop
#endif

////////////////////////////////////////////////////////////////////////////////

namespace DAVA
{
struct StbState : public STB_TexteditState
{
};

StbTextEditBridge::StbTextEditBridge()
    : stb_state(new StbState())
{
    stb_textedit_initialize_state(stb_state, 0);
}

StbTextEditBridge::StbTextEditBridge(const StbTextEditBridge& c)
    : stb_state(new StbState(*c.stb_state))
{
}

StbTextEditBridge::~StbTextEditBridge()
{
    SafeDelete(stb_state);
}

void StbTextEditBridge::CopyStbStateFrom(const StbTextEditBridge& c)
{
    DVASSERT(stb_state);
    DVASSERT(c.stb_state);
    Memcpy(stb_state, c.stb_state, sizeof(StbState));
}

void StbTextEditBridge::SendKey(uint32 codePoint)
{
    stb_textedit_key(this, stb_state, codePoint);
}

void StbTextEditBridge::Cut()
{
    stb_textedit_cut(this, stb_state);
}

void StbTextEditBridge::Paste(const WideString& str)
{
    stb_textedit_paste(this, stb_state, str.c_str(), int(str.length()));
}

void StbTextEditBridge::Click(const Vector2& point)
{
    stb_textedit_click(this, stb_state, point.x, point.y);
}

void StbTextEditBridge::Drag(const Vector2& point)
{
    stb_textedit_drag(this, stb_state, point.x, point.y);
}

uint32 StbTextEditBridge::GetSelectionStart() const
{
    return static_cast<uint32>(stb_state->select_start);
}

uint32 StbTextEditBridge::GetSelectionEnd() const
{
    return static_cast<uint32>(stb_state->select_end);
}

uint32 StbTextEditBridge::GetCursor() const
{
    return static_cast<uint32>(stb_state->cursor);
}

void StbTextEditBridge::SetCursor(uint32 position) const
{
    stb_state->cursor = int(position);
}

bool StbTextEditBridge::IsInsertMode() const
{
    return stb_state->insert_mode != 0;
}
}
