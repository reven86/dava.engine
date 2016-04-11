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
class SmartDlcImpl;

class SmartDlc final
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

        PackState(const String& name, Status state, float priority, float progress);
        PackState() = delete;

        String name; // уникальное имя пака
        Status state; // NotRequested default;
        float priority; // текущий приоритет закачки
        float downloadProgress; // from 0.0 to 1.0
        bool isCrc32Valid;
    };

    // 1. открываю SQlite базу вычитываю всю инфу по пакам (в случае ошибки - исключение, продолжать загружать игру не возможно)
    // 2. подключаю все паки которые находятся по пути localPacksDir (в случае ошибки, исключение, кто-то поудалял внутреннии ресурсы игры)
    // 3. сохраняю url на сервер для докачки новых паков (не трачу время на его проверку, если сервер лежит например или не доступен)
    SmartDlc(const FilePath& packsDB, const FilePath& localPacksDir, const String& remotePacksUrl);
    ~SmartDlc();

    // контроль фоновых загрузок (обработка запросов)
    // если обработка запросов не включена, то запросы всех паков переключаются в Queued состояние
    bool IsProcessingEnabled() const;
    void EnableProcessing();
    void DisableProcessing();

    // обновление состояния, всех паков, и т.д. должно вызываться каждый кадр
    void Update();

    // получение имени пака по относительному имени файла внтутри пака (если файл не принадлежит ни одному паку исключение)
    const String& FindPack(const FilePath& relativePathInPack) const;

    // получение статуса пака (исключение если неверный айдишник пака?)
    const PackState& GetPackState(const String& packID) const;

    // запрос пака или файла (запрос файла на самом деле
    // сделает запрос пака в котором этот файл находится)
    const PackState& RequestPack(const String& packID, float priority = 0.5f);

    // получение всех паков их состояний
    const Vector<PackState*>& GetAllState() const;

    // возможность освободить место на устройстве пользователя
    // удалив скаченный пак
    void DeletePack(const String& packID);

    // отслеживание статуса запросов
    Signal<const PackState&> onPackStateChanged;

private:
    std::unique_ptr<SmartDlcImpl> impl;
};

} // end namespace DAVA
