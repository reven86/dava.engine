#pragma once

#include "LockedButton.h"

#include <UI/UIControl.h>
#include <Base/BaseTypes.h>

class ExclusiveSet;
class ExclusiveSetListener
{
public:
    virtual void OnOptionChanged(ExclusiveSet*) = 0;
};

class ExclusiveSet : public DAVA::UIControl, public LockedButtonHolder
{
public:
    using OptionID = DAVA::uint32;

    explicit ExclusiveSet(ExclusiveSetListener& listener, DAVA::Font* font);

    bool AddOption(OptionID optionId, const DAVA::WideString& text, bool toSelect = false);
    void SetOptionSelected(OptionID optionID);
    OptionID GetSelectedOptionID() const;

private:
    using OptionsMap = DAVA::UnorderedMap<OptionID, DAVA::ScopedPtr<LockedButton>>;

    void OnButtonPressed(LockedButton* button) override;
    OptionsMap::iterator FindOptionByValue(LockedButton* button);

private:
    ExclusiveSetListener& listener;
    DAVA::Font* font = nullptr;
    DAVA::float32 nextButtonX = 10.f;
    OptionsMap options;
    OptionsMap::iterator selectedOption = options.end();
};
