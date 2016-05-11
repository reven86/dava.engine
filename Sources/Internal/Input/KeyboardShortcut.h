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

#ifndef __DAVAENGINE_KEYBOARD_SHORTCUT_H__
#define __DAVAENGINE_KEYBOARD_SHORTCUT_H__

#include "Base/BaseTypes.h"

#include "KeyboardDevice.h"

namespace DAVA
{
class KeyboardShortcut final
{
public:
    enum Modifier
    {
        MODIFIER_SHIFT = 0x01,
        MODIFIER_CTRL = 0x02,
        MODIFIER_ALT = 0x04,
        MODIFIER_WIN = 0x08,

        LAST_MODIFIER = 0x08
    };

    KeyboardShortcut();
    KeyboardShortcut(const KeyboardShortcut& shortcut);
    KeyboardShortcut(Key key, int32 modifiers = 0);
    KeyboardShortcut(const String& str);

    ~KeyboardShortcut();

    KeyboardShortcut& operator=(const KeyboardShortcut& shortcut);
    bool operator==(const KeyboardShortcut& other) const;
    bool operator!=(const KeyboardShortcut& other) const;

    Key GetKey() const;
    int32 GetModifiers() const;

    String ToString() const;

    static int32 ConvertKeyToModifier(Key key);

private:
    Key key = Key::UNKNOWN;
    int32 modifiers = 0;
};
}

namespace std
{
template <>
struct hash<DAVA::KeyboardShortcut>
{
    size_t operator()(const DAVA::KeyboardShortcut& shortcut) const
    {
        return static_cast<size_t>(shortcut.GetKey()) | (static_cast<size_t>(shortcut.GetModifiers()) << 16);
    }
};
}

#endif //__DAVAENGINE_KEYBOARD_SHORTCUT_H__