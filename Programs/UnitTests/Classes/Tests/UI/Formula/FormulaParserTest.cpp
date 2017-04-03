#include "DAVAEngine.h"

#include "UI/Formula/FormulaParser.h"
#include "UI/Formula/FormulaFormatter.h"

#include "UnitTests/UnitTests.h"

using namespace DAVA;

class FormulaExpressionStack : FormulaExpressionVisitor
{
public:
    ~FormulaExpressionStack()
    {
    }

    void Traverse(FormulaExpression* exp)
    {
        exp->Accept(this);
    }

    void Visit(FormulaValueExpression* exp) override
    {
        stack.push_back(FormulaFormatter::AnyToString(exp->GetValue()));
    }

    void Visit(FormulaNegExpression* exp) override
    {
        stack.push_back("-");
        exp->GetExp()->Accept(this);
    }

    void Visit(FormulaNotExpression* exp) override
    {
        stack.push_back("!");
        exp->GetExp()->Accept(this);
    }

    void Visit(FormulaBinaryOperatorExpression* exp) override
    {
        stack.push_back(FormulaFormatter::BinaryOpToString(exp->GetOperator()));
        exp->GetLhs()->Accept(this);
        exp->GetRhs()->Accept(this);
    }

    void Visit(FormulaFunctionExpression* exp) override
    {
        stack.push_back("func_" + exp->GetName());

        for (const std::shared_ptr<FormulaExpression>& paramExp : exp->GetParms())
        {
            paramExp->Accept(this);
        }
    }

    void Visit(FormulaFieldAccessExpression* exp) override
    {
        stack.push_back("field_" + exp->GetFieldName());
        if (exp->GetExp())
        {
            exp->GetExp()->Accept(this);
        }
    }

    void Visit(FormulaIndexExpression* exp) override
    {
        stack.push_back("index");
        exp->GetExp()->Accept(this);
        exp->GetIndexExp()->Accept(this);
    }

    Vector<String> stack;
};

DAVA_TESTCLASS (FormulaParserTest)
{

    void SetUp(const String& testName) override
    {

    }
    
    void TearDown(const String& testName) override
    {

    }

    // FormulaParser::ParseExpression
    DAVA_TEST (ParseBinaryOperations)
    {
        TEST_VERIFY(Parse("1 + 2") == Vector<String>({ "+", "1", "2" }));
        TEST_VERIFY(Parse("1 - 2") == Vector<String>({ "-", "1", "2" }));
        TEST_VERIFY(Parse("1 * 2") == Vector<String>({ "*", "1", "2" }));
        TEST_VERIFY(Parse("1 / 2") == Vector<String>({ "/", "1", "2" }));
        TEST_VERIFY(Parse("1 % 2") == Vector<String>({ "%", "1", "2" }));
        TEST_VERIFY(Parse("true && false") == Vector<String>({ "&&", "true", "false" }));
        TEST_VERIFY(Parse("true || false") == Vector<String>({ "||", "true", "false" }));
        TEST_VERIFY(Parse("true == false") == Vector<String>({ "==", "true", "false" }));
        TEST_VERIFY(Parse("true != false") == Vector<String>({ "!=", "true", "false" }));
        TEST_VERIFY(Parse("a <= b") == Vector<String>({ "<=", "field_a", "field_b" }));
        TEST_VERIFY(Parse("a < b") == Vector<String>({ "<", "field_a", "field_b" }));
        TEST_VERIFY(Parse("a >= b") == Vector<String>({ ">=", "field_a", "field_b" }));
        TEST_VERIFY(Parse("a > b") == Vector<String>({ ">", "field_a", "field_b" }));
    }

    // FormulaParser::ParseExpression
    DAVA_TEST (ParseUnaryOperations)
    {
        TEST_VERIFY(Parse("-a") == Vector<String>({ "-", "field_a" }));
        TEST_VERIFY(Parse("!b") == Vector<String>({ "!", "field_b" }));
    }

    // FormulaParser::ParseExpression
    DAVA_TEST (ParsePriorities)
    {
        TEST_VERIFY(Parse("1 + 2 - 3") == Vector<String>({ "-", "+", "1", "2", "3" }));
        TEST_VERIFY(Parse("1 * 2 / 3") == Vector<String>({ "/", "*", "1", "2", "3" }));
        TEST_VERIFY(Parse("1 + 2 / 3") == Vector<String>({ "+", "1", "/", "2", "3" }));
        TEST_VERIFY(Parse("1 + 2 < 3 - 4") == Vector<String>({ "<", "+", "1", "2", "-", "3", "4" }));
        TEST_VERIFY(Parse("1 < 2 == 3 - 4") == Vector<String>({ "==", "<", "1", "2", "-", "3", "4" }));
        TEST_VERIFY(Parse("1 < 2 && 3 == 4") == Vector<String>({ "&&", "<", "1", "2", "==", "3", "4" }));
        TEST_VERIFY(Parse("true || false && true") == Vector<String>({ "||", "true", "&&", "false", "true" }));
    }

    // FormulaParser::ParseExpression
    DAVA_TEST (ParseFunction)
    {
        TEST_VERIFY(Parse("foo(a + c, d)") == Vector<String>({ "func_foo", "+", "field_a", "field_c", "field_d" }));
        TEST_VERIFY(Parse("foo()") == Vector<String>({ "func_foo" }));
    }

    // FormulaParser::ParseExpression
    DAVA_TEST (ParseField)
    {
        TEST_VERIFY(Parse("var") == Vector<String>({ "field_var" }));
        TEST_VERIFY(Parse("a.b") == Vector<String>({ "field_b", "field_a" }));
    }

    // FormulaParser::ParseExpression
    DAVA_TEST (ParseIndex)
    {
        TEST_VERIFY(Parse("var[25]") == Vector<String>({ "index", "field_var", "25" }));
    }

    // FormulaParser::ParseMap
    DAVA_TEST (ParseMap)
    {
        TEST_VERIFY(Parse("a = exp1 + 3"
                          "b = {a = 2}"
                          "c = [1 2 3]") == Vector<String>({ "index", "field_var", "25" }));
    }

    Vector<String> Parse(const String& str)
    {
        std::shared_ptr<FormulaExpression> exp = FormulaParser(str).ParseExpression();
        FormulaExpressionStack stack;
        stack.Traverse(exp.get());
        return stack.stack;
    }
    
};
