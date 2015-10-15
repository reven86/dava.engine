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


#ifndef __DAVAENGINE_AUTOTESTING_DB_H__
#define __DAVAENGINE_AUTOTESTING_DB_H__

#include "DAVAConfig.h"

#ifdef __DAVAENGINE_AUTOTESTING__


#include "Database/MongodbClient.h"

#include "Autotesting/MongodbUpdateObject.h"
#include "Autotesting/AutotestingSystem.h"

namespace DAVA
{
	class Image;

	class AutotestingDB : public Singleton < AutotestingDB >
	{
	public:
		AutotestingDB();
		~AutotestingDB();

		static const String DB_ERROR_STR_VALUE;
		static const int32 DB_ERROR_INT_VALUE = -9999;

		bool ConnectToDB(const String &collection, const String &dbName, const String &dbHost, const int32 dbPort);
		void CloseConnection();
		void FailOnLocalBuild();

		// Work with log object in DB
		KeyedArchive *FindBuildArchive(MongodbUpdateObject *dbUpdateObject, const String &auxArg);
		KeyedArchive *FindOrInsertBuildArchive(MongodbUpdateObject *dbUpdateObject, const String &auxArg);

		KeyedArchive *FindOrInsertGroupArchive(KeyedArchive *buildArchive, const String &groupId);
		KeyedArchive *InsertTestArchive(KeyedArchive *currentGroupArchive, const String &testId);
		KeyedArchive *InsertStepArchive(KeyedArchive *testArchive, const String &stepId, const String &description);

		KeyedArchive *FindOrInsertTestArchive(MongodbUpdateObject *dbUpdateObject, const String &testId);
		KeyedArchive *FindOrInsertStepArchive(KeyedArchive *testArchive, const String &stepId);
		KeyedArchive *FindOrInsertTestStepLogEntryArchive(KeyedArchive *testStepArchive, const String &logId);

		// Getting and Setting data from/in DB
		bool SaveToDB(MongodbUpdateObject *dbUpdateObject);

		void WriteLogHeader();
		void WriteLog(const char8 *text, ...);
		void Log(const String &level, const String &message);

		String GetStringTestParameter(const String &deviceName, const String &parameter);
		int32 GetIntTestParameter(const String &deviceName, const String &parameter);

		String ReadString(const String &name);
		void WriteString(const String &name, const String &text);

		bool SaveKeyedArchiveToDevice(const String &archiveName, KeyedArchive *archive);

		void UploadScreenshot(const String &name, Image *image);

		// multiplayer api
		String ReadState(const String &device, const String &param);
		void WriteState(const String &device, const String &param, const String &state);

		void SetTestStarted();

		FilePath logsFolder;

	protected:
		MongodbClient *dbClient;
		FilePath logFilePath;
		AutotestingSystem *autoSys;

	};


}

#endif //__DAVAENGINE_AUTOTESTING__

#endif //__DAVAENGINE_AUTOTESTING_DB_H__