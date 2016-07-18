/*-------------------------------------------------------------------------------------------------
file:   token.h
author: Mosiychuck Dmitry
dsc:
-------------------------------------------------------------------------------------------------*/
#pragma once
#include "common.h"
#include <string>

///////////////////////////////////////////////////////////////////////////////////////////////////
class CToken
{
public:
    CToken(void)
        : m_column(0)
        , m_line(0)
        , m_operator(false)
    {
    }
    CToken(const char* string)
        : m_string(string)
        , m_column(0)
        , m_line(0)
        , m_operator(false)
    {
    }
    CToken(const char* string, uint32 column, uint32 line, bool op = false)
        : m_string(string)
        , m_column(column)
        , m_line(line)
        , m_operator(op)
    {
    }

    CToken& operator=(const char* string)
    {
        m_string = string;
        return *this;
    }
    bool operator==(const CToken& token) const
    {
        return m_string == token.m_string;
    }
    bool operator!=(const CToken& token) const
    {
        return m_string != token.m_string;
    }
    bool operator==(const char* string) const
    {
        if (!strcmp(m_string.data(), string))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    bool operator!=(const char* string) const
    {
        if (strcmp(m_string.data(), string))
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool Empty(void) const
    {
        return !m_string.size();
    }
    const char* GetString(void) const
    {
        return m_string.data();
    }
    uint32 GetColumn(void) const
    {
        return m_column;
    }
    uint32 GetLine(void) const
    {
        return m_line;
    }
    bool IsOperator(void)
    {
        return m_operator;
    }

    //  void SetString(const char* string){m_string = string;}

protected:
    std::string m_string;
    uint32 m_column;
    uint32 m_line;
    bool m_operator;
};
