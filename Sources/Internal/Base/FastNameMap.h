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



#ifndef __DAVAENGINE_FAST_NAME_MAP__
#define __DAVAENGINE_FAST_NAME_MAP__

#include "FastName.h"

namespace DAVA
{

template<> struct Hash<FastName>
{
	size_t operator()(const FastName &name) const
	{
		return name.Index();
	}

	bool Compare(const FastName &name1, const FastName &name2)
	{
		return name1.Index() == name2.Index();
	}
};

class FastNameSet : public HashMap<FastName, int>
{
public:
	void Insert(const char *name)
	{
		HashMap<FastName, int>::Insert(FastName(name), 0);
	}

private:
	int Insert(const char *name, const int &value);
	int Value(const FastName &key);
	int operator()(const FastName &key);
};

template <typename V>
class FastNameMap : public HashMap<FastName, V>
{
public:
	void Insert(const char *name, const V &value)
	{
		HashMap<FastName, V>::Insert(FastName(name), value);
	}
	void Insert(const FastName & name, const V &value)
	{
		HashMap<FastName, V>::Insert(name, value);
	}
};

};

#endif // __DAVAENGINE_FAST_NAME_MAP__
