#include "Base/RttiType.h"
#include "Base/RttiInheritance.h"

namespace DAVA
{
RttiType::RttiType()
    : inheritance(nullptr, [](const RttiInheritance* inh) { if (nullptr != inh) delete inh; })
{
}
} // namespace DAVA
