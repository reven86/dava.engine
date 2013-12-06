#if defined(__DAVAENGINE_HTML5__)
#include <emscripten/emscripten.h>
#include <ScriptLoadHelper.h>
#include <Logger.h>


void ScriptLoadHelper::EnqueScript(const String& script, void (*onload)(), void (*onerror)())
{
	DAVA::List<ScriptLoadingInfo>::iterator it = std::find(resourceQueue.begin(), resourceQueue.end(), script);
	if(it != resourceQueue.end())
	{
		// Already in list, just add listeners
		(*it).loadListeners.push_back(onload);
		(*it).errorListeners.push_back(onerror);
	}
	else
	{
		ScriptLoadingInfo newItem;
		newItem.scriptPath = script;
		newItem.loadListeners.push_back(onload);
		newItem.errorListeners.push_back(onerror);
		resourceQueue.push_back(newItem);
		
		if(resourceQueue.size() == 1)
		{
			emscripten_async_load_script(script.c_str(), ScriptLoadHelper::OnScriptLoaded, ScriptLoadHelper::OnErrorLoadingScript);
		}
	}
}

void ScriptLoadHelper::OnScriptLoaded()
{
	ScriptLoadingInfo loadedItem = ScriptLoadHelper::Instance()->resourceQueue.front();
	ScriptLoadHelper::Instance()->resourceQueue.pop_front();
	Logger::Debug("Succefull loading script %s", loadedItem.scriptPath.c_str());
	
	DAVA::List<void (*)()>::iterator it = loadedItem.loadListeners.begin();
	for( ; it != loadedItem.loadListeners.end(); ++it )
	{
		(*it)();
	}
	
	if(ScriptLoadHelper::Instance()->resourceQueue.size() > 0)
	{
		emscripten_async_load_script(ScriptLoadHelper::Instance()->resourceQueue.front().scriptPath.c_str(),
									 ScriptLoadHelper::OnScriptLoaded,
									 ScriptLoadHelper::OnErrorLoadingScript);
	}
}

void ScriptLoadHelper::OnErrorLoadingScript()
{
	ScriptLoadingInfo loadedItem = ScriptLoadHelper::Instance()->resourceQueue.front();
	ScriptLoadHelper::Instance()->resourceQueue.pop_front();
	Logger::Debug("Error loading script %s", loadedItem.scriptPath.c_str());
	
	DAVA::List<void (*)()>::iterator it = loadedItem.errorListeners.begin();
	for( ; it != loadedItem.errorListeners.end(); ++it )
	{
		(*it)();
	}
	
	if(ScriptLoadHelper::Instance()->resourceQueue.size() > 0)
	{
		emscripten_async_load_script(ScriptLoadHelper::Instance()->resourceQueue.front().scriptPath.c_str(),
									 ScriptLoadHelper::OnScriptLoaded,
									 ScriptLoadHelper::OnErrorLoadingScript);
	}
}

#endif