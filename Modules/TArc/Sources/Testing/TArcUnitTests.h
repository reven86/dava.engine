#pragma once

#include "Testing/TArcTestClass.h"
#include "Testing/TArcTestClassFactory.h"
#include "UnitTests/UnitTests.h"

#define DAVA_TARC_TESTCLASS(classname) \
    DAVA_TESTCLASS_CUSTOM_BASE(classname, DAVA::TArc::TestClass)
