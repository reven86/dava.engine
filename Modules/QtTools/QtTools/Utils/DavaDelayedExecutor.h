#pragma once
#include <Functional/Function.h>
#include <Base/BaseTypes.h>

class DAVADelayedExecutor final
{
public:
    DAVADelayedExecutor();

    void DelayedExecute(DAVA::uint32 framesCount, const DAVA::Function<void()>& functor);

private:
    void OnFrame(DAVA::float32);

    DAVA::List<std::pair<DAVA::uint32 /*framesCount*/, DAVA::Function<void()>>> functionsToExecute;
};
