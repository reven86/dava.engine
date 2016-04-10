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


#ifndef PREFERENCES_STORAGE
#define PREFERENCES_STORAGE

#include "FileSystem/KeyedArchive.h"
#include "FileSystem/FilePath.h"
#include "Base/StaticSingleton.h"
#include "PreferencesRegistrator.h"

namespace DAVA
{
class InspBase;
};

class PreferencesStorage : private DAVA::StaticSingleton<PreferencesStorage>
{
    friend struct PreferencesStorageWrapper;

public:
    PreferencesStorage();

    static void RegisterType(const DAVA::InspInfo* inspInfo, const PreferencesRegistrator::DefaultValuesList& defaultValues);
    static void RegisterPreferences(DAVA::InspBase* inspBase);
    static void UnregisterPreferences(const DAVA::InspBase* inspBase);
    static void SetupStoragePath(const DAVA::FilePath& defaultStorage, const DAVA::FilePath& localStorage);

private:
    void SetupStoragePathImpl(const DAVA::FilePath& defaultStorage, const DAVA::FilePath& localStorage);
    void RegisterPreferencesImpl(DAVA::InspBase* inspBase);
    void UnregisterPreferencesImpl(const DAVA::InspBase* inspBase);

    static DAVA::String GenerateKey(const DAVA::InspInfo* inspInfo);

    DAVA::FilePath localStorage;
    DAVA::ScopedPtr<DAVA::KeyedArchive> editorPreferences;

    struct ClassInfo;
    DAVA::Set<std::unique_ptr<ClassInfo>> registeredInsp;
};

struct PreferencesStorageWrapper
{
    PreferencesStorageWrapper();
    ~PreferencesStorageWrapper();
};

#endif //PREFERENCES_STORAGE
