#ifndef __SCRIPT_LOAD_HELPER_H__
#define __SCRIPT_LOAD_HELPER_H__

#if defined(__DAVAENGINE_HTML5__)

#include "Base/BaseTypes.h"
#include "Base/Singleton.h"

using namespace DAVA;

struct ScriptLoadingInfo
{
	String scriptPath;
	DAVA::List<void (*)()> loadListeners;
	DAVA::List<void (*)()> errorListeners;
	
	bool operator==(String rhs) { return scriptPath == rhs; }
};

class ScriptLoadHelper : public DAVA::Singleton<ScriptLoadHelper>
{
public:
	void EnqueScript(const String& script, void (*onload)(), void (*onerror)());
	
private:
	static void OnScriptLoaded();
	static void OnErrorLoadingScript();

private:
	DAVA::List<ScriptLoadingInfo> resourceQueue;
};

#endif

#endif //__SCRIPT_LOAD_HELPER_H__