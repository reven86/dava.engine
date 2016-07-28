#include "Platform/TemplateAndroid/AssetsManagerAndroid.h"
#include "FileSystem/FilePath.h"
#include "FileSystem/File.h"
#include "FileSystem/Private/ZipArchive.h"
#include "Base/RefPtr.h"

namespace DAVA
{
AssetsManager::AssetsManager() = default;

AssetsManager::~AssetsManager() = default;

void AssetsManager::Init(const String& apkFileName)
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

bool AssetsManager::HasFile(const String& relativeFilePath) const
{
    return apk->HasFile("assets/" + relativeFilePath);
}

bool AssetsManager::LoadFile(const String& relativeFilePath, Vector<uint8>& output) const
{
    return apk->LoadFile("assets/" + relativeFilePath, output);
}

} // DAVA namespace
