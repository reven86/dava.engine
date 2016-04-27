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


#include "Base/FastName.h"
#include "Base/Introspection.h"
#include "QtTools/EditorPreferences/PreferencesActionsFactory.h"
#include "Preferences/PreferencesStorage.h"
#include "QtTools/EditorPreferences/Actions/Actions.h"

QAction* PreferencesActionsFactory::CreateActionForPreference(const DAVA::FastName& className, const DAVA::FastName& propertyName, QObject* parent)
{
    const DAVA::InspInfo* inspInfo = PreferencesStorage::Instance()->GetInspInfo(className);
    const DAVA::InspMember* inspMember = inspInfo->Member(propertyName);
    DVASSERT(inspInfo != nullptr && inspMember != nullptr);
    const DAVA::MetaInfo* metaInfo = inspMember->Type();
    AbstractAction* action = nullptr;
    if (metaInfo == DAVA::MetaInfo::Instance<bool>())
    {
        action = new BoolAction(inspMember, parent);
    }
    else if (metaInfo == DAVA::MetaInfo::Instance<DAVA::int32>()
             || metaInfo == DAVA::MetaInfo::Instance<DAVA::int64>()
             || metaInfo == DAVA::MetaInfo::Instance<DAVA::uint32>()
             || metaInfo == DAVA::MetaInfo::Instance<DAVA::uint64>())
    {
        action = new IntAction(inspMember, parent);
    }
    else if (metaInfo == DAVA::MetaInfo::Instance<DAVA::float32>()
             || metaInfo == DAVA::MetaInfo::Instance<DAVA::float64>())
    {
        action = new DoubleAction(inspMember, parent);
    }
    else if (metaInfo == DAVA::MetaInfo::Instance<DAVA::String>()
             || metaInfo == DAVA::MetaInfo::Instance<DAVA::WideString>()
             || metaInfo == DAVA::MetaInfo::Instance<DAVA::FastName>()
             || metaInfo == DAVA::MetaInfo::Instance<DAVA::FilePath>())
    {
        action = new StringAction(inspMember, parent);
    }
    else if (metaInfo == DAVA::MetaInfo::Instance<DAVA::Color>())
    {
        action = new ColorAction(inspMember, parent);
    }
    else
    {
        DVASSERT(nullptr != action && "please create delegate for type");
        return nullptr;
    }
    action->Init();
    return action;
}
