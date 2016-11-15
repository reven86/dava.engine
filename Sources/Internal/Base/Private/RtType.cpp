#include "Base/RtType.h"
#include "Base/RtTypeInheritance.h"

namespace DAVA
{
RtType::RtType()
    : inheritance(nullptr, [](const RtTypeInheritance* inh) { if (nullptr != inh) delete inh; })
{
}
} // namespace DAVA
