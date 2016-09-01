#include "Testing/TArcUnitTests.h"

#include "LibraryModule.h"

class MockClass
{
public:
    MOCK_CONST_METHOD0(GetValue, int());
};

DAVA_TARC_TESTCLASS(DummyTest)
{
    BEGIN_CLASSES_COVERED_BY_TESTS()
        DECLARE_COVERED_FILES("LibraryModule")
    END_FILES_COVERED_BY_TESTS()

    BEGIN_TESTED_MODULES()
        DECLARE_TESTED_MODULE(LibraryModule)
    END_TESTED_MODULES()

    DAVA_TEST(DummyTestCase)
    {
        MockClass* mock = new MockClass();
        EXPECT_CALL(*mock, GetValue()).WillOnce(::testing::Return(1));
        mock->GetValue();
    }
};