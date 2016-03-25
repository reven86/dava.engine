#ifndef __DAVA_UITextField2Impl_h__
#define __DAVA_UITextField2Impl_h__

#include "UI/Private/IUITextField2Impl.h"

namespace DAVA
{
class UITextField2;

class UITextField2Impl : public IUITextField2Impl
{
public:
    UITextField2Impl(UITextField2* weakPtr);

    void Focus() override;
    void Unfocus() override;

    void OpenKeyboard() override;
    void CloseKeyboard() override;
    void SetAutoCapitalizationType(int32 value) override;
    void SetAutoCorrectionType(int32 value) override;
    void SetSpellCheckingType(int32 value) override;
    void SetKeyboardAppearanceType(int32 value) override;
    void SetKeyboardType(int32 value) override;
    void SetReturnKeyType(int32 value) override;
    void SetEnableReturnKeyAutomatically(bool value) override;

};

}

#endif // __DAVA_UITextField2Impl_h__
