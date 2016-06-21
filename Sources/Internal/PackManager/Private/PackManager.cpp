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

void PackManager::Initialize(const String& dbFileName_,
                             const FilePath& downloadPacksDir_,
                             const FilePath& readOnlyPacksDir_,
                             const String& packsUrlCommon_,
                             const String& architecture_)
{
    if (!FileSystem::Instance()->IsFile(dbFileName_))
    {
        throw std::runtime_error("can't find: " + dbFileName_);
    }

    if (!FileSystem::Instance()->IsDirectory(downloadPacksDir_))
    {
        throw std::runtime_error("can't find dir: " + downloadPacksDir_.GetAbsolutePathname());
    }

    impl->Initialize(dbFileName_, downloadPacksDir_, readOnlyPacksDir_, packsUrlCommon_, architecture_, this);
}

PackManager::IInit& PackManager::GetInitialization()
{
    return *impl;
}

bool PackManager::IsRequestingEnabled() const
{
    return impl->IsProcessingEnabled();
}

void PackManager::EnableRequesting()
{
    impl->EnableProcessing();
}

void PackManager::DisableRequesting()
{
    impl->DisableProcessing();
}

void PackManager::Update()
{
    // first check if something downloading
    impl->Update();
}

const String& PackManager::FindPackName(const FilePath& relativePathInPack) const
{
    return impl->FindPackName(relativePathInPack);
}

const PackManager::Pack& PackManager::FindPack(const String& packName) const
{
    return impl->GetPack(packName);
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

const String& PackManager::GetRemotePacksUrl(bool isGPU) const
{
    return impl->GetRemotePacksURL(isGPU);
}

} // end namespace DAVA