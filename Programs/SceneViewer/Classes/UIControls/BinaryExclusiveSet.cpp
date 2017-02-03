#include "BinaryExclusiveSet.h"

namespace BinaryExclusiveSetDetails
{
ExclusiveSet::OptionID OptionOn = 0;
ExclusiveSet::OptionID OptionOff = 1;
}

BinaryExclusiveSet::BinaryExclusiveSet(ExclusiveSetListener& listener, DAVA::Font* font, const DAVA::WideString& onOptionText, const DAVA::WideString& offOptionText)
    : ExclusiveSet(listener, font)
{
    AddOption(BinaryExclusiveSetDetails::OptionOn, onOptionText, true);
    AddOption(BinaryExclusiveSetDetails::OptionOff, offOptionText);
}

void BinaryExclusiveSet::SetOn(bool isOn)
{
    SetOptionSelected(isOn ? BinaryExclusiveSetDetails::OptionOn : BinaryExclusiveSetDetails::OptionOff);
}

bool BinaryExclusiveSet::IsOn() const
{
    return (GetSelectedOptionID() == BinaryExclusiveSetDetails::OptionOn);
}
