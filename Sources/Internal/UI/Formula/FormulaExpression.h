#pragma once

#include "Base/Any.h"
#include "Base/AnyFn.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
    
class FormulaContext;

class FormulaData;
class FormulaLiteralExpression;
class FormulaNegExpression;
class FormulaNotExpression;
class FormulaBinaryOperatorExpression;
class FormulaFunctionExpression;
class FormulaFieldAccessExpression;
class FormulaIndexExpression;

class FormulaExpressionVisitor
{
public:
    virtual ~FormulaExpressionVisitor()
    {
    }

    virtual void Visit(FormulaLiteralExpression* exp)
    {
    }
    virtual void Visit(FormulaNegExpression* exp)
    {
    }
    virtual void Visit(FormulaNotExpression* exp)
    {
    }
    virtual void Visit(FormulaBinaryOperatorExpression* exp)
    {
    }
    virtual void Visit(FormulaFunctionExpression* exp)
    {
    }
    virtual void Visit(FormulaFieldAccessExpression* exp)
    {
    }
    virtual void Visit(FormulaIndexExpression* exp)
    {
    }
};

class FormulaExpression
{
public:
    FormulaExpression();
    virtual ~FormulaExpression();

    virtual Any Calculate(FormulaContext* context) const = 0;
    virtual bool ApplyValue(FormulaContext* context, const Any& value) const;
    virtual Reflection GetData(FormulaContext* context) const;
    virtual bool IsLiteral() const;
    virtual void CollectDepencencies(FormulaContext *context, Vector<void*> &dependencies) const = 0;

    virtual void Accept(FormulaExpressionVisitor* visitor) = 0;
};

class FormulaLiteralExpression : public FormulaExpression
{
public:
    FormulaLiteralExpression(const Any& value);

    Any Calculate(FormulaContext* context) const override;
    Reflection GetData(FormulaContext* context) const override;
    bool IsLiteral() const override;
    void CollectDepencencies(FormulaContext *context, Vector<void*> &dependencies) const override;
    
    const Any& GetValue() const;

    void Accept(FormulaExpressionVisitor* visitor) override;

private:
    Any value;
};

class FormulaNegExpression : public FormulaExpression
{
public:
    FormulaNegExpression(const std::shared_ptr<FormulaExpression>& exp);

    Any Calculate(FormulaContext* context) const override;

    void Accept(FormulaExpressionVisitor* visitor) override;
    void CollectDepencencies(FormulaContext *context, Vector<void*> &dependencies) const override;
    FormulaExpression* GetExp() const;

private:
    std::shared_ptr<FormulaExpression> exp;
};

class FormulaNotExpression : public FormulaExpression
{
public:
    FormulaNotExpression(const std::shared_ptr<FormulaExpression>& exp);

    Any Calculate(FormulaContext* context) const override;

    void Accept(FormulaExpressionVisitor* visitor) override;
    void CollectDepencencies(FormulaContext *context, Vector<void*> &dependencies) const override;
    
    FormulaExpression* GetExp() const;

private:
    std::shared_ptr<FormulaExpression> exp;
};

class FormulaBinaryOperatorExpression : public FormulaExpression
{
public:
    enum Operator
    {
        OP_PLUS,
        OP_MINUS,
        OP_MUL,
        OP_DIV,
        OP_MOD,
        OP_AND,
        OP_OR,
        OP_EQ,
        OP_NOT_EQ,
        OP_LE,
        OP_LT,
        OP_GE,
        OP_GT
    };

    FormulaBinaryOperatorExpression(Operator op, const std::shared_ptr<FormulaExpression>& lhs, const std::shared_ptr<FormulaExpression>& rhs);

    Any Calculate(FormulaContext* context) const override;
    void Accept(FormulaExpressionVisitor* visitor) override;
    void CollectDepencencies(FormulaContext *context, Vector<void*> &dependencies) const override;
    
    Operator GetOperator() const;
    String GetOperatorAsString() const;
    int32 GetOperatorPriority() const;

    FormulaExpression* GetLhs() const;
    FormulaExpression* GetRhs() const;

private:
    template <typename T>
    Any CalculateNumberValues(Operator op, T lVal, T rVal) const;

    Operator op;
    std::shared_ptr<FormulaExpression> lhs;
    std::shared_ptr<FormulaExpression> rhs;
};

class FormulaFunctionExpression : public FormulaExpression
{
public:
    FormulaFunctionExpression(const String& name, const Vector<std::shared_ptr<FormulaExpression>>& params_);

    Any Calculate(FormulaContext* context) const override;
    Reflection GetData(FormulaContext* context) const override;

    void Accept(FormulaExpressionVisitor* visitor) override;
    void CollectDepencencies(FormulaContext *context, Vector<void*> &dependencies) const override;

    const String& GetName() const;
    const Vector<std::shared_ptr<FormulaExpression>>& GetParms() const;

private:
    String name;
    Vector<std::shared_ptr<FormulaExpression>> params;
};

class FormulaFieldAccessExpression : public FormulaExpression
{
public:
    FormulaFieldAccessExpression(const std::shared_ptr<FormulaExpression>& exp, const String& fieldName);

    Any Calculate(FormulaContext* context) const override;
    bool ApplyValue(FormulaContext* context, const Any& value) const override;
    Reflection GetData(FormulaContext* context) const override;

    void Accept(FormulaExpressionVisitor* visitor) override;
    void CollectDepencencies(FormulaContext *context, Vector<void*> &dependencies) const override;

    FormulaExpression* GetExp() const;
    const String& GetFieldName() const;

private:
    std::shared_ptr<FormulaExpression> exp;
    String fieldName;
};

class FormulaIndexExpression : public FormulaExpression
{
public:
    FormulaIndexExpression(const std::shared_ptr<FormulaExpression>& exp, const std::shared_ptr<FormulaExpression>& indexExp);

    Any Calculate(FormulaContext* context) const override;
    bool ApplyValue(FormulaContext* context, const Any& value) const override;
    Reflection GetData(FormulaContext* context) const override;

    void Accept(FormulaExpressionVisitor* visitor) override;
    void CollectDepencencies(FormulaContext *context, Vector<void*> &dependencies) const override;

    FormulaExpression* GetExp() const;
    FormulaExpression* GetIndexExp() const;

private:
    std::shared_ptr<FormulaExpression> exp;
    std::shared_ptr<FormulaExpression> indexExp;
};
}
