#pragma once

#ifndef __Dava_Token__
#include "Base/Token.h"
#endif

#include <atomic>

namespace DAVA
{
inline Token::Token(Tid tid_)
    : tid(tid_)
{
}

template <typename T>
Token TokenProvider::Generate()
{
    static std::atomic<Token::Tid> tid = { 1 };
    return Token(tid++);
}

template <typename T>
bool TokenProvider::IsValid(const Token& token)
{
    return (token.tid > 0);
}

} // namespace DAVA
