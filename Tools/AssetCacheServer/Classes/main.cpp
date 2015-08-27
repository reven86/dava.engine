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

#include "Core/Core.h"
#include "FileSystem/Logger.h"
#include "QtTools/RunGuard/RunGuard.h"

#include "ServerCore.h"
#include "AssetCacheServerWindow.h"


#include <QApplication>
#include <QCryptographicHash>

void FrameworkWillTerminate()
{
}

void FrameworkDidLaunched()
{
}

int main(int argc, char *argv[])
{
    DAVA::Core::Run(argc, argv);
	QApplication a(argc, argv);

	const QString appUid = "{AACCAACC-6CE2-459A-B26F-79AAF05E0C6B}";
	const QString appUidPath = QCryptographicHash::hash((appUid + QApplication::applicationDirPath()).toUtf8(), QCryptographicHash::Sha1).toHex();
	RunGuard runGuard(appUidPath);
	if (runGuard.tryToRun())
	{
		DAVA::Logger::Instance()->SetLogFilename("AssetCacheServer.txt");
		DAVA::Logger::Instance()->SetLogLevel(DAVA::Logger::LEVEL_FRAMEWORK);

		ServerCore server;
		AssetCacheServerWindow mainWindow(server);

		if (server.Settings().IsFirstLaunch())
		{
			mainWindow.show();
		}

		if (server.Settings().IsAutoStart())
		{
			server.Start();
		}

		return a.exec();
	}

	return -1;
}
