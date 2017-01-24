#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
class Token final
{
    friend class TokenProvider;

public:
    Token() = default;
    ~Token() = default;

    Token(const Token&) = default;

private:
    using Tid = uint64;

    Tid tid = 0;

    Token(Tid tid_);
};

struct TokenProvider final
{
    template <typename T>
    static Token Generate();

    template <typename T>
    static bool IsValid(const Token& token);
};
} // namespace DAVA

#define __Dava_Token__
#include "Base/Private/Token_impl.h"
