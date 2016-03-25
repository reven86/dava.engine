#ifndef __DAVA_UITEXTFIELD2STBBIND_H__
#define __DAVA_UITEXTFIELD2STBBIND_H__

#include "UI/UITextField2.h"

#define STB_TEXTEDIT_CHARTYPE DAVA::WideString::value_type
#define STB_TEXTEDIT_STRING StbTextStruct
//#define STB_TEXTEDIT_UNDOSTATECOUNT   99
//#define STB_TEXTEDIT_UNDOCHARCOUNT   999
//#define STB_TEXTEDIT_POSITIONTYPE    int

#include <stb/stb_textedit.h>

struct StbTextStruct
{
    DAVA::UITextField2* field;
    STB_TexteditState state;
};

//    STB_TEXTEDIT_LAYOUTROW(&r,obj,n)  returns the results of laying out a line of characters
//                                        starting from character #n (see discussion below)
inline void layout_func(StbTexteditRow* row, STB_TEXTEDIT_STRING* str, int start_i)
{
    int remaining_chars = static_cast<int>(str->field->GetText().length() - start_i);
    row->num_chars = remaining_chars > 20 ? 20 : remaining_chars; // should do real word wrap here
    row->x0 = 0;
    row->x1 = 20; // need to account for actual size of characters
    row->baseline_y_delta = 1.25;
    row->ymin = -1;
    row->ymax = 0;
}

//    STB_TEXTEDIT_INSERTCHARS(obj,i,c*,n)   insert n characters at i (pointed to by STB_TEXTEDIT_CHARTYPE*)
inline int insert_chars(STB_TEXTEDIT_STRING* str, int pos, STB_TEXTEDIT_CHARTYPE* newtext, int num)
{
    str->field->innerInsertText(pos, newtext, num);
    return 1; // always succeeds
}

//    STB_TEXTEDIT_DELETECHARS(obj,i,n)      delete n characters starting at i
inline int delete_chars(STB_TEXTEDIT_STRING* str, int pos, int num)
{
    str->field->innerDeleteText(pos, num);
    return 1; // always succeeds
}

//    STB_TEXTEDIT_STRINGLEN(obj)       the length of the string (ideally O(1))
inline int get_length(STB_TEXTEDIT_STRING* str)
{
    return static_cast<int>(str->field->GetText().length());
}

//    STB_TEXTEDIT_GETWIDTH(obj,n,i)    returns the pixel delta from the xpos of the i'th character
//                                        to the xpos of the i+1'th char for a line of characters
//                                        starting at character #n (i.e. accounts for kerning
//                                        with previous char)
inline float get_width(STB_TEXTEDIT_STRING* str, int n, int i)
{
    return 1.f;
}

//    STB_TEXTEDIT_KEYTOTEXT(k)         maps a keyboard input to an insertable character
//                                        (return type is int, -1 means not valid to insert)
inline int key_to_text(int key)
{
    if ('\r' == key) return '\n';
    return key;
}

//    STB_TEXTEDIT_GETCHAR(obj,i)       returns the i'th character of obj, 0-based
inline STB_TEXTEDIT_CHARTYPE get_char(STB_TEXTEDIT_STRING* str, int i)
{
    return str->field->GetText()[i];
}

// define all the #defines needed 
#define STB_TEXTEDIT_STRINGLEN get_length
#define STB_TEXTEDIT_LAYOUTROW layout_func
#define STB_TEXTEDIT_GETWIDTH get_width
#define STB_TEXTEDIT_KEYTOTEXT key_to_text
#define STB_TEXTEDIT_GETCHAR get_char
#define STB_TEXTEDIT_DELETECHARS delete_chars
#define STB_TEXTEDIT_INSERTCHARS insert_chars
#define STB_TEXTEDIT_IS_SPACE isspace
#define STB_TEXTEDIT_NEWLINE L'\n'

//#define STB_TEXTEDIT_K_CONTROL         0x20000000 //Not required
#define STB_TEXTEDIT_K_SHIFT 0x40000000 //SHIFT MODIFICATOR
#define STB_TEXTEDIT_K_LEFT 0x00010000 //KEY_DOWN(VK_LEFT)
#define STB_TEXTEDIT_K_RIGHT 0x00010001 //KEY_DOWN(VK_RIGHT)
#define STB_TEXTEDIT_K_UP 0x00010002 //KEY_DOWN(VK_UP)
#define STB_TEXTEDIT_K_DOWN 0x00010004 //KEY_DOWN(VK_DOWN)
#define STB_TEXTEDIT_K_LINESTART 0x00010008 //KEY_DOWN(VK_HOME)
#define STB_TEXTEDIT_K_LINEEND 0x00010010 //KEY_DOWN(VK_END)
#define STB_TEXTEDIT_K_TEXTSTART 0x00010020 //KEY_DOWN(VK_HOME + VK_CTRL)
#define STB_TEXTEDIT_K_TEXTEND 0x00010040 //KEY_DOWN(VK_END + VK_CTRL)
#define STB_TEXTEDIT_K_DELETE 0x00010080 //KEY_DOWN(VK_DELETE)
#define STB_TEXTEDIT_K_BACKSPACE 8 //CHAR(8) or KEY_DOWN(VK_BACKSPACE)
#define STB_TEXTEDIT_K_UNDO 26 //CHAR(26) or KEY_DOWN(VK_Z + VK_CTRL)
#define STB_TEXTEDIT_K_REDO 25 //CHAR(25) or KEY_DOWN(VK_Y + VK_CTRL)
#define STB_TEXTEDIT_K_INSERT 0x00010800 //KEY_DOWN(VK_INSERT)
#define STB_TEXTEDIT_K_WORDLEFT 0x00011000 //KEY_DOWN(VK_LEFT + VK_CTRL)
#define STB_TEXTEDIT_K_WORDRIGHT 0x00012000 //KEY_DOWN(VK_RIGHT + VK_CTRL)
#define STB_TEXTEDIT_K_PGUP 0x00014000 //KEY_DOWN(VK_PGUP)
#define STB_TEXTEDIT_K_PGDOWN 0x00018000 //KEY_DOWN(VK_PGDN)

#define STB_TEXTEDIT_IMPLEMENTATION
#include <stb/stb_textedit.h>

#endif //__DAVA_UITEXTFIELD2STBBIND_H__