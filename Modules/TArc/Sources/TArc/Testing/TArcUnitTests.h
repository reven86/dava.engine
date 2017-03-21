#pragma once

#include "TArc/Testing/TArcTestClass.h"
#include "UnitTests/UnitTests.h"

#include <gmock/gmock.h>

#define DAVA_TARC_TESTCLASS(classname) \
    DAVA_TESTCLASS_CUSTOM_BASE_AND_FACTORY(classname, DAVA::TArc::TestClass, DAVA::TArc::TestClassHolderFactory)

#define BEGIN_TESTED_MODULES() \
    void CreateTestedModules() override {

#define DECLARE_TESTED_MODULE(moduleTypeName) \
        core->CreateModule<moduleTypeName>();

#define END_TESTED_MODULES() \
    }
