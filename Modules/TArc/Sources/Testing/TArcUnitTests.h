#pragma once

#include "Testing/TArcTestClass.h"
#include "UnitTests/UnitTests.h"

#include <gmock/gmock.h>

#define DAVA_TARC_TESTCLASS(classname) \
    DAVA_TESTCLASS_CUSTOM_BASE(classname, DAVA::TArc::TestClass)

#define BEGIN_TESTED_MODULES() \
    void CreateTestedModules() override {

#define DECLARE_TESTED_MODULE(moduleTypeName) \
        core->CreateModule<moduleTypeName>();

#define END_TESTED_MODULES() \
    }