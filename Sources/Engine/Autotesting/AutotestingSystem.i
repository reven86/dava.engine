/* File : AutotestingSystem.i */
%module AutotestingSystem

%import Config.h

%import Base/Singleton.h
%import Base/BaseTypes.h



%{
#include "AutotestingSystemLua.h"

%}

%template(Singleton_Autotesting) DAVA::Singleton<DAVA::AutotestingSystemLua>;

/* Let's just grab the original header file here */
%include "std_string.i"

%import "UIControl.i"
%import "UI/UIList.h"
%import "UI/UIEvent.h"
%include "KeyedArchive.i"

%include "AutotestingSystemLua.h"