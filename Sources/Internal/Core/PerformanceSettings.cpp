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


#include "Core/PerformanceSettings.h"
namespace DAVA 
{

PerformanceSettings::PerformanceSettings()
{
	//init with some default values
	psPerformanceMinFPS = 35;
	psPerformanceMaxFPS = 50;
	psPerformanceLodOffset = 11.7f;
	psPerformanceLodMult = 3.9f;
	psPerformanceSpeedMult = 2;
}
float32 PerformanceSettings::GetPsPerformanceMinFPS()
{
	return psPerformanceMinFPS;
}
float32 PerformanceSettings::GetPsPerformanceMaxFPS()
{
	return psPerformanceMaxFPS;
}
float32 PerformanceSettings::GetPsPerformanceSpeedMult()
{
	return psPerformanceSpeedMult;
}
float32 PerformanceSettings::GetPsPerformanceLodOffset()
{
	return psPerformanceLodOffset;
}
float32 PerformanceSettings::GetPsPerformanceLodMult()
{
	return psPerformanceLodMult;
}

void PerformanceSettings::SetPsPerformanceMinFPS(float32 minFPS)
{
	psPerformanceMinFPS = minFPS;
}
void PerformanceSettings::SetPsPerformanceMaxFPS(float32 maxFPS)
{
	psPerformanceMaxFPS = maxFPS;
}
void PerformanceSettings::SetPsPerformanceSpeedMult(float32 speedMult)
{
	psPerformanceSpeedMult = speedMult;
}
void PerformanceSettings::SetPsPerformanceLodOffset(float32 lodOffset)
{
	psPerformanceLodOffset = lodOffset;
}
void PerformanceSettings::SetPsPerformanceLodMult(float32 lodMult)
{
	psPerformanceLodMult = lodMult;
}

}