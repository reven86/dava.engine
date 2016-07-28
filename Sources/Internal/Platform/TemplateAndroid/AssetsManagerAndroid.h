#pragma once

#include "Base/BaseTypes.h"
#include "Base/Singleton.h"

namespace DAVA
{
class ZipArchive;

class AssetsManager : public Singleton<AssetsManager>
{
public:
    AssetsManager();
    virtual ~AssetsManager();

    void Init(const String& packageName);

    bool HasFile(const String& relativeFilePath) const;
    bool LoadFile(const String& relativeFilePath, Vector<uint8>& output) const;

private:
    String packageName;

    std::unique_ptr<ZipArchive> apk;
};
};
