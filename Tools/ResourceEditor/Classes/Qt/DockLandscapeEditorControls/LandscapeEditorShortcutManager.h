#ifndef __RESOURCEEDITORQT__LANDSCAPEEDITORSHORTCUTMANAGER__
#define __RESOURCEEDITORQT__LANDSCAPEEDITORSHORTCUTMANAGER__

#include "DAVAEngine.h"
#include "Base/StaticSingleton.h"

#include <QShortcut>

using namespace DAVA;

class LandscapeEditorShortcutManager: public DAVA::StaticSingleton<LandscapeEditorShortcutManager>
{
public:
	LandscapeEditorShortcutManager();
	~LandscapeEditorShortcutManager();

	QShortcut* GetShortcutByName(const String& name);
	QShortcut* CreateOrUpdateShortcut(const String& name, QKeySequence keySequence, bool autoRepeat = true, const String& description = "");

private:
	Map<String, QShortcut*> shortcutsMap;

	void InitDefaultShortcuts();
};

#endif /* defined(__RESOURCEEDITORQT__LANDSCAPEEDITORSHORTCUTMANAGER__) */