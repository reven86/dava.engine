#include "KeyboardDevice.h"

#include "UI/UIEvent.h"
#include "UI/UIControlSystem.h"
#include "Time/SystemTimer.h"

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

  "LCMD",
  "RCMD",
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

const Key KeyboardDevice::GetKeyByName(const String& name)
{
    for (size_t i = 0; i < keyNames.size(); i++)
    {
        if (keyNames[i] == name)
            return static_cast<Key>(i);
    }
    return Key::UNKNOWN;
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
        Key key = keyTranslator[systemKeyCode];
#if defined(ENABLE_CEF_WEBVIEW)
        backCodeTranslator[static_cast<int32>(key)] = systemKeyCode;
#endif
        return key;
    }
    DVASSERT(false && "bad system key code");
    return Key::UNKNOWN;
}

#if defined(ENABLE_CEF_WEBVIEW)
uint32 KeyboardDevice::GetSystemKeyForDavaKey(Key key) const
{
    DVASSERT(backCodeTranslator[static_cast<int32>(key)] != -1 && "Fail, before GetSystemKeyForDavaKey need add native code inside backCodeTranslator");
    return backCodeTranslator[static_cast<int32>(key)];
}
#endif

void KeyboardDevice::PrepareKeyTranslator()
{
    std::uninitialized_fill(begin(keyTranslator), end(keyTranslator), Key::UNKNOWN);
#if defined(ENABLE_CEF_WEBVIEW)
    std::uninitialized_fill(begin(backCodeTranslator), end(backCodeTranslator), -1);
#endif

#if defined(__DAVAENGINE_WINDOWS__)
    // see https://msdn.microsoft.com/en-us/library/windows/desktop/dd375731%28v=vs.85%29.aspx

    keyTranslator[VK_LEFT] = Key::LEFT;
    keyTranslator[VK_RIGHT] = Key::RIGHT;
    keyTranslator[VK_UP] = Key::UP;
    keyTranslator[VK_DOWN] = Key::DOWN;
    keyTranslator[VK_DELETE] = Key::DELETE;
    keyTranslator[VK_ESCAPE] = Key::ESCAPE;
    keyTranslator[VK_BACK] = Key::BACKSPACE;
    keyTranslator[VK_RETURN] = Key::ENTER;

    keyTranslator[256 + VK_LEFT] = Key::LEFT; // extended key
    keyTranslator[256 + VK_RIGHT] = Key::RIGHT; // extended key
    keyTranslator[256 + VK_UP] = Key::UP; // extended key
    keyTranslator[256 + VK_DOWN] = Key::DOWN; // extended key
    keyTranslator[256 + VK_DELETE] = Key::DELETE; // extended key
    keyTranslator[256 + VK_RETURN] = Key::NUMPADENTER; // extended key

    keyTranslator[VK_CONTROL] = Key::LCTRL;
    keyTranslator[VK_MENU] = Key::LALT;
    keyTranslator[VK_SHIFT] = Key::LSHIFT;
    keyTranslator[VK_APPS] = Key::APPS;

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

    keyTranslator[VK_HOME] = Key::HOME;
    keyTranslator[VK_END] = Key::END;
    keyTranslator[VK_PRIOR] = Key::PGUP;
    keyTranslator[VK_NEXT] = Key::PGDN;
    keyTranslator[VK_INSERT] = Key::INSERT;
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
    keyTranslator[54] = Key::RCMD;
    keyTranslator[55] = Key::LCMD; // LGUI in SDL
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
    keyTranslator[0] = Key::UNKNOWN;
    keyTranslator[1] = Key::LEFT;
    keyTranslator[2] = Key::RIGHT;
    keyTranslator[3] = Key::HOME;
    keyTranslator[4] = Key::BACK;
    //keyTranslator[5] = Key::CALL;
    //keyTranslator[6] = Key::ENDCALL;
    keyTranslator[7] = Key::KEY_0;
    keyTranslator[8] = Key::KEY_1;
    keyTranslator[9] = Key::KEY_2;
    keyTranslator[10] = Key::KEY_3;
    keyTranslator[11] = Key::KEY_4;
    keyTranslator[12] = Key::KEY_5;
    keyTranslator[13] = Key::KEY_6;
    keyTranslator[14] = Key::KEY_7;
    keyTranslator[15] = Key::KEY_8;
    keyTranslator[16] = Key::KEY_9;
    //keyTranslator[17] = Key::STAR;
    //keyTranslator[18] = Key::POUND;
    //keyTranslator[19] = Key::DPAD_UP;
    //keyTranslator[20] = Key::DPAD_DOWN;
    //keyTranslator[21] = Key::DPAD_LEFT;
    //keyTranslator[22] = Key::DPAD_RIGHT;
    //keyTranslator[23] = Key::DPAD_CENTER;
    //keyTranslator[24] = Key::VOLUME_UP;
    //keyTranslator[25] = Key::VOLUME_DOWN;
    //keyTranslator[26] = Key::POWER;
    //keyTranslator[27] = Key::CAMERA;
    //keyTranslator[28] = Key::CLEAR;
    keyTranslator[29] = Key::KEY_A;
    keyTranslator[30] = Key::KEY_B;
    keyTranslator[31] = Key::KEY_C;
    keyTranslator[32] = Key::KEY_D;
    keyTranslator[33] = Key::KEY_E;
    keyTranslator[34] = Key::KEY_F;
    keyTranslator[35] = Key::KEY_G;
    keyTranslator[36] = Key::KEY_H;
    keyTranslator[37] = Key::KEY_I;
    keyTranslator[38] = Key::KEY_J;
    keyTranslator[39] = Key::KEY_K;
    keyTranslator[40] = Key::KEY_L;
    keyTranslator[41] = Key::KEY_M;
    keyTranslator[42] = Key::KEY_N;
    keyTranslator[43] = Key::KEY_O;
    keyTranslator[44] = Key::KEY_P;
    keyTranslator[45] = Key::KEY_Q;
    keyTranslator[46] = Key::KEY_R;
    keyTranslator[47] = Key::KEY_S;
    keyTranslator[48] = Key::KEY_T;
    keyTranslator[49] = Key::KEY_U;
    keyTranslator[50] = Key::KEY_V;
    keyTranslator[51] = Key::KEY_W;
    keyTranslator[52] = Key::KEY_X;
    keyTranslator[53] = Key::KEY_Y;
    keyTranslator[54] = Key::KEY_Z;
    keyTranslator[55] = Key::COMMA;
    keyTranslator[56] = Key::PERIOD;
    keyTranslator[57] = Key::LALT;
    keyTranslator[58] = Key::RALT;
    keyTranslator[59] = Key::LSHIFT;
    keyTranslator[60] = Key::RSHIFT;
    keyTranslator[61] = Key::TAB;
    keyTranslator[62] = Key::SPACE;
    //keyTranslator[63] = Key::SYM;
    //keyTranslator[64] = Key::EXPLORER;
    //keyTranslator[65] = Key::ENVELOPE;
    keyTranslator[66] = Key::ENTER;
    keyTranslator[67] = Key::DELETE;
    keyTranslator[68] = Key::GRAVE;
    keyTranslator[69] = Key::MINUS;
    keyTranslator[70] = Key::EQUALS;
    keyTranslator[71] = Key::LBRACKET;
    keyTranslator[72] = Key::RBRACKET;
    keyTranslator[73] = Key::BACKSLASH;
    keyTranslator[74] = Key::SEMICOLON;
    keyTranslator[75] = Key::APOSTROPHE;
    keyTranslator[76] = Key::SLASH;
    //keyTranslator[77] = Key::AT;
    //keyTranslator[78] = Key::NUM;
    //keyTranslator[79] = Key::HEADSETHOOK;
    //keyTranslator[80] = Key::FOCUS;
    //keyTranslator[81] = Key::PLUS;
    keyTranslator[82] = Key::MENU;
    //keyTranslator[83] = Key::NOTIFICATION;
    //keyTranslator[84] = Key::SEARCH;
    //keyTranslator[85] = Key::MEDIA_PLAY_PAUSE;
    //keyTranslator[86] = Key::MEDIA_STOP;
    //keyTranslator[87] = Key::MEDIA_NEXT;
    //keyTranslator[88] = Key::MEDIA_PREVIOUS;
    //keyTranslator[89] = Key::MEDIA_REWIND;
    //keyTranslator[90] = Key::MEDIA_FAST_FORWARD;
    //keyTranslator[91] = Key::MUTE;
    keyTranslator[92] = Key::PGUP;
    keyTranslator[93] = Key::PGDN;
    //keyTranslator[94] = Key::PICTSYMBOLS;
    //keyTranslator[95] = Key::SWITCH_CHARSET;
    //keyTranslator[96] = Key::BUTTON_A;
    //keyTranslator[97] = Key::BUTTON_B;
    //keyTranslator[98] = Key::BUTTON_C;
    //keyTranslator[99] = Key::BUTTON_X;
    //keyTranslator[100] = Key::BUTTON_Y;
    //keyTranslator[101] = Key::BUTTON_Z;
    //keyTranslator[102] = Key::BUTTON_L1;
    //keyTranslator[103] = Key::BUTTON_R1;
    //keyTranslator[104] = Key::BUTTON_L2;
    //keyTranslator[105] = Key::BUTTON_R2;
    //keyTranslator[106] = Key::BUTTON_THUMBL;
    //keyTranslator[107] = Key::BUTTON_THUMBR;
    //keyTranslator[108] = Key::BUTTON_START;
    //keyTranslator[109] = Key::BUTTON_SELECT;
    //keyTranslator[110] = Key::BUTTON_MODE;
    keyTranslator[111] = Key::ESCAPE;
    //keyTranslator[112] = Key::FORWARD_DEL;
    keyTranslator[113] = Key::LCTRL;
    keyTranslator[114] = Key::RCTRL;
    keyTranslator[115] = Key::CAPSLOCK;
    keyTranslator[116] = Key::SCROLLLOCK;
    keyTranslator[117] = Key::LCMD;
    keyTranslator[118] = Key::RCMD;
    //keyTranslator[119] = Key::FUNCTION;
    //keyTranslator[120] = Key::SYSRQ;
    keyTranslator[121] = Key::PAUSE;
    keyTranslator[122] = Key::HOME;
    keyTranslator[123] = Key::END;
    keyTranslator[124] = Key::INSERT;
    //keyTranslator[125] = Key::FORWARD;
    //keyTranslator[126] = Key::MEDIA_PLAY;
    //keyTranslator[127] = Key::MEDIA_PAUSE;
    //keyTranslator[128] = Key::MEDIA_CLOSE;
    //keyTranslator[129] = Key::MEDIA_EJECT;
    //keyTranslator[130] = Key::MEDIA_RECORD;
    keyTranslator[131] = Key::F1;
    keyTranslator[132] = Key::F2;
    keyTranslator[133] = Key::F3;
    keyTranslator[134] = Key::F4;
    keyTranslator[135] = Key::F5;
    keyTranslator[136] = Key::F6;
    keyTranslator[137] = Key::F7;
    keyTranslator[138] = Key::F8;
    keyTranslator[139] = Key::F9;
    keyTranslator[140] = Key::F10;
    keyTranslator[141] = Key::F11;
    keyTranslator[142] = Key::F12;
    keyTranslator[143] = Key::NUMLOCK;
    keyTranslator[144] = Key::NUMPAD0;
    keyTranslator[145] = Key::NUMPAD1;
    keyTranslator[146] = Key::NUMPAD2;
    keyTranslator[147] = Key::NUMPAD3;
    keyTranslator[148] = Key::NUMPAD4;
    keyTranslator[149] = Key::NUMPAD5;
    keyTranslator[150] = Key::NUMPAD6;
    keyTranslator[151] = Key::NUMPAD7;
    keyTranslator[152] = Key::NUMPAD8;
    keyTranslator[153] = Key::NUMPAD9;
    keyTranslator[154] = Key::DIVIDE;
    keyTranslator[155] = Key::MULTIPLY;
    keyTranslator[156] = Key::SUBTRACT;
    keyTranslator[157] = Key::ADD;
    keyTranslator[158] = Key::DECIMAL;
    //keyTranslator[159] = Key::NUMPAD_COMMA;
    keyTranslator[160] = Key::NUMPADENTER;
    keyTranslator[161] = Key::EQUALS;
//keyTranslator[162] = Key::NUMPAD_LEFT_PAREN;
//keyTranslator[163] = Key::NUMPAD_RIGHT_PAREN;
//keyTranslator[164] = Key::VOLUME_MUTE;
//keyTranslator[165] = Key::INFO;
//keyTranslator[166] = Key::CHANNEL_UP;
//keyTranslator[167] = Key::CHANNEL_DOWN;
//keyTranslator[168] = Key::ZOOM_IN;
//keyTranslator[169] = Key::ZOOM_OUT;
//keyTranslator[170] = Key::TV;
//keyTranslator[171] = Key::WINDOW;
//keyTranslator[172] = Key::GUIDE;
//keyTranslator[173] = Key::DVR;
//keyTranslator[174] = Key::BOOKMARK;
//keyTranslator[175] = Key::CAPTIONS;
//keyTranslator[176] = Key::SETTINGS;
//keyTranslator[177] = Key::TV_POWER;
//keyTranslator[178] = Key::TV_INPUT;
//keyTranslator[179] = Key::STB_POWER;
//keyTranslator[180] = Key::STB_INPUT;
//keyTranslator[181] = Key::AVR_POWER;
//keyTranslator[182] = Key::AVR_INPUT;
//keyTranslator[183] = Key::PROG_RED;
//keyTranslator[184] = Key::PROG_GREEN;
//keyTranslator[185] = Key::PROG_YELLOW;
//keyTranslator[186] = Key::PROG_BLUE;
//keyTranslator[187] = Key::APP_SWITCH;
//    keyTranslator[188] = Key::BUTTON_1;
//    keyTranslator[189] = Key::BUTTON_2;
//    keyTranslator[190] = Key::BUTTON_3;
//    keyTranslator[191] = Key::BUTTON_4;
//    keyTranslator[192] = Key::BUTTON_5;
//    keyTranslator[193] = Key::BUTTON_6;
//    keyTranslator[194] = Key::BUTTON_7;
//    keyTranslator[195] = Key::BUTTON_8;
//    keyTranslator[196] = Key::BUTTON_9;
//    keyTranslator[197] = Key::BUTTON_10;
//    keyTranslator[198] = Key::BUTTON_11;
//    keyTranslator[199] = Key::BUTTON_12;
//    keyTranslator[200] = Key::BUTTON_13;
//    keyTranslator[201] = Key::BUTTON_14;
//    keyTranslator[202] = Key::BUTTON_15;
//    keyTranslator[203] = Key::BUTTON_16;
//keyTranslator[204] = Key::LANGUAGE_SWITCH;
//keyTranslator[205] = Key::MANNER_MODE;
//keyTranslator[206] = Key::3D_MODE;
//keyTranslator[207] = Key::CONTACTS;
//keyTranslator[208] = Key::CALENDAR;
//keyTranslator[209] = Key::MUSIC;
//keyTranslator[210] = Key::CALCULATOR;
//keyTranslator[211] = Key::ZENKAKU_HANKAKU;
//keyTranslator[212] = Key::EISU;
//keyTranslator[213] = Key::MUHENKAN;
//keyTranslator[214] = Key::HENKAN;
//keyTranslator[215] = Key::KATAKANA_HIRAGANA;
//keyTranslator[216] = Key::YEN;
//keyTranslator[217] = Key::RO;
//keyTranslator[218] = Key::KANA;
//keyTranslator[219] = Key::ASSIST;
//keyTranslator[220] = Key::BRIGHTNESS_DOWN;
//keyTranslator[221] = Key::BRIGHTNESS_UP;
//keyTranslator[222] = Key::MEDIA_AUDIO_TRACK;
//keyTranslator[223] = Key::SLEEP;
//keyTranslator[224] = Key::WAKEUP;
//keyTranslator[225] = Key::PAIRING;
//keyTranslator[226] = Key::MEDIA_TOP_MENU;
//keyTranslator[227] = Key::11;
//keyTranslator[228] = Key::12;
//keyTranslator[229] = Key::LAST_CHANNEL;
//keyTranslator[230] = Key::TV_DATA_SERVICE;
//keyTranslator[231] = Key::VOICE_ASSIST;
//keyTranslator[232] = Key::TV_RADIO_SERVICE;
//keyTranslator[233] = Key::TV_TELETEXT;
//    keyTranslator[234] = Key::TV_NUMBER_ENTRY;
//    keyTranslator[235] = Key::TV_TERRESTRIAL_ANALOG;
//    keyTranslator[236] = Key::TV_TERRESTRIAL_DIGITAL;
//    keyTranslator[237] = Key::TV_SATELLITE;
//    keyTranslator[238] = Key::TV_SATELLITE_BS;
//    keyTranslator[239] = Key::TV_SATELLITE_CS;
//    keyTranslator[240] = Key::TV_SATELLITE_SERVICE;
//    keyTranslator[241] = Key::TV_NETWORK;
//    keyTranslator[242] = Key::TV_ANTENNA_CABLE;
//    keyTranslator[243] = Key::TV_INPUT_HDMI_1;
//    keyTranslator[244] = Key::TV_INPUT_HDMI_2;
//    keyTranslator[245] = Key::TV_INPUT_HDMI_3;
//    keyTranslator[246] = Key::TV_INPUT_HDMI_4;
//    keyTranslator[247] = Key::TV_INPUT_COMPOSITE_1;
//    keyTranslator[248] = Key::TV_INPUT_COMPOSITE_2;
//    keyTranslator[249] = Key::TV_INPUT_COMPONENT_1;
//    keyTranslator[250] = Key::TV_INPUT_COMPONENT_2;
//    keyTranslator[251] = Key::TV_INPUT_VGA_1;
//    keyTranslator[252] = Key::TV_AUDIO_DESCRIPTION;
//    keyTranslator[253] = Key::TV_AUDIO_DESCRIPTION_MIX_UP;
//    keyTranslator[254] = Key::TV_AUDIO_DESCRIPTION_MIX_DOWN;
//    keyTranslator[255] = Key::TV_ZOOM_MODE;
//    keyTranslator[256] = Key::TV_CONTENTS_MENU;
//    keyTranslator[257] = Key::TV_MEDIA_CONTEXT_MENU;
//    keyTranslator[258] = Key::TV_TIMER_PROGRAMMING;
//    keyTranslator[259] = Key::HELP;
#endif
}

void KeyboardDevice::ClearAllKeys()
{
    currentFrameKeyStatus.reset();

    UIControlSystem* uiControlSys = UIControlSystem::Instance();
    if (uiControlSys != nullptr)
    {
        UIEvent e;
        e.phase = UIEvent::Phase::KEY_UP;
        e.device = eInputDevices::KEYBOARD;
        e.timestamp = SystemTimer::GetMs() / 1000.0;
        for (uint32 key = static_cast<uint32>(Key::ESCAPE); key < static_cast<uint32>(Key::TOTAL_KEYS_COUNT); key += 1)
        {
            if (realKeyStatus[key])
            {
                e.key = static_cast<Key>(key);
                uiControlSys->OnInput(&e);
            }
        }
    }

    realKeyStatus.reset();
}
} // end namespace DAVA
