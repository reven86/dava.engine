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

#include "Base/Result.h"
#include "UnitTests/UnitTests.h"

using namespace DAVA;

DAVA_TESTCLASS(ResultTest)
{
    DAVA_TEST(GetResultFunction)
    {
        TEST_VERIFY(GetResultFunction(Result::RESULT_SUCCESS));
        TEST_VERIFY(!GetResultFunction(Result::RESULT_FAILURE));
        TEST_VERIFY(!GetResultFunction(Result::RESULT_ERROR));

        TEST_VERIFY(GetResultFunction(Result::RESULT_SUCCESS).IsSuccess());
        TEST_VERIFY(!GetResultFunction(Result::RESULT_FAILURE).IsSuccess());
        TEST_VERIFY(!GetResultFunction(Result::RESULT_ERROR).IsSuccess());

        Deque<Result> results;
        results.emplace_back(Result::RESULT_SUCCESS, "this is ");
        results.emplace_back(Result::RESULT_FAILURE, "result ");
        results.emplace_back(Result::RESULT_ERROR, "test.");
        ResultList resultList;
        for (const auto &result : results)
        {
            resultList.AddResultList(GetResultFunction(result));
        }
        TEST_VERIFY(resultList.GetResults().size() == results.size());
        auto resultIt = resultList.GetResults().begin();
        for (const auto &result : results)
        {
            TEST_VERIFY(result == *resultIt++);
        }
    }
        
    ResultList GetResultFunction(const Result &result)
    {
        return ResultList(result);
    }

    ResultList GetResultFunction(const Result &&result)
    {
        return ResultList(result);
    }
};
