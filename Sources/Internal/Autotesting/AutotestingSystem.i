/* File : AutotestingSystem.i */
%module AutotestingSystem

%import AutotestingSystemConfig.h

%import Base/Singleton.h
%import Base/BaseTypes.h

%{
#include "AutotestingSystemLua.h"
#include "AutotestingSystem.h"
%}

%template(Singleton_Autotesting) DAVA::Singleton<DAVA::AutotestingSystemLua>;

/* Let's just grab the original header file here */
%include "std_string.i"
%import "UIControl.i"
%import "UI/UIList.h"
%import "UI/UIEvent.h"

%include "KeyedArchive.i"
%include "AutotestingSystemLua.h"

namespace DAVA
{
    class AutotestingSystem
    {
        public:
            AutotestingSystem();
            ~AutotestingSystem();

            String GetDeviceName();
            String GetPlatform();

            bool IsPhoneScreen();
    };
};