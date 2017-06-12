#pragma once

#include <TArc/DataProcessing/DataNode.h>

class PreferencesData : public DAVA::TArc::DataNode
{
public:
    bool IsGuidesEnabled() const;

    static DAVA::FastName guidesEnabledPropertyName;

private:
    void SetGuidesEnabled(bool value);

    DAVA_VIRTUAL_REFLECTION(PreferencesData, DAVA::TArc::DataNode);
};
