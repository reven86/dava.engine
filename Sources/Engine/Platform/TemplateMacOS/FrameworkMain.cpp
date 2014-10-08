#include "FrameworkMain.h"

FrameworkMain* FrameworkMain::_framework_main = 0;

void FrameworkMain::SetHandle( FrameworkMain* framework_main )
{
    _framework_main = framework_main;
}

FrameworkMain* FrameworkMain::GetHandle( )
{
    return _framework_main;
}



