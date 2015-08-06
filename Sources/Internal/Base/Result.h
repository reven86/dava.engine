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


#ifndef __DAVAENGINE_RESULT_H_
#define __DAVAENGINE_RESULT_H_

#include "Base/BaseTypes.h"
#include "FileSystem/VariantType.h"

namespace DAVA
{

struct Result
{
    enum ResultType 
    {
        RESULT_SUCCESS,
        RESULT_FAILURE,
        RESULT_ERROR
    };
    Result(const ResultType type = RESULT_SUCCESS, const String &message = String());
    Result(const Result &result) = default;
    Result(Result &&result);
    Result& operator = (const Result &result) = default;
    Result& operator = (Result &&result);
    operator bool() const;
    ResultType type = RESULT_SUCCESS;
    String message;
};

inline Result::operator bool() const
{
    return type == RESULT_SUCCESS;
}

class ResultList
{
public:
    explicit ResultList();
    explicit ResultList(const Result &result);
    ResultList(Result &&result);
    ResultList(const ResultList &resultList) = default;
    ResultList(ResultList &&resultList);
    ~ResultList() = default;
    operator bool() const;
    bool IsSuccess() const;
    ResultList& operator = (const ResultList &resultList) = default;
    ResultList& operator = (ResultList&& resultList);
    ResultList& operator << (const Result &result);
    ResultList& operator << (Result &&result);
    ResultList &AddResult(const Result &result);
    ResultList &AddResult(Result &&result);
    ResultList &AddResult(const Result::ResultType type = Result::RESULT_SUCCESS, const String &message = String());
    ResultList &AddResultList(const ResultList &resultList);
    ResultList &AddResultList(ResultList &&resultList);
    
    const Deque<Result> &GetResults() const;
private:
    bool allOk;
    Deque < Result > results;
};

inline ResultList::operator bool() const
{
    return allOk;
}

inline bool ResultList::IsSuccess() const
{
    return allOk;
}

    
inline const Deque<Result> &ResultList::GetResults() const
{
    return results;
}

}
#endif // __DAVAENGINE_RESULT_H_
