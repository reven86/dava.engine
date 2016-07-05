
#include "rhi_Pool.h"
#include "CommonImpl.h"

namespace rhi
{
namespace FrameLoop
{
void ProcessFrame();
static DAVA::Vector<FrameBase> frames;
static DAVA::Spinlock frameSync;
}
}
