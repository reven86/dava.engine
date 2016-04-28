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

#if __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#endif

#define STB_TEXTEDIT_CHARTYPE DAVA::WideString::value_type
#define STB_TEXTEDIT_STRING DAVA::StbTextEditBridge
//#define STB_TEXTEDIT_UNDOSTATECOUNT   99 // Use by default
//#define STB_TEXTEDIT_UNDOCHARCOUNT   999 // Use by default
//#define STB_TEXTEDIT_POSITIONTYPE    int // Use by default
#define STB_TEXTEDIT_NEWLINE L'\n'

#include <stb/stb_textedit.h>

inline void stb_layoutrow(StbTexteditRow* row, STB_TEXTEDIT_STRING* str, int start_i)
{
    auto start = static_cast<DAVA::uint32>(start_i);
    if (start >= str->GetDelegate()->GetTextLength())
        return;

    auto linesInfo = str->GetDelegate()->GetMultilineInfo();
    if (linesInfo.empty())
        return;

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

    // If single line mode enabled then extend height for first/last
    // lines to size of text field
    if (str->IsSingleLineMode())
    {
        if (!linesInfo.empty() && lineInfoIt == linesInfo.begin())
        {
            row->ymin = std::numeric_limits<float>::lowest();
        }
        if (!linesInfo.empty() && lineInfoIt == linesInfo.end() - 1)
        {
            row->ymax = std::numeric_limits<float>::max();
        }
    }
}

inline int stb_insertchars(STB_TEXTEDIT_STRING* str, int pos, STB_TEXTEDIT_CHARTYPE* newtext, int num)
{
    return int(str->GetDelegate()->InsertText(static_cast<DAVA::uint32>(pos), newtext, static_cast<DAVA::uint32>(num)));
}

inline int stb_deletechars(STB_TEXTEDIT_STRING* str, int pos, int num)
{
    return int(str->GetDelegate()->DeleteText(static_cast<DAVA::uint32>(pos), static_cast<DAVA::uint32>(num)));
}

inline int stb_stringlen(STB_TEXTEDIT_STRING* str)
{
    return int(str->GetDelegate()->GetTextLength());
}

inline float stb_getwidth(STB_TEXTEDIT_STRING* str, int n, int i)
{
    auto charsSizes = str->GetDelegate()->GetCharactersSizes();
    if (static_cast<DAVA::uint32>(charsSizes.size()) > static_cast<DAVA::uint32>(n + i))
    {
        return charsSizes[n + i];
    }
    return 0.f;
}

inline int stb_keytotext(int key)
{
    return key;
}

inline STB_TEXTEDIT_CHARTYPE stb_getchar(STB_TEXTEDIT_STRING* str, int i)
{
    return str->GetDelegate()->GetCharAt(static_cast<DAVA::uint32>(i));
}

inline int stb_isspace(STB_TEXTEDIT_CHARTYPE ch)
{
    return iswspace(ch) || iswpunct(ch);
}

#define STB_TEXTEDIT_LAYOUTROW stb_layoutrow
#define STB_TEXTEDIT_INSERTCHARS stb_insertchars
#define STB_TEXTEDIT_DELETECHARS stb_deletechars
#define STB_TEXTEDIT_STRINGLEN stb_stringlen
#define STB_TEXTEDIT_GETWIDTH stb_getwidth
#define STB_TEXTEDIT_KEYTOTEXT stb_keytotext
#define STB_TEXTEDIT_GETCHAR stb_getchar
#define STB_TEXTEDIT_IS_SPACE stb_isspace

#define STB_TEXTEDIT_K_SHIFT DAVA::StbTextEditBridge::KEY_SHIFT_MASK
#define STB_TEXTEDIT_K_LEFT DAVA::StbTextEditBridge::KEY_LEFT
#define STB_TEXTEDIT_K_RIGHT DAVA::StbTextEditBridge::KEY_RIGHT
#define STB_TEXTEDIT_K_UP DAVA::StbTextEditBridge::KEY_UP
#define STB_TEXTEDIT_K_DOWN DAVA::StbTextEditBridge::KEY_DOWN
#define STB_TEXTEDIT_K_LINESTART DAVA::StbTextEditBridge::KEY_LINESTART
#define STB_TEXTEDIT_K_LINEEND DAVA::StbTextEditBridge::KEY_LINEEND
#define STB_TEXTEDIT_K_TEXTSTART DAVA::StbTextEditBridge::KEY_TEXTSTART
#define STB_TEXTEDIT_K_TEXTEND DAVA::StbTextEditBridge::KEY_TEXTEND
#define STB_TEXTEDIT_K_DELETE DAVA::StbTextEditBridge::KEY_DELETE
#define STB_TEXTEDIT_K_BACKSPACE DAVA::StbTextEditBridge::KEY_BACKSPACE
#define STB_TEXTEDIT_K_UNDO DAVA::StbTextEditBridge::KEY_UNDO
#define STB_TEXTEDIT_K_REDO DAVA::StbTextEditBridge::KEY_REDO
#define STB_TEXTEDIT_K_INSERT DAVA::StbTextEditBridge::KEY_INSERT
#define STB_TEXTEDIT_K_WORDLEFT DAVA::StbTextEditBridge::KEY_WORDLEFT
#define STB_TEXTEDIT_K_WORDRIGHT DAVA::StbTextEditBridge::KEY_WORDRIGHT
//#define STB_TEXTEDIT_K_PGUP
//#define STB_TEXTEDIT_K_PGDOWN

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

StbTextEditBridge::StbTextEditBridge(StbTextDelegate* delegate)
    : stb_state(new StbState())
    , delegate(delegate)
{
    DVASSERT_MSG(delegate, "StbTextEditBridge must be inited with delegate!");
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

void StbTextEditBridge::SetSelectionStart(uint32 position) const
{
    stb_state->select_start = static_cast<int>(position);
}

uint32 StbTextEditBridge::GetSelectionEnd() const
{
    return static_cast<uint32>(stb_state->select_end);
}

void StbTextEditBridge::SetSelectionEnd(uint32 position) const
{
    stb_state->select_end = static_cast<int>(position);
}

uint32 StbTextEditBridge::GetCursorPosition() const
{
    return static_cast<uint32>(stb_state->cursor);
}

void StbTextEditBridge::SetCursorPosition(uint32 position) const
{
    stb_state->cursor = static_cast<int>(position);
}

void StbTextEditBridge::SetSingleLineMode(bool signleLine)
{
    stb_state->single_line = static_cast<unsigned char>(signleLine);
}

bool StbTextEditBridge::IsSingleLineMode() const
{
    return stb_state->single_line != 0;
}

bool StbTextEditBridge::IsInsertMode() const
{
    return stb_state->insert_mode != 0;
}
}
