#pragma once

#include <Functional/Function.h>
#include <Base/BaseTypes.h>

namespace DAVA
{
class Landscape;
class Texture;
}

namespace LandscapeThumbnails
{
using Callback = DAVA::Function<void(DAVA::Landscape*, DAVA::Texture*)>;
using RequestID = DAVA::uint32;

const RequestID InvalidID = 0;

RequestID Create(DAVA::Landscape*, Callback callback);
void CancelRequest(RequestID);
};
