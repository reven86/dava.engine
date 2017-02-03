#pragma once

#include "ExclusiveSet.h"

class BinaryExclusiveSet : public ExclusiveSet
{
public:
    BinaryExclusiveSet(ExclusiveSetListener& listener, DAVA::Font* font, const DAVA::WideString& onOptionText, const DAVA::WideString& offOptionText);
    void SetOn(bool isOn);
    bool IsOn() const;
};
