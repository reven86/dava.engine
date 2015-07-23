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

#include "DAVAEngine.h"
#include "UnitTests/UnitTests.h"

#include "DataStorage/DataStorage.h"

using namespace DAVA;

DAVA_TESTCLASS(DataVaultTest)
{
    DAVA_TEST(TestFunction)
    {
        IDataStorage *storage = DataStorage::Create();

        storage->Clear();
        storage->Push();
        storage->SetStringValue("TestStringKey", "Test");
        storage->Push();
        String ret = storage->GetStringValue("TestStringKey");
#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__)
        TEST_VERIFY("" == ret);
#else
        TEST_VERIFY("Test" == ret);
        storage->RemoveEntry("TestStringKey");
        storage->Push();
        ret = storage->GetStringValue("TestStringKey");
        TEST_VERIFY("Test" != ret);

        int64 iret = storage->GetLongValue("TestIntKey");
        TEST_VERIFY(0 == iret);

        storage->SetLongValue("TestIntKey", 1);
        storage->Push();
        iret = storage->GetLongValue("TestIntKey");
        TEST_VERIFY(1 == iret);

        storage->Clear();
        iret = storage->GetLongValue("TestIntKey");
        TEST_VERIFY(0 == iret);

#endif
        SafeRelease(storage);
    }
};
