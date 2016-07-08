
#include "rhi_Pool.h"
#include "CommonImpl.h"

namespace rhi
{
namespace FrameLoop
{
void ProcessFrame();
void RejectFrames();
static DAVA::Vector<FrameBase> frames;
static DAVA::Spinlock frameSync;
}
}
