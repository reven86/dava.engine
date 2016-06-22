#include "DAVAEngine.h"

#include "Utils/StringUtils.h"
#include "UnitTests/UnitTests.h"

using namespace DAVA;

DAVA_TESTCLASS (StringUtilsTest)
{
    // Utils::Trim
    DAVA_TEST (TrimTestFunction)
    {
        TEST_VERIFY(StringUtils::Trim(String("abc")) == "abc");
        TEST_VERIFY(StringUtils::Trim(String("   abc")) == "abc");
        TEST_VERIFY(StringUtils::Trim(String("abc     ")) == "abc");
        TEST_VERIFY(StringUtils::Trim(String("   abc    ")) == "abc");
        TEST_VERIFY(StringUtils::Trim(String("\tabc\t")) == "abc");
        TEST_VERIFY(StringUtils::Trim(String("    ")) == "");
        TEST_VERIFY(StringUtils::Trim(String("\t\t")) == "");

        TEST_VERIFY(StringUtils::TrimLeft(String("abc")) == "abc");
        TEST_VERIFY(StringUtils::TrimLeft(String("   abc")) == "abc");
        TEST_VERIFY(StringUtils::TrimLeft(String("abc     ")) == "abc     ");
        TEST_VERIFY(StringUtils::TrimLeft(String("   abc    ")) == "abc    ");
        TEST_VERIFY(StringUtils::TrimLeft(String("\tabc\t")) == "abc\t");
        TEST_VERIFY(StringUtils::TrimLeft(String("    ")) == "");
        TEST_VERIFY(StringUtils::TrimLeft(String("\t\t")) == "");

        TEST_VERIFY(StringUtils::TrimRight(String("abc")) == "abc");
        TEST_VERIFY(StringUtils::TrimRight(String("   abc")) == "   abc");
        TEST_VERIFY(StringUtils::TrimRight(String("abc     ")) == "abc");
        TEST_VERIFY(StringUtils::TrimRight(String("   abc    ")) == "   abc");
        TEST_VERIFY(StringUtils::TrimRight(String("\tabc\t")) == "\tabc");
        TEST_VERIFY(StringUtils::TrimRight(String("    ")) == "");
        TEST_VERIFY(StringUtils::TrimRight(String("\t\t")) == "");
    }

    DAVA_TEST (StartsWithTest)
    {
        TEST_VERIFY(StringUtils::StartsWith("abcdef", "abc") == true);
        TEST_VERIFY(StringUtils::StartsWith("abc", "abc") == true);
        TEST_VERIFY(StringUtils::StartsWith("ab", "abc") == false);
        TEST_VERIFY(StringUtils::StartsWith("", "abc") == false);
        TEST_VERIFY(StringUtils::StartsWith("abcdef", "ac") == false);
    }
};
