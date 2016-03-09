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

#include "Infrastructure/TextureUtils.h"

#include "Platform/DeviceInfo.h"

using namespace DAVA;

DAVA_TESTCLASS (DeviceInfoTest)
{
    DAVA_TEST (TestFunction)
    {
        String osVersion = DeviceInfo::GetVersion();
        TEST_VERIFY("" != osVersion && "Not yet implemented" != osVersion);

        Logger::Debug(osVersion.c_str());

        String model = DeviceInfo::GetModel();
        Logger::Debug(model.c_str());

        eGPUFamily gpuModel = DeviceInfo::GetGPUFamily();
        DeviceInfo::NetworkInfo ninfo = DeviceInfo::GetNetworkInfo();

        String locale = DeviceInfo::GetLocale();
        TEST_VERIFY("" != locale && "Not yet implemented" != locale);

        String region = DeviceInfo::GetRegion();
        TEST_VERIFY("" != region && "Not yet implemented" != region);

        String timeZone = DeviceInfo::GetTimeZone();
        TEST_VERIFY("" != timeZone && "Not yet implemented" != timeZone);
    }
}
;
