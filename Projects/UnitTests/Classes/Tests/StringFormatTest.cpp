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


#include "StringFormatTest.h"

using namespace DAVA;

StringFormatTest::~StringFormatTest()
{

}

StringFormatTest::StringFormatTest()
    : TestTemplate<StringFormatTest>("StringFormatTest")
{
    RegisterFunction(this, &StringFormatTest::StringTestFunction, "StringTestFunction", NULL);
    RegisterFunction(this, &StringFormatTest::IntegerTestFunction, "IntegerTestFunction", NULL);
    RegisterFunction(this, &StringFormatTest::FloatTestFunction, "FloatTestFunction", NULL);
}

void StringFormatTest::LoadResources()
{

}

void StringFormatTest::UnloadResources()
{

}

void StringFormatTest::StringTestFunction( PerfFuncData * data )
{
//     WideString formatStr1 = L"%ls %ls";
//     WideString value1 = L"test string";
//     WideString value2 = L"second";
//     TEST_VERIFY( Format( formatStr1.c_str(), value1.c_str(), value2.c_str()) == StringToWString( Format( WStringToString(formatStr1).c_str(), WStringToString(value1).c_str(), WStringToString(value2).c_str() ) ) );
}

void StringFormatTest::IntegerTestFunction( PerfFuncData * data )
{
    WideString formatStr1 = L"%i%%"  ;
    WideString formatStr2 = L"%d%%"  ;
    WideString formatStr3 = L"%lld%%";

    int32 value = 1234567890;
    int64 value64 = 1234567890123456789;
    
    TEST_VERIFY( Format(formatStr1.c_str(), value) == StringToWString( Format( WStringToString(formatStr1).c_str(), value ) ) );
    TEST_VERIFY( Format(formatStr2.c_str(), value) == StringToWString( Format( WStringToString(formatStr2).c_str(), value ) ) );
    TEST_VERIFY( Format(formatStr3.c_str(), value64) == StringToWString( Format( WStringToString(formatStr3).c_str(), value64 ) ) );
    
    value *= -1;
    value64 *= -1;

    TEST_VERIFY( Format(formatStr1.c_str(), value) == StringToWString( Format( WStringToString(formatStr1).c_str(), value ) ) );
    TEST_VERIFY( Format(formatStr2.c_str(), value) == StringToWString( Format( WStringToString(formatStr2).c_str(), value ) ) );
    TEST_VERIFY( Format(formatStr3.c_str(), value64) == StringToWString( Format( WStringToString(formatStr3).c_str(), value64 ) ) );
}

inline void checkFloatFormat( const WideString &formatStr, float32 value, TestTemplate<StringFormatTest>::PerfFuncData * data )
{
    WideString testStr = Format(formatStr.c_str() , value);
	String tmpStr = WStringToString(formatStr);
	String tmpFormated = Format(tmpStr.c_str(), value);
    WideString checkStr = StringToWString(tmpFormated);
    if( data )
        data->testData.message = "\"" + WStringToString(testStr) + "\" == \"" + WStringToString(checkStr) + "\"";
    TEST_VERIFY( testStr == checkStr );
    if( data )
        data->testData.message.clear();
}

void StringFormatTest::FloatTestFunction( PerfFuncData * data )
{
    WideString formatStr1 = L"%f"   ;
    WideString formatStr2 = L"%6.f";
    WideString formatStr3 = L"%5.f";
    WideString formatStr4 = L"%4.f";
    WideString formatStr5 = L"%3.f";
    WideString formatStr6 = L"%2.f";
    WideString formatStr7 = L"%1.f";
    WideString formatStr8 = L"%0.f";
    WideString formatStr9 = L"%.6f";
    WideString formatStr10= L"%.5f";
    WideString formatStr11= L"%.4f";
    WideString formatStr12= L"%.3f";
    WideString formatStr13= L"%.2f";
    WideString formatStr14= L"%.1f";
    WideString formatStr15= L"%.0f";
    WideString formatStr16= L"%6.6f";
    WideString formatStr17= L"%5.5f";
    WideString formatStr18= L"%4.4f";
    WideString formatStr19= L"%3.3f";
    WideString formatStr20= L"%2.2f";
    WideString formatStr21= L"%1.1f";
    WideString formatStr22= L"%0.0f";

    {
        float32 value1 = 1234.1234f;
        checkFloatFormat( formatStr1 , value1, data );
        checkFloatFormat( formatStr2 , value1, data );
        checkFloatFormat( formatStr3 , value1, data );
        checkFloatFormat( formatStr4 , value1, data );
        checkFloatFormat( formatStr5 , value1, data );
        checkFloatFormat( formatStr6 , value1, data );
        checkFloatFormat( formatStr7 , value1, data );
        checkFloatFormat( formatStr8 , value1, data );
        checkFloatFormat( formatStr9 , value1, data );
        checkFloatFormat( formatStr10, value1, data );
        checkFloatFormat( formatStr11, value1, data );
        checkFloatFormat( formatStr12, value1, data );
        checkFloatFormat( formatStr13, value1, data );
        checkFloatFormat( formatStr14, value1, data );
        checkFloatFormat( formatStr15, value1, data );
        checkFloatFormat( formatStr16, value1, data );
        checkFloatFormat( formatStr17, value1, data );
        checkFloatFormat( formatStr18, value1, data );
        checkFloatFormat( formatStr19, value1, data );
        checkFloatFormat( formatStr20, value1, data );
        checkFloatFormat( formatStr21, value1, data );
        checkFloatFormat( formatStr22, value1, data );

        float32 value2 = 876.876f;
        checkFloatFormat( formatStr1 , value2, data );
        checkFloatFormat( formatStr2 , value2, data );
        checkFloatFormat( formatStr3 , value2, data );
        checkFloatFormat( formatStr4 , value2, data );
        checkFloatFormat( formatStr5 , value2, data );
        checkFloatFormat( formatStr6 , value2, data );
        checkFloatFormat( formatStr7 , value2, data );
        checkFloatFormat( formatStr8 , value2, data );
        checkFloatFormat( formatStr9 , value2, data );
        checkFloatFormat( formatStr10, value2, data );
        checkFloatFormat( formatStr11, value2, data );
        checkFloatFormat( formatStr12, value2, data );
        checkFloatFormat( formatStr13, value2, data );
        checkFloatFormat( formatStr14, value2, data );
        checkFloatFormat( formatStr15, value2, data );
        checkFloatFormat( formatStr16, value2, data );
        checkFloatFormat( formatStr17, value2, data );
        checkFloatFormat( formatStr18, value2, data );
        checkFloatFormat( formatStr19, value2, data );
        checkFloatFormat( formatStr20, value2, data );
        checkFloatFormat( formatStr21, value2, data );
        checkFloatFormat( formatStr22, value2, data );

        float32 value3 = 0.1234f;
        float32 value4 = 0.2567f;
        float32 value5 = 0.5f;
        float32 value6 = 0.7543f;

        checkFloatFormat( formatStr15, value3, data );
        checkFloatFormat( formatStr15, value4, data );
        checkFloatFormat( formatStr15, value5, data );
        checkFloatFormat( formatStr15, value6, data );
    }
    {
        float32 value1 = -1234.1234f;
        checkFloatFormat( formatStr1 , value1, data );
        checkFloatFormat( formatStr2 , value1, data );
        checkFloatFormat( formatStr3 , value1, data );
        checkFloatFormat( formatStr4 , value1, data );
        checkFloatFormat( formatStr5 , value1, data );
        checkFloatFormat( formatStr6 , value1, data );
        checkFloatFormat( formatStr7 , value1, data );
        checkFloatFormat( formatStr8 , value1, data );
        checkFloatFormat( formatStr9 , value1, data );
        checkFloatFormat( formatStr10, value1, data );
        checkFloatFormat( formatStr11, value1, data );
        checkFloatFormat( formatStr12, value1, data );
        checkFloatFormat( formatStr13, value1, data );
        checkFloatFormat( formatStr14, value1, data );
        checkFloatFormat( formatStr15, value1, data );
        checkFloatFormat( formatStr16, value1, data );
        checkFloatFormat( formatStr17, value1, data );
        checkFloatFormat( formatStr18, value1, data );
        checkFloatFormat( formatStr19, value1, data );
        checkFloatFormat( formatStr20, value1, data );
        checkFloatFormat( formatStr21, value1, data );
        checkFloatFormat( formatStr22, value1, data );
        
        float32 value2 = -876.876f;
        checkFloatFormat( formatStr1 , value2, data );
        checkFloatFormat( formatStr2 , value2, data );
        checkFloatFormat( formatStr3 , value2, data );
        checkFloatFormat( formatStr4 , value2, data );
        checkFloatFormat( formatStr5 , value2, data );
        checkFloatFormat( formatStr6 , value2, data );
        checkFloatFormat( formatStr7 , value2, data );
        checkFloatFormat( formatStr8 , value2, data );
        checkFloatFormat( formatStr9 , value2, data );
        checkFloatFormat( formatStr10, value2, data );
        checkFloatFormat( formatStr11, value2, data );
        checkFloatFormat( formatStr12, value2, data );
        checkFloatFormat( formatStr13, value2, data );
        checkFloatFormat( formatStr14, value2, data );
        checkFloatFormat( formatStr15, value2, data );
        checkFloatFormat( formatStr16, value2, data );
        checkFloatFormat( formatStr17, value2, data );
        checkFloatFormat( formatStr18, value2, data );
        checkFloatFormat( formatStr19, value2, data );
        checkFloatFormat( formatStr20, value2, data );
        checkFloatFormat( formatStr21, value2, data );
        checkFloatFormat( formatStr22, value2, data );
        
        float32 value3 = -0.1234f;
        float32 value4 = -0.2567f;
        float32 value5 = -0.5f;
        float32 value6 = -0.7543f;
        
        checkFloatFormat( formatStr15, value3, data );
        checkFloatFormat( formatStr15, value4, data );
        checkFloatFormat( formatStr15, value5, data );
        checkFloatFormat( formatStr15, value6, data );
    }

	checkFloatFormat(L"%.3f", 10, data);
	checkFloatFormat(L"%.0f", (float32)12980 / 1000.0f, data);
	checkFloatFormat(L"%.3f", 2.00671148f, data);
}
