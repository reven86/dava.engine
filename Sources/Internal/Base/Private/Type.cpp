#include "Base/Type.h"
#include "Base/TypeInheritance.h"

namespace DAVA
{
Type::Type()
    : inheritance(nullptr, [](const TypeInheritance* inh) { if (nullptr != inh) delete inh; })
{
}
} // namespace DAVA
