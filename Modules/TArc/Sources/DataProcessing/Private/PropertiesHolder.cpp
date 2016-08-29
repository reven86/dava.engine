#include "TArcCore/PropertiesHolder.h"
#include "Base/BaseTypes.h"
#include "Base/Any.h"

namespace DAVA
{
namespace TArc
{
struct PropertiesHolder::Impl
{
    //store all preferences to the file
    void StoreToFile();
    PropertiesHolder *parent = nullptr;
    DAVA::Vector<PropertiesHolder*> childs;
    DAVA::Map<String, Any> values;
};

PropertiesHolder::PropertiesHolder(const DAVA::String &projectName)
{

}

PropertiesHolder::PropertiesHolder(const PropertiesHolder &parent, &path)
{
}

void PropertiesHolder::Save(const Any &value, const DAVA::String &key);

Any PropertiesHolder::Load(const DAVA::String &key) const;

}
}