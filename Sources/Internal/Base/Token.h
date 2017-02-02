#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
struct Token final
{
    Token() = default;

    void Reset();
    bool IsValid() const;

    operator bool() const;
    bool operator<(const Token&) const;

private:
    using Tid = uint64;
    static const Tid tidInvalid = 0;

    Token(Tid tid);

    Tid tid = Token::tidInvalid;

    template <typename T>
    friend class TokenProvider;
};

template <typename T>
class TokenProvider final
{
public:
    static Token Generate();
    static bool IsValid(const Token& token);
};

} // namespace DAVA

#define __Dava_Token__
#include "Base/Private/Token_impl.h"
