#include "SharedIcon.h"
#include "Base/BaseTypes.h"

#include <QCoreApplication>

namespace SharedIconLocal
{
DAVA::UnorderedMap<DAVA::String, QIcon> sharedMap;
struct CleanUpRegistrator
{
    CleanUpRegistrator()
    {
        qAddPostRoutine([]()
                        {
                            sharedMap.clear();
                        });
    }
} cleanUpRegistrator;
}

const QIcon& SharedIcon(const char* path)
{
    using namespace SharedIconLocal;

    DAVA::String stringPath(path);
    auto iconIter = sharedMap.find(stringPath);
    if (iconIter != sharedMap.end())
        return iconIter->second;

    return sharedMap.emplace(std::move(stringPath), QIcon(path)).first->second;
}
