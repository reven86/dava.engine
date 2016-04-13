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
#pragma once

#include <Functional/Signal.h>

namespace DAVA
{
class ArchiveManagerImpl;

class PackManager final
{
public:
    struct PackState
    {
        enum Status : uint32
        {
            NotRequested = 0, // не загружен на FS
            Requested = 1, // поставлен в очередь на загрузку
            Downloading = 2, // загружается на FS
            Mounted = 3 // существует на FS и готов к использованию
        };

        String name = ""; // уникальное имя пака
        Status state = NotRequested; // NotRequested default;
        String remoteUrl = ""; // url used for download archive or empty
        float32 downloadProgress = 0.f; // 0.0f to 1.0f
        float32 priority = 0.f; // 0.0f to 1.0f
        uint32 crc32FromMeta = 0; // crc32 from sub file or 0 (not read from
        uint32 crc32FromDB = 0; // crc32 from filesdb
        Vector<String> dependensy{}; // names of dependency archive
    };

    // 1. вычитываю данные по всем пакам из бызы
    // 2. перебираю все паки на файловой системе
    // 3. рядом с каждым паком лежит мета файл в котором его CRC32 которую сравниваю со значением в базе
    // 4. монтирую каждый пак у которого совпадает CRC32, если не совпадает, то было обновление игры и этот пак изменился его нужно удалить
    // 5. если что-то качалось и был краш или выход из игры то мы в любом случае докачиваем из промежуточных временных файлов, если можем, или качаем заново(по запросу)
    PackManager(const FilePath& filesDB, const FilePath& localPacksDir, const String& remotePacksUrl);
    ~PackManager();

    // контроль фоновых загрузок (обработка запросов)
    // если обработка запросов не включена, то запросы всех паков переключаются в Queued состояние
    bool IsProcessingEnabled() const;
    // включаем обработку если что-то есть в очереди - стартуем
    void EnableProcessing();
    // отключаем обработку запросов и останавливаем закачку если она была
    void DisableProcessing();

    // обновление состояния, всех паков, и т.д. должно вызываться каждый кадр
    void Update();

    // получение имени пака по относительному имени файла внтутри пака (если файл не принадлежит ни одному паку исключение)
    const String& FindPack(const FilePath& relativePathInArchive) const;

    // получение статуса пака (исключение если неверный айдишник пака?)
    const PackState& GetPackState(const String& packID) const;

    // запрос пака
    // 1. Важно! Если мы уже качаем один пак, и тут приходит заброс с более высоким приоритетом
    // 2. то мы останавливаем закачку прошлого пака и переключаемся на новый, т.к. это могут быть виртуальные паки
    // 3. но дополнительно мы проверяем зависимые паки, что бы не переключиться с него, если он нужен в новом запрошеном
    const PackState& RequestPack(const String& packID, float priority = 0.5f);

    // получение всех паков их состояний
    const Vector<PackState*>& GetAllState() const;

    // возможность освободить место на устройстве пользователя
    // удалив скаченный пак (так же отмонтирует его от FS)
    void DeletePack(const String& packID);

    // отслеживание статуса запросов
    Signal<const PackState&> onPackStateChanged;

private:
    std::unique_ptr<ArchiveManagerImpl> impl;
};

} // end namespace DAVA
