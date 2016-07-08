#ifndef __DAVAENGINE_ANDROID_CRASH_REPORT_H__
#define __DAVAENGINE_ANDROID_CRASH_REPORT_H__

#include "Base/BaseTypes.h"
#if defined(__DAVAENGINE_ANDROID__)

#include "Platform/TemplateAndroid/JniHelpers.h"
#include <signal.h>

namespace DAVA
{
class File;
class JniCrashReporter
{
public:
    struct CrashStep
    {
        const char* module;
        const char* function;
        int32 fileLine;
    };
    JniCrashReporter(JNIEnv* env = nullptr);
    void ThrowJavaExpetion(const Vector<CrashStep>& chashSteps);

private:
    static jclass classID;
    static jclass stringID;
    static jmethodID mid;
};

class AndroidCrashReport
{
public:
    static void Init(JNIEnv* env);
    static void ThrowExeption(const String& message);
    static void Unload();

private:
    static void SignalHandler(int signal, siginfo_t* info, void* uapVoid);
    static void OnStackFrame(pointer_size addr);
    static JniCrashReporter::CrashStep FormatTeamcityIdStep(int32 addr);

private:
    static stack_t s_sigstk;

    //pre allocated here to be used inside signal handler
    static Vector<JniCrashReporter::CrashStep> crashSteps;
    static const size_t functionStringSize = 30;
    static const size_t maxStackSize = 256;

    static const char* teamcityBuildNamePrototype;
    static const char* teamcityBuildNamePrototypeEnd;
    static const char* teamcityBuildNamePrototypePlaceHolder;

    static char* teamcityBuildName;
    static char functionString[maxStackSize][functionStringSize];
    static JniCrashReporter* crashReporter;
};
}

#endif //#if defined(__DAVAENGINE_ANDROID__)

#endif /* #ifndef __DAVAENGINE_ANDROID_CRASH_HANDLER_H__ */