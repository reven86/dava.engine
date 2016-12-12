#pragma once

#include "ModuleManager/IModule.h"

#define EXPORT __attribute__((visibility("default")))


extern "C" {

DAVA::IModule* CreatPlugin( DAVA::Engine* );
void DestroyPlugin(  DAVA::IModule* );


}

