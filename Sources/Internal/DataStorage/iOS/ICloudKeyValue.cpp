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


#include "ICloudKeyValue.h"

#if defined(__DAVAENGINE_IPHONE__)

#import "Utils/NSStringUtils.h"

namespace DAVA
{

ICloudKeyValue::ICloudKeyValue()
{
    Sync();
}

String ICloudKeyValue::GetStringValue(const String &key)
{
    Logger::FrameworkDebug("Trying to Get String value for %s key", key.c_str());

    NSUbiquitousKeyValueStore *iCloudStorage = [NSUbiquitousKeyValueStore defaultStore];
    NSString *value = [iCloudStorage stringForKey : NSStringFromString(key)];

    if (nil != value)
    {
        return String([value UTF8String]);
    }

    return "";
}
    
int64 ICloudKeyValue::GetLongValue(const String &key)
{
    Logger::FrameworkDebug("Trying to Get Long value for %s key", key.c_str());
    
    NSUbiquitousKeyValueStore *iCloudStorage = [NSUbiquitousKeyValueStore defaultStore];
    int64 value = [iCloudStorage longLongForKey : NSStringFromString(key)];
    
    return value;
}

void ICloudKeyValue::SetStringValue(const String &key, const String &value)
{
    Logger::FrameworkDebug("Trying to set %s value for %s key", value.c_str(), key.c_str());
    
    NSUbiquitousKeyValueStore *iCloudStorage = [NSUbiquitousKeyValueStore defaultStore];
    [iCloudStorage setString: NSStringFromString(value) forKey: NSStringFromString(key)];
}

void ICloudKeyValue::SetLongValue(const String &key, int64 value)
{
    Logger::FrameworkDebug("Trying to set long %lld value for %s key", value, key.c_str());
    
    NSUbiquitousKeyValueStore *iCloudStorage = [NSUbiquitousKeyValueStore defaultStore];
    [iCloudStorage setLongLong: value forKey: NSStringFromString(key)];
}

void ICloudKeyValue::RemoveEntry(const String &key)
{
    NSUbiquitousKeyValueStore *iCloudStorage = [NSUbiquitousKeyValueStore defaultStore];
    [iCloudStorage removeObjectForKey: NSStringFromString(key)];
}

void ICloudKeyValue::Clear()
{
    NSUbiquitousKeyValueStore *iCloudStorage = [NSUbiquitousKeyValueStore defaultStore];
    Sync();
    NSDictionary *dict = [iCloudStorage dictionaryRepresentation];
    NSArray *arr = [dict allKeys];
    
    for (uint32 i=0; i < static_cast<uint32>(arr.count); i++)
    {
        NSString *key = [arr objectAtIndex: i];
        [iCloudStorage removeObjectForKey: key];
    }
}

void ICloudKeyValue::Push()
{
    Sync();
}
    
void ICloudKeyValue::Sync()
{
    NSUbiquitousKeyValueStore *iCloudStorage = [NSUbiquitousKeyValueStore defaultStore];
    [iCloudStorage synchronize];
}

}

#endif

