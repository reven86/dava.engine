#pragma once
#include <stdexcept>
#include "Base/BaseTypes.h"

namespace DAVA
{
struct Exception : public ::std::runtime_error
{
    Exception(const String& message, const char* file_, size_t line_);

    String file;
    size_t line;
    Vector<void*> callstack;
};

} // namespace DAVA

#define DAVA_THROW(e, ...) throw e(__VA_ARGS__, __FILE__, __LINE__)
