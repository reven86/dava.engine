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
#include "Utils/Utils.h"

namespace DAVA
{

static const String START_TEST = "start test ";
static const String FINISH_TEST = "finish test ";
static const String ERROR_TEST = "test error ";
static const String AT_FILE_TEST = " at file: ";

void TeamcityTestsOutput::Output(Logger::eLogLevel ll, const char8 *text)
{
    String textStr = text;
    Vector<String> lines;
    Split(textStr, "\n", lines);

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

String TeamcityTestsOutput::FormatTestStarted(const String& testName)
{
    return START_TEST + "\n" + testName;
}

String TeamcityTestsOutput::FormatTestFinished(const String& testName)
{
    return FINISH_TEST + "\n" + testName;
}

String TeamcityTestsOutput::FormatTestFailed(const String& testName, const String& condition, const String& errMsg)
{
    return ERROR_TEST + "\n" + testName + "\n" + condition + "\n" + errMsg;
}

void TeamcityTestsOutput::TestOutput(const String& data)
{
    TeamcityOutput::PlatformOutput(data);
}

}; // end of namespace DAVA
