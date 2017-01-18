#include "DAVAEngine.h"
#include "UnitTests/UnitTests.h"

#include "Utils/Random.h"
#include "Base/HashMap.h"

using namespace DAVA;

DAVA_TESTCLASS (HashMapTest)
{
    DEDUCE_COVERED_FILES_FROM_TESTCLASS()

    DAVA_TEST (HashMapInsertRemoveGetTest)
    {
        const int32 SIZE = 20000;
        Vector<DAVA::uint32> vect(SIZE, 0);
        HashMap<DAVA::int32, DAVA::uint32> map;

        for (int32 i = 0; i < SIZE; ++i)
        {
            uint32 v = (i + 1); // any value
            vect[i] = v;
            map.insert(i, v);
        }

        // Get test
        for (int32 i = 0; i < SIZE; ++i)
        {
            TEST_VERIFY(vect[i] == map[i]);
        }

        //// remove some items
        //for (int i = 0; i < sz/10; i++)
        //{
        //    int index = DAVA::Random::Instance()->Rand(sz);
        //    vect[i] = 0;
        //    map.Remove(i);
        //}

        // check get after remove
        for (int32 i = 0; i < SIZE; ++i)
        {
            if (0 != vect[i])
            {
                TEST_VERIFY(vect[i] == map[i]);
            }
        }

        // iterator test
        HashMap<int32, uint32>::iterator iter = map.begin();
        for (; iter != map.end(); ++iter)
        {
            // TEST_VERIFY(vect[iter.GetKey()] == iter.GetValue());
        }

        // 0-size hash map iterator test
        HashMap<int32, uint32> map0;
        iter = map0.begin();
        for (; iter != map0.end(); ++iter)
        {
        }
    }
}
;
