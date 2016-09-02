#include "DVAssert.h"

#include <cstdarg>
#include <cstdio>

using namespace DAVA::Assert;

static const size_t formattedMessageBufferSize = 256;

static DAVA::Vector<Handler> registredHandlers;

void DAVA::Assert::AddHandler(const Handler handler)
{
    DAVA::Vector<Handler>::iterator position = std::find(registredHandlers.begin(), registredHandlers.end(), handler);
    if (position != registredHandlers.end())
    {
        return;
    }

    registredHandlers.push_back(handler);
}

void DAVA::Assert::RemoveHandler(const Handler handler)
{
    DAVA::Vector<Handler>::iterator position = std::find(registredHandlers.begin(), registredHandlers.end(), handler);
    if (position != registredHandlers.end())
    {
        registredHandlers.erase(position);
    }
}

const std::vector<Handler>& DAVA::Assert::GetHandlers()
{
    return registredHandlers;
}