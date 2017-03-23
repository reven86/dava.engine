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

inline void Token::Reset()
{
    tid = tidInvalid;
}

inline bool Token::IsValid() const
{
    return tid != tidInvalid;
}

inline Token::operator bool() const
{
    return IsValid();
}

inline bool Token::operator==(const Token& t) const
{
    return t.tid == tid;
}

inline bool Token::operator<(const Token& t) const
{
    return t.tid < tid;
}

template <typename T>
Token TokenProvider<T>::Generate()
{
    static std::atomic<Token::Tid> stid = { Token::tidInvalid };
    return Token(++stid);
}

template <typename T>
bool TokenProvider<T>::IsValid(const Token& token)
{
    return token.IsValid();
}

} // namespace DAVA
