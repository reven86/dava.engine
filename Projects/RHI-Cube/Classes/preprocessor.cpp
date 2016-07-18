/*-------------------------------------------------------------------------------------------------
file:   preprocessor.cpp
author: Mosiychuck Dmitry
dsc:
-------------------------------------------------------------------------------------------------*/
#include "preprocessor.h"
#include "file.h"

std::string CPreprocessor::m_errorString;

///////////////////////////////////////////////////////////////////////////////////////////////////
CPreprocessor::CExpression* CPreprocessor::CExpression::CreateExpression(CToken* token)
{
    if (*token == "+")
    {
        return new CExpPlus(token);
    }
    else if (*token == "-")
    {
        return new CExpMinus(token);
    }
    else if (*token == "(")
    {
        return new CExpParenthesis(token, EXP_OP_PARENTHESIS_OPEN);
    }
    else if (*token == ")")
    {
        return new CExpParenthesis(token, EXP_OP_PARENTHESIS_CLOSE);
    }
    else if (*token == "*")
    {
        return new CExpMul(token);
    }
    else if (*token == "/")
    {
        return new CExpDiv(token);
    }
    else if (*token == "!")
    {
        return new CExpNot(token);
    }
    else if (*token == "&&")
    {
        return new CExpLogicalAnd(token);
    }
    else if (*token == "||")
    {
        return new CExpLogicalOr(token);
    }
    else if (*token == "==")
    {
        return new CExpEqual(token);
    }
    else if (*token == "!=")
    {
        return new CExpNotEqual(token);
    }
    else if (*token == "<")
    {
        return new CExpLess(token);
    }
    else if (*token == "<=")
    {
        return new CExpLessEqual(token);
    }
    else if (*token == ">")
    {
        return new CExpMore(token);
    }
    else if (*token == ">=")
    {
        return new CExpMoreEqual(token);
    }

    return new CExpConstant(token);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CPreprocessor::CPreprocessor(void)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CPreprocessor::~CPreprocessor(void)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CPreprocessor::TokenizeFile(const char* path, std::list<CToken>* tokens)
{
    if (!path || !tokens)
    {
        return false;
    }

    // load file
    TFile file;
    if (!file.OpenExisting(path))
    {
        return false;
    }
    uint32 fileSize = 0;
    if (!file.Size(&fileSize))
    {
        return false;
    }
    if (!fileSize)
    {
        return false;
    }
    char* string = new char[fileSize];
    if (!file.Read((void*)string, fileSize))
    {
        delete[] string;
        return false;
    }

    // tokenize file contents
    TokenizeString(string, fileSize, tokens);

    delete[] string;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CPreprocessor::TokenizeString(const char* string, size_t strLen, std::list<CToken>* tokens)
{
    ASSERT(string)
    ASSERT(tokens)

    if (!strLen)
    {
        return;
    }
    uint32 tab_size = 4;
    char c[3];
    c[2] = 0;
    bool lineCommentStarted = false;
    bool scopeCommentStarted = false;
    bool bracketsStarted = false;
    uint32 line = 1;
    uint32 column = 1;
    uint32 token_start = 1;
    std::string token;
    token.reserve(100);
    for (size_t i = 0; i < strLen;)
    {
        c[0] = string[i];
        c[1] = i < strLen - 1 ? string[i + 1] : 0;
        if (c[0] == '\r' && c[1] == '\n')
        {
            if (token.size())
            {
                tokens->push_back(CToken(token.data(), token_start, line));
                token.clear();
            } // flush token
            tokens->push_back(CToken("", 0, 0)); // end_of_line token
            ++line;
            i += 2;
            column = 1;
            lineCommentStarted = false;
            continue;
        }
        if (c[0] == '*' && c[1] == '/')
        {
            i += 2;
            column += 2;
            scopeCommentStarted = false;
            continue;
        }
        if (lineCommentStarted || scopeCommentStarted)
        {
            ++i;
            if (c[0] == '\t')
            {
                column += tab_size - (column - 1) % tab_size;
            }
            else
            {
                ++column;
            }
            continue;
        }
        if (c[0] == '/' && c[1] == '/')
        {
            i += 2;
            column += 2;
            lineCommentStarted = true;
            continue;
        }
        if (c[0] == '/' && c[1] == '*')
        {
            i += 2;
            column += 2;
            scopeCommentStarted = true;
            continue;
        }

        // check for brackets(") start/end
        if (c[0] == '\"')
        {
            bracketsStarted = !bracketsStarted;
            if (!bracketsStarted) // flush token on brackets end
            {
                c[1] = 0;
                token += c;
                tokens->push_back(CToken(token.data(), token_start, line));
                token.clear(); // flush token
                ++i;
                ++column;
                continue;
            }
        }

        // ignore symbol tokens if we are inside brackets
        if (!bracketsStarted)
        {
            if (c[0] == ' ' || c[0] == '\t')
            {
                if (token.size())
                {
                    tokens->push_back(CToken(token.data(), token_start, line));
                    token.clear();
                } // flush token
                ++i;
                if (c[0] == ' ')
                {
                    ++column;
                }
                else
                {
                    column += tab_size - (column - 1) % tab_size;
                }
                continue;
            }

            if ((c[0] == '&' && c[1] == '&') ||
                (c[0] == '|' && c[1] == '|') ||
                (c[0] == '+' && c[1] == '+') ||
                (c[0] == '-' && c[1] == '-') ||
                (c[0] == '=' && c[1] == '=') ||
                (c[0] == '!' && c[1] == '=') ||
                (c[0] == '+' && c[1] == '=') ||
                (c[0] == '-' && c[1] == '=') ||
                (c[0] == '*' && c[1] == '=') ||
                (c[0] == '/' && c[1] == '=') ||
                (c[0] == '&' && c[1] == '=') ||
                (c[0] == '|' && c[1] == '=') ||
                (c[0] == '>' && c[1] == '=') ||
                (c[0] == '<' && c[1] == '=') ||
                (c[0] == '>' && c[1] == '>') ||
                (c[0] == '<' && c[1] == '<') ||
                (c[0] == '-' && c[1] == '>'))
            {
                if (token.size())
                {
                    tokens->push_back(CToken(token.data(), token_start, line));
                    token.clear();
                } // flush token
                tokens->push_back(CToken(c, column, line, true));
                token.clear();
                i += 2;
                column += 2;
                continue;
            }

            if (c[0] == '{' || c[0] == '}' ||
                c[0] == '(' || c[0] == ')' ||
                c[0] == '[' || c[0] == ']' ||
                c[0] == '>' || c[0] == '<' ||
                c[0] == ',' ||
                //c[0] == '.' ||    // TODO: temporary solution to handle floats correctly
                c[0] == ';' ||
                c[0] == ':' ||
                c[0] == '+' ||
                c[0] == '-' ||
                c[0] == '*' ||
                c[0] == '/' ||
                c[0] == '%' ||
                c[0] == '&' ||
                c[0] == '|' ||
                c[0] == '!' ||
                c[0] == '~' ||
                c[0] == '?' ||
                c[0] == '=')
            {
                if (token.size())
                {
                    tokens->push_back(CToken(token.data(), token_start, line));
                    token.clear();
                } // flush token
                c[1] = 0;
                tokens->push_back(CToken(c, column, line, true));
                token.clear();
                ++i;
                ++column;
                continue;
            }
        }

        // fill current token
        if (!token.size())
        {
            token_start = column;
        }

        c[1] = 0;
        token += c;

        ++i;
        if (c[0] == '\t')
        {
            column += tab_size - (column - 1) % tab_size;
        }
        else
        {
            ++column;
        }
    }

    // flush last token
    if (token.size())
    {
        tokens->push_back(CToken(token.data(), token_start, line));
    }
    tokens->push_back(CToken("", 0, 0)); // end_of_line token
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CPreprocessor::Preprocess(const char* path, std::list<CToken>& tokens, std::vector<CDefineDsc>* pDefines)
{
    if (!path)
    {
        return false;
    }

    m_errorString.clear();

    if (!TokenizeFile(path, &tokens))
    {
        m_errorString = "Bad file path";
        return false;
    }

    std::vector<std::string> includePathes;
    std::list<CDefineDscInternal> defines;

    // get initial defines
    if (pDefines)
    {
        size_t numDefines = pDefines->size();
        for (size_t i = 0; i < numDefines; ++i)
        {
            CDefineDscInternal dsc;
            dsc.m_name = CToken(pDefines->at(i).m_name.data());
            TokenizeString(pDefines->at(i).m_dsc.data(), pDefines->at(i).m_dsc.size(), &dsc.m_dsc);
            if (dsc.m_dsc.size())
            {
                // remove <end_of_line> token from description tokens
                dsc.m_dsc.erase(--dsc.m_dsc.end());
            }
            defines.push_back(dsc);
        }
    }

    // preprocess tokens
    std::list<CToken>::iterator iterTokensEnd = tokens.end();
    for (std::list<CToken>::iterator iterToken = tokens.begin();
         iterToken != iterTokensEnd;)
    {
        //
        // #if, #elif, #else, #endif
        //
        if (*iterToken == "#if")
        {
            // find all #if #elif #else branches and their expressions
            std::vector<std::list<CToken>::iterator> branches;
            branches.reserve(20);
            branches.push_back(iterToken);
            branches.push_back(GetNextLine(iterToken));

            int nesting = 0;
            bool elseFound = false;
            std::list<CToken>::iterator iterTmp = iterToken;
            ++iterTmp;
            for (;;)
            {
                if (nesting < 0)
                {
                    break;
                }
                if (iterTmp == iterTokensEnd)
                {
                    m_errorString = FormatString("No corresponding #endif instruction found for #if instruction in line: %d, column: %d", iterToken->GetLine(), iterToken->GetColumn());
                    return false;
                }
                if (*iterTmp == "#ifdef" || *iterTmp == "#ifndef" || *iterTmp == "#if")
                {
                    ++iterTmp;
                    ++nesting;
                    continue;
                }
                if (*iterTmp == "#endif")
                {
                    ++iterTmp;
                    --nesting;
                    continue;
                }
                if (nesting)
                {
                    ++iterTmp;
                    continue;
                }
                ///////////////////////////////////////////

                if (*iterTmp == "#else")
                {
                    elseFound = true;
                    branches.push_back(iterTmp);
                    ++iterTmp;
                    branches.push_back(iterTmp);
                    continue;
                }
                if (*iterTmp == "#elif")
                {
                    if (elseFound)
                    {
                        m_errorString = FormatString("Unexpected #elif instruction found in line: %d, column: %d", iterTmp->GetLine(), iterTmp->GetColumn());
                        return false;
                    }
                    branches.push_back(iterTmp);
                    iterTmp = GetNextLine(iterTmp);
                    branches.push_back(iterTmp);
                    continue;
                }
                ++iterTmp;
            }
            // store last (#endif) branch
            --iterTmp;
            branches.push_back(iterTmp);
            ++iterTmp;
            branches.push_back(iterTmp);

            // evaluate branches
            size_t numEntrys = branches.size();
            for (size_t i = 0; i < numEntrys; ++++i)
            {
                if (*branches[i] == "#endif")
                {
                    tokens.erase(branches[0], branches[numEntrys - 1]);
                    iterToken = branches[numEntrys - 1];
                    break;
                }

                int expResult = 0;
                if (*branches[i] != "#else")
                {
                    std::list<CToken>::iterator iterExpBegin = branches[i];
                    std::list<CToken>::iterator iterExp = iterExpBegin;
                    ++iterExp;
                    if (*iterExp == "")
                    {
                        m_errorString = FormatString("Empty expression after #if / #elif instruction in line: %d, column: %d", branches[i]->GetLine(), branches[i]->GetColumn());
                        return false;
                    }
                    while (*iterExp != "")
                    {
                        if (!PreprocessDeclaration(tokens, defines, iterExp))
                        {
                            return false;
                        }
                    }
                    ++iterExpBegin;
                    if (!EvaluateExpression(iterExpBegin, &expResult))
                    {
                        return false;
                    }
                }
                if (expResult || *branches[i] == "#else")
                {
                    if (branches[i + 1] == branches[i + 2])
                    {
                        // this may happen if there are no tokens inside result branch

                        tokens.erase(branches[0], branches[numEntrys - 1]);
                        iterToken = branches[numEntrys - 1];
                        break;
                    }
                    tokens.erase(branches[0], branches[i + 1]);
                    tokens.erase(branches[i + 2], branches[numEntrys - 1]);
                    iterToken = branches[i + 1];
                    break;
                }
            }

            continue;
        }

        //
        // #ifdef, #ifndef, #else, #endif
        //
        if (*iterToken == "#ifdef" || *iterToken == "#ifndef")
        {
            std::string errorString = FormatString("Bad #ifdef / #ifndef syntax in line: %d, column: %d", iterToken->GetLine(), iterToken->GetColumn());

            std::list<CToken>::iterator iterTmp = iterToken;
            ++iterTmp;
            if (*iterTmp == "")
            {
                m_errorString = errorString.data();
                return false;
            }

            bool defined = false;
            std::list<CDefineDscInternal>::iterator iterDefine = std::find(defines.begin(), defines.end(), *iterTmp);
            if (iterDefine != defines.end())
            {
                defined = true;
            }

            ++iterTmp;
            if (*iterTmp != "")
            {
                m_errorString = errorString.data();
                return false;
            }

            ++iterTmp;

            // look for #else, #endif
            std::vector<std::list<CToken>::iterator> branches;
            branches.reserve(6);
            branches.push_back(iterToken);
            branches.push_back(iterTmp);

            int nesting = 0;
            for (;;)
            {
                if (nesting < 0)
                {
                    break;
                }
                if (iterTmp == iterTokensEnd)
                {
                    m_errorString = FormatString("No corresponding #endif instruction found for #ifdef / #ifndef instruction in line: %d, column: %d", iterToken->GetLine(), iterToken->GetColumn());
                    return false;
                }
                if (*iterTmp == "#ifdef" || *iterTmp == "#ifndef" || *iterTmp == "#if")
                {
                    ++iterTmp;
                    ++nesting;
                    continue;
                }
                if (*iterTmp == "#endif")
                {
                    ++iterTmp;
                    --nesting;
                    continue;
                }
                if (nesting)
                {
                    ++iterTmp;
                    continue;
                }
                ///////////////////////////////////////////

                if (*iterTmp == "#else")
                {
                    branches.push_back(iterTmp);
                    ++iterTmp;
                    branches.push_back(iterTmp);
                    continue;
                }
                ++iterTmp;
            }
            // store last (#endif) branch
            --iterTmp;
            branches.push_back(iterTmp);
            ++iterTmp;
            branches.push_back(iterTmp);

            // evaluate branches
            size_t numEntrys = branches.size();
            for (size_t i = 0; i < numEntrys; ++++i)
            {
                if (*branches[i] == "#endif")
                {
                    tokens.erase(branches[0], branches[numEntrys - 1]);
                    iterToken = branches[numEntrys - 1];
                    break;
                }
                if ((*branches[i] == "#ifdef" && defined) ||
                    (*branches[i] == "#ifndef" && !defined) ||
                    (*branches[i] == "#else"))
                {
                    if (branches[i + 1] == branches[i + 2])
                    {
                        // this may happen if there are no tokens inside result branch

                        tokens.erase(branches[0], branches[numEntrys - 1]);
                        iterToken = branches[numEntrys - 1];
                        break;
                    }
                    tokens.erase(branches[0], branches[i + 1]);
                    tokens.erase(branches[i + 2], branches[numEntrys - 1]);
                    iterToken = branches[i + 1];
                    break;
                }
            }

            continue;
        }

        //
        // #undef
        //
        if (*iterToken == "#undef")
        {
            std::list<CToken>::iterator iterTmp = iterToken;
            ++iterTmp;
            if (*iterTmp == "")
            {
                m_errorString = FormatString("Bad #undef syntax in line: %d, column: %d", iterToken->GetLine(), iterToken->GetColumn());
                return false;
            }
            std::list<CDefineDscInternal>::iterator iterDefine = std::find(defines.begin(), defines.end(), *iterTmp);
            if (iterDefine != defines.end())
            {
                defines.erase(iterDefine);
            }
            ++iterTmp;
            if (*iterTmp != "")
            {
                m_errorString = FormatString("Bad #undef syntax in line: %d, column: %d", iterToken->GetLine(), iterToken->GetColumn());
                return false;
            }
            ++iterTmp;
            iterToken = tokens.erase(iterToken, iterTmp);
            continue;
        }

        //
        // #define
        //
        if (*iterToken == "#define")
        {
            std::string errorString = FormatString("Bad #define syntax in line: %d, column: %d", iterToken->GetLine(), iterToken->GetColumn());
            std::list<CToken>::iterator iterTmp = iterToken;
            ++iterTmp;
            if (*iterTmp == "")
            {
                m_errorString = errorString.data();
                return false;
            }

            CDefineDscInternal dsc;
            dsc.m_name = *iterTmp;

            std::list<CDefineDscInternal>::iterator iterDefine = std::find(defines.begin(), defines.end(), dsc.m_name);
            if (iterDefine != defines.end())
            {
                m_errorString = FormatString("Definition of: %s was already made in line: %d, column: %d", iterDefine->m_name.GetString(), iterDefine->m_name.GetLine(), iterDefine->m_name.GetColumn());
                return false;
            }

            ++iterTmp;
            if (*iterTmp == "")
            {
                // #define without body (#define NAME <end_of_line>)
            }
            else if (*iterTmp == "(")
            {
                // #define with arguments (#define NAME(a, b, ...) DSC <end_of_line>)
                ++iterTmp;

                bool error = false;
                bool comma = true;
                for (;;)
                {
                    if (*iterTmp == "")
                    {
                        error = true;
                        break;
                    }
                    if (*iterTmp == ")")
                    {
                        ++iterTmp;
                        break;
                    }
                    if (*iterTmp == "," && comma)
                    {
                        error = true;
                        break;
                    }
                    if (*iterTmp != "," && !comma)
                    {
                        error = true;
                        break;
                    }
                    if (comma)
                    {
                        dsc.m_args.push_back(*iterTmp);
                    }
                    comma = !comma;
                    ++iterTmp;
                }
                while (*iterTmp != "")
                {
                    dsc.m_dsc.push_back(*iterTmp);
                    ++iterTmp;
                }

                size_t numArgs = dsc.m_args.size();
                if (!numArgs || comma)
                {
                    error = true;
                }
                std::list<CToken>::iterator iterDscBegin = dsc.m_dsc.begin();
                std::list<CToken>::iterator iterDscEnd = dsc.m_dsc.end();
                for (size_t i = 0; i < numArgs; ++i)
                {
                    if (std::find(iterDscBegin, iterDscEnd, dsc.m_args[i]) == iterDscEnd)
                    {
                        error = true;
                        break;
                    }
                }
                if (error)
                {
                    m_errorString = errorString.data();
                    return false;
                }
            }
            else
            {
                // #define without arguments (#define NAME DSC <end_of_line>)
                while (*iterTmp != "")
                {
                    dsc.m_dsc.push_back(*iterTmp);
                    ++iterTmp;
                }
            }

            defines.push_back(dsc);

            ++iterTmp;
            iterToken = tokens.erase(iterToken, iterTmp);
            continue;
        }

        //
        // #include
        //
        if (*iterToken == "#include")
        {
            std::string errorString = FormatString("Bad #include syntax in line: %d, column: %d", iterToken->GetLine(), iterToken->GetColumn());
            std::list<CToken>::iterator iterTmp = iterToken;
            ++iterTmp;
            if (*iterTmp == "")
            {
                m_errorString = errorString.data();
                return false;
            }

            std::string includeFilePath = UnquoteArgument(iterTmp->GetString());
            if (!includeFilePath.size())
            {
                m_errorString = errorString.data();
                return false;
            }

            ++iterTmp;

            if (std::find(includePathes.begin(), includePathes.end(), includeFilePath) == includePathes.end())
            {
                std::list<CToken> includeTokens;
                if (!TokenizeFile(includeFilePath.data(), &includeTokens))
                {
                    m_errorString = FormatString("Bad #include file path in line: %d, column: %d", iterToken->GetLine(), iterToken->GetColumn());
                    return false;
                }

                iterTmp = tokens.erase(iterToken, iterTmp);

                bool firstTokenInserted = false;
                std::list<CToken>::iterator iter_end = includeTokens.end();
                for (std::list<CToken>::iterator iter = includeTokens.begin();
                     iter != iter_end; ++iter)
                {
                    if (!firstTokenInserted)
                    {
                        iterToken = tokens.insert(iterTmp, *iter);
                    }
                    else
                    {
                        tokens.insert(iterTmp, *iter);
                    }
                    firstTokenInserted = true;
                }
                includePathes.push_back(includeFilePath);
                continue;
            }

            iterToken = tokens.erase(iterToken, iterTmp);
            continue;
        }

        //
        // declaration
        //
        if (!PreprocessDeclaration(tokens, defines, iterToken))
        {
            return false;
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CPreprocessor::PreprocessDeclaration(std::list<CToken>& tokens, std::list<CDefineDscInternal>& defines, std::list<CToken>::iterator& iterToken)
{
    if (*iterToken == "defined")
    {
        std::string errorString = FormatString("Bad declaration syntax in line: %d, column: %d", iterToken->GetLine(), iterToken->GetColumn());
        std::list<CToken>::iterator iterTmp = iterToken;

        ++iterTmp;
        if (*iterTmp != "(" || *iterTmp == "")
        {
            m_errorString = errorString.data();
            return false;
        }

        ++iterTmp;
        if (*iterTmp == "")
        {
            m_errorString = errorString.data();
            return false;
        }

        bool defined = false;
        std::list<CDefineDscInternal>::iterator iterDefine = std::find(defines.begin(), defines.end(), *iterTmp);
        if (iterDefine != defines.end())
        {
            defined = true;
        }

        ++iterTmp;
        if (*iterTmp != ")" || *iterTmp == "")
        {
            m_errorString = errorString.data();
            return false;
        }

        ++iterTmp;

        tokens.erase(iterToken, iterTmp);
        if (defined)
        {
            tokens.insert(iterTmp, CToken("true"));
        }
        else
        {
            tokens.insert(iterTmp, CToken("false"));
        }
        iterToken = iterTmp;

        return true;
    }

    std::list<CDefineDscInternal>::iterator iterDefine = std::find(defines.begin(), defines.end(), *iterToken);
    if (iterDefine != defines.end())
    {
        std::string errorString = FormatString("Bad declaration syntax in line: %d, column: %d", iterToken->GetLine(), iterToken->GetColumn());
        std::list<CToken>::iterator iterTmp = iterToken;
        ++iterTmp;
        if (*iterTmp == "(")
        {
            // declaration with arguments
            ++iterTmp;

            std::vector<CToken> args;

            bool error = false;
            bool comma = true;
            for (;;)
            {
                if (*iterTmp == "")
                {
                    error = true;
                    break;
                }
                if (*iterTmp == ")")
                {
                    ++iterTmp;
                    break;
                }
                if (*iterTmp == "," && comma)
                {
                    error = true;
                    break;
                }
                if (*iterTmp != "," && !comma)
                {
                    error = true;
                    break;
                }
                if (comma)
                {
                    args.push_back(*iterTmp);
                }
                comma = !comma;
                ++iterTmp;
            }
            size_t numArgs = args.size();
            if (!numArgs || numArgs != iterDefine->m_args.size() || comma)
            {
                m_errorString = errorString.data();
                return false;
            }

            iterTmp = tokens.erase(iterToken, iterTmp);

            std::list<CToken> dsc = iterDefine->m_dsc;
            std::list<CToken>::iterator iterDscBegin = dsc.begin();
            std::list<CToken>::iterator iterDscEnd = dsc.end();

            for (size_t i = 0; i < numArgs; ++i)
            {
                std::list<CToken>::iterator iter = std::find(iterDscBegin, iterDscEnd, iterDefine->m_args[i]);
                ASSERT(iter != iterDscEnd)
                *iter = args[i];
            }

            iterToken = tokens.insert(iterTmp, *iterDscBegin);
            ++iterDscBegin;
            for (std::list<CToken>::iterator iter = iterDscBegin;
                 iter != iterDscEnd; ++iter)
            {
                tokens.insert(iterTmp, *iter);
            }
        }
        else
        {
            // declaration without arguments
            if (iterDefine->m_args.size())
            {
                m_errorString = errorString.data();
                return false;
            }
            if (!iterDefine->m_dsc.size())
            {
                m_errorString = errorString.data();
                return false;
            }

            iterTmp = tokens.erase(iterToken);

            std::list<CToken>::iterator iterDscBegin = iterDefine->m_dsc.begin();
            std::list<CToken>::iterator iterDscEnd = iterDefine->m_dsc.end();

            iterToken = tokens.insert(iterTmp, *iterDscBegin);
            ++iterDscBegin;
            for (std::list<CToken>::iterator iter = iterDscBegin;
                 iter != iterDscEnd; ++iter)
            {
                tokens.insert(iterTmp, *iter);
            }
        }

        return true;
    }

    ++iterToken;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CPreprocessor::EvaluateExpression(std::list<CToken>::iterator token, int* pResult)
{
    ASSERT(pResult)

    uint32 expLine = token->GetLine();
    uint32 expColumn = token->GetColumn();

    CExpression* exp = Lex(token);
    if (!BuildEvaluationTree(exp))
    {
        if (!m_errorString.size())
        {
            m_errorString = FormatString("Bad expression in line: %d, column: %d", expLine, expColumn);
        }
        return false;
    }
    CExpression* expBody = exp->m_nextExp; // skip terminator
    if (expBody->GetReturnType() != CExpression::EXP_RETURN_TYPE_BOOLEAN &&
        expBody->GetReturnType() != CExpression::EXP_RETURN_TYPE_INTEGER)
    {
        m_errorString = FormatString("Wrong expression type in line: %d, column: %d", expLine, expColumn);
        return false;
    }
    *pResult = (int)expBody->Evaluate();

    DestroyExpression(exp);

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CPreprocessor::CExpression* CPreprocessor::Lex(std::list<CToken>::iterator token)
{
    CExpression* expStart = new CExpression;
    CExpression* expEnd = new CExpression;
    CExpression* expPrevious = expStart;

    while (!token->Empty())
    {
        CExpression* expNext = CExpression::CreateExpression(&*token);
        expNext->m_prevExp = expPrevious;
        expPrevious->m_nextExp = expNext;
        expPrevious = expNext;
        ++token;
    }

    expPrevious->m_nextExp = expEnd;
    expEnd->m_prevExp = expPrevious;

    return expStart;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CPreprocessor::CExpression* CPreprocessor::BuildEvaluationTree(CExpression* exp, bool expectParenthesis)
{
    ASSERT(exp)

    CExpression::EXP_OP_CLASS expectedOp = CExpression::EXP_OP_TERMINATOR;
    if (expectParenthesis)
    {
        expectedOp = CExpression::EXP_OP_PARENTHESIS_CLOSE;
    }

    // recursively parse parenthesis ()
    CExpression* expStart = exp;
    exp = expStart->m_nextExp;
    while (*exp != expectedOp)
    {
        if (expectedOp == CExpression::EXP_OP_PARENTHESIS_CLOSE && *exp == CExpression::EXP_OP_TERMINATOR)
        {
            m_errorString = FormatString("No closing ')' operator found for the '(' operator in line: %d, column: %d", expStart->m_token->GetLine(), expStart->m_token->GetColumn());
            return 0;
        }
        if (expectedOp == CExpression::EXP_OP_TERMINATOR && *exp == CExpression::EXP_OP_PARENTHESIS_CLOSE)
        {
            m_errorString = FormatString("Unexpected ')' operator in line: %d, column: %d", exp->m_token->GetLine(), exp->m_token->GetColumn());
            return 0;
        }
        if (*exp == CExpression::EXP_OP_PARENTHESIS_OPEN)
        {
            exp = BuildEvaluationTree(exp, true);
            if (!exp)
            {
                return 0;
            }
        }
        else
        {
            exp = exp->m_nextExp;
        }
    }
    CExpression* expEnd = exp;

    // parse unary +- (right associative)
    exp = expEnd->m_prevExp;
    while (exp != expStart)
    {
        if (*exp == CExpression::EXP_OP_PLUS ||
            *exp == CExpression::EXP_OP_MINUS)
        {
            if (!exp->Parse(true))
            {
                return 0;
            }
        }
        exp = exp->m_prevExp;
    }

    // parse ! (right associative)
    exp = expEnd->m_prevExp;
    while (exp != expStart)
    {
        if (*exp == CExpression::EXP_OP_NOT)
        {
            if (!exp->Parse())
            {
                return 0;
            }
        }
        exp = exp->m_prevExp;
    }

    // parse binary */ (left associative)
    exp = expStart->m_nextExp;
    while (exp != expEnd)
    {
        if (*exp == CExpression::EXP_OP_MUL ||
            *exp == CExpression::EXP_OP_DIV)
        {
            if (!exp->Parse())
            {
                return 0;
            }
        }
        exp = exp->m_nextExp;
    }

    // parse binary +- (left associative)
    exp = expStart->m_nextExp;
    while (exp != expEnd)
    {
        if (*exp == CExpression::EXP_OP_PLUS ||
            *exp == CExpression::EXP_OP_MINUS)
        {
            if (!exp->Parse())
            {
                return 0;
            }
        }
        exp = exp->m_nextExp;
    }

    // parse < <= > >= (left associative)
    exp = expStart->m_nextExp;
    while (exp != expEnd)
    {
        if (*exp == CExpression::EXP_OP_LESS ||
            *exp == CExpression::EXP_OP_LESSEQUAL ||
            *exp == CExpression::EXP_OP_MORE ||
            *exp == CExpression::EXP_OP_MOREEQUAL)
        {
            if (!exp->Parse())
            {
                return 0;
            }
        }
        exp = exp->m_nextExp;
    }

    // parse == != (left associative)
    exp = expStart->m_nextExp;
    while (exp != expEnd)
    {
        if (*exp == CExpression::EXP_OP_EQUAL ||
            *exp == CExpression::EXP_OP_NOTEQUAL)
        {
            if (!exp->Parse())
            {
                return 0;
            }
        }
        exp = exp->m_nextExp;
    }

    // parse && || (left associative)
    exp = expStart->m_nextExp;
    while (exp != expEnd)
    {
        if (*exp == CExpression::EXP_OP_LOGICAL_AND ||
            *exp == CExpression::EXP_OP_LOGICAL_OR)
        {
            if (!exp->Parse())
            {
                return 0;
            }
        }
        exp = exp->m_nextExp;
    }

    // get number of expressions in subtree
    uint32 numExp = 0;
    exp = expStart->m_nextExp;
    while (*exp != expectedOp)
    {
        ++numExp;
        exp = exp->m_nextExp;
    }
    if (numExp != 1)
    {
        return 0;
    }

    // remove parenthesis
    if (expectedOp == CExpression::EXP_OP_PARENTHESIS_CLOSE)
    {
        CExpression* tmp = expEnd->m_nextExp;
        delete expStart;
        delete expEnd;
        expEnd = tmp;
    }

    return expEnd;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CPreprocessor::DestroyExpression(CExpression* exp)
{
    ASSERT(exp)
    CExpression* nextExp = 0;
    do
    {
        nextExp = exp->m_nextExp;
        delete exp;
        exp = nextExp;
    } while (nextExp);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
std::list<CToken>::iterator CPreprocessor::GetNextLine(std::list<CToken>::iterator iter)
{
    while (*iter != "")
    {
        ++iter;
    }
    ++iter;
    return iter;
}
