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

/**
 * @brief Class that implements bridge for stb_textedit
 */
class StbTextEditBridge
{
public:
    /**
     * @brief Default constructor
     */
    StbTextEditBridge();

    /**
     * @brief Copy constructor
     * @param c original object
     */
    StbTextEditBridge(const StbTextEditBridge& c);

    /**
     * @brief Destructor
     */
    virtual ~StbTextEditBridge();

    /**
     * @brief Copy class data to currect instanse
     * @param c object to copy data
     */
    virtual void CopyStbStateFrom(const StbTextEditBridge& c);

    /**
     * @brief Send key to STB text edit
     * @param codePoint key code
     */
    virtual void SendKey(uint32 codePoint);

    /**
     * @brief Cut (delete) selected text
     */
    virtual void Cut();

    /**
     * @brief Insert (replace selected) new text in field
     * @param str string to pasting
     */
    virtual void Paste(const WideString& str);

    /**
     * @brief Send mouse click to STB text edit
     * @param point mouse point (x,y) in control's local cordinates
     */
    virtual void Click(const Vector2& point);

    /**
     * @brief Send mouse drag event to STB text edit
     * @param point mouse point (x,y) in control's local cordinates
     */
    virtual void Drag(const Vector2& point);

    /**
     * @brief Returs character index of selection start
     * @return character index
     */
    uint32 GetSelectionStart() const;

    /**
     * @brief Returs character index of selection end
     * @return character index
     */
    uint32 GetSelectionEnd() const;

    /**
     * @brief Returs character index of cursor position. 
     *        Cursor equal 0 - cursor before first symbol, 
     *        cursor equal text length - cursro after last symbol
     * @return character index
     */
    uint32 GetCursor() const;

    /**
     * @brief Return inserting mode flag
     * @return if True that insertiog mode is enabled
     */
    bool IsInsertMode() const;

    /**
     * @brief Service fuction for insert text in data structure
     * @param position poisiton of inserting
     * @param str string to inserting
     * @param length string length
     */
    virtual void InsertText(uint32 position, const WideString::value_type* str, uint32 length) = 0;

    /**
     * @brief Service function for delete text from data structure
     * @param position positon of deleting
     * @param length deleting substring length
     */
    virtual void DeleteText(uint32 position, uint32 length) = 0;

    /**
     * @brief Service function for getting information about lines in text
     * @return vector of lines infromation
     */
    virtual const Vector<TextBlock::Line>& GetMultilineInfo() = 0;

    /**
     * @brief Service function for getting infromation of cahracters sizes
     * @return vector of characters sizes
     */
    virtual const Vector<float32>& GetCharactersSizes() = 0;

    /**
     * @brief Service function for getting text length
     * @return text length
     */
    virtual uint32 GetLength() = 0;

    /**
     * @brief Service function for getting character from text
     * @param i character index
     * @return character
     */
    virtual WideString::value_type GetChar(uint32 i) = 0;

private:
    StbState* stb_state = nullptr; //!< Inner STB state structure ptr
};
}

#define STB_TEXTEDIT_K_SHIFT 0x00020000 //SHIFT MODIFICATOR
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