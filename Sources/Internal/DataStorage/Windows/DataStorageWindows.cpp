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


#include "DataStorage/DataStorage.h"
#include "DataStorageWindows.h"

namespace DAVA
{

#if defined(__DAVAENGINE_WINDOWS__)

IDataStorage *DataStorage::Create()
{
    return new DataStorageWin();
}

#endif

#if defined(__DAVAENGINE_WIN_UAP__)
DataStorageWin::DataStorageWin()
{
    roamingSettings = ApplicationData::Current->RoamingSettings;
    DVASSERT(nullptr != roamingSettings); // something goes wrong
}

String DataStorageWin::GetStringValue(const String &key)
{

    auto values = roamingSettings->Values;
    IPropertyValue^ value = safe_cast<IPropertyValue^>(values->Lookup(StringToRTString(key)));

    if (nullptr != value && PropertyType::String == value->Type)
    {
        return RTStringToString(value->GetString());
    }
    else
    {
        return String();
    }
}

int64 DataStorageWin::GetLongValue(const String &key)
{
    auto values = roamingSettings->Values;
    IPropertyValue^ value = safe_cast<IPropertyValue^>(values->Lookup(StringToRTString(key)));
    if (nullptr != value && PropertyType::Int64 == value->Type)
    {
        return value->GetInt64();
    }
    else
    {
        return 0;
    }
}

void DataStorageWin::SetStringValue(const String &key, const String &value)
{
    auto values = roamingSettings->Values;
    values->Insert(StringToRTString(key), dynamic_cast<PropertyValue^>(PropertyValue::CreateString(StringToRTString(value))));
}

void DataStorageWin::SetLongValue(const String &key, int64 value)
{
    auto values = roamingSettings->Values;
    values->Insert(StringToRTString(key), dynamic_cast<PropertyValue^>(PropertyValue::CreateInt64(value)));
}

void DataStorageWin::RemoveEntry(const String &key)
{
    auto values = roamingSettings->Values;
    values->Remove(StringToRTString(key));
}

void DataStorageWin::Clear()
{
    auto values = roamingSettings->Values;
    values->Clear();
}
#endif
}