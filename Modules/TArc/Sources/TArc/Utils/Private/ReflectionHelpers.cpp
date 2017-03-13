#include "TArc/Utils/ReflectionHelpers.h"

#include <algorithm>

namespace DAVA
{
namespace TArc
{
void ForEachField(const Reflection& r, const Function<void(Reflection::Field&& field)>& fn)
{
    Vector<Reflection::Field> fields = r.GetFields();
    for (Reflection::Field& f : fields)
    {
        fn(std::move(f));
    }
}
} // namespace TArc
} // namespace DAVA