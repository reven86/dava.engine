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


#include "FastName.h"
#include "Debug/DVAssert.h"

#include "Concurrency/LockGuard.h"

namespace DAVA
{

FastName::FastName()
	: index(-1)
{
    // make sure FastNameDB exists
    FastNameDB::Instance();

#ifdef __DAVAENGINE_DEBUG__
    debug_str_ptr = NULL;
#endif
}
    
FastName::FastName(const String & name)
    : index(-1)
{
    Init(name.c_str());
}

FastName::FastName(const char *name)
	: index(-1)
{
    Init(name);
}
    
FastName::FastName(const FastName &_name)
{
    index = _name.index;
    
#ifdef __DAVAENGINE_DEBUG__
    debug_str_ptr = _name.debug_str_ptr;
#endif
    
	if(-1 != index)
	{
		AddRef(index);
	}
}

FastName::~FastName()
{
	if(-1 != index)
	{
		RemRef(index);
	}
}

void FastName::Init(const char * name)
{
    DVASSERT(NULL != name);

    FastNameDB *db = FastNameDB::Instance();
    LockGuard<Mutex> guard(FastNameDB::Instance()->dbMutex);

    // search if that name is already in hash
    if(db->namesHash.count(name))
    {
        // already exist, so we just need to set the same index to this object
        index = db->namesHash[name];
        db->namesRefCounts[index]++;
    }
    else
    {
        // string isn't in hash and it isn't in names table, so we need to copy it
        // and find place for copied string in names table and set it
        size_t nameLen = strlen(name);
        char *nameCopy = (char *) malloc(nameLen + 1);
        memcpy(nameCopy, name, nameLen + 1);
        
        // search for empty indexes in names table
        if(db->namesEmptyIndexes.size() > 0)
        {
            // take last empty index from emptyIndexes table
            index = db->namesEmptyIndexes.back();
            db->namesEmptyIndexes.pop_back();
        }
        else
        {
            // index will be a new row in names table
            index = static_cast<int32>(db->namesTable.size());
            db->namesTable.resize(index + 1);
            db->namesRefCounts.resize(index + 1);
        }
        
        // set name to names table
        db->namesTable[index] = nameCopy;
        db->namesRefCounts[index] = 1;
        
        // add name and its index into hash
        db->namesHash.insert(nameCopy, index);
    }

    DVASSERT(index != -1);
#ifdef __DAVAENGINE_DEBUG__
    debug_str_ptr = c_str();
#endif
}

void FastName::AddRef(int i) const
{
    LockGuard<Mutex> guard(FastNameDB::Instance()->dbMutex);

	FastNameDB *db = FastNameDB::Instance();
	DVASSERT(i >= -1 && i < (int)db->namesTable.size());
    if(i >= 0)
    {
        db->namesRefCounts[i]++;
    }
}

void FastName::RemRef(int i) const
{
	LockGuard<Mutex> guard(FastNameDB::Instance()->dbMutex);

	FastNameDB *db = FastNameDB::Instance();
	DVASSERT(i >= -1 && i < (int)db->namesTable.size());
	if (i >= 0)
	{
		db->namesRefCounts[i]--;
		if(0 == db->namesRefCounts[i])
		{
			DVASSERT(db->namesTable[i] && "Need be not NULL");

			// remove name and index from hash
			db->namesHash.erase(db->namesTable[i]);

			// delete allocated memory for this string
			free((void *) db->namesTable[i]);

			// remove name from names table
			db->namesTable[i] = NULL;

			// remember that this index is empty already
			db->namesEmptyIndexes.push_back(i);
		}
	}
}
    
};
