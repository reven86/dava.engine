/*==================================================================================
Copyright (c) 2008, binaryzebra
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of the binaryzebra nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "Base/BaseTypes.h"
#include "FileSystem/FileSystem2.h"
#include "Functional/Signal.h"

namespace DAVA
{
    // Что-то начало проясняться, но не ясно, как сюда теперь ТЭГи добавить?
    class PackageImpl;
    class Dlc2Impl;
    class PackageRequestImpl;

    // on iOS represent Pakfile loaded throu On_Demand_Resources
    class Package
    {
    public:
        Package();
        Package(const Package&) = delete;
        ~Package();
        enum class Status
        {
            NOT_DOWNLOADED,
            PARTIALLY_DOWNLOADED,
            DOWNLOADING,
            DOWNLOADED
        };
        // TODO what about dependencies?
    private:
        std::unique_ptr<PackageImpl> impl;
    };

    class PackageRequest
    {
    public:
        PackageRequest();
        PackageRequest(const PackageRequest&) = delete;
        ~PackageRequest();

        Signal<void> finishLoading;
        Signal<const String&> errorLoading;
        Signal<float> downloadingProgressChange; // 0.0f..1.0f
        void Pause();
        void Resume();
        void Cancel();
    private:
        std::unique_ptr<PackageRequestImpl> impl; // can be platform dependent
    };
    
    class Dlc2
    {
    public:
        Dlc2();
        ~Dlc2();

        // assume game installed with database of all game files, for current version
        void Init(const Path& dlc2DbFilePath, bool mountLocalPackages = false);
        // during game start we have to do local cash update and build all data structures
        // to quickly reload and find any file later with constant speed
        void MountLocalPackages();
        // list all local packages, for debug purposes 
        const Vector<Package>& GetLocalPackages() const;

        // give client ability to read files from local cache
        // this is all user need when all packages loaded
        const FileSystem2& GetFileSystem() const;

        Package::Status GetPackageStatus(const String& packageName) const;
        // async download package to local cache, when done fire callback
        // if all good errorMessage is empty
        PackageRequest StartDownloadingPackage(const String& packageName, const String& url, float priority_0_1) const;
        // return empty string if not found
        String FindPackageName(const Path& filepath) const;

        // TODO
        // 1. add signals on finish loading package
        // 2. add progress status to show user current multiple packages downloading status and time
        // 3. add TAGs to better handle HD/SD ru/en etc resources and API for it
        // 4. pause, resume, cancel downloads
        // 5. fire LOW_DISKSPACE_WARNING
        // 6. АХТУНГ!!! если делать так как сделано в доках Мака
        // https://developer.apple.com/library/prerelease/ios/documentation/FileManagement/Conceptual/On_Demand_Resources_Guide/
        // то есть проблема с тем, что тэги на файлах, сделаны по другому принципу, и в разных Паках НЕ может быть
        // ресурсов с одним и тем же ТЭГом!
        // Мы решаем эту проблему, так - через систему качаем только наши Паки, с одноименным названием файла и ТЭГа
        // а дальше используем свои тэги которые прозрачны и например ТЭГ (ru) может быть в куче разных Паках, на отдельных файлах
    private:
        std::unique_ptr<Dlc2Impl> impl;
    };
} // end namespace DAVA