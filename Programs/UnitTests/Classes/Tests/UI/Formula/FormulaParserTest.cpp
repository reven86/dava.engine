#include "DAVAEngine.h"

#include "UI/Formula/FormulaParser.h"

#include "UnitTests/UnitTests.h"

using namespace DAVA;

DAVA_TESTCLASS (FormulaParserTest)
{

    void SetUp(const String& testName) override
    {

    }
    
    void TearDown(const String& testName) override
    {

    }

    // FormulaTokenizer::ReadToken
    DAVA_TEST (Read)
    {
        FormulaParser parser("5 + 5");

        std::shared_ptr<FormulaExpression> exp = parser.ParseExpression();
    }
    
};
