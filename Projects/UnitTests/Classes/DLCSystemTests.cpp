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
//  DLCSystemTests.cpp
//  WoTSniperMacOS
//
//  Created by Andrey Panasyuk on 4/12/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "DLCSystemTests.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/File.h"

#ifdef __DAVAENGINE_WIN32__
#include "Windows.h"
#define sleep(x) Sleep((x) * 1000)
#endif

namespace DAVA
{

DLCSystemTests::DLCSystemTests()
:	isFinished(false),
	needNextTest(false)
{
}

void DLCSystemTests::StartTests()
{
    Test1();
}
    
//--------------------
// Prepare tests
//--------------------
void DLCSystemTests::Test1()
{
    // Clear before tests
    FileSystem::Instance()->DeleteDirectory( "~doc:/downloads/" );

    state = TEST_1;
    isSucsess = true;

	Logger::Debug(" TEST %d Started", state + 1);

    new DLCSystem();
    
    DLCSystem::Instance()->AddDelegate(this);
    DLCSystem::Instance()->InitSystem("http://transportcontrol.davamobile.com/temp/test_1/", "~doc:/downloads/res/");

    sleep(7);
    DLCSystem::Instance()->Pause();
    sleep(3);
    DLCSystem::Instance()->Resume();
}

void DLCSystemTests::Test2()
{
    state = TEST_2;
    isSucsess = true;

	Logger::Debug(" TEST %d Started", state + 1);

    new DLCSystem();
    
    // Delete one file
    FilePath fullPath( "~doc:/downloads/res/Maps/tree_level1/bush01.DAE" );
    FileSystem::Instance()->DeleteFile(fullPath);
    
    DLCSystem::Instance()->AddDelegate(this);
    DLCSystem::Instance()->InitSystem("http://transportcontrol.davamobile.com/temp/test_1/", "~doc:/downloads/res/");
}

void DLCSystemTests::Test3()
{
    state = TEST_3;
    isSucsess = true;

	Logger::Debug(" TEST %d Started", state + 1);

    new DLCSystem();
    
    // Delete one file
    FilePath fullPath( "~doc:/downloads/dlc_2.ka");
    FileSystem::Instance()->DeleteFile(fullPath);
    
    //
    DLCSystem::Instance()->AddDelegate(this);
    DLCSystem::Instance()->InitSystem("http://transportcontrol.davamobile.com/temp/test_1/", "~doc:/downloads/res/");

    sleep(2);
    DLCSystem::Instance()->Stop();
}

void DLCSystemTests::Test4()
{
    state = TEST_4;
    isSucsess = true;

	Logger::Debug(" TEST %d Started", state + 1);

    new DLCSystem();
    
    //
    DLCSystem::Instance()->AddDelegate(this);
    DLCSystem::Instance()->InitSystem("http://transportcontrol.davamobile.com/temp/test_1/", "~doc:/downloads/res/");
}

void DLCSystemTests::Test5()
{
    state = TEST_5;
    isSucsess = true;

	Logger::Debug(" TEST %d Started", state + 1);

    new DLCSystem();
    
    //
    DLCSystem::Instance()->AddDelegate(this);
    DLCSystem::Instance()->InitSystem("http://transportcontrol.davamobile.com/temp/test_5/", "~doc:/downloads/res/");
}

void DLCSystemTests::Test6()
{
    state = TEST_6;
    isSucsess = true;

	Logger::Debug(" TEST %d Started", state + 1);

    new DLCSystem();
    
    // Delete one file
    FilePath fullPath( "~doc:/downloads/dlc_2.ka" );
    FileSystem::Instance()->DeleteFile(fullPath);
    
    //
    DLCSystem::Instance()->AddDelegate(this);
    DLCSystem::Instance()->InitSystem("http://transportcontrol.davamobile.com/temp/test_5/", "~doc:/downloads/res/");
    
    sleep(2);
    DLCSystem::Instance()->Stop();
}

void DLCSystemTests::Test7()
{
    state = TEST_7;
    isSucsess = true;

	Logger::Debug(" TEST %d Started", state + 1);

    new DLCSystem();
    
    //
    DLCSystem::Instance()->AddDelegate(this);
    DLCSystem::Instance()->InitSystem("http://transportcontrol.davamobile.com/temp/test_7/", "~doc:/downloads/res/");
}

void DLCSystemTests::Test8()
{
    state = TEST_8;
    isSucsess = true;

	Logger::Debug(" TEST %d Started", state + 1);

    new DLCSystem();
    
    //
    DLCSystem::Instance()->AddDelegate(this);
    DLCSystem::Instance()->InitSystem("http://transportcontrol.davamobile.com/temp/test_8/", "~doc:/downloads/res/");
}

//--------------------
// Check States
//--------------------
void DLCSystemTests::InitCompleted(DLCStatusCode withStatus)
{
    switch ( state )
    {
        case TEST_1:
            if ( withStatus == DOWNLOAD_SUCCESS )
            {
                if ( DLCSystem::Instance()->GetDLCSource(0)->state != DLCSystem::DLC_NOT_DOWNLOADED )
                {
                    isSucsess = false;
                }
                if ( DLCSystem::Instance()->GetDLCSource(1)->state != DLCSystem::DLC_NOT_DOWNLOADED )
                {
                    isSucsess = false;
                }
                DLCSystem::Instance()->DownloadAllDLC();
            }
            else
            {
                isSucsess = false;
				needNextTest = true;
            }
            break;
        case TEST_2:
            if ( withStatus == DOWNLOAD_SUCCESS )
            {
                if ( DLCSystem::Instance()->GetDLCSource(0)->state != DLCSystem::DLC_FILES_TREE_CORRUPT )
                {
                    isSucsess = false;
                }
                if ( DLCSystem::Instance()->GetDLCSource(1)->state != DLCSystem::DLC_DOWNLOADED )
                {
                    isSucsess = false;
                }
                DLCSystem::Instance()->DownloadAllDLC();
            }
            else
            {
                isSucsess = false;
				needNextTest = true;
            }
            break;
        case TEST_3:
        case TEST_6:
            if ( withStatus == DOWNLOAD_SUCCESS )
            {
                if ( DLCSystem::Instance()->GetDLCSource(0)->state != DLCSystem::DLC_DOWNLOADED )
                {
                    isSucsess = false;
                }
                if ( DLCSystem::Instance()->GetDLCSource(1)->state != DLCSystem::DLC_NOT_DOWNLOADED )
                {
                    isSucsess = false;
                }
                DLCSystem::Instance()->DownloadAllDLC();
            }
            else
            {
                isSucsess = false;
				needNextTest = true;
            }
            break;
        case TEST_4:
            if ( withStatus == DOWNLOAD_SUCCESS )
            {
                if ( DLCSystem::Instance()->GetDLCSource(0)->state != DLCSystem::DLC_DOWNLOADED )
                {
                    isSucsess = false;
                }
                if ( DLCSystem::Instance()->GetDLCSource(1)->state != DLCSystem::DLC_DOWNLOAD_NOT_FINISHED )
                {
                    isSucsess = false;
                }
                DLCSystem::Instance()->DownloadAllDLC();
            }
            else
            {
                isSucsess = false;
				needNextTest = true;
            }
            break;
        case TEST_5:
            if ( withStatus == DOWNLOAD_SUCCESS )
            {
                DLCSystem::Instance()->DownloadAllDLC();
                if ( DLCSystem::Instance()->GetDLCSource(0)->state != DLCSystem::DLC_OLD_VERSION )
                {
                    isSucsess = false;
                    Logger::Debug("  INIT FALSE 1 state = %d", DLCSystem::Instance()->GetDLCSource(0)->state);
                }
                if ( DLCSystem::Instance()->GetDLCSource(1)->state != DLCSystem::DLC_DOWNLOADED )
                {
                    isSucsess = false;
                    Logger::Debug("  INIT FALSE 2 state = %d", DLCSystem::Instance()->GetDLCSource(1)->state);
                }
            }
            else
            {
                Logger::Debug("  INIT FALSE 3");
                isSucsess = false;
				needNextTest = true;
            }
            break;
        case TEST_7:
            if ( withStatus == DOWNLOAD_SUCCESS )
            {
                DLCSystem::Instance()->DownloadAllDLC();
                if ( DLCSystem::Instance()->GetDLCSource(0)->state != DLCSystem::DLC_DOWNLOADED )
                {
                    isSucsess = false;
                }
                if ( DLCSystem::Instance()->GetDLCSource(1)->state != DLCSystem::DLC_OLD_VERSION )
                {
                    isSucsess = false;
                }
            }
            else
            {
                isSucsess = false;
				needNextTest = true;
//                NextTest();
            }
            break;
        case TEST_8:
            if ( withStatus == DOWNLOAD_SUCCESS )
            {
                DLCSystem::Instance()->DownloadAllDLC();
                if ( DLCSystem::Instance()->GetDLCSource(0)->state != DLCSystem::DLC_DOWNLOADED )
                {
                    isSucsess = false;
                }
                if ( DLCSystem::Instance()->GetDLCSource(1)->state != DLCSystem::DLC_NOT_DOWNLOADED )
                {
                    isSucsess = false;
                }
                if ( DLCSystem::Instance()->GetDLCSource(2)->state != DLCSystem::DLC_DOWNLOADED )
                {
                    isSucsess = false;
                }
            }
            else
            {
                isSucsess = false;
				needNextTest = true;
//                NextTest();
            }
            break;
        default:
            break;
    }
}
    
void DLCSystemTests::DLCCompleted(DLCStatusCode withStatus, uint16 index)
{
    switch ( state )
    {
        case TEST_1:
        case TEST_2:
        case TEST_4:
        case TEST_8:
            if ( withStatus != DOWNLOAD_SUCCESS )
            {
                Logger::Debug("  COMPLETE FALSE 1");
                isSucsess = false;
            }
            break;
        case TEST_3:
        case TEST_6:
        {
            if ( index == 0 && withStatus != DOWNLOAD_SUCCESS )
            {
                Logger::Debug("  COMPLETE FALSE 2");
                isSucsess = false;
            }
            if ( index == 1 && withStatus != DOWNLOAD_ERROR )
            {
                Logger::Debug("  COMPLETE FALSE 3");
                isSucsess = false;
            }
            break;
        }
        case TEST_5:
        {
            const DLCSource * dlc = DLCSystem::Instance()->GetDLCSource(index);
            if ( index == 0 && ( withStatus != DOWNLOAD_SUCCESS || dlc->curVersion <= 1))
            {
                isSucsess = false;
                Logger::Debug("  COMPLETE FALSE 4 status = %d", withStatus);
            }
            if ( index == 1 && ( withStatus != DOWNLOAD_SUCCESS || dlc->curVersion != 1))
            {
                isSucsess = false;
                Logger::Debug("  COMPLETE FALSE 5");
            }
            break;
        }
        case TEST_7:
        {
            const DLCSource * dlc = DLCSystem::Instance()->GetDLCSource(index);
            if ( index == 0 && ( withStatus != DOWNLOAD_SUCCESS || dlc->curVersion <= 1))
            {
                isSucsess = false;
            }
            if ( index == 1 && ( withStatus != DOWNLOAD_SUCCESS || dlc->curVersion <= 1))
            {
                isSucsess = false;
            }
            break;
        }
        default:
            break;
    }
}

//--------------------
// Result & clean
//--------------------
void DLCSystemTests::AllDLCCompleted()
{
//    NextTest();
	needNextTest = true;
}

void DLCSystemTests::NextTest()
{
	needNextTest = false;

    delete DLCSystem::Instance();

    switch ( state )
    {
        case TEST_1:
            Logger::Debug( " TEST 1 = %s", (isSucsess) ? "Success" : "Fail" );
            Test2();
            break;
        case TEST_2:
            Logger::Debug( " TEST 2 = %s", (isSucsess) ? "Success" : "Fail" );
            Test3();
            break;
        case TEST_3:
            Logger::Debug( " TEST 3 = %s", (isSucsess) ? "Success" : "Fail" );
            Test4();
            break;
        case TEST_4:
            Logger::Debug( " TEST 4 = %s", (isSucsess) ? "Success" : "Fail" );
            Test5();
            break;
        case TEST_5:
            Logger::Debug( " TEST 5 = %s", (isSucsess) ? "Success" : "Fail" );
            Test6();
            break;
        case TEST_6:
            Logger::Debug( " TEST 6 = %s", (isSucsess) ? "Success" : "Fail" );
            Test7();
            break;
        case TEST_7:
            Logger::Debug( " TEST 7 = %s", (isSucsess) ? "Success" : "Fail" );
            Test8();
            break;
        case TEST_8:
            Logger::Debug( " TEST 8 = %s", (isSucsess) ? "Success" : "Fail" );
			isFinished = true;

            break;
        default:
            break;
    }
}


    
    
    
};// End DAVA


DLCTest::DLCTest()
:	TestTemplate<DLCTest>("DLCTest"),
	isStarted(false)
{
	RegisterFunction(this, &DLCTest::DLCTestFunction, "DLCTestFunction", NULL);
}

void DLCTest::LoadResources()
{
	tests = new DLCSystemTests();
}

void DLCTest::UnloadResources()
{
	delete tests;
}

void DLCTest::DLCTestFunction(TestTemplate<DLCTest>::PerfFuncData *data)
{
	if (!isStarted)
	{
		isStarted = true;
		tests->StartTests();
	}

	if (!tests->IsFinished())
	{
		if (tests->IsNeedNextTest())
		{
			tests->NextTest();
		}

		RegisterFunction(this, &DLCTest::DLCTestFunction, "DLCTestFunction", NULL);
	}
}
