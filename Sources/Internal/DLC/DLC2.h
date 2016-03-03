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

#include <functional>

namespace DAVA
{
    // Я не понимаю как это сделать!!!!
    // по всем правилам, если у тебя единица с которой ты работаешь это файл, то всегда работаешь с файлами!
    // а если нужно пакеты, то тогда единица работы - пакет, для которого определены операции, скачать,
    // подключить, получить список файлов, получить список зависимостей(других пакетов), мы же
    // пытаемся клиенту дать возможность работать с файлами, а сами каким - то удобным образом докачиваем пакеты и 
    // кидаем об этом клиенту нотификации, я не предаставляю как это сделать, если только не отдать управление этим механизмом
    // самому клиенту, пускай он сам решает, что и когда догрузить, сам проверяет, что и где находится
    // Так же мне не ясно как файловая система должна сотрудничать c ДЛЦ системой
    // И еще как должны работать ТЭГИ и должны ли они быть в этой системе? Или это отдельная система
    // ресурсов которая работает поверх и файловой системы и ДЛЦ системы?

    // logically bunch of files
    // may be build over FileArchive LZ4
    // do we need it here?
    class Package;

    class Dlc2Impl;
    class Dlc2
    {
    public:
        Dlc2();
        ~Dlc2();

        // assume game installed with database of all game files, for current version
        void Init(const Path& dlc2DbFilePath);
        // during game start we have to do local cash update and build all data structures
        // to quickly reload and find any file later with constant speed
        void CheckLocalCashIntegrity(std::function<void(bool result, const String& errorMessage)>) const;
        // list all local packages
        const Vector<String>& GetLocalPackages() const;
        // give client ability to read files from local cache
        // this is all user need when all packages loaded
        FileSystem2& GetLocalCacheFileSystem();

        bool FindPackageInLocalCash(const String& packageName) const;
        // async download package to local cache, when done fire callback
        // if all good errorMessage is empty
        void AsyncDownloadPackageToLocalCash(const String& packageName, const String& url, float32 timeout, 
            std::function<void(const String& package, const String& errorMessage)> callback) const;
        // return empty string if not found
        String FindPackageNameByFilePath(const Path& filepath) const;
    private:
        std::unique_ptr<Dlc2Impl> impl;
    };
} // end namespace DAVA