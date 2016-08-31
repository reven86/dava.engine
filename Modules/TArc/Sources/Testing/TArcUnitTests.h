#pragma once

#include "Testing/TArcTestClass.h"
#include "Testing/TArcTestClassFactory.h"
#include "UnitTests/UnitTests.h"

#define DAVA_TARC_TESTCLASS(classname) \
    DAVA_TESTCLASS_CUSTOM_BASE_AND_FACTORY(classname, DAVA::TArc::TestClass, DAVA::TArc::TestClassFactory<classname>)
