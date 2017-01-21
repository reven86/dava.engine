#include "UnitTests/UnitTests.h"

#include <Reflection/Reflection.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/MetaObjects.h>

#include <Base/Any.h>
#include <Base/GlobalEnum.h>

namespace ReflectedMetaDetails
{
DAVA::M::ValidatorResult ValidateY(const DAVA::Any& value, const DAVA::Any& prevValue)
{
    using namespace DAVA::M;

    const double& v = value.Get<double>();
    ValidatorResult result;
    result.state = (v < -10.0 || v > 20.0) ? ValidatorResult::eState::Valid : ValidatorResult::eState::Invalid;

    return result;
}

enum TestEnum
{
    Value1,
    Value2
};

class TestClass
{
public:
    int x;
    double y;
    TestEnum e;

    DAVA_REFLECTION(TestClass)
    {
        DAVA::ReflectionRegistrator<TestClass>::Begin()[DAVA::M::File(false)]
        .Field("x", &TestClass::x)[DAVA::M::ReadOnly(), DAVA::M::Range(10, 20)]
        .Field("y", &TestClass::y)[DAVA::M::Validator(&ValidateY), DAVA::M::Group("geometry")]
        .Field("enum", &TestClass::e)[DAVA::M::EnumT<TestEnum>()]
        .End();
    }
};
}

ENUM_DECLARE(ReflectedMetaDetails::TestEnum)
{
    ENUM_ADD_DESCR(ReflectedMetaDetails::Value1, "Value1");
    ENUM_ADD_DESCR(ReflectedMetaDetails::Value2, "Value2");
}

DAVA_TESTCLASS (ReflectedMetaTest)
{
    DAVA_TEST (MetaTest)
    {
        ReflectedMetaDetails::TestClass cls;
        DAVA::Reflection r = DAVA::Reflection::Create(&cls);

        const DAVA::M::File* f = r.GetMeta<DAVA::M::File>();
        TEST_VERIFY(f != nullptr);
        TEST_VERIFY(f->shouldExists == false);

        DAVA::Reflection fieldX = r.GetField("x");
        TEST_VERIFY(fieldX.IsValid());
        TEST_VERIFY(fieldX.GetMeta<DAVA::M::ReadOnly>() != nullptr);
        const DAVA::M::Range* range = fieldX.GetMeta<DAVA::M::Range>();
        TEST_VERIFY(range != nullptr);
        TEST_VERIFY(range->minValue.Cast<DAVA::int32>() == 10);
        TEST_VERIFY(range->maxValue.Cast<DAVA::int32>() == 20);

        DAVA::Reflection fieldY = r.GetField("y");
        TEST_VERIFY(fieldY.IsValid());
        TEST_VERIFY(fieldY.GetMeta<DAVA::M::ReadOnly>() == nullptr);
        const DAVA::M::Group* group = fieldY.GetMeta<DAVA::M::Group>();
        TEST_VERIFY(group != nullptr);
        TEST_VERIFY(std::strcmp(group->groupName, "geometry") == 0);

        const DAVA::M::Validator* validator = fieldY.GetMeta<DAVA::M::Validator>();
        TEST_VERIFY(validator != nullptr);
        DAVA::M::ValidatorResult result = validator->Validate(double(30.0), 0);
        TEST_VERIFY(result.state == DAVA::M::ValidatorResult::eState::Valid);

        result = validator->Validate(double(0.0), 0);
        TEST_VERIFY(result.state == DAVA::M::ValidatorResult::eState::Invalid);

        DAVA::Reflection fieldE = r.GetField("enum");
        TEST_VERIFY(fieldE.IsValid());
        TEST_VERIFY(fieldE.GetMeta<DAVA::M::ReadOnly>() == nullptr);

        const DAVA::M::Enum* enumMeta = fieldE.GetMeta<DAVA::M::Enum>();
        TEST_VERIFY(enumMeta != nullptr);
        TEST_VERIFY(enumMeta->GetEnumMap() == GlobalEnumMap<ReflectedMetaDetails::TestEnum>::Instance());
    }
};