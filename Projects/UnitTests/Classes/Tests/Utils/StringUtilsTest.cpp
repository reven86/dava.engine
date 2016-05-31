#include "DAVAEngine.h"

#include "Utils/StringUtils.h"
#include "UnitTests/UnitTests.h"

using namespace DAVA;

DAVA_TESTCLASS (StringUtilsTest)
{
    // Utils::Trim
    DAVA_TEST (TrimTestFunction)
    {
        TEST_VERIFY(StringUtils::Trim("abc") == "abc");
        TEST_VERIFY(StringUtils::Trim("   abc") == "abc");
        TEST_VERIFY(StringUtils::Trim("abc     ") == "abc");
        TEST_VERIFY(StringUtils::Trim("   abc    ") == "abc");
        TEST_VERIFY(StringUtils::Trim("\tabc\t") == "abc");
        TEST_VERIFY(StringUtils::Trim("    ") == "");
        TEST_VERIFY(StringUtils::Trim("\t\t") == "");

        TEST_VERIFY(StringUtils::TrimLeft("abc") == "abc");
        TEST_VERIFY(StringUtils::TrimLeft("   abc") == "abc");
        TEST_VERIFY(StringUtils::TrimLeft("abc     ") == "abc     ");
        TEST_VERIFY(StringUtils::TrimLeft("   abc    ") == "abc    ");
        TEST_VERIFY(StringUtils::TrimLeft("\tabc\t") == "abc\t");
        TEST_VERIFY(StringUtils::TrimLeft("    ") == "");
        TEST_VERIFY(StringUtils::TrimLeft("\t\t") == "");

        TEST_VERIFY(StringUtils::TrimRight("abc") == "abc");
        TEST_VERIFY(StringUtils::TrimRight("   abc") == "   abc");
        TEST_VERIFY(StringUtils::TrimRight("abc     ") == "abc");
        TEST_VERIFY(StringUtils::TrimRight("   abc    ") == "   abc");
        TEST_VERIFY(StringUtils::TrimRight("\tabc\t") == "\tabc");
        TEST_VERIFY(StringUtils::TrimRight("    ") == "");
        TEST_VERIFY(StringUtils::TrimRight("\t\t") == "");
    }
};
