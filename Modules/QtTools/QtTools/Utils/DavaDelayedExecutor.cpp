#include "QtTools/Utils/DavaDelayedExecutor.h"

#include <Engine/Engine.h>
#include <Debug/DVAssert.h>

DAVADelayedExecutor::DAVADelayedExecutor()
{
    using namespace DAVA;
    Engine* engine = Engine::Instance();
    DVASSERT(nullptr != engine);
    engine->update.Connect(this, &DAVADelayedExecutor::OnFrame);
    engine->backgroundUpdate.Connect(this, &DAVADelayedExecutor::OnFrame);
}

void DAVADelayedExecutor::DelayedExecute(DAVA::uint32 framesCount, const DAVA::Function<void()>& functor)
{
    functionsToExecute.push_back(std::make_pair(framesCount, functor));
}

void DAVADelayedExecutor::OnFrame(DAVA::float32)
{
    for (auto iter = functionsToExecute.begin(); iter != functionsToExecute.end();)
    {
        std::pair<DAVA::uint32, DAVA::Function<void()>>& functionPair = *iter;
        if (--functionPair.first == 0)
        {
            functionPair.second();
            iter = functionsToExecute.erase(iter);
        }
        else
        {
            ++iter;
        }
    }
}
