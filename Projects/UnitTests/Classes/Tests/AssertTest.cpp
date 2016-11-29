#include "DAVAEngine.h"
#include "Utils/UTF8Utils.h"
#include "UnitTests/UnitTests.h"

DAVA_TESTCLASS (AssertTest)
{
    DAVA_TEST (TestFunction)
    {
        Vector<String> testData = {
            "THIS SOFTWARE IS PROVIDED BY THE DAVA",
            "هذا البرنامج يتم توفيرها من قبل DAVA",
            "Эта программа обеспечивается Dava",
        };
        for (size_t k = 0; k < testData.size(); ++k)
        {
            //String tmp =
            DVASSERT_MSG(false, testData[k].c_str());
        }
        TEST_VERIFY(false);
    }
};
