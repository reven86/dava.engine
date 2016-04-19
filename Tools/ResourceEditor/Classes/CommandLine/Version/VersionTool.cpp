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


#include "CommandLine/Version/VersionTool.h"
#include "Logger/Logger.h"

#include "DAVAVersion.h"
#include "Version.h"


#include <QtGlobal>
#include <QString>

using namespace DAVA;

VersionTool::VersionTool()
    : CommandLineTool("-version")
{
}

void VersionTool::ConvertOptionsToParamsInternal()
{
}

bool VersionTool::InitializeInternal()
{
    return true;
}

void VersionTool::ProcessInternal()
{
    auto logLevel = DAVA::Logger::Instance()->GetLogLevel();
    DAVA::Logger::Instance()->SetLogLevel(DAVA::Logger::LEVEL_INFO);

    DAVA::Logger::Info("========================================");
    DAVA::Logger::Info("Qt: %s", QT_VERSION_STR);
    DAVA::Logger::Info("Engine: %s", DAVAENGINE_VERSION);
    DAVA::Logger::Info("Appication: %s", APPLICATION_BUILD_VERSION);
    DAVA::Logger::Info("%u bit", static_cast<DAVA::uint32>(sizeof(DAVA::pointer_size) * 8));
    DAVA::Logger::Info("========================================");

    DAVA::Logger::Instance()->SetLogLevel(logLevel);
}
