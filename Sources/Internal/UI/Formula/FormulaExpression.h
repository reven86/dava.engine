#pragma once

#include "Base/Any.h"

namespace DAVA
{
    
class FormulaValueExpression;
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

    virtual void Visit(FormulaValueExpression* exp)
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

    virtual bool IsValue() const;

    virtual void Accept(FormulaExpressionVisitor* visitor) = 0;
};

class FormulaValueExpression : public FormulaExpression
{
public:
    FormulaValueExpression(const Any& value);

    bool IsValue() const override;
    const Any& GetValue() const;

    void Accept(FormulaExpressionVisitor* visitor) override;

private:
    Any value;
};

class FormulaNegExpression : public FormulaExpression
{
public:
    FormulaNegExpression(const std::shared_ptr<FormulaExpression>& exp);

    void Accept(FormulaExpressionVisitor* visitor) override;
    FormulaExpression* GetExp() const;

private:
    std::shared_ptr<FormulaExpression> exp;
};

class FormulaNotExpression : public FormulaExpression
{
public:
    FormulaNotExpression(const std::shared_ptr<FormulaExpression>& exp);

    void Accept(FormulaExpressionVisitor* visitor) override;
    
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

    void Accept(FormulaExpressionVisitor* visitor) override;
    
    Operator GetOperator() const;
    int32 GetOperatorPriority() const;

    FormulaExpression* GetLhs() const;
    FormulaExpression* GetRhs() const;

private:
    Operator op;
    std::shared_ptr<FormulaExpression> lhs;
    std::shared_ptr<FormulaExpression> rhs;
};

class FormulaFunctionExpression : public FormulaExpression
{
public:
    FormulaFunctionExpression(const String& name, const Vector<std::shared_ptr<FormulaExpression>>& params_);

    void Accept(FormulaExpressionVisitor* visitor) override;

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

    void Accept(FormulaExpressionVisitor* visitor) override;

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

    void Accept(FormulaExpressionVisitor* visitor) override;

    FormulaExpression* GetExp() const;
    FormulaExpression* GetIndexExp() const;

private:
    std::shared_ptr<FormulaExpression> exp;
    std::shared_ptr<FormulaExpression> indexExp;
};
}
