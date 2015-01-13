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


#ifndef __TESTTEMPLATE_H__
#define __TESTTEMPLATE_H__

#include "DAVAEngine.h"
using namespace DAVA;
#include "GameCore.h"
#include "BaseScreen.h"

template <class T>
class TestTemplate : public BaseScreen
{
protected:
	~TestTemplate(){}
public:

	struct PerfFuncData
	{
		void (T::*func)(PerfFuncData * data);
		T				* screen;

        TestData testData;
	};
    

	TestTemplate(const String & screenName);

	virtual void Draw(const UIGeometricData &geometricData);
	virtual void DidAppear();

	void RegisterFunction(T * screen, void (T::*func)(PerfFuncData * data), const String & name, int32 runCount, void * userData);

	void SubmitTime(PerfFuncData * data, uint64 time);
	void LogError(PerfFuncData * data, const String &errorMessage);
    
    int32 GetScreenId();
    
    virtual int32 GetTestCount();
    virtual TestData * GetTestData(int32 iTest);

    virtual bool RunTest(int32 testNum);
    
protected:
    
	Vector<PerfFuncData> perfFuncs;
	int32 runIndex;

private:
    
    static int32 globalScreenId; // 1, on create of screen increment  
    int32 currentScreenId;
    
	TestTemplate();
};

template <class T>
int32 TestTemplate<T>::globalScreenId = 1;

template <class T>
void TestTemplate<T>::RegisterFunction(T * screen, void (T::*func)(PerfFuncData * data), const String & name, int32 runCount, void * userData)
{
	PerfFuncData data;
	data.testData.name = name;
	data.func = func;
	data.screen = screen;
	data.testData.runCount = runCount;
	data.testData.userData = userData;

	perfFuncs.push_back(data);
}

template <class T>
void TestTemplate<T>::LogError(PerfFuncData * data, const String &errorMessage)
{
	GameCore::Instance()->LogMessage(Format("Error %s at test: %s", errorMessage.c_str(), data->testData.name.c_str()));
}

template <class T>
void TestTemplate<T>::SubmitTime(PerfFuncData * data, uint64 time)
{
    data->testData.eachRunTime.push_back(time);
	data->testData.totalTime += time;
	if (runIndex == 0)
	{
		data->testData.minTime = time;
		
        data->testData.maxTime = time;
        data->testData.maxTimeIndex = 0;
	}
	if(time < data->testData.minTime)
	{
		data->testData.minTime = time;
	}
	if(time > data->testData.maxTime)
	{
		data->testData.maxTime = time;
        data->testData.maxTimeIndex = runIndex;
	}
}

template <class T>
TestTemplate<T>::TestTemplate(const String & _screenName)
{
    SetName(_screenName);
	runIndex = -1;
    
    currentScreenId = globalScreenId++;
}

template <class T>
void TestTemplate<T>::DidAppear()
{
	runIndex = 0;
}


template <class T>
void TestTemplate<T>::Draw(const UIGeometricData &geometricData)
{
    UIScreen::Draw(geometricData);
}


template <class T>
bool TestTemplate<T>::RunTest(int32 testNum)
{
	int32 testCount = perfFuncs.size();
    
    if(0 <= testNum && testNum < testCount)
    {
		PerfFuncData * data = &(perfFuncs[testNum]);
		if(runIndex < data->testData.runCount)
		{
			uint64 startTime = SystemTimer::Instance()->GetAbsoluteNano();
			if(0 == runIndex)
			{
				data->testData.startTime = startTime;
			}
			
            (data->screen->*data->func)(data);
            
			uint64 endTime = SystemTimer::Instance()->GetAbsoluteNano();
			SubmitTime(data, endTime-startTime);
            
			++runIndex;
			if(data->testData.runCount == runIndex)
			{
				data->testData.endTime = endTime;
                runIndex = 0;
                return true;
			}
		}
    }
    
    return false;
}


template <class T>
int32 TestTemplate<T>::GetTestCount()
{
    return perfFuncs.size();
}

template <class T>
TestData * TestTemplate<T>::GetTestData(int32 iTest)
{
    DVASSERT((0 <= iTest) && (iTest < GetTestCount()));
    
    return &perfFuncs[iTest].testData;
}



#endif // __TESTTEMPLATE_H__
