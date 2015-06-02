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


#include "HashMapTest.h"
#include "Utils/Random.h"
#include "Base/HashMap.h"

HashMapTest::HashMapTest()
: TestTemplate<HashMapTest>("HashMapTest")
{
    ObjectFactory::Instance()->Dump();
	RegisterFunction(this, &HashMapTest::HashMapInsertRemoveGetTest, "HashMapInsertRemoveGetTest", NULL);
}

void HashMapTest::LoadResources()
{
    GetBackground()->SetColor(Color::White);
}

void HashMapTest::UnloadResources()
{
    
}

void HashMapTest::HashMapInsertRemoveGetTest(PerfFuncData * data)
{
	const int sz = 20000;
	DAVA::int32 i;
	DAVA::Vector<DAVA::uint32> vect;
	DAVA::HashMap<DAVA::int32, DAVA::uint32> map;
	
	vect.resize(sz);

	for(i = 0; i < sz; ++i)
	{
		DAVA::uint32 v = (i + 1); // any value
		vect[i] = v;
		map.insert(i, v);
	}

	// Get test
	for(i = 0; i < sz; ++i)
	{
		TEST_VERIFY(vect[i] == map[i]);
	}

// 	// remove some items
// 	for (int i = 0; i < sz/10; i++)
// 	{
// 		int index = DAVA::Random::Instance()->Rand(sz);
// 		vect[i] = 0;
// 		map.Remove(i);
// 	}

	// check get after remove
	for (int i = 0; i < sz; i++)
	{
		if(0 != vect[i])
		{
			TEST_VERIFY(vect[i] == map[i]);
		}
	}

	// iterator test
 	DAVA::HashMap<DAVA::int32, DAVA::uint32>::iterator iter = map.begin();
	for(; iter != map.end(); ++iter)
	{
//		TEST_VERIFY(vect[iter.GetKey()] == iter.GetValue());
	}

	// 0-size hash map iterator test
	DAVA::HashMap<DAVA::int32, DAVA::uint32> map0;
	iter = map0.begin();
	for (; iter != map0.end(); ++iter)
	{}
}
