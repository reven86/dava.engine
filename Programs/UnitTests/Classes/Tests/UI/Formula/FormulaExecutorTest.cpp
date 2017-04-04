#include "DAVAEngine.h"

#include "UI/Formula/FormulaParser.h"
#include "UI/Formula/FormulaExecutor.h"
#include "UI/Formula/FormulaError.h"

#include "UnitTests/UnitTests.h"

using namespace DAVA;

DAVA_TESTCLASS (FormulaExecutorTest)
{
    void SetUp(const String& testName) override
    {
    }

    void TearDown(const String& testName) override
    {
    }

    // FormulaTokenizer::ReadToken
    DAVA_TEST (ReadStringsAndIdentifiers)
    {
        FormulaExecutor executor;
    }
};
