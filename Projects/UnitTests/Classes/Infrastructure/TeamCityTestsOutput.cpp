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



#include "TeamcityTestsOutput.h"

#include <sstream>

namespace DAVA
{

	namespace 
	{
		const String START_TEST = "start test ";
		const String FINISH_TEST = "finish test ";
		const String ERROR_TEST = "test error ";
		const String AT_FILE_TEST = " at file: ";
	}

void TeamcityTestOutput::Output(Logger::eLogLevel ll, const char8 *text) const
{
	std::stringstream ss(text);

	Vector<String> lines;
	String line;
	while (std::getline(ss, line, '\n')) {
		lines.push_back(line);
	}

	String output;


	if (START_TEST == lines[0])
	{
		String testName = lines.at(1);
		output = "##teamcity[testStarted name=\'" + testName + "\']\n";
	} else if (FINISH_TEST == lines[0])
	{
		String testName = lines.at(1);
		output = "##teamcity[testFinished name=\'" + testName + "\']\n";
	} else if (ERROR_TEST == lines[0])
	{
		String testName = lines.at(1);
		String condition = NormalizeString(lines.at(2).c_str());
		String errorFileLine = NormalizeString(lines.at(3).c_str());
		output = "##teamcity[testFailed name=\'" + testName 
			+ "\' message=\'" + condition 
			+ "\' details=\'" + errorFileLine + "\']\n";
	} else
	{
		TeamcityOutput::Output(ll, text);
		return;
	}

	TestOutput(output);
}

String TeamcityTestOutput::FormatTestStarted(const String& testName)
{
	return START_TEST + "\n" + testName;
}

String TeamcityTestOutput::FormatTestFinished(const String& testName)
{
	return FINISH_TEST + "\n" + testName;
}

String TeamcityTestOutput::FormatTestFailed(const String& testName, const String& condition, const String& errMsg)
{
	return ERROR_TEST + "\n" + testName + "\n" + condition + "\n" + errMsg;
}

void TeamcityTestOutput::connect(const String& host, unsigned int port)
{
	sf::Socket::Status status = socket.connect(host, port, sf::seconds(0.2f));
	if (status != sf::Socket::Done)
	{
		DAVA::Logger::Error("can't connect to server: %s:%hu", host.c_str(), port);
	} else
	{
		connected = true;
	}
}

void TeamcityTestOutput::sendTestResult(const String& testResult) const
{
	if (connected)
	{
		if (socket.send(testResult.c_str(), testResult.size()) != sf::Socket::Done)
		{
			connected = false; // prevent recursion
			DAVA::Logger::Error("can't send data to server\n");
			socket.disconnect();
		}
	}
}

void TeamcityTestOutput::disconnect()
{
	if (connected)
	{
		socket.disconnect();
		connected = false;
	}
}

void TeamcityTestOutput::TestOutput(const String& data) const
{
	sendTestResult(data);

	TeamcityOutput::PlatformOutput(data);
}

}; // end of namespace DAVA
