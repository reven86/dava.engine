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

#include "ExternC/AndroidLayer.h"

namespace DAVA
{

typedef uint32_t  kernel_sigmask_t[2];
typedef struct ucontext {
	uint32_t uc_flags;
	struct ucontext* uc_link;
	stack_t uc_stack;
	sigcontext uc_mcontext;
	kernel_sigmask_t uc_sigmask;
} ucontext_t;

static int fatalSignals[] = {
	SIGABRT,
	SIGBUS,
	SIGFPE,
	SIGILL,
	SIGSEGV,
	SIGTRAP
};

static int fatalSignalsCount = (sizeof(fatalSignals) / sizeof(fatalSignals[0]));

stack_t AndroidCrashReport::s_sigstk;

jclass JniCrashReporter::gJavaClass = NULL;
jclass JniCrashReporter::gStringClass = NULL;
const char* JniCrashReporter::gJavaClassName = NULL;

jclass JniCrashReporter::GetJavaClass() const
{
	return gJavaClass;
}

const char* JniCrashReporter::GetJavaClassName() const
{
	return gJavaClassName;
}

void JniCrashReporter::ThrowJavaExpetion(const Vector<CrashStep>& chashSteps)
{
	jmethodID mid = GetMethodID("ThrowJavaExpetion", "([Ljava/lang/String;[Ljava/lang/String;[I)V");
	if (mid)
	{
		jobjectArray jModuleArray = GetEnvironment()->NewObjectArray(chashSteps.size(), gStringClass, 0);
		jobjectArray jFunctionArray = GetEnvironment()->NewObjectArray(chashSteps.size(), gStringClass, 0);
		jintArray jFileLineArray = GetEnvironment()->NewIntArray(chashSteps.size());

		int* fileLines = new int[chashSteps.size()];
		for (uint i = 0; i < chashSteps.size(); ++i)
		{
			GetEnvironment()->SetObjectArrayElement(jModuleArray, i, GetEnvironment()->NewStringUTF(chashSteps[i].module.c_str()));
			GetEnvironment()->SetObjectArrayElement(jFunctionArray, i, GetEnvironment()->NewStringUTF(chashSteps[i].function.c_str()));
			fileLines[i] = chashSteps[i].fileLine;
		}
		GetEnvironment()->SetIntArrayRegion(jFileLineArray, 0, chashSteps.size(), fileLines);

		env->CallStaticVoidMethod(GetJavaClass(), mid, jModuleArray, jFunctionArray, jFileLineArray);

		delete [] fileLines;
	}
}

//libcorkscrew definition
typedef struct map_info_t map_info_t;

typedef struct {
    uintptr_t absolute_pc;
    uintptr_t stack_top;
    size_t stack_size;
} backtrace_frame_t;

typedef struct {
    uintptr_t relative_pc;
    uintptr_t relative_symbol_addr;
    char* map_name;
    char* symbol_name;
    char* demangled_name;
} backtrace_symbol_t;

typedef ssize_t (*t_unwind_backtrace_signal_arch)(siginfo_t* si, void* sc, const map_info_t* lst, backtrace_frame_t* bt, size_t ignore_depth, size_t max_depth);
static t_unwind_backtrace_signal_arch unwind_backtrace_signal_arch;

typedef map_info_t* (*t_acquire_my_map_info_list)();
static t_acquire_my_map_info_list acquire_my_map_info_list;

typedef void (*t_release_my_map_info_list)(map_info_t* milist);
static t_release_my_map_info_list release_my_map_info_list;

typedef void (*t_get_backtrace_symbols)(const backtrace_frame_t* backtrace, size_t frames, backtrace_symbol_t* symbols);
static t_get_backtrace_symbols get_backtrace_symbols;

typedef void (*t_free_backtrace_symbols)(backtrace_symbol_t* symbols, size_t frames);
static t_free_backtrace_symbols free_backtrace_symbols;
//libcorkscrew definition

void AndroidCrashReport::Init()
{
	void* libcorkscrew = dlopen("/system/lib/libcorkscrew.so", RTLD_NOW);
	if (libcorkscrew)
	{
		unwind_backtrace_signal_arch = (t_unwind_backtrace_signal_arch) dlsym(libcorkscrew, "unwind_backtrace_signal_arch");
		if (!unwind_backtrace_signal_arch) LOGE("unwind_backtrace_signal_arch not found");

		acquire_my_map_info_list = (t_acquire_my_map_info_list) dlsym(libcorkscrew, "acquire_my_map_info_list");
		if (!acquire_my_map_info_list) LOGE("acquire_my_map_info_list not found");

		get_backtrace_symbols = (t_get_backtrace_symbols) dlsym(libcorkscrew, "get_backtrace_symbols");
		if (!get_backtrace_symbols) LOGE("get_backtrace_symbols not found");

		free_backtrace_symbols = (t_free_backtrace_symbols) dlsym(libcorkscrew, "free_backtrace_symbols");
		if (!free_backtrace_symbols) LOGE("free_backtrace_symbols not found");

		release_my_map_info_list = (t_release_my_map_info_list) dlsym(libcorkscrew, "release_my_map_info_list");
		if (!release_my_map_info_list) LOGE("release_my_map_info_list not found");
	}
	else
	{
		LOGE("libcorkscrew not found");
	}

	s_sigstk.ss_size = 64 * 1024;
	s_sigstk.ss_sp = malloc(s_sigstk.ss_size);
	s_sigstk.ss_flags = 0;
	
	if (sigaltstack(&s_sigstk, 0) < 0)
	{
		Logger::Error("Could not initialize alternative signal stack");
	}
	
	for (int i = 0; i < fatalSignalsCount; i++)
	{
		struct sigaction sa;
		memset(&sa, 0, sizeof(sa));
		sa.sa_flags = SA_SIGINFO|SA_ONSTACK;
		sigemptyset(&sa.sa_mask);
		sa.sa_sigaction = &SignalHandler;
		
		if (sigaction(fatalSignals[i], &sa, NULL) != 0)
		{
			Logger::Error("Signal registration for failed:");
		}
	}
}

void AndroidCrashReport::SignalHandler(int signal, struct siginfo *siginfo, void *sigcontext)
{
	Vector<JniCrashReporter::CrashStep> crashSteps;
	if (unwind_backtrace_signal_arch != NULL)
	{
		map_info_t *map_info = acquire_my_map_info_list();
		backtrace_frame_t frames[256] = {0};
		backtrace_symbol_t symbols[256] = {0};

		const ssize_t size = unwind_backtrace_signal_arch(siginfo, sigcontext, map_info, frames, 0, 255);
		get_backtrace_symbols(frames, size, symbols);
		for (int i = 0; i < size; ++i)
		{
			JniCrashReporter::CrashStep step;
			step.module = symbols[i].map_name;
			if (symbols[i].demangled_name)
				step.function = symbols[i].demangled_name;
			else if (symbols[i].symbol_name)
				step.function = symbols[i].symbol_name;

			step.fileLine = symbols[i].relative_pc;

			crashSteps.push_back(step);
		}
		//free_backtrace_symbols(symbols, size);
		//release_my_map_info_list(map_info);
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
	step.module = message;
	crashSteps.push_back(step);

	JniCrashReporter crashReport;
	crashReport.ThrowJavaExpetion(crashSteps);
}


} // namespace DAVA
