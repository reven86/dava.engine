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


//
//  DLCSystemTests.h
//  WoTSniperMacOS
//
//  Created by Andrey Panasyuk on 4/12/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef __DAVAENGINE_DLCSystemTests_h
#define __DAVAENGINE_DLCSystemTests_h

#include "DLC/DLCSystem.h"
#include "TestTemplate.h"

namespace DAVA
{

class DLCSystemTests: public DLCSystemDelegate
{
public:
	DLCSystemTests();

    void StartTests();
    
    void Test1();
    void Test2();
    void Test3();
    void Test4();
    void Test5();
    void Test6();
    void Test7();
    void Test8();

    void NextTest();
    
    // File with contents of all DLCs has getted 
    virtual void InitCompleted(DLCStatusCode withStatus);
    // Single DLC file has downloaded or end with error
    virtual void DLCCompleted(DLCStatusCode withStatus, uint16 index);
    // All DLC files has downloaded
    virtual void AllDLCCompleted();

	bool IsFinished() { return isFinished; };
	bool IsNeedNextTest() { return needNextTest; };
	uint32 GetCurTestNumber() { return state; };

public:
    enum States
    {
        TEST_1 = 0,
        TEST_2,
        TEST_3,
        TEST_4,
        TEST_5,
        TEST_6,
        TEST_7,
        TEST_8
    };
private:
    States state;
    bool isSucsess;
	bool isFinished;
	bool needNextTest;
};

}// End DAVA

class DLCTest: public TestTemplate<DLCTest>
{
protected:
    ~DLCTest(){}
public:
	DLCTest();

	virtual void LoadResources();
	virtual void UnloadResources();

	void DLCTestFunction(PerfFuncData * data);

private:
	DLCSystemTests* tests;

	bool isStarted;
};


#endif
