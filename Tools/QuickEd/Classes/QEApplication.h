#pragma once

#include "NgtTools/Application/NGTApplication.h"

class QEApplication : public NGTLayer::BaseApplication
{
public:
    QEApplication(int argc, char** argv);

protected:
    void GetPluginsForLoad(DAVA::Vector<DAVA::WideString>& names) const override;
    void OnPostLoadPugins() override;
};
