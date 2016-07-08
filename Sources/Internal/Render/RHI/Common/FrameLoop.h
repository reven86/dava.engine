
#include "rhi_Pool.h"
#include "CommonImpl.h"

namespace rhi
{
namespace FrameLoop
{
static DAVA::Vector<FrameBase> frames;
static DAVA::Spinlock frameSync;

void ProcessFrame();
}
}
