#pragma once

#include "NgtTools/Application/NGTApplication.h"

class Application : public NGTLayer::BaseApplication
{
public:
    Application(int argc, char** argv);

protected:
    void GetPluginsForLoad(DAVA::Vector<DAVA::WideString>& names) const override;
};