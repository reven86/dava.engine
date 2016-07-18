/*-------------------------------------------------------------------------------------------------
file:	preprocessor.h
author:	Mosiychuck Dmitry
dsc:
-------------------------------------------------------------------------------------------------*/
#pragma once
#include "text.h"
#include "token.h"
#include <list>

///////////////////////////////////////////////////////////////////////////////////////////////////
class CPreprocessor
{
public:
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CDefineDsc
    {
    public:
        CDefineDsc(const char* name)
        {
            m_name = name;
        }
        CDefineDsc(const char* name, const char* dsc)
        {
            m_name = name;
            m_dsc = dsc;
        }
        std::string m_name;
        std::string m_dsc;
    };

private:
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CDefineDscInternal
    {
    public:
        bool operator==(const CToken& name) const
        {
            return m_name == name;
        }

        CToken m_name;
        std::vector<CToken> m_args;
        std::list<CToken> m_dsc;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CExpression
    {
    public:
        enum EXP_CLASS
        {
            EXP_CONSTANT = 1,
            EXP_OPERATOR
        };
        enum EXP_OP_CLASS
        {
            EXP_OP_NOT_OP = 1, // not an op
            EXP_OP_TERMINATOR, // utility operator signing the end of the sequence
            EXP_OP_PLUS, // +
            EXP_OP_MINUS, // -
            EXP_OP_MUL, // *
            EXP_OP_DIV, // /
            EXP_OP_PARENTHESIS_OPEN, // (
            EXP_OP_PARENTHESIS_CLOSE, // )
            EXP_OP_NOT, // !
            EXP_OP_LOGICAL_AND, // &&
            EXP_OP_LOGICAL_OR, // ||
            EXP_OP_EQUAL, // ==
            EXP_OP_NOTEQUAL, // !=
            EXP_OP_LESS, // <
            EXP_OP_LESSEQUAL, // <=
            EXP_OP_MORE, // >
            EXP_OP_MOREEQUAL, // >=
        };
        enum EXP_RETURN_TYPE
        {
            EXP_RETURN_TYPE_UNKNOWN = 1,
            EXP_RETURN_TYPE_BOOLEAN,
            EXP_RETURN_TYPE_INTEGER,
            EXP_RETURN_TYPE_STRING
        };

    private:
        // don't allow copying expressions
        CExpression(const CExpression& r)
        {
        }
        CExpression& operator=(const CExpression& r)
        {
            return *this;
        }

    protected:
        CExpression(CToken* token, EXP_CLASS expClass, EXP_OP_CLASS opClass, EXP_RETURN_TYPE returnType)
            :
            m_expClass(expClass)
            , m_opClass(opClass)
            , m_returnType(returnType)
            , m_token(token)
            , m_prevExp(0)
            , m_nextExp(0)
            , m_leftExp(0)
            , m_rightExp(0)
            , m_constant(0)
        {
        }

    public:
        CExpression(void)
            :
            m_expClass(EXP_OPERATOR)
            , m_opClass(EXP_OP_TERMINATOR)
            , m_returnType(EXP_RETURN_TYPE_UNKNOWN)
            , m_token(0)
            , m_prevExp(0)
            , m_nextExp(0)
            , m_leftExp(0)
            , m_rightExp(0)
            , m_constant(0)
        {
        }
        virtual ~CExpression(void)
        {
            Disconect();
            if (m_leftExp)
            {
                delete m_leftExp;
            }
            if (m_rightExp)
            {
                delete m_rightExp;
            }
        }

        void Disconect(void)
        {
            if (m_prevExp)
            {
                if (m_nextExp)
                {
                    m_prevExp->m_nextExp = m_nextExp;
                }
                else
                {
                    m_prevExp->m_nextExp = 0;
                }
            }
            if (m_nextExp)
            {
                if (m_prevExp)
                {
                    m_nextExp->m_prevExp = m_prevExp;
                }
                else
                {
                    m_nextExp->m_prevExp = 0;
                }
            }
            m_prevExp = 0;
            m_nextExp = 0;
        }
        static CExpression* CreateExpression(CToken* token);

        virtual bool Parse(bool flag = false)
        {
            ASSERT(false);
            return false;
        }
        virtual void* Evaluate(void) const
        {
            ASSERT(false);
            return 0;
        }

        bool operator==(EXP_CLASS expClass) const
        {
            return m_expClass == expClass;
        }
        bool operator!=(EXP_CLASS expClass) const
        {
            return m_expClass != expClass;
        }
        bool operator==(EXP_OP_CLASS opClass) const
        {
            return m_opClass == opClass;
        }
        bool operator!=(EXP_OP_CLASS opClass) const
        {
            return m_opClass != opClass;
        }
        bool operator==(EXP_RETURN_TYPE returnType) const
        {
            return m_returnType == returnType;
        }
        bool operator!=(EXP_RETURN_TYPE returnType) const
        {
            return m_returnType != returnType;
        }

        EXP_CLASS GetExpClass(void) const
        {
            return m_expClass;
        }
        EXP_OP_CLASS GetOpClass(void) const
        {
            return m_opClass;
        }
        EXP_RETURN_TYPE GetReturnType(void) const
        {
            return m_returnType;
        }

        CToken* m_token;
        CExpression* m_prevExp;
        CExpression* m_nextExp;
        CExpression* m_leftExp;
        CExpression* m_rightExp;

        int m_constant;

    protected:
        EXP_CLASS m_expClass;
        EXP_OP_CLASS m_opClass;
        EXP_RETURN_TYPE m_returnType;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CExpConstant : public CExpression
    {
    public:
        CExpConstant(CToken* token)
            : CExpression(token, EXP_CONSTANT, EXP_OP_NOT_OP, EXP_RETURN_TYPE_UNKNOWN)
        {
            if (1 == sscanf_s(m_token->GetString(), "%i", &m_constant))
            {
                m_returnType = EXP_RETURN_TYPE_INTEGER;
                return;
            }

            m_constant = StringToBool(m_token->GetString());
            if (m_constant != -1)
            {
                m_returnType = EXP_RETURN_TYPE_BOOLEAN;
                return;
            }
            m_constant = 0;

            m_returnType = EXP_RETURN_TYPE_STRING;
        }

        virtual bool Parse(bool flag)
        {
            return true;
        }
        virtual void* Evaluate(void) const
        {
            switch (m_returnType)
            {
            case EXP_RETURN_TYPE_INTEGER:
            case EXP_RETURN_TYPE_BOOLEAN:
                return (void*)m_constant;
            case EXP_RETURN_TYPE_STRING:
                return (void*)(m_token);
            default:
                ASSERT(false)
                return 0;
            }
        }
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CExpPlus : public CExpression
    {
    public:
        CExpPlus(CToken* token)
            : CExpression(token, EXP_OPERATOR, EXP_OP_PLUS, EXP_RETURN_TYPE_UNKNOWN)
        {
        }
        virtual bool Parse(bool flag)
        {
            if (m_returnType != EXP_RETURN_TYPE_UNKNOWN)
            {
                return true;
            }
            if (flag)
            {
                // unary plus
                if (m_prevExp->GetReturnType() == EXP_RETURN_TYPE_INTEGER ||
                    m_nextExp->GetReturnType() != EXP_RETURN_TYPE_INTEGER)
                {
                    return true;
                }
                if (m_nextExp->GetReturnType() != EXP_RETURN_TYPE_INTEGER)
                {
                    m_errorString = FormatString("Wrong argument type for operator '+' in line: %d, column: %d", m_token->GetLine(), m_token->GetColumn());
                    return false;
                }
                m_rightExp = m_nextExp;
                m_nextExp->Disconect();
            }
            else
            {
                // binary plus
                if (m_prevExp->GetReturnType() != EXP_RETURN_TYPE_INTEGER ||
                    m_nextExp->GetReturnType() != EXP_RETURN_TYPE_INTEGER)
                {
                    m_errorString = FormatString("Wrong argument type for operator '+' in line: %d, column: %d", m_token->GetLine(), m_token->GetColumn());
                    return false;
                }
                m_leftExp = m_prevExp;
                m_rightExp = m_nextExp;
                m_prevExp->Disconect();
                m_nextExp->Disconect();
            }
            m_returnType = EXP_RETURN_TYPE_INTEGER;
            return true;
        }
        virtual void* Evaluate(void) const
        {
            if (!m_leftExp)
            {
                ASSERT(m_rightExp)
                return (void*)(+(int)m_rightExp->Evaluate());
            }
            else
            {
                ASSERT(m_leftExp && m_rightExp)
                return (void*)((int)m_leftExp->Evaluate() + (int)m_rightExp->Evaluate());
            }
        }
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CExpMinus : public CExpression
    {
    public:
        CExpMinus(CToken* token)
            : CExpression(token, EXP_OPERATOR, EXP_OP_MINUS, EXP_RETURN_TYPE_UNKNOWN)
        {
        }
        virtual bool Parse(bool flag)
        {
            if (m_returnType != EXP_RETURN_TYPE_UNKNOWN)
            {
                return true;
            }
            if (flag)
            {
                // unary minus
                if (m_prevExp->GetReturnType() == EXP_RETURN_TYPE_INTEGER ||
                    m_nextExp->GetReturnType() != EXP_RETURN_TYPE_INTEGER)
                {
                    return true;
                }
                if (m_nextExp->GetReturnType() != EXP_RETURN_TYPE_INTEGER)
                {
                    m_errorString = FormatString("Wrong argument type for operator '-' in line: %d, column: %d", m_token->GetLine(), m_token->GetColumn());
                    return false;
                }
                m_rightExp = m_nextExp;
                m_nextExp->Disconect();
            }
            else
            {
                // binary minus
                if (m_prevExp->GetReturnType() != EXP_RETURN_TYPE_INTEGER ||
                    m_nextExp->GetReturnType() != EXP_RETURN_TYPE_INTEGER)
                {
                    m_errorString = FormatString("Wrong argument type for operator '-' in line: %d, column: %d", m_token->GetLine(), m_token->GetColumn());
                    return false;
                }
                m_leftExp = m_prevExp;
                m_rightExp = m_nextExp;
                m_prevExp->Disconect();
                m_nextExp->Disconect();
            }
            m_returnType = EXP_RETURN_TYPE_INTEGER;
            return true;
        }
        virtual void* Evaluate(void) const
        {
            if (!m_leftExp)
            {
                ASSERT(m_rightExp)
                return (void*)(-(int)m_rightExp->Evaluate());
            }
            else
            {
                ASSERT(m_leftExp && m_rightExp)
                return (void*)((int)m_leftExp->Evaluate() - (int)m_rightExp->Evaluate());
            }
        }
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CExpMul : public CExpression
    {
    public:
        CExpMul(CToken* token)
            : CExpression(token, EXP_OPERATOR, EXP_OP_MUL, EXP_RETURN_TYPE_UNKNOWN)
        {
        }
        virtual bool Parse(bool flag)
        {
            if (m_returnType != EXP_RETURN_TYPE_UNKNOWN)
            {
                return true;
            }
            if (m_prevExp->GetReturnType() != EXP_RETURN_TYPE_INTEGER ||
                m_nextExp->GetReturnType() != EXP_RETURN_TYPE_INTEGER)
            {
                m_errorString = FormatString("Wrong argument type for operator '*' in line: %d, column: %d", m_token->GetLine(), m_token->GetColumn());
                return false;
            }
            m_leftExp = m_prevExp;
            m_rightExp = m_nextExp;
            m_prevExp->Disconect();
            m_nextExp->Disconect();
            m_returnType = EXP_RETURN_TYPE_INTEGER;
            return true;
        }
        virtual void* Evaluate(void) const
        {
            ASSERT(m_leftExp && m_rightExp)
            return (void*)((int)m_leftExp->Evaluate() * (int)m_rightExp->Evaluate());
        }
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CExpDiv : public CExpression
    {
    public:
        CExpDiv(CToken* token)
            : CExpression(token, EXP_OPERATOR, EXP_OP_DIV, EXP_RETURN_TYPE_UNKNOWN)
        {
        }
        virtual bool Parse(bool flag)
        {
            if (m_returnType != EXP_RETURN_TYPE_UNKNOWN)
            {
                return true;
            }
            if (m_prevExp->GetReturnType() != EXP_RETURN_TYPE_INTEGER ||
                m_nextExp->GetReturnType() != EXP_RETURN_TYPE_INTEGER)
            {
                m_errorString = FormatString("Wrong argument type for operator '/' in line: %d, column: %d", m_token->GetLine(), m_token->GetColumn());
                return false;
            }
            m_leftExp = m_prevExp;
            m_rightExp = m_nextExp;
            m_prevExp->Disconect();
            m_nextExp->Disconect();
            m_returnType = EXP_RETURN_TYPE_INTEGER;
            return true;
        }
        virtual void* Evaluate(void) const
        {
            ASSERT(m_leftExp && m_rightExp)
            return (void*)((int)m_leftExp->Evaluate() / (int)m_rightExp->Evaluate());
        }
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CExpNot : public CExpression
    {
    public:
        CExpNot(CToken* token)
            : CExpression(token, EXP_OPERATOR, EXP_OP_NOT, EXP_RETURN_TYPE_UNKNOWN)
        {
        }
        virtual bool Parse(bool flag)
        {
            if (m_returnType != EXP_RETURN_TYPE_UNKNOWN)
            {
                return true;
            }
            if (m_nextExp->GetReturnType() != EXP_RETURN_TYPE_BOOLEAN)
            {
                m_errorString = FormatString("Wrong argument type for operator '!' in line: %d, column: %d", m_token->GetLine(), m_token->GetColumn());
                return false;
            }
            m_rightExp = m_nextExp;
            m_nextExp->Disconect();
            m_returnType = EXP_RETURN_TYPE_BOOLEAN;
            return true;
        }
        virtual void* Evaluate(void) const
        {
            ASSERT(m_rightExp)
            bool b = (int)m_rightExp->Evaluate() != 0;
            return (void*)!b;
        }
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CExpLogicalAnd : public CExpression
    {
    public:
        CExpLogicalAnd(CToken* token)
            : CExpression(token, EXP_OPERATOR, EXP_OP_LOGICAL_AND, EXP_RETURN_TYPE_UNKNOWN)
        {
        }
        virtual bool Parse(bool flag)
        {
            if (m_returnType != EXP_RETURN_TYPE_UNKNOWN)
            {
                return true;
            }
            if ((m_prevExp->GetReturnType() != EXP_RETURN_TYPE_BOOLEAN && m_prevExp->GetReturnType() != EXP_RETURN_TYPE_INTEGER) ||
                (m_nextExp->GetReturnType() != EXP_RETURN_TYPE_BOOLEAN && m_nextExp->GetReturnType() != EXP_RETURN_TYPE_INTEGER))
            {
                m_errorString = FormatString("Wrong argument type for operator '&&' in line: %d, column: %d", m_token->GetLine(), m_token->GetColumn());
                return false;
            }
            m_leftExp = m_prevExp;
            m_rightExp = m_nextExp;
            m_prevExp->Disconect();
            m_nextExp->Disconect();
            m_returnType = EXP_RETURN_TYPE_BOOLEAN;
            return true;
        }
        virtual void* Evaluate(void) const
        {
            ASSERT(m_leftExp && m_rightExp)
            bool left = (int)m_leftExp->Evaluate() != 0;
            bool right = (int)m_rightExp->Evaluate() != 0;
            return (void*)(left && right);
        }
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CExpLogicalOr : public CExpression
    {
    public:
        CExpLogicalOr(CToken* token)
            : CExpression(token, EXP_OPERATOR, EXP_OP_LOGICAL_OR, EXP_RETURN_TYPE_UNKNOWN)
        {
        }
        virtual bool Parse(bool flag)
        {
            if (m_returnType != EXP_RETURN_TYPE_UNKNOWN)
            {
                return true;
            }
            if ((m_prevExp->GetReturnType() != EXP_RETURN_TYPE_BOOLEAN && m_prevExp->GetReturnType() != EXP_RETURN_TYPE_INTEGER) ||
                (m_nextExp->GetReturnType() != EXP_RETURN_TYPE_BOOLEAN && m_nextExp->GetReturnType() != EXP_RETURN_TYPE_INTEGER))
            {
                m_errorString = FormatString("Wrong argument type for operator '||' in line: %d, column: %d", m_token->GetLine(), m_token->GetColumn());
                return false;
            }
            m_leftExp = m_prevExp;
            m_rightExp = m_nextExp;
            m_prevExp->Disconect();
            m_nextExp->Disconect();
            m_returnType = EXP_RETURN_TYPE_BOOLEAN;
            return true;
        }
        virtual void* Evaluate(void) const
        {
            ASSERT(m_leftExp && m_rightExp)
            bool left = (int)m_leftExp->Evaluate() != 0;
            bool right = (int)m_rightExp->Evaluate() != 0;
            return (void*)(left || right);
        }
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CExpEqual : public CExpression
    {
    public:
        CExpEqual(CToken* token)
            : CExpression(token, EXP_OPERATOR, EXP_OP_EQUAL, EXP_RETURN_TYPE_UNKNOWN)
        {
        }
        virtual bool Parse(bool flag)
        {
            if (m_returnType != EXP_RETURN_TYPE_UNKNOWN)
            {
                return true;
            }
            if ((m_prevExp->GetReturnType() != EXP_RETURN_TYPE_BOOLEAN && m_prevExp->GetReturnType() != EXP_RETURN_TYPE_INTEGER) ||
                (m_nextExp->GetReturnType() != EXP_RETURN_TYPE_BOOLEAN && m_nextExp->GetReturnType() != EXP_RETURN_TYPE_INTEGER))
            {
                m_errorString = FormatString("Wrong argument type for operator '==' in line: %d, column: %d", m_token->GetLine(), m_token->GetColumn());
                return false;
            }
            m_leftExp = m_prevExp;
            m_rightExp = m_nextExp;
            m_prevExp->Disconect();
            m_nextExp->Disconect();
            m_returnType = EXP_RETURN_TYPE_BOOLEAN;
            return true;
        }
        virtual void* Evaluate(void) const
        {
            ASSERT(m_leftExp && m_rightExp)
            int left = (int)m_leftExp->Evaluate();
            int right = (int)m_rightExp->Evaluate();
            return (void*)(left == right);
        }
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CExpNotEqual : public CExpression
    {
    public:
        CExpNotEqual(CToken* token)
            : CExpression(token, EXP_OPERATOR, EXP_OP_NOTEQUAL, EXP_RETURN_TYPE_UNKNOWN)
        {
        }
        virtual bool Parse(bool flag)
        {
            if (m_returnType != EXP_RETURN_TYPE_UNKNOWN)
            {
                return true;
            }
            if ((m_prevExp->GetReturnType() != EXP_RETURN_TYPE_BOOLEAN && m_prevExp->GetReturnType() != EXP_RETURN_TYPE_INTEGER) ||
                (m_nextExp->GetReturnType() != EXP_RETURN_TYPE_BOOLEAN && m_nextExp->GetReturnType() != EXP_RETURN_TYPE_INTEGER))
            {
                m_errorString = FormatString("Wrong argument type for operator '!=' in line: %d, column: %d", m_token->GetLine(), m_token->GetColumn());
                return false;
            }
            m_leftExp = m_prevExp;
            m_rightExp = m_nextExp;
            m_prevExp->Disconect();
            m_nextExp->Disconect();
            m_returnType = EXP_RETURN_TYPE_BOOLEAN;
            return true;
        }
        virtual void* Evaluate(void) const
        {
            ASSERT(m_leftExp && m_rightExp)
            int left = (int)m_leftExp->Evaluate();
            int right = (int)m_rightExp->Evaluate();
            return (void*)(left != right);
        }
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CExpLess : public CExpression
    {
    public:
        CExpLess(CToken* token)
            : CExpression(token, EXP_OPERATOR, EXP_OP_LESS, EXP_RETURN_TYPE_UNKNOWN)
        {
        }
        virtual bool Parse(bool flag)
        {
            if (m_returnType != EXP_RETURN_TYPE_UNKNOWN)
            {
                return true;
            }
            if ((m_prevExp->GetReturnType() != EXP_RETURN_TYPE_BOOLEAN && m_prevExp->GetReturnType() != EXP_RETURN_TYPE_INTEGER) ||
                (m_nextExp->GetReturnType() != EXP_RETURN_TYPE_BOOLEAN && m_nextExp->GetReturnType() != EXP_RETURN_TYPE_INTEGER))
            {
                m_errorString = FormatString("Wrong argument type for operator '<' in line: %d, column: %d", m_token->GetLine(), m_token->GetColumn());
                return false;
            }
            m_leftExp = m_prevExp;
            m_rightExp = m_nextExp;
            m_prevExp->Disconect();
            m_nextExp->Disconect();
            m_returnType = EXP_RETURN_TYPE_BOOLEAN;
            return true;
        }
        virtual void* Evaluate(void) const
        {
            ASSERT(m_leftExp && m_rightExp)
            int left = (int)m_leftExp->Evaluate();
            int right = (int)m_rightExp->Evaluate();
            return (void*)(left < right);
        }
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CExpLessEqual : public CExpression
    {
    public:
        CExpLessEqual(CToken* token)
            : CExpression(token, EXP_OPERATOR, EXP_OP_LESSEQUAL, EXP_RETURN_TYPE_UNKNOWN)
        {
        }
        virtual bool Parse(bool flag)
        {
            if (m_returnType != EXP_RETURN_TYPE_UNKNOWN)
            {
                return true;
            }
            if ((m_prevExp->GetReturnType() != EXP_RETURN_TYPE_BOOLEAN && m_prevExp->GetReturnType() != EXP_RETURN_TYPE_INTEGER) ||
                (m_nextExp->GetReturnType() != EXP_RETURN_TYPE_BOOLEAN && m_nextExp->GetReturnType() != EXP_RETURN_TYPE_INTEGER))
            {
                m_errorString = FormatString("Wrong argument type for operator '<=' in line: %d, column: %d", m_token->GetLine(), m_token->GetColumn());
                return false;
            }
            m_leftExp = m_prevExp;
            m_rightExp = m_nextExp;
            m_prevExp->Disconect();
            m_nextExp->Disconect();
            m_returnType = EXP_RETURN_TYPE_BOOLEAN;
            return true;
        }
        virtual void* Evaluate(void) const
        {
            ASSERT(m_leftExp && m_rightExp)
            int left = (int)m_leftExp->Evaluate();
            int right = (int)m_rightExp->Evaluate();
            return (void*)(left <= right);
        }
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CExpMore : public CExpression
    {
    public:
        CExpMore(CToken* token)
            : CExpression(token, EXP_OPERATOR, EXP_OP_MORE, EXP_RETURN_TYPE_UNKNOWN)
        {
        }
        virtual bool Parse(bool flag)
        {
            if (m_returnType != EXP_RETURN_TYPE_UNKNOWN)
            {
                return true;
            }
            if ((m_prevExp->GetReturnType() != EXP_RETURN_TYPE_BOOLEAN && m_prevExp->GetReturnType() != EXP_RETURN_TYPE_INTEGER) ||
                (m_nextExp->GetReturnType() != EXP_RETURN_TYPE_BOOLEAN && m_nextExp->GetReturnType() != EXP_RETURN_TYPE_INTEGER))
            {
                m_errorString = FormatString("Wrong argument type for operator '>' in line: %d, column: %d", m_token->GetLine(), m_token->GetColumn());
                return false;
            }
            m_leftExp = m_prevExp;
            m_rightExp = m_nextExp;
            m_prevExp->Disconect();
            m_nextExp->Disconect();
            m_returnType = EXP_RETURN_TYPE_BOOLEAN;
            return true;
        }
        virtual void* Evaluate(void) const
        {
            ASSERT(m_leftExp && m_rightExp)
            int left = (int)m_leftExp->Evaluate();
            int right = (int)m_rightExp->Evaluate();
            return (void*)(left > right);
        }
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CExpMoreEqual : public CExpression
    {
    public:
        CExpMoreEqual(CToken* token)
            : CExpression(token, EXP_OPERATOR, EXP_OP_MOREEQUAL, EXP_RETURN_TYPE_UNKNOWN)
        {
        }
        virtual bool Parse(bool flag)
        {
            if (m_returnType != EXP_RETURN_TYPE_UNKNOWN)
            {
                return true;
            }
            if ((m_prevExp->GetReturnType() != EXP_RETURN_TYPE_BOOLEAN && m_prevExp->GetReturnType() != EXP_RETURN_TYPE_INTEGER) ||
                (m_nextExp->GetReturnType() != EXP_RETURN_TYPE_BOOLEAN && m_nextExp->GetReturnType() != EXP_RETURN_TYPE_INTEGER))
            {
                m_errorString = FormatString("Wrong argument type for operator '>=' in line: %d, column: %d", m_token->GetLine(), m_token->GetColumn());
                return false;
            }
            m_leftExp = m_prevExp;
            m_rightExp = m_nextExp;
            m_prevExp->Disconect();
            m_nextExp->Disconect();
            m_returnType = EXP_RETURN_TYPE_BOOLEAN;
            return true;
        }
        virtual void* Evaluate(void) const
        {
            ASSERT(m_leftExp && m_rightExp)
            int left = (int)m_leftExp->Evaluate();
            int right = (int)m_rightExp->Evaluate();
            return (void*)(left >= right);
        }
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CExpParenthesis : public CExpression
    {
    public:
        CExpParenthesis(CToken* token, EXP_OP_CLASS opClass)
            : CExpression(token, EXP_OPERATOR, opClass, EXP_RETURN_TYPE_UNKNOWN)
        {
            ASSERT(opClass == EXP_OP_PARENTHESIS_OPEN ||
                   opClass == EXP_OP_PARENTHESIS_CLOSE)
        }
        virtual bool Parse(void)
        {
            ASSERT(false)
            return false;
        }
        virtual void* Evaluate(void) const
        {
            ASSERT(false)
            return 0;
        }
    };

public:
    CPreprocessor(void);
    ~CPreprocessor(void);

    static const char* GetErrorString(void)
    {
        return m_errorString.data();
    }

    bool TokenizeFile(const char* path, std::list<CToken>* tokens);
    void TokenizeString(const char* string, size_t strLen, std::list<CToken>* tokens);
    bool Preprocess(const char* path, std::list<CToken>& tokens, std::vector<CDefineDsc>* pDefines = 0);

private:
    bool PreprocessDeclaration(std::list<CToken>& tokens, std::list<CDefineDscInternal>& defines, std::list<CToken>::iterator& iterToken);
    bool EvaluateExpression(std::list<CToken>::iterator token, int* pResult);
    CExpression* Lex(std::list<CToken>::iterator token);
    CExpression* BuildEvaluationTree(CExpression* exp, bool expectParenthesis = false);
    void DestroyExpression(CExpression* exp);
    std::list<CToken>::iterator GetNextLine(std::list<CToken>::iterator iter);

    static std::string m_errorString;
};
