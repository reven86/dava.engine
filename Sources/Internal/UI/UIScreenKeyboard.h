//
//  UIScreenKeyboard.h
//  TestBed
//
//  Created by m_polubisok on 3/21/16.
//
//

#ifndef __DAVA_UISCREENKEYBOARD_H__
#define __DAVA_UISCREENKEYBOARD_H__

#include "Base/BaseTypes.h"

namespace DAVA
{
class UIScreenKeyboard
{
public:
    void OpenKeyboard();
    void CloseKeyboard();

    void SetAutoCapitalizationType(int32 value){};
    void SetAutoCorrectionType(int32 value){};
    void SetSpellCheckingType(int32 value){};
    void SetKeyboardAppearanceType(int32 value){};
    void SetKeyboardType(int32 value){};
    void SetReturnKeyType(int32 value){};
    void SetEnableReturnKeyAutomatically(bool value){};
};
}

#endif /* __DAVA_UISCREENKEYBOARD_H__ */
