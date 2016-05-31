#include "PackManager/PackManager.h"
#include "PackManager/Private/PackManagerImpl.h"
#include "FileSystem/FileSystem.h"

namespace DAVA
{
PackManager::PackManager()
{
    impl.reset(new PackManagerImpl());
};

PackManager::~PackManager() = default;

PackManager::IRequest::~IRequest() = default;

void PackManager::Initialize(const FilePath& filesDB_,
                             const FilePath& downloadPacksDir_,
                             const FilePath& readOnlyPacksDir_,
                             const String& packsUrlCommon_,
                             const String& packsUrlGpu_)
{
    if (!FileSystem::Instance()->IsFile(filesDB_))
    {
        throw std::runtime_error("can't find: " + filesDB_.GetAbsolutePathname());
    }
    if (!FileSystem::Instance()->IsDirectory(downloadPacksDir_))
    {
        throw std::runtime_error("can't find dir: " + downloadPacksDir_.GetAbsolutePathname());
    }

    impl->Initialize(filesDB_, downloadPacksDir_, readOnlyPacksDir_, packsUrlCommon_, packsUrlGpu_, onPackStateChanged, onRequestProgressChanged);
}

bool PackManager::IsProcessingEnabled() const
{
    return impl->IsProcessingEnabled();
}

void PackManager::EnableProcessing()
{
    impl->EnableProcessing();
}

void PackManager::DisableProcessing()
{
    impl->DisableProcessing();
}

void PackManager::Update()
{
    // first check if something downloading
    impl->Update();
}

const String& PackManager::FindPack(const FilePath& relativePathInPack) const
{
    return impl->FindPack(relativePathInPack);
}

const PackManager::Pack& PackManager::RequestPack(const String& packID, float priority)
{
    return impl->RequestPack(packID, priority);
}

const Vector<PackManager::Pack>& PackManager::GetPacks() const
{
    return impl->GetAllState();
}

void PackManager::DeletePack(const String& packID)
{
    impl->DeletePack(packID);
}

const FilePath& PackManager::GetLocalPacksDirectory() const
{
    return impl->GetLocalPacksDir();
}

const String& PackManager::GetRemotePacksUrl() const
{
    return impl->GetRemotePacksURL();
}

} // end namespace DAVA