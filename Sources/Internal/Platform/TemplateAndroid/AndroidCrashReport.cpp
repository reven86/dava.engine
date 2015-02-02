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



#include "AndroidCrashReport.h"

#include "Platform/TemplateAndroid/ExternC/AndroidLayer.h"
#include "FileSystem/File.h"
#include <dlfcn.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "ExternC/AndroidLayer.h"

#include "BacktraceAndroid/BacktraceInterface.h"
#include "BacktraceAndroid/AndroidBacktraceChooser.h"
/* Maximum value of a caught signal. */
#define SIG_NUMBER_MAX 32

namespace DAVA
{


char * AndroidCrashReport::teamcityBuildName  = nullptr;
Vector<JniCrashReporter::CrashStep> AndroidCrashReport::crashSteps ;
const char * AndroidCrashReport::teamcityBuildNamePrototype = "CrashApp::CrashedSignal";
const char * AndroidCrashReport::teamcityBuildNamePrototypeEnd = "()";
const char * AndroidCrashReport::teamcityBuildNamePrototypePlaceHolder = "FFFFFFFFFF";
char AndroidCrashReport::functionString[AndroidCrashReport::maxStackSize][AndroidCrashReport::functionStringSize];

static int fatalSignals[] = {
	SIGABRT,
	SIGBUS,
	SIGFPE,
	SIGILL,
	SIGSEGV,
	SIGTRAP
};
//This function is needed to format string inside signal
// unfortunatly "man signal" does not list sprintf as signal safe 
//so it depends on particular implementation
char map[] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
template< typename T>
void ToHex(T value, char * str, int size,bool endline)
{
    if(size < 4) return;
    int strLen = static_cast<int>(sizeof(value)*2)+(endline?3:2);
    if(strLen > size)
        strLen = size;

	if(endline)
    	str[strLen-1] = '\0';
    str[0] = '0';
    str[1] = 'x';
    for(int i = strLen-2; i >= 2; i--)
    {
        T tmp =(0xF & value);
        str[i] = tmp;
        str[i] = map[str[i]];
        value = value >> 4;
    }
	if(endline)
    	str[size-1] = '\0';
}

static int fatalSignalsCount = (sizeof(fatalSignals) / sizeof(fatalSignals[0]));

stack_t AndroidCrashReport::s_sigstk;

JniCrashReporter::JniCrashReporter()
	: jniCrashReporter("com/dava/framework/JNICrashReporter")
    , jniString("java/lang/String")
{
    throwJavaExpetion = jniCrashReporter.GetStaticMethod<void, jstringArray, jstringArray, jintArray>("ThrowJavaExpetion");
}

void JniCrashReporter::ThrowJavaExpetion(const Vector<CrashStep>& chashSteps)
{
    JNIEnv *env = JNI::GetEnv();

    jobjectArray jModuleArray = env->NewObjectArray(chashSteps.size(), jniString, 0);
    jobjectArray jFunctionArray = env->NewObjectArray(chashSteps.size(), jniString, 0);
    jintArray jFileLineArray = env->NewIntArray(chashSteps.size());

    int* fileLines = new int[chashSteps.size()];
    for (uint i = 0; i < chashSteps.size(); ++i)
    {
        env->SetObjectArrayElement(jModuleArray, i, env->NewStringUTF(chashSteps[i].module));
        env->SetObjectArrayElement(jFunctionArray, i, env->NewStringUTF(chashSteps[i].function));
        fileLines[i] = chashSteps[i].fileLine;
    }
    env->SetIntArrayRegion(jFileLineArray, 0, chashSteps.size(), fileLines);

    throwJavaExpetion(jModuleArray, jFunctionArray, jFileLineArray);

    delete [] fileLines;
}



static struct sigaction *sa_old;
void AndroidCrashReport::Init()
{
#if defined(DCRASH_HANDLER_CUSTOMSIGNALS)
	//creating custom signal handler
	//this is legacy implementation and uses some asynch unsafe functions
	BacktraceInterface * backtraceProvider = AndroidBacktraceChooser::ChooseBacktraceAndroid();
	if(backtraceProvider != nullptr)
	{
		s_sigstk.ss_size = 64 * 1024;
		s_sigstk.ss_sp = malloc(s_sigstk.ss_size);
		s_sigstk.ss_flags = 0;

		if (sigaltstack(&s_sigstk, 0) < 0)
		{
			LOGE("CUSTOMSIGNALS Could not initialize alternative signal stack");
		}
		sa_old = (struct sigaction*)::malloc(sizeof(struct sigaction)*SIG_NUMBER_MAX);

		for (int i = 0; i < fatalSignalsCount; i++)
		{
			struct sigaction sa;
			memset(&sa, 0, sizeof(sa));
			sa.sa_flags = SA_SIGINFO|SA_ONSTACK;
			sigemptyset(&sa.sa_mask);
			sa.sa_sigaction = &SignalHandler;
		
			if (sigaction(fatalSignals[i], &sa, &sa_old[fatalSignals[i]]) != 0)
			{
				LOGE("CUSTOMSIGNALS Signal registration for failed:");
			}
		}
		// all important libs are likely to be loaded at this point
		backtraceProvider->BuildMemoryMap();
	}
	else
	{
		LOGE("CUSTOMSIGNALS This device is doesn't have a valid backtrace implementattion!");
	}
	crashSteps.reserve(maxStackSize);
	//we pre-format the first step here to avoid doing it in signal
	//crashSteps.push_back(FormatTeamcityIdStep(0,));

	size_t protoLen = strlen(teamcityBuildNamePrototype) 
		+strlen(teamcityBuildNamePrototypeEnd)+  strlen(teamcityBuildNamePrototypePlaceHolder) ;
	
    teamcityBuildName = new char[protoLen];
	strcpy(teamcityBuildName,teamcityBuildNamePrototype);
	
	size_t offset = strlen(teamcityBuildNamePrototype);
	strcpy(teamcityBuildName+ offset
		,teamcityBuildNamePrototypePlaceHolder);
	
	offset += strlen(teamcityBuildNamePrototypePlaceHolder);
	strcpy(teamcityBuildName+offset,teamcityBuildNamePrototypeAddr);
	
	
	offset += strlen(teamcityBuildNamePrototypePlaceHolder);
	strcpy(teamcityBuildName+offset,teamcityBuildNamePrototypeEnd);
	
	
#endif

	
}
JniCrashReporter::CrashStep AndroidCrashReport::FormatTeamcityIdStep(int32 addr)
{
    JniCrashReporter::CrashStep buildId;
#ifdef TEAMCITY_BUILD_TYPE_ID
    buildId.module = TEAMCITY_BUILD_TYPE_ID;
#endif
    
	
	ToHex(addr,teamcityBuildName+strlen(teamcityBuildNamePrototype),strlen(teamcityBuildNamePrototypePlaceHolder),false);
	buildId.function = teamcityBuildNamePrototype;
    buildId.fileLine = (addr);
    return buildId;
}
void AndroidCrashReport::OnStackFrame(pointer_size addr)
{
	if(crashSteps.size() >= maxStackSize) return;
#ifdef TEAMCITY_BUILD_TYPE_ID	
	//no sence in adding crash step without crash id
	if(crashSteps.size() == 0)
	{
		crashSteps.push_back(FormatTeamcityIdStep(addr));
	}
#endif
	const char * libName = NULL;
	pointer_size relAddres = 0;
	BacktraceInterface * backtraceProvider = AndroidBacktraceChooser::ChooseBacktraceAndroid();
	backtraceProvider->GetMemoryMap()->Resolve(addr,&libName,&relAddres);
	ToHex(relAddres,functionString[crashSteps.size()],functionStringSize,true);
	JniCrashReporter::CrashStep step;
	step.module = libName;
	step.function = functionString[crashSteps.size()];
	step.fileLine = relAddres;
	//LOGE("FRAME_STACK frame stack 0x%lx %s %s",step.fileLine,step.function, step.module);
	crashSteps.push_back(step);

}
void AndroidCrashReport::SignalHandler(int signal, struct siginfo *siginfo, void *sigcontext)
{
	if(signal<SIG_NUMBER_MAX)
	{
		sa_old[signal].sa_sigaction(signal, siginfo, sigcontext);
	}

	alarm(30);
	//kill the app if it freezes
	
	BacktraceInterface * backtraceProvider = AndroidBacktraceChooser::ChooseBacktraceAndroid();
	if(backtraceProvider != nullptr)
	{
		//LOGE("FRAME_STACK backtracing %d %d",sigcontext,siginfo);
		//backtraceProvider->Backtrace(&AndroidCrashReport::onStackFrame,sigcontext,siginfo);
		backtraceProvider->Backtrace(AndroidCrashReport::OnStackFrame
				,sigcontext,siginfo);
		return;
	}
	else
	{
		JniCrashReporter::CrashStep step;
		step.module = "There is no cpp stack";
		crashSteps.push_back(step);
	}
    
	JniCrashReporter crashReport;
	crashReport.ThrowJavaExpetion(crashSteps);
}

void AndroidCrashReport::ThrowExeption(const String& message)
{
	Vector<JniCrashReporter::CrashStep> crashSteps;

	JniCrashReporter::CrashStep step;
	step.module = message.c_str();
	crashSteps.push_back(step);

	JniCrashReporter crashReport;
	crashReport.ThrowJavaExpetion(crashSteps);
}


} // namespace DAVA
