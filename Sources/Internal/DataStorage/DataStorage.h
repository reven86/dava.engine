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


#ifndef __DATA_STORAGE_H__
#define __DATA_STORAGE_H__

#include "Base/BaseObject.h"

namespace DAVA
{
// we suppose that we can have a couple of Storages for windows Regystry or MacOS file system, or some new cloud API
class IDataStorage : public BaseObject
{
public:
    virtual String GetStringValue(const String &key) = 0;
    virtual int64 GetLongValue(const String &key) = 0;
    virtual void SetStringValue(const String &key, const String &value) = 0;
    virtual void SetLongValue(const String &key, int64 value) = 0;
    virtual void RemoveEntry(const String &key) = 0;
    virtual void Clear() = 0;
    virtual void Push() = 0;
};

// when we have a couple of DataStorages - we could change constructor and add some parameter to determine impl type.
class DataStorage
{
public:
    static IDataStorage *Create();

private:
    DataStorage(){};
    ~DataStorage(){};
};

} //namespace DAVA

#endif // __DATA_STORAGE_H__

