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

#include "Infrastructure/Utils/ControlHelpers.h"

using namespace DAVA;

FilePath ControlHelpers::GetPathToUIYaml(const String &yamlFileName)
{
    const FilePath path = Format("~res:/UI/win/%s", yamlFileName.c_str());
    return path;
}

const String ControlHelpers::ReportItem::TEST_NAME_PATH = "TestName";
const String ControlHelpers::ReportItem::MIN_DELTA_PATH = "MinDelta/MinDeltaValue";
const String ControlHelpers::ReportItem::MAX_DELTA_PATH = "MaxDelta/MaxDeltaValue";
const String ControlHelpers::ReportItem::AVERAGE_DELTA_PATH = "AverageDelta/AverageDeltaValue";
const String ControlHelpers::ReportItem::TEST_TIME_PATH = "TestTime/TestTimeValue";
const String ControlHelpers::ReportItem::ELAPSED_TIME_PATH = "ElapsedTime/ElapsedTimeValue";
const String ControlHelpers::ReportItem::FRAMES_RENDERED_PATH = "FramesRendered/FramesRenderedValue";
