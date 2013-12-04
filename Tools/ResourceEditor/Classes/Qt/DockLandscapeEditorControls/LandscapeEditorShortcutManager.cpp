#include "LandscapeEditorShortcutManager.h"
#include "../Main/mainwindow.h"

LandscapeEditorShortcutManager::LandscapeEditorShortcutManager()
{
	InitDefaultShortcuts();
}

LandscapeEditorShortcutManager::~LandscapeEditorShortcutManager()
{
}

QShortcut* LandscapeEditorShortcutManager::GetShortcutByName(const String& name)
{
	Map<String, QShortcut*>::iterator it = shortcutsMap.find(name);
	if (it != shortcutsMap.end())
	{
		return it->second;
	}

	return NULL;
}

QShortcut* LandscapeEditorShortcutManager::CreateOrUpdateShortcut(const String& name, QKeySequence keySequence,
																  bool autoRepeat, const String& description)
{
	QShortcut* shortcut = GetShortcutByName(name);
	if (shortcut == NULL)
	{
		shortcut = new QShortcut(QKeySequence(keySequence), QtMainWindow::Instance(), 0, 0, Qt::ApplicationShortcut);
		shortcutsMap[name] = shortcut;
	}

	shortcut->setKey(QKeySequence(keySequence));
	shortcut->setEnabled(true);
	shortcut->setAutoRepeat(autoRepeat);
	shortcut->setWhatsThis(description.c_str());

	return shortcut;
}

void LandscapeEditorShortcutManager::InitDefaultShortcuts()
{
	CreateOrUpdateShortcut(ResourceEditor::SHORTCUT_BRUSH_SIZE_INCREASE_SMALL,	Qt::CTRL | Qt::Key_Equal);
	CreateOrUpdateShortcut(ResourceEditor::SHORTCUT_BRUSH_SIZE_DECREASE_SMALL,	Qt::CTRL | Qt::Key_Minus);
	CreateOrUpdateShortcut(ResourceEditor::SHORTCUT_BRUSH_SIZE_INCREASE_LARGE,	Qt::CTRL | Qt::ALT | Qt::Key_Equal);
	CreateOrUpdateShortcut(ResourceEditor::SHORTCUT_BRUSH_SIZE_DECREASE_LARGE,	Qt::CTRL | Qt::ALT | Qt::Key_Minus);

	CreateOrUpdateShortcut(ResourceEditor::SHORTCUT_BRUSH_IMAGE_NEXT,	Qt::CTRL | Qt::Key_BracketRight, false);
	CreateOrUpdateShortcut(ResourceEditor::SHORTCUT_BRUSH_IMAGE_PREV,	Qt::CTRL | Qt::Key_BracketLeft, false);

	CreateOrUpdateShortcut(ResourceEditor::SHORTCUT_TEXTURE_NEXT,	Qt::CTRL | Qt::ALT | Qt::Key_BracketRight, false);
	CreateOrUpdateShortcut(ResourceEditor::SHORTCUT_TEXTURE_PREV,	Qt::CTRL | Qt::ALT | Qt::Key_BracketLeft, false);

	CreateOrUpdateShortcut(ResourceEditor::SHORTCUT_STRENGTH_INCREASE_SMALL,	Qt::CTRL | Qt::Key_I);
	CreateOrUpdateShortcut(ResourceEditor::SHORTCUT_STRENGTH_DECREASE_SMALL,	Qt::CTRL | Qt::Key_U);
	CreateOrUpdateShortcut(ResourceEditor::SHORTCUT_STRENGTH_INCREASE_LARGE,	Qt::CTRL | Qt::ALT | Qt::Key_I);
	CreateOrUpdateShortcut(ResourceEditor::SHORTCUT_STRENGTH_DECREASE_LARGE,	Qt::CTRL | Qt::ALT | Qt::Key_U);

	CreateOrUpdateShortcut(ResourceEditor::SHORTCUT_AVG_STRENGTH_INCREASE_SMALL,	Qt::CTRL | Qt::Key_K);
	CreateOrUpdateShortcut(ResourceEditor::SHORTCUT_AVG_STRENGTH_DECREASE_SMALL,	Qt::CTRL | Qt::Key_J);
	CreateOrUpdateShortcut(ResourceEditor::SHORTCUT_AVG_STRENGTH_INCREASE_LARGE,	Qt::CTRL | Qt::ALT | Qt::Key_K);
	CreateOrUpdateShortcut(ResourceEditor::SHORTCUT_AVG_STRENGTH_DECREASE_LARGE,	Qt::CTRL | Qt::ALT | Qt::Key_J);

	CreateOrUpdateShortcut(ResourceEditor::SHORTCUT_VISIBILITY_TOOL_SET_POINT,	Qt::ALT | Qt::Key_P, false);
	CreateOrUpdateShortcut(ResourceEditor::SHORTCUT_VISIBILITY_TOOL_SET_AREA,	Qt::ALT | Qt::Key_A, false);

	CreateOrUpdateShortcut(ResourceEditor::SHORTCUT_SET_ABSOLUTE,	Qt::CTRL | Qt::Key_1, false);
	CreateOrUpdateShortcut(ResourceEditor::SHORTCUT_SET_RELATIVE,	Qt::CTRL | Qt::Key_2, false);
	CreateOrUpdateShortcut(ResourceEditor::SHORTCUT_SET_AVERAGE,	Qt::CTRL | Qt::Key_3, false);
	CreateOrUpdateShortcut(ResourceEditor::SHORTCUT_SET_ABS_DROP,	Qt::CTRL | Qt::Key_4, false);
	CreateOrUpdateShortcut(ResourceEditor::SHORTCUT_SET_DROPPER,	Qt::CTRL | Qt::Key_5, false);
	CreateOrUpdateShortcut(ResourceEditor::SHORTCUT_SET_COPY_PASTE,	Qt::CTRL | Qt::Key_6, false);

	CreateOrUpdateShortcut(ResourceEditor::SHORTCUT_NORMAL_DRAW_TILEMASK,	Qt::CTRL | Qt::Key_1, false);
	CreateOrUpdateShortcut(ResourceEditor::SHORTCUT_COPY_PASTE_TILEMASK,	Qt::CTRL | Qt::Key_2, false);
}
