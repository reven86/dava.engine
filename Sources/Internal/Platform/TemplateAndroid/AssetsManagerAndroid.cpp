#include "AssetsManagerAndroid.h"
#include "FileSystem/FilePath.h"
#include "FileSystem/File.h"
#include "FileSystem/Private/ZipArchive.h"
#include "Base/RefPtr.h"

namespace DAVA
{
AssetsManagerAndroid::AssetsManagerAndroid() = default;

AssetsManagerAndroid::~AssetsManagerAndroid() = default;

static const String assetsDirectory = "assets/Data/";

void AssetsManagerAndroid::Init(const String& apkFileName)
{
    if (apk)
    {
        throw std::runtime_error("[AssetsManager::Init] should be initialized only once.");
    }

    FilePath apkPath(apkFileName);
    RefPtr<File> file(File::Create(apkPath, File::OPEN | File::READ));
    if (!file)
    {
        throw std::runtime_error("[AssetsManager::Init] can't open: " + apkFileName);
    }

    apk.reset(new ZipArchive(file, apkPath));
}

bool AssetsManagerAndroid::HasDirectory(const String& relativeDirName) const
{
    String nameInApk = assetsDirectory + relativeDirName;
    const Vector<ResourceArchive::FileInfo>& files = apk->GetFilesInfo();
    for (const ResourceArchive::FileInfo& info : files)
    {
        if (info.relativeFilePath.find(nameInApk) == 0)
        {
            return true;
        }
    }
    return false;
}

bool AssetsManagerAndroid::HasFile(const String& relativeFilePath) const
{
    return apk->HasFile(assetsDirectory + relativeFilePath);
}

bool AssetsManagerAndroid::LoadFile(const String& relativeFilePath, Vector<uint8>& output) const
{
    return apk->LoadFile(assetsDirectory + relativeFilePath, output);
}

} // DAVA namespace
