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

#ifndef __TEST_SCREEN_H__
#define __TEST_SCREEN_H__

#include "DAVAEngine.h"
#include "DLC/DLC.h"
#include "Infrastructure/BaseScreen.h"

using namespace DAVA;

struct DLCCrashTest
{
    uint64 cancelTimeout;
    uint64 exitTimeout;
    uint32 retryCount;

    DAVA::FilePath testingFileFlag;
    DAVA::String dbObjectId;

    bool forceExit;
    bool inExitMode;

    Thread* exitThread;

    void Init(const DAVA::FilePath& workingDir, const DAVA::FilePath& destinationDir);
    void Update(float32 timeElapsed, DLC* dlc);

    void ExitThread(BaseObject* caller, void* callerData, void* userData);
};

class DlcTest : public BaseScreen
{
public:
    DlcTest();

protected:
    ~DlcTest()
    {
    }

public:
    void LoadResources() override;
    void UnloadResources() override;
    void OnActive() override;

    void Update(float32 timeElapsed) override;
    void Draw(const UIGeometricData& geometricData) override;

private:
    void UpdateInfoStr();
    void SetInternalDlServer(BaseObject* obj, void* data, void* callerData);
    void SetExternalDlServer(BaseObject* obj, void* data, void* callerData);
    void IncDlThreads(BaseObject* obj, void* data, void* callerData);
    void DecDlThreads(BaseObject* obj, void* data, void* callerData);
    void Start(BaseObject* obj, void* data, void* callerData);
    void Cancel(BaseObject* obj, void* data, void* callerData);
    void Restart(BaseObject* obj, void* data, void* callerData);

protected:
    String gameVersion = "dlcdevtest";
    String currentDownloadUrl;

    DAVA::FilePath workingDir;
    DAVA::FilePath sourceDir;
    DAVA::FilePath destinationDir;

    UITextField* gameVersionIn = nullptr;
    UIStaticText* infoText = nullptr;
    WideString infoStr;

    uint32 downloadTreadsCount = 4;

    UIStaticText* staticText = nullptr;
    UIControl* animControl = nullptr;
    UIControl* progressControl = nullptr;

    float32 angle = 0;
    float32 lastUpdateTime = 0.f;
    uint32 lastDLCState = 0;

    DLC* dlc = nullptr;
    DLCCrashTest crashTest;
};

#endif
