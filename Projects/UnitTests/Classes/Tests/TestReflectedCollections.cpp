#include "Base/Result.h"
#include "Reflection/Reflection.h"
#include "Reflection/ReflectionRegistrator.h"
#include "UnitTests/UnitTests.h"
#include "Logger/Logger.h"

#include <functional>

#if !defined(__DAVAENGINE_ANDROID__)

struct RelfCollectionsHolder
{
    /// VectorTest
    DAVA::Vector<int> intVector;
    DAVA::Vector<DAVA::String> stringVector;
    DAVA::Vector<int>* intPtrVector;

    /// ListTest
    DAVA::List<int> intList;
    DAVA::Map<int, int> mapColl;
    DAVA::UnorderedMap<int, int> unorderMap;

    DAVA::Set<int> intSet;
    DAVA::UnorderedSet<int> intUnorderSet;

    DAVA_DECLARE_TYPE_INITIALIZER;
};

DAVA_TYPE_INITIALIZER(RelfCollectionsHolder)
{
    DAVA::ReflectionRegistrator<RelfCollectionsHolder>::Begin()
    .Field("intVector", &RelfCollectionsHolder::intVector)
    .Field("stringVector", &RelfCollectionsHolder::stringVector)
    .Field("intPtrVector", &RelfCollectionsHolder::intPtrVector)
    .Field("intList", &RelfCollectionsHolder::intList)
    .Field("mapColl", &RelfCollectionsHolder::mapColl)
    .Field("unorderMap", &RelfCollectionsHolder::unorderMap)
    .Field("intSet", &RelfCollectionsHolder::intSet)
    .Field("intUnorderSet", &RelfCollectionsHolder::intUnorderSet)
    .End();
}

DAVA_TESTCLASS (TestReflectedCollections)
{
    template <typename T, typename TIter>
    void CollectionTestHelper(const DAVA::Ref::Field& collectionField, const TIter& startExpected, const TIter& endExpected)
    {
        DAVA::Ref::FieldsList fields = collectionField.valueRef.GetStructure()->GetFields(collectionField.valueRef.GetValueObject());
        DAVA::Vector<T> actualData;
        std::for_each(fields.begin(), fields.end(), [&actualData](const DAVA::Ref::Field& field) { actualData.push_back(field.valueRef.GetValue().Get<T>()); });

        TEST_VERIFY(actualData.size() == std::distance(startExpected, endExpected));
        TEST_VERIFY(std::equal(startExpected, endExpected, actualData.begin()));
    }

    template <typename K, typename T, typename TIter>
    void CollectionMapTestHelper(const DAVA::Ref::Field& collectionField, const TIter& startExpected, const TIter& endExpected)
    {
        DAVA::Ref::FieldsList fields = collectionField.valueRef.GetStructure()->GetFields(collectionField.valueRef.GetValueObject());
        DAVA::Map<K, T> actualData;
        std::for_each(fields.begin(), fields.end(), [&actualData](const DAVA::Ref::Field& field)
                      {
                          actualData.emplace(field.key.Cast<K>(), field.valueRef.GetValue().Get<T>());
                      });

        TEST_VERIFY(actualData.size() == std::distance(startExpected, endExpected));
        for (TIter i = startExpected; i != endExpected; ++i)
        {
            TEST_VERIFY(actualData.count(i->first) > 0);
            TEST_VERIFY(actualData[i->first] == i->second);
        }
    }

    template <typename T, typename TIter>
    void CollectionSetTestHelper(const DAVA::Ref::Field& collectionField, const TIter& startExpected, const TIter& endExpected)
    {
        DAVA::Ref::FieldsList fields = collectionField.valueRef.GetStructure()->GetFields(collectionField.valueRef.GetValueObject());
        DAVA::Set<T> actualData;
        std::for_each(fields.begin(), fields.end(), [&actualData](const DAVA::Ref::Field& field)
                      {
                          actualData.emplace(field.valueRef.GetValue().Get<T>());
                      });

        TEST_VERIFY(actualData.size() == std::distance(startExpected, endExpected));
        for (TIter i = startExpected; i != endExpected; ++i)
        {
            TEST_VERIFY(actualData.count(*i) > 0);
        }
    }

    template <typename TIter>
    void AddInsertRemoveTest(const DAVA::Ref::Field& field, const TIter& startExpected, const TIter& endExpected)
    {
        const DAVA::StructureWrapper* structure = field.valueRef.GetStructure();
        DAVA::ReflectedObject object = field.valueRef.GetValueObject();
        std::ptrdiff_t size = std::distance(startExpected, endExpected);
        TEST_VERIFY(size >= 0);
        for (size_t i = 0; i < static_cast<size_t>(size); ++i)
        {
            TEST_VERIFY(*(std::next(startExpected, i)) == structure->GetField(object, i).valueRef.GetValue().Cast<int>());
        }

        TEST_VERIFY(structure->AddField(object, DAVA::Any(), int(5)));
        TEST_VERIFY(structure->GetField(object, size_t(5)).valueRef.GetValue().Cast<int>() == 5);
        TEST_VERIFY(structure->AddField(object, DAVA::Any(), int(6)));
        TEST_VERIFY(structure->GetField(object, size_t(6)).valueRef.GetValue().Cast<int>() == 6);
        TEST_VERIFY(structure->InsertField(object, DAVA::Any(), size_t(6), int(7)));
        TEST_VERIFY(structure->GetField(object, size_t(5)).valueRef.GetValue().Cast<int>() == 5);
        TEST_VERIFY(structure->GetField(object, size_t(6)).valueRef.GetValue().Cast<int>() == 7);
        TEST_VERIFY(structure->GetField(object, size_t(7)).valueRef.GetValue().Cast<int>() == 6);

        TEST_VERIFY(structure->RemoveField(object, size_t(7)));
        TEST_VERIFY(structure->RemoveField(object, size_t(5)));
        TEST_VERIFY(structure->RemoveField(object, size_t(5)));
    }

    template <typename TIter>
    void AddInsertRemoveMapTest(const DAVA::Ref::Field& field, const TIter& startExpected, const TIter& endExpected)
    {
        const DAVA::StructureWrapper* structure = field.valueRef.GetStructure();
        DAVA::ReflectedObject object = field.valueRef.GetValueObject();
        for (TIter i = startExpected; i != endExpected; ++i)
        {
            TEST_VERIFY(i->first == structure->GetField(object, i->first).valueRef.GetValue().Cast<int>());
        }

        TEST_VERIFY(structure->AddField(object, int(5), int(5)));
        TEST_VERIFY(structure->GetField(object, int(5)).valueRef.GetValue().Cast<int>() == 5);
        TEST_VERIFY(structure->AddField(object, int(6), int(6)));
        TEST_VERIFY(structure->GetField(object, int(6)).valueRef.GetValue().Cast<int>() == 6);
        TEST_VERIFY(structure->InsertField(object, int(7), int(6), int(7)));
        TEST_VERIFY(structure->GetField(object, int(5)).valueRef.GetValue().Cast<int>() == 5);
        TEST_VERIFY(structure->GetField(object, int(6)).valueRef.GetValue().Cast<int>() == 6);
        TEST_VERIFY(structure->GetField(object, int(7)).valueRef.GetValue().Cast<int>() == 7);

        TEST_VERIFY(structure->RemoveField(object, int(7)));
        TEST_VERIFY(structure->RemoveField(object, int(5)));
        TEST_VERIFY(!structure->RemoveField(object, int(5)));
    }

    template <typename TIter>
    void AddInsertRemoveSetTest(const DAVA::Ref::Field& field, const TIter& startExpected, const TIter& endExpected)
    {
        const DAVA::StructureWrapper* structure = field.valueRef.GetStructure();
        DAVA::ReflectedObject object = field.valueRef.GetValueObject();
        for (TIter i = startExpected; i != endExpected; ++i)
        {
            TEST_VERIFY((*i) == structure->GetField(object, *i).valueRef.GetValue().Cast<int>());
        }

        TEST_VERIFY(structure->AddField(object, DAVA::Any(), int(5)));
        TEST_VERIFY(structure->GetField(object, int(5)).valueRef.GetValue().Cast<int>() == 5);
        TEST_VERIFY(structure->AddField(object, DAVA::Any(), int(6)));
        TEST_VERIFY(structure->GetField(object, int(6)).valueRef.GetValue().Cast<int>() == 6);
        TEST_VERIFY(structure->InsertField(object, DAVA::Any(), int(6), int(7)));
        TEST_VERIFY(structure->GetField(object, int(5)).valueRef.GetValue().Cast<int>() == 5);
        TEST_VERIFY(structure->GetField(object, int(6)).valueRef.GetValue().Cast<int>() == 6);
        TEST_VERIFY(structure->GetField(object, int(7)).valueRef.GetValue().Cast<int>() == 7);

        TEST_VERIFY(structure->RemoveField(object, int(7)));
        TEST_VERIFY(structure->RemoveField(object, int(5)));
        TEST_VERIFY(!structure->RemoveField(object, int(5)));

        Ref::Field f = structure->GetField(object, int(6));
        TEST_VERIFY(f.valueRef.IsReadonly());
        TEST_VERIFY(!f.valueRef.SetValue(int(10)));
        f.key.Set(int(10));
    }

    DAVA_TEST (VectorTest)
    {
        int testIntData[] = { 0, 1, 2, 3, 4 };
        DAVA::String testStringData[] = { "0", "0", "0", "0" };
        RelfCollectionsHolder holder;
        std::for_each(std::begin(testIntData), std::end(testIntData), [&holder](int v) { holder.intVector.push_back(v); });
        std::for_each(std::begin(testStringData), std::end(testStringData), [&holder](const DAVA::String& v) { holder.stringVector.push_back(v); });
        holder.intPtrVector = &holder.intVector;

        DAVA::Reflection r = DAVA::Reflection::Reflect(&holder);
        const DAVA::StructureWrapper* structure = r.GetStructure();
        CollectionTestHelper<int>(structure->GetField(r.GetValueObject(), DAVA::String("intPtrVector")), holder.intVector.begin(), holder.intVector.end());
        CollectionTestHelper<DAVA::String>(structure->GetField(r.GetValueObject(), DAVA::String("stringVector")), holder.stringVector.begin(), holder.stringVector.end());

        DAVA::Ref::Field vecField = structure->GetField(r.GetValueObject(), DAVA::String("intVector"));
        AddInsertRemoveTest(vecField, holder.intVector.begin(), holder.intVector.end());
        CollectionTestHelper<int>(vecField, holder.intVector.begin(), holder.intVector.end());
    }

    DAVA_TEST (ListTest)
    {
        int testIntData[] = { 0, 1, 2, 3, 4 };
        RelfCollectionsHolder holder;
        std::for_each(std::begin(testIntData), std::end(testIntData), [&holder](int v) { holder.intList.push_back(v); });

        DAVA::Reflection r = DAVA::Reflection::Reflect(&holder);
        const DAVA::StructureWrapper* structure = r.GetStructure();
        DAVA::Ref::Field listField = structure->GetField(r.GetValueObject(), DAVA::String("intList"));
        CollectionTestHelper<int>(listField, holder.intList.begin(), holder.intList.end());
        AddInsertRemoveTest(listField, holder.intList.begin(), holder.intList.end());
    }

    DAVA_TEST (MapTest)
    {
        int testIntData[] = { 0, 1, 2, 3, 4 };
        RelfCollectionsHolder holder;
        std::for_each(std::begin(testIntData), std::end(testIntData), [&holder](int v) { holder.mapColl.emplace(v, v); });

        DAVA::Reflection r = DAVA::Reflection::Reflect(&holder);
        const DAVA::StructureWrapper* structure = r.GetStructure();
        DAVA::Ref::Field mapField = structure->GetField(r.GetValueObject(), DAVA::String("mapColl"));
        CollectionMapTestHelper<int, int>(mapField, holder.mapColl.begin(), holder.mapColl.end());
        AddInsertRemoveMapTest(mapField, holder.mapColl.begin(), holder.mapColl.end());
        CollectionMapTestHelper<int, int>(mapField, holder.mapColl.begin(), holder.mapColl.end());
    }

    DAVA_TEST (UnorderedMapTest)
    {
        int testIntData[] = { 0, 1, 2, 3, 4 };
        RelfCollectionsHolder holder;
        std::for_each(std::begin(testIntData), std::end(testIntData), [&holder](int v) { holder.unorderMap.emplace(v, v); });

        DAVA::Reflection r = DAVA::Reflection::Reflect(&holder);
        const DAVA::StructureWrapper* structure = r.GetStructure();
        DAVA::Ref::Field mapField = structure->GetField(r.GetValueObject(), DAVA::String("unorderMap"));
        CollectionMapTestHelper<int, int>(mapField, holder.unorderMap.begin(), holder.unorderMap.end());
        AddInsertRemoveMapTest(mapField, holder.unorderMap.begin(), holder.unorderMap.end());
        CollectionMapTestHelper<int, int>(mapField, holder.unorderMap.begin(), holder.unorderMap.end());
    }

    DAVA_TEST (SetTest)
    {
        int testIntData[] = { 0, 1, 2, 3, 4 };
        RelfCollectionsHolder holder;
        std::for_each(std::begin(testIntData), std::end(testIntData), [&holder](int v) { holder.intSet.emplace(v); });

        DAVA::Reflection r = DAVA::Reflection::Reflect(&holder);
        const DAVA::StructureWrapper* structure = r.GetStructure();
        DAVA::Ref::Field setField = structure->GetField(r.GetValueObject(), DAVA::String("intSet"));
        CollectionSetTestHelper<int>(setField, holder.intSet.begin(), holder.intSet.end());
        AddInsertRemoveSetTest(setField, holder.intSet.begin(), holder.intSet.end());
        CollectionSetTestHelper<int>(setField, holder.intSet.begin(), holder.intSet.end());
    }

    DAVA_TEST (UnorderSetTest)
    {
        int testIntData[] = { 0, 1, 2, 3, 4 };
        RelfCollectionsHolder holder;
        std::for_each(std::begin(testIntData), std::end(testIntData), [&holder](int v) { holder.intUnorderSet.emplace(v); });

        DAVA::Reflection r = DAVA::Reflection::Reflect(&holder);
        const DAVA::StructureWrapper* structure = r.GetStructure();
        DAVA::Ref::Field setField = structure->GetField(r.GetValueObject(), DAVA::String("intUnorderSet"));
        CollectionSetTestHelper<int>(setField, holder.intUnorderSet.begin(), holder.intUnorderSet.end());
        AddInsertRemoveSetTest(setField, holder.intUnorderSet.begin(), holder.intUnorderSet.end());
        CollectionSetTestHelper<int>(setField, holder.intUnorderSet.begin(), holder.intUnorderSet.end());
    }
};

#endif