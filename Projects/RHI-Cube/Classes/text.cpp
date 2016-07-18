/*-------------------------------------------------------------------------------------------------
file:   text.cpp
author: Mosiychuck Dmitry
dsc:
-------------------------------------------------------------------------------------------------*/
#include "text.h"
#include "fileinterface.h"
//#include "framework.h"
#include <stdarg.h>

///////////////////////////////////////////////////////////////////////////////////////////////////
CExpressionsContainer::CExpressionsContainer(CExpressionsContainer* container)
{
    Clear();

    // copy expressions
    uint32 num_exps = (uint32)container->m_exps.size();
    for (uint32 i = 0; i < num_exps; ++i)
    {
        m_exps.push_back(NEW CExpression(container->m_exps[i]));
    }

    // copy containers
    uint32 num_containers = (uint32)container->m_containers.size();
    for (uint32 i = 0; i < num_containers; ++i)
    {
        m_containers.push_back(NEW CExpressionsContainer(container->m_containers[i]));
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CExpression* CExpressionsContainer::FindExpression(const char* arg)
{
    uint32 num_expressions = (uint32)m_exps.size();
    for (uint32 i = 0; i < num_expressions; ++i)
    {
        if (m_exps[i]->m_arg == arg)
        {
            return m_exps[i];
        }
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CExpressionsContainer::Clear(void)
{
    // destroy expressions
    uint32 num_expressions = (uint32)m_exps.size();
    for (uint32 i = 0; i < num_expressions; ++i)
    {
        DELETE(m_exps[i])
    }
    m_exps.clear();

    // destroy containers
    uint32 num_containers = (uint32)m_containers.size();
    for (uint32 i = 0; i < num_containers; ++i)
    {
        DELETE(m_containers[i])
    }
    m_containers.clear();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CRC CExpressionsContainer::Save(CFileInterface* file, uint32 depth)
{
    // save expressions
    uint32 num_exps = (uint32)m_exps.size();
    for (uint32 i = 0; i < num_exps; ++i)
    {
        CExpression* exp = m_exps[i];
        uint32 num_elements = (uint32)exp->m_val.size();
        if (exp->Empty())
        {
            continue;
        }

        for (uint32 d = 0; d < depth; ++d)
        {
            TRY(file->WriteString("\t"))
        } // add tabs

        TRY(file->WriteString(exp->m_arg.GetString()))
        TRY(file->WriteString(" = "))

        if (num_elements == 0)
        {
            // container val
            TRY(file->WriteString("\r\n"))

            for (uint32 d = 0; d < depth; ++d)
            {
                TRY(file->WriteString("\t"))
            } // add tabs
            TRY(file->WriteString("{"))
            TRY(file->WriteString("\r\n"))
            TRY(exp->m_val_container.Save(file, depth + 1))
            for (uint32 d = 0; d < depth; ++d)
            {
                TRY(file->WriteString("\t"))
            } // add tabs
            TRY(file->WriteString("}"))
        }
        else if (num_elements == 1)
        {
            // single val
            TRY(file->WriteString(exp->m_val[0].GetString()))
        }
        else
        {
            // array val
            TRY(file->WriteString("\r\n"))
            for (uint32 d = 0; d < depth; ++d)
            {
                TRY(file->WriteString("\t"))
            } // add tabs
            TRY(file->WriteString("{"))
            TRY(file->WriteString("\r\n"))
            for (uint32 j = 0; j < num_elements; ++j)
            {
                for (uint32 d = 0; d < depth + 1; ++d)
                {
                    TRY(file->WriteString("\t"))
                } // add tabs
                TRY(file->WriteString(exp->m_val[j].GetString()))
                if (j < num_elements - 1)
                {
                    TRY(file->WriteString(","))
                    TRY(file->WriteString("\r\n"))
                }
            }
            TRY(file->WriteString("\r\n"))
            for (uint32 d = 0; d < depth; ++d)
            {
                TRY(file->WriteString("\t"))
            } // add tabs
            TRY(file->WriteString("}"))
        }
        TRY(file->WriteString("\r\n"))
    }

    // save containers
    uint32 num_containers = (uint32)m_containers.size();
    for (uint32 i = 0; i < num_containers; ++i)
    {
        for (uint32 d = 0; d < depth; ++d)
        {
            TRY(file->WriteString("\t"))
        } // add tabs
        TRY(file->WriteString("{"))
        TRY(file->WriteString("\r\n"))
        TRY(m_containers[i]->Save(file, depth + 1))
        for (uint32 d = 0; d < depth; ++d)
        {
            TRY(file->WriteString("\t"))
        } // add tabs
        TRY(file->WriteString("}"))
        TRY(file->WriteString("\r\n"))
    }

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void BuildTokensList(const _string& text, _vector<CToken>& tokens)
{
    size_t line_begin = 0;
    size_t line_comment = 0;
    size_t line_end = 0;
    bool last_line = false;
    uint32 line_idx = 0;
    for (;;)
    {
        // get next line
        line_end = text.find("\n", line_begin);
        if (_string::npos == line_end)
        {
            last_line = true;
            line_end = text.size();
        }
        _string line = text.substr(line_begin, line_end - line_begin);
        line_comment = line.find("//");
        if (line_comment != _string::npos)
        {
            line = line.substr(0, line_comment);
        }

        // extract tokens from the line
        _string token;
        uint32 token_column = -1;
        bool brackets = false;
        uint32 num_characters = (uint32)line.size();
        for (uint32 i = 0; i < num_characters; ++i)
        {
            char c = line[i];

            if (c == '\r')
            {
                continue;
            }

            if (c == '\"')
            {
                brackets = !brackets;
            }

            if (c == '{' || c == '}' || c == ',' || c == '=')
            {
                if (token.size())
                {
                    tokens.push_back(CToken(token.c_str(), token_column + 1, line_idx + 1));
                    token.clear();
                }
                token = c;
                tokens.push_back(CToken(token.c_str(), i + 1, line_idx + 1));
                token.clear();
                token_column = -1;
            }
            else if ((c == ' ' || c == '\t') && !brackets)
            {
                if (token.size())
                {
                    tokens.push_back(CToken(token.c_str(), token_column + 1, line_idx + 1));
                    token.clear();
                }
                token_column = -1;
            }
            else
            {
                if (token_column == -1)
                {
                    token_column = i;
                }
                token += c;
            }
        }

        if (token.size())
        {
            tokens.push_back(CToken(token.c_str(), token_column + 1, line_idx + 1));
        }

        // go to the next line
        line_begin = line_end + 1;
        ++line_idx;

        // exit if it was the last line
        if (last_line)
        {
            return;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/*
returns next token idx
*/
#define UNEXPECTED_TOKEN_ERROR(token) \
    err_dsc = FormatString("unexpected token in line: %u, column: %u", token.GetLine(), token.GetColumn());\
    return 0;

uint32 BuildExpressionsHierarchy(const _vector<CToken>& tokens, CExpressionsContainer* container, _string& err_dsc, uint32 idx_token)
{
    uint32 idx = idx_token;

    for (;;)
    {
        // get next token
        if (idx >= tokens.size())
        {
            if (!idx_token)
            {
                return 0; // exit point from parsing process: EOF on zero level of recursion
            }
            else
            {
                err_dsc = "unexpected end of file";
                return 0;
            }
        }
        const CToken& arg = tokens[idx];
        ++idx;

        if (arg == "}")
        {
            if (!idx_token)
            {
                UNEXPECTED_TOKEN_ERROR(arg)
            }
            else
            {
                return idx;
            }
        }
        else if (arg == "{")
        {
            CExpressionsContainer* new_container = NEW CExpressionsContainer;
            container->m_containers.push_back(new_container);
            idx = BuildExpressionsHierarchy(tokens, new_container, err_dsc, idx);
            if (err_dsc.size())
            {
                return 0;
            }
        }
        else if (arg == "=" || arg == ",")
        {
            UNEXPECTED_TOKEN_ERROR(arg)
        }
        else
        {
            // build expression
            CExpression* new_exp = NEW CExpression;
            new_exp->m_arg = arg;

            // look for '=' sign
            if (idx >= tokens.size())
            {
                err_dsc = "unexpected end of file";
                return 0;
            }
            const CToken& es = tokens[idx];
            ++idx;
            if (es != "=")
            {
                UNEXPECTED_TOKEN_ERROR(es)
            }

            // look for val
            if (idx >= tokens.size())
            {
                err_dsc = "unexpected end of file";
                return 0;
            }
            const CToken& val = tokens[idx];
            ++idx;
            if (val == "=" || val == "," || val == "}")
            {
                UNEXPECTED_TOKEN_ERROR(val)
            }
            else if (val == "{")
            {
                // array val or container val
                CToken t1;
                CToken t2;
                if (idx <= tokens.size() - 2)
                {
                    t1 = tokens[idx];
                    t2 = tokens[idx + 1];
                }
                if (t1 == "}")
                {
                    UNEXPECTED_TOKEN_ERROR(t1)
                } // arg = {}

                if (t2 == "," || t2 == "}")
                {
                    // array val e.g. arg = {val, ... || arg = {val}
                    for (;;)
                    {
                        if (idx >= tokens.size())
                        {
                            err_dsc = "unexpected end of file";
                            return 0;
                        }
                        const CToken& t1 = tokens[idx];
                        ++idx;
                        if (idx >= tokens.size())
                        {
                            err_dsc = "unexpected end of file";
                            return 0;
                        }
                        const CToken& t2 = tokens[idx];
                        ++idx;
                        if (t1 == "=" || t1 == "," || t1 == "{" || t1 == "}")
                        {
                            UNEXPECTED_TOKEN_ERROR(t1)
                        }
                        if (t2 != "," && t2 != "}")
                        {
                            UNEXPECTED_TOKEN_ERROR(t2)
                        }
                        new_exp->m_val.push_back(t1);
                        if (t2 == "}")
                        {
                            break;
                        }
                    }
                }
                else
                {
                    // container val e.g. arg = {arg = val}
                    idx = BuildExpressionsHierarchy(tokens, &new_exp->m_val_container, err_dsc, idx);
                    if (err_dsc.size())
                    {
                        return 0;
                    }
                }
            }
            else
            {
                // single val
                new_exp->m_val.push_back(val);
            }

            // add expression
            container->m_exps.push_back(new_exp);
        }
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
_string FormatString(const char* text, ...)
{
    _string str;
    va_list arg_list;
    va_start(arg_list, text);
    int str_len = Platform_vscprintf(text, arg_list);
    str.resize(str_len);
    Platform_vsprintf((char*)str.c_str(), str_len + 1, text, arg_list);
    va_end(arg_list);
    return str;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
const char* FormatStringPreallocated(const char* text, ...)
{
    static char format_string_preallocated_buffer[512];
    format_string_preallocated_buffer[0] = 0;

    va_list arg_list;
    va_start(arg_list, text);
    Platform_vsprintf(format_string_preallocated_buffer, 512, text, arg_list);
    va_end(arg_list);

    return format_string_preallocated_buffer;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FormatBuffer(char* buffer, uint32 buffer_size, const char* text, ...)
{
    buffer[0] = 0;

    va_list arg_list;
    va_start(arg_list, text);
    Platform_vsprintf(buffer, buffer_size, text, arg_list);
    va_end(arg_list);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
_string ExtractFileNameFromPath(const _string& path)
{
    size_t p1 = path.find_last_of("\\");
    if (p1 == _string::npos)
    {
        p1 = path.find_last_of("/");
    }
    if (p1 == _string::npos)
    {
        p1 = -1;
    }

    size_t p2 = path.find_last_of(".");
    if (p2 == _string::npos)
    {
        return "";
    }

    _string result = path.substr(p1 + 1, p2 - p1 - 1);

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
_string ExtractFileNameAndExtFromPath(const _string& path)
{
    size_t len = path.size();
    size_t p = path.find_last_of("\\");
    if (p == _string::npos)
    {
        p = path.find_last_of("/");
    }
    if (p == _string::npos)
    {
        p = -1;
    }

    _string tmp = path.substr(p + 1, len - p - 1);

    return tmp;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
_string ExcludeFileNameAndExtFromPath(const _string& path)
{
    size_t p = path.find_last_of("\\");
    if (p == _string::npos)
    {
        p = path.find_last_of("/");
    }
    if (p == _string::npos)
    {
        return "";
    }

    _string tmp = path.substr(0, p + 1);

    return tmp;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
_string ExtractParentFolderNameFromPath(const _string& path)
{
    size_t p2 = path.find_last_of("\\");
    if (p2 == _string::npos)
    {
        p2 = path.find_last_of("/");
    }
    if (p2 == _string::npos)
    {
        return "";
    }

    size_t p1 = path.find_last_of("\\", p2 - 1);
    if (p1 == _string::npos)
    {
        p1 = path.find_last_of("/", p2 - 1);
    }
    if (p1 == _string::npos)
    {
        return "";
    }

    _string tmp = path.substr(p1 + 1, p2 - p1 - 1);

    return tmp;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
_string RemoveWhitespacesAndTabsFromArgument(const char* arg)
{
    _string tmp = arg;

    size_t p = tmp.find_first_not_of(" \t");
    if (_string::npos == p)
    {
        return "";
    }

    tmp = tmp.substr(p, tmp.size() - p);

    p = tmp.find_last_not_of(" \t");
    if (_string::npos != p)
    {
        tmp = tmp.substr(0, p + 1);
    }

    return tmp;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
_string UnquoteArgument(const char* arg)
{
    _string tmp = arg;

    size_t p = tmp.find("\"");
    if (_string::npos == p)
    {
        return "";
    }

    tmp = tmp.substr(p + 1, tmp.size() - p - 1);

    p = tmp.find("\"");
    if (_string::npos == p)
    {
        return "";
    }

    tmp = tmp.substr(0, p);

    return tmp;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
_string QuoteArgument(const char* arg)
{
    return FormatString("\"%s\"", arg);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ParseCommaSeparatedArguments(const char* arg_, _vector<_string>& result)
{
    _string arg = arg_;
    size_t last = 0;
    for (;;)
    {
        size_t next = arg.find(",", last);
        if (_string::npos == next)
        {
            size_t len = arg.size() - last;
            if (len)
            {
                result.push_back(arg.substr(last, len));
            }
            break;
        }

        result.push_back(arg.substr(last, next - last));

        last = next + 1;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
uint32 StringToBool(const char* arg)
{
    _string tmp = arg;

    errno_t result = _strupr_s((char*)tmp.c_str(), tmp.size() + 1);
    ASSERT(!result)

    if (tmp == "TRUE")
    {
        return 1;
    }
    else if (tmp == "FALSE")
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
const char* BoolToString(bool b)
{
    if (b)
    {
        return "true";
    }
    else
    {
        return "false";
    }
}

void Platform_vsprintf(char* buffer, size_t buf_size, const char* format, va_list pargs)
{
    vsprintf_s(buffer, buf_size, format, pargs);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int Platform_vscprintf(const char* format, va_list ap)
{
    return _vscprintf(format, ap);
}
