/*-------------------------------------------------------------------------------------------------
file:   text.h
author: Mosiychuck Dmitry
dsc:
-------------------------------------------------------------------------------------------------*/
#pragma once
#include "common.h"
#include "rc.h"
#include "token.h"
#include <vector>

class CFileInterface;
class CExpression;

///////////////////////////////////////////////////////////////////////////////////////////////////
class SYSTEM_API CExpressionsContainer
{
private:
    CExpressionsContainer(const CExpressionsContainer& r)
    {
    }
    CExpressionsContainer& operator=(const CExpressionsContainer& r)
    {
        return *this;
    }

public:
    CExpressionsContainer(void)
    {
    }
    CExpressionsContainer(CExpressionsContainer* container);
    ~CExpressionsContainer(void)
    {
        Clear();
    }

    bool Empty(void)
    {
        return m_exps.size() == 0 && m_containers.size() == 0;
    }
    CExpression* FindExpression(const char* arg);
    void Clear(void);
    CRC Save(CFileInterface* file, uint32 depth = 0);

    _vector<CExpression*> m_exps;
    _vector<CExpressionsContainer*> m_containers;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class SYSTEM_API CExpression
{
private:
    CExpression(const CExpression& r)
    {
    }
    CExpression& operator=(const CExpression& r)
    {
        return *this;
    }

public:
    CExpression(void)
    {
    }
    CExpression(CExpression* exp)
        :
        m_arg(exp->m_arg)
        , m_val(exp->m_val)
        , m_val_container(&exp->m_val_container)
    {
    }
    CExpression(const CToken& arg)
        : m_arg(arg)
    {
    }
    CExpression(const CToken& arg,
                const CToken& val)
        : m_arg(arg)
    {
        m_val.push_back(val);
    }
    ~CExpression(void)
    {
        m_val_container.Clear();
    }

    bool Empty(void)
    {
        return m_val.size() == 0 && m_val_container.Empty();
    }
    bool SingleVal(void)
    {
        return m_val.size() == 1 && m_val_container.Empty();
    }
    bool ArrayVal(void)
    {
        return m_val.size() > 1 && m_val_container.Empty();
    }
    bool ContainerVal(void)
    {
        return m_val.size() == 0 && !m_val_container.Empty();
    }

    CToken m_arg;

    //  used for regular expressions:
    //  arg = {val1, val2, ...}
    _vector<CToken> m_val;

    //  used for container expressions:
    //  arg =
    //  {
    //      arg1 = val
    //      arg2 = {val1, val2, ...}
    //  }
    CExpressionsContainer m_val_container;
};

SYSTEM_API void BuildTokensList(const _string& text, _vector<CToken>& tokens);
SYSTEM_API uint32 BuildExpressionsHierarchy(const _vector<CToken>& tokens, CExpressionsContainer* container, _string& err_dsc, uint32 idx_token = 0);

SYSTEM_API _string FormatString(const char* text, ...);
SYSTEM_API const char* FormatStringPreallocated(const char* text, ...);
SYSTEM_API void FormatBuffer(char* buffer, uint32 buffer_size, const char* text, ...);

SYSTEM_API _string ExtractFileNameFromPath(const _string& path);
SYSTEM_API _string ExtractFileNameAndExtFromPath(const _string& path);
SYSTEM_API _string ExcludeFileNameAndExtFromPath(const _string& path);
SYSTEM_API _string ExtractParentFolderNameFromPath(const _string& path);

SYSTEM_API _string RemoveWhitespacesAndTabsFromArgument(const char* arg);
SYSTEM_API _string UnquoteArgument(const char* arg);
SYSTEM_API _string QuoteArgument(const char* arg);
SYSTEM_API void ParseCommaSeparatedArguments(const char* arg, _vector<_string>& result);

SYSTEM_API uint32 StringToBool(const char* arg);
SYSTEM_API const char* BoolToString(bool b);
