#ifndef __DAVA__IUITextField2Impl_h__
#define __DAVA__IUITextField2Impl_h__

#include "Base/BaseTypes.h"

namespace DAVA
{
class IUITextField2Impl
{
public:
    virtual void Focus() = 0;
    virtual void Unfocus() = 0;

    virtual void OpenKeyboard() = 0;
    virtual void CloseKeyboard() = 0;
    virtual void SetAutoCapitalizationType(int32 value) = 0;
    virtual void SetAutoCorrectionType(int32 value) = 0;
    virtual void SetSpellCheckingType(int32 value) = 0;
    virtual void SetKeyboardAppearanceType(int32 value) = 0;
    virtual void SetKeyboardType(int32 value) = 0;
    virtual void SetReturnKeyType(int32 value) = 0;
    virtual void SetEnableReturnKeyAutomatically(bool value) = 0;
};

}

#endif // __DAVA__IUITextField2Impl_h__
