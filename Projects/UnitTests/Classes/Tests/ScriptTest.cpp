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

#include "UnitTests/UnitTests.h"
#include "Base/BaseTypes.h"
#include "Math/Color.h"
#include "Math/MathDefines.h"
#include "Reflection/Registrator.h"
#include "Scripting/Script.h"

struct ReflClass : public DAVA::ReflectedBase
{
    DAVA_VIRTUAL_REFLECTION(ReflClass)
    {
        DAVA::ReflectionRegistrator<ReflClass>::Begin()
        .Field("intVal", &ReflClass::intVal)
        .Field("floatVal", &ReflClass::floatVal)
        .Field("stringVal", &ReflClass::stringVal)
        .Field("boolVal", &ReflClass::boolVal)
        .Field("colorVal", &ReflClass::colorVal)
        .Field("subClass", &ReflClass::subClass)
        .End();
    }

public:
    DAVA::int32 intVal = 0;
    DAVA::float32 floatVal = 0.0f;
    bool boolVal = false;
    DAVA::String stringVal;
    DAVA::Color colorVal = DAVA::Color::White;
    ReflClass* subClass = nullptr;
};

DAVA_TESTCLASS (ScriptTest)
{
    DAVA_TEST (ReflectionTest)
    {
        ReflClass subcl;
        subcl.intVal = 10;

        ReflClass cl;
        cl.intVal = 5;
        cl.floatVal = 1.4f;
        cl.boolVal = true;
        cl.stringVal = "Demo string";
        cl.colorVal = DAVA::Color::Black;
        cl.subClass = &subcl;

        DAVA::Reflection clRef = DAVA::Reflection::Create(&cl).ref;

        const DAVA::String script = R"script(
function main(context)
    -- Get value tests
    local intVal = context.intVal
    assert(intVal == 5, "Test fail! context.intVal " .. intVal .. " != 5")
    local floatVal = context.floatVal
    assert(math.abs(floatVal - 1.4) <= 0.00001, "Test fail! context.floatVal " .. floatVal .. " != 1.4")
    local boolVal = context.boolVal
    assert(boolVal == true, "Test fail! context.boolVal " .. tostring(boolVal) .. " != true")
    local stringVal = context.stringVal
    assert(stringVal == "Demo string", "Test fail! context.stringVal '" .. stringVal .. "' != 'Demo string'")

    local subClass = context.subClass
    intVal = subClass.intVal
    assert(intVal == 10, "Test fail! subClass.intVal " .. intVal .. " != 10")

    -- Set value tests
    context.intVal = 42
    assert(context.intVal == 42, "Test fail! context.intVal " .. context.intVal .. " != 42")

    context.floatVal = 3.14
    assert(math.abs(context.floatVal - 3.14) <= 0.00001, "Test fail! context.floatVal " .. context.floatVal .. " != 3.14")

    context.boolVal = false
    assert(context.boolVal == false, "Test fail! context.boolVal " .. tostring(context.boolVal) .. " != false")

    context.stringVal = "New demo string"
    assert(context.stringVal == "New demo string", "Test fail! context.stringVal '" .. context.stringVal .. "' != 'New demo string'")
    
    -- Test complex type DAVA::Color as userdata
    subClass.colorVal = context.colorVal
    --assert(subClass.colorVal == context.colorVal, "Test fail!  subClass.colorVal (" ..  tostring(subClass.colorVal) .. ") != context.colorVal (" .. tostring(context.colorVal) .. ")")
end
)script";

        DAVA::Script s;
        TEST_VERIFY(s.LoadString(script));
        TEST_VERIFY(s.Run(clRef));
        TEST_VERIFY(cl.intVal == 42);
        TEST_VERIFY(FLOAT_EQUAL(cl.floatVal, 3.14f));
        TEST_VERIFY(cl.boolVal == false);
        TEST_VERIFY(cl.stringVal == "New demo string");
        TEST_VERIFY(cl.colorVal == subcl.colorVal);
    }
};
