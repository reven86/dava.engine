#include "Debug/DVAssert.h"

using namespace DAVA::Assert;

static DAVA::Vector<Handler> registredHandlers;

void DAVA::Assert::AddHandler(const Handler handler)
{
    const DAVA::Vector<Handler>::iterator position = std::find(registredHandlers.begin(), registredHandlers.end(), handler);
    if (position != registredHandlers.end())
    {
        return;
    }

    registredHandlers.push_back(handler);
}

void DAVA::Assert::RemoveHandler(const Handler handler)
{
    const DAVA::Vector<Handler>::iterator position = std::find(registredHandlers.begin(), registredHandlers.end(), handler);
    if (position != registredHandlers.end())
    {
        registredHandlers.erase(position);
    }
}

const std::vector<Handler>& DAVA::Assert::GetHandlers()
{
    return registredHandlers;
}