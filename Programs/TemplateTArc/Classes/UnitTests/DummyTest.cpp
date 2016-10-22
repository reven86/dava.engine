#include "Testing/TArcUnitTests.h"

#include "LibraryModule.h"

class MockClass
{
public:
    MOCK_CONST_METHOD0(GetValue, int());
};

DAVA_TARC_TESTCLASS(DummyTest)
{
    BEGIN_FILES_COVERED_BY_TESTS()
        FIND_FILES_IN_TARGET(TemplateTArc)
        DECLARE_COVERED_FILES("LibraryModule.cpp")
    END_FILES_COVERED_BY_TESTS()

    BEGIN_TESTED_MODULES()
        DECLARE_TESTED_MODULE(LibraryModule)
    END_TESTED_MODULES()

    DAVA_TEST(DummyTestCase)
    {
        EXPECT_CALL(mock, GetValue()).WillOnce(::testing::Return(1));
        mock.GetValue();
    }
    
    MockClass mock;
};