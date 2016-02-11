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


#include "KeyboardDevice.h"

#include <algorithm>

namespace DAVA
{
static const char* keys[] =
{
  "UNKNOWN",
  "ESCAPE",
  "BACKSPACE",
  "TAB",
  "ENTER",
  "SPACE",
  "LSHIFT",
  "LCTRL",
  "LALT",

  "LWIN",
  "RWIN",
  "APPS",

  "PAUSE",
  "CAPSLOCK",
  "NUMLOCK",
  "SCROLLLOCK",

  "PGUP",
  "PGDN",
  "HOME",
  "END",
  "INSERT",
  "DELETE",

  "LEFT",
  "UP",
  "RIGHT",
  "DOWN",

  "0",
  "1",
  "2",
  "3",
  "4",
  "5",
  "6",
  "7",
  "8",
  "9",

  "A",
  "B",
  "C",
  "D",
  "E",
  "F",
  "G",
  "H",
  "I",
  "J",
  "K",
  "L",
  "M",
  "N",
  "O",
  "P",
  "Q",
  "R",
  "S",
  "T",
  "U",
  "V",
  "W",
  "X",
  "Y",
  "Z",

  "GRAVE",
  "MINUS",
  "EQUALS",
  "BACKSLASH",
  "LBRACKET",
  "RBRACKET",
  "SEMICOLON",
  "APOSTROPHE",
  "COMMA",
  "PERIOD",
  "SLASH",

  "NUMPAD0",
  "NUMPAD1",
  "NUMPAD2",
  "NUMPAD3",
  "NUMPAD4",
  "NUMPAD5",
  "NUMPAD6",
  "NUMPAD7",
  "NUMPAD8",
  "NUMPAD9",

  "MULTIPLY",
  "DIVIDE",
  "ADD",
  "SUBTRACT",
  "DECIMAL",

  "F1",
  "F2",
  "F3",
  "F4",
  "F5",
  "F6",
  "F7",
  "F8",
  "F9",
  "F10",
  "F11",
  "F12",

  "BACK",
  "MENU",

  "NONUSBACKSLASH",
  "NUMPADENTER",
  "PRINTSCREEN",
  "RSHIFT",
  "RCTRL",
  "RALT",

  "F13", // on mac - printscreen
  "F14", // on mac - scrlock
  "F15", // on mac - pause/break
  "F16",
  "F17",
  "F18",
  "F19"
};

KeyboardDevice::KeyboardDevice()
{
    static_assert(static_cast<size_t>(Key::TOTAL_KEYS_COUNT) < MAX_KEYS, "check array size");
    DVASSERT(static_cast<size_t>(Key::TOTAL_KEYS_COUNT) == keyNames.size());

    std::copy(keys, keys + keyNames.size(), keyNames.begin());

    ClearAllKeys();
    PrepareKeyTranslator();
}

KeyboardDevice::~KeyboardDevice()
{
}

bool KeyboardDevice::IsKeyPressed(Key key) const
{
    return currentFrameKeyStatus[static_cast<unsigned>(key)];
}

const String& KeyboardDevice::GetKeyName(Key key)
{
    return keyNames[static_cast<unsigned>(key)];
}

void KeyboardDevice::OnKeyPressed(Key key)
{
    unsigned index = static_cast<unsigned>(key);
    currentFrameKeyStatus[index] = true;
    realKeyStatus[index] = true;
}

void KeyboardDevice::OnKeyUnpressed(Key key)
{
    realKeyStatus[static_cast<unsigned>(key)] = false;
}

void KeyboardDevice::OnFinishFrame()
{
    currentFrameKeyStatus = realKeyStatus;
}

Key KeyboardDevice::GetDavaKeyForSystemKey(uint32 systemKeyCode) const
{
    if (systemKeyCode < MAX_KEYS)
    {
        return keyTranslator[systemKeyCode];
    }
    DVASSERT(false && "bad system key code");
    return Key::UNKNOWN;
}

void KeyboardDevice::PrepareKeyTranslator()
{
    std::uninitialized_fill(begin(keyTranslator), end(keyTranslator), Key::UNKNOWN);

#if defined(__DAVAENGINE_WINDOWS__)
    // see https://msdn.microsoft.com/en-us/library/windows/desktop/dd375731%28v=vs.85%29.aspx

    keyTranslator[256 + VK_LEFT] = Key::LEFT; // extended key
    keyTranslator[256 + VK_RIGHT] = Key::RIGHT; // extended key
    keyTranslator[256 + VK_UP] = Key::UP; // extended key
    keyTranslator[256 + VK_DOWN] = Key::DOWN; // extended key
    keyTranslator[VK_ESCAPE] = Key::ESCAPE;
    keyTranslator[256 + VK_DELETE] = Key::DELETE; // extended key
    keyTranslator[VK_BACK] = Key::BACKSPACE;
    keyTranslator[VK_RETURN] = Key::ENTER;
    keyTranslator[256 + VK_RETURN] = Key::NUMPADENTER;

    keyTranslator[VK_CONTROL] = Key::LCTRL;
    keyTranslator[VK_MENU] = Key::LALT;
    keyTranslator[VK_SHIFT] = Key::LSHIFT;
    keyTranslator[VK_APPS] = Key::APPS;

    keyTranslator[256 + VK_LWIN] = Key::LWIN; // extended key
    keyTranslator[256 + VK_RWIN] = Key::RWIN; // extended key

    keyTranslator[256 + VK_CONTROL] = Key::RCTRL;
    keyTranslator[256 + VK_MENU] = Key::RALT;
    keyTranslator[256 + VK_SHIFT] = Key::RSHIFT;
    keyTranslator[256 + VK_APPS] = Key::APPS; // win api mark this key as extended

    keyTranslator[256 + VK_NUMLOCK] = Key::NUMLOCK;
    keyTranslator[VK_CAPITAL] = Key::CAPSLOCK;
    keyTranslator[VK_PAUSE] = Key::PAUSE;
    keyTranslator[VK_SCROLL] = Key::SCROLLLOCK;
    keyTranslator[256 + VK_SNAPSHOT] = Key::PRINTSCREEN;
    keyTranslator[VK_SPACE] = Key::SPACE;
    keyTranslator[VK_TAB] = Key::TAB;
    keyTranslator[VK_ADD] = Key::ADD;
    keyTranslator[VK_SUBTRACT] = Key::SUBTRACT;
    keyTranslator[256 + VK_HOME] = Key::HOME; // extended key
    keyTranslator[256 + VK_END] = Key::END; // extended key
    keyTranslator[256 + VK_PRIOR] = Key::PGUP; // extended key
    keyTranslator[256 + VK_NEXT] = Key::PGDN; // extended key
    keyTranslator[256 + VK_INSERT] = Key::INSERT; // extended key

    keyTranslator[VK_OEM_PLUS] = Key::EQUALS;
    keyTranslator[VK_OEM_MINUS] = Key::MINUS;
    keyTranslator[VK_OEM_PERIOD] = Key::PERIOD;
    keyTranslator[VK_OEM_COMMA] = Key::COMMA;
    keyTranslator[VK_OEM_1] = Key::SEMICOLON;
    keyTranslator[VK_OEM_2] = Key::SLASH;
    keyTranslator[VK_OEM_3] = Key::GRAVE;
    keyTranslator[VK_OEM_4] = Key::LBRACKET;
    keyTranslator[VK_OEM_5] = Key::BACKSLASH;
    keyTranslator[VK_OEM_6] = Key::RBRACKET;
    keyTranslator[VK_OEM_7] = Key::APOSTROPHE;

    keyTranslator[VK_OEM_102] = Key::NONUSBACKSLASH;

    const unsigned numFuncKeys = static_cast<unsigned>(Key::F12) - static_cast<unsigned>(Key::F1);
    for (unsigned i = 0; i <= numFuncKeys; i++)
    {
        unsigned keyValue = static_cast<unsigned>(Key::F1) + i;
        keyTranslator[VK_F1 + i] = static_cast<Key>(keyValue);
    }

    // alpha keys
    for (unsigned i = 0; i < 26; ++i)
    {
        unsigned keyValue = static_cast<unsigned>(Key::KEY_A) + i;
        keyTranslator[0x41 + i] = static_cast<Key>(keyValue);
    }

    // numeric keys & keys at num pad
    for (unsigned i = 0; i < 10; ++i)
    {
        unsigned keyNum = static_cast<unsigned>(Key::KEY_0) + i;
        unsigned keyNumpad = static_cast<unsigned>(Key::NUMPAD0) + i;
        keyTranslator[0x30 + i] = static_cast<Key>(keyNum);
        keyTranslator[0x60 + i] = static_cast<Key>(keyNumpad);
    }
    keyTranslator[VK_MULTIPLY] = Key::MULTIPLY;
    keyTranslator[256 + VK_DIVIDE] = Key::DIVIDE; // extended key
    keyTranslator[VK_DECIMAL] = Key::DECIMAL;
#endif

#if defined(__DAVAENGINE_MACOS__)
    keyTranslator[0x7B] = Key::LEFT;
    keyTranslator[0x7C] = Key::RIGHT;
    keyTranslator[0x7E] = Key::UP;
    keyTranslator[0x7D] = Key::DOWN;
    keyTranslator[0x75] = Key::DELETE;
    keyTranslator[0x35] = Key::ESCAPE;
    keyTranslator[0x33] = Key::BACKSPACE;
    keyTranslator[0x24] = Key::ENTER;
    keyTranslator[0x30] = Key::TAB;

    keyTranslator[59] = Key::LCTRL;
    keyTranslator[58] = Key::LALT;
    keyTranslator[56] = Key::LSHIFT;
    keyTranslator[62] = Key::RCTRL;
    keyTranslator[61] = Key::RALT;
    keyTranslator[60] = Key::RSHIFT;

    keyTranslator[57] = Key::CAPSLOCK;
    keyTranslator[55] = Key::LWIN; // LGUI in SDL
    keyTranslator[0x31] = Key::SPACE;

    // from SDL2 scancodes_darwin.h
    keyTranslator[10] = Key::NONUSBACKSLASH;
    keyTranslator[24] = Key::EQUALS;
    keyTranslator[27] = Key::MINUS;
    keyTranslator[47] = Key::PERIOD;
    keyTranslator[43] = Key::COMMA;
    keyTranslator[41] = Key::SEMICOLON;
    keyTranslator[44] = Key::SLASH;
    keyTranslator[50] = Key::GRAVE;
    keyTranslator[33] = Key::LBRACKET;
    keyTranslator[42] = Key::BACKSLASH;
    keyTranslator[30] = Key::RBRACKET;
    keyTranslator[39] = Key::APOSTROPHE;
    keyTranslator[114] = Key::INSERT;
    keyTranslator[115] = Key::HOME;
    keyTranslator[116] = Key::PGUP;
    keyTranslator[119] = Key::END;
    keyTranslator[121] = Key::PGDN;
    keyTranslator[69] = Key::ADD;
    keyTranslator[78] = Key::MINUS;
    keyTranslator[67] = Key::MULTIPLY;
    keyTranslator[75] = Key::DIVIDE;
    keyTranslator[81] = Key::EQUALS;
    keyTranslator[65] = Key::PERIOD;

    keyTranslator[0x00] = Key::KEY_A;
    keyTranslator[0x0B] = Key::KEY_B;
    keyTranslator[0x08] = Key::KEY_C;
    keyTranslator[0x02] = Key::KEY_D;
    keyTranslator[0x0E] = Key::KEY_E;
    keyTranslator[0x03] = Key::KEY_F;
    keyTranslator[0x05] = Key::KEY_G;
    keyTranslator[0x04] = Key::KEY_H;
    keyTranslator[0x22] = Key::KEY_I;
    keyTranslator[0x26] = Key::KEY_J;
    keyTranslator[0x28] = Key::KEY_K;
    keyTranslator[0x25] = Key::KEY_L;
    keyTranslator[0x2D] = Key::KEY_N;
    keyTranslator[0x2E] = Key::KEY_M;
    keyTranslator[0x1F] = Key::KEY_O;
    keyTranslator[0x23] = Key::KEY_P;
    keyTranslator[0x0C] = Key::KEY_Q;
    keyTranslator[0x0F] = Key::KEY_R;
    keyTranslator[0x01] = Key::KEY_S;
    keyTranslator[0x11] = Key::KEY_T;
    keyTranslator[0x20] = Key::KEY_U;
    keyTranslator[0x09] = Key::KEY_V;
    keyTranslator[0x0D] = Key::KEY_W;
    keyTranslator[0x07] = Key::KEY_X;
    keyTranslator[0x10] = Key::KEY_Y;
    keyTranslator[0x06] = Key::KEY_Z;

    keyTranslator[0x1D] = Key::KEY_0;
    keyTranslator[0x12] = Key::KEY_1;
    keyTranslator[0x13] = Key::KEY_2;
    keyTranslator[0x14] = Key::KEY_3;
    keyTranslator[0x15] = Key::KEY_4;
    keyTranslator[0x17] = Key::KEY_5;
    keyTranslator[0x16] = Key::KEY_6;
    keyTranslator[0x1A] = Key::KEY_7;
    keyTranslator[0x1C] = Key::KEY_8;
    keyTranslator[0x19] = Key::KEY_9;
    keyTranslator[0x1B] = Key::MINUS;
    keyTranslator[0x18] = Key::EQUALS;

    keyTranslator[0x7A] = Key::F1;
    keyTranslator[0x78] = Key::F2;
    keyTranslator[0x76] = Key::F4;
    keyTranslator[0x60] = Key::F5;
    keyTranslator[0x61] = Key::F6;
    keyTranslator[0x62] = Key::F7;
    keyTranslator[0x63] = Key::F3;
    keyTranslator[0x64] = Key::F8;
    keyTranslator[0x65] = Key::F9;
    keyTranslator[0x6D] = Key::F10;
    keyTranslator[0x67] = Key::F11;
    keyTranslator[0x6F] = Key::F12;

    keyTranslator[113] = Key::F15;
    keyTranslator[0x6A] = Key::F16;
    keyTranslator[0x40] = Key::F17;
    keyTranslator[0x4F] = Key::F18;
    keyTranslator[0x50] = Key::F19;

    // numeric keys at numpad
    for (unsigned i = 0; i < 8; ++i)
    {
        keyTranslator[0x52 + i] = static_cast<Key>(static_cast<unsigned>(Key::NUMPAD0) + i);
    }
    keyTranslator[91] = Key::NUMPAD8;
    keyTranslator[92] = Key::NUMPAD9;

    keyTranslator[71] = Key::NUMLOCK;
    keyTranslator[76] = Key::NUMPADENTER;
    keyTranslator[65] = Key::DECIMAL;
    keyTranslator[110] = Key::APPS;
    keyTranslator[107] = Key::SCROLLLOCK;
    keyTranslator[105] = Key::PRINTSCREEN;
#endif
    
#if defined(__DAVAENGINE_ANDROID__)
    keyTranslator[0x04] = Key::BACK;
    keyTranslator[0x52] = Key::MENU;
#endif
}

void KeyboardDevice::ClearAllKeys()
{
    currentFrameKeyStatus.reset();
    realKeyStatus.reset();
}
};
