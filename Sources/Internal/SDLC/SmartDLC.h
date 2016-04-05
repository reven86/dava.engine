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

#include <Functional/Signal.h>

namespace DAVA
{
class SmartDlcImpl;

class SmartDlc final
{
public:
    using PackID = String;

    struct PackState
    {
        enum Status
        {
            NotExists, // не существует в списке файлов
            Exists, // существует в списке файлов, но не загружен на FS
            Queued, // существует в списке файлов, поставлен в очередь на загрузку
            Downloading, // существует в списке файлов, загружается на FS
            Ready // существует на FS и готов к использованию
        };

        PackID name = "";
        Status state = NotExists;
        float priority = 0.5f; // текущий приоритет закачки
        float progress = 0.0f; // download progress - from 0.0 to 1.0
    };

    SmartDlc(const FilePath& packsDB, const FilePath& localPacksDir, const String& remotePacksUrl);
    ~SmartDlc();

    // контроль фоновых загрузок (обработка запросов)
    bool IsStarted();
    void Start(); // TODO Restart()? Continue()?
    void Stop();

    // получение имени пока по запросу или по имени файла
    PackID FindPack(const FilePath& file);

    // получение статуса файла, пака, запроса
    const PackState& GetPackState(PackID packID);

    // получение паков, находящихся в очереди
    void GetRequestedPacks(Vector<PackState>& out) const;

    // запрос пака или файла (запрос файла на самом деле
    // сделает запрос пака в котором этот файл находится)
    const PackState& RequestFile(const FilePath& file, float priority = 0.5f);
    const PackState& RequestPack(const String& packName, float priority = 0.5f);

    // возможность освободить место на устройстве пользователя
    // удалив скаченный пак
    void DeletePack(PackID pid);

    // отслеживание статуса запросов
    Signal<const String, const PackState&> onPackStateChanged;

private:
    std::unique_ptr<SmartDlcImpl> impl;
};

} // end namespace DAVA
