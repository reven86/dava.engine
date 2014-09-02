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


#ifndef __DAVAENGINE_MONGODB_OBJECT_H__
#define __DAVAENGINE_MONGODB_OBJECT_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"

namespace DAVA 
{
    
/** 
 \ingroup Mongodb
 \brief this class is mongodb object and it used if you want to work with mongodb
 */
class MongodbClient;
class MongodbObjectInternalData;
class MongodbObject: public BaseObject
{
    friend class MongodbClient;
    
protected:
    
    void * InternalObject();
    virtual ~MongodbObject();
public:
    
    MongodbObject();
    
    
    void EnableForEdit();
    void Finish();
    bool IsFinished();

	void SetUniqueObjectName();
    void SetObjectName(const String &objectname);
    String GetObjectName();

    void AddInt32(const String &fieldname, int32 value);
    int32 GetInt32(const String &fieldname);

    void AddInt64(const String &fieldname, int64 value);
    int64 GetInt64(const String &fieldname);

    void AddData(const String &fieldname, uint8 *data, int32 dataSize);
    bool GetData(const String &fieldname, uint8 *outData, int32 dataSize);

    void AddString(const String &fieldname, const String &value);
    String GetString(const String &fieldname);

    void AddDouble(const String &fieldname, double value);
    double GetDouble(const String &fieldname);

    void AddObject(const String &fieldname, MongodbObject *addObject);
    
    bool GetSubObject(MongodbObject *subObject, const String &fieldname);
    bool GetSubObject(MongodbObject *subObject, const String &fieldname, bool needFinished);
    
    void StartArray(const String &fieldname);
    void FinishArray();
    
    void StartObject(const String &fieldname);
    void FinishObject();
    
    void CopyFinished(MongodbObject *fromObject);
    
    void Copy(MongodbObject *fromObject);
    
    void Print();
    
protected:

	MongodbObjectInternalData *objectData;
};

};

#endif // __DAVAENGINE_MONGODB_OBJECT_H__

