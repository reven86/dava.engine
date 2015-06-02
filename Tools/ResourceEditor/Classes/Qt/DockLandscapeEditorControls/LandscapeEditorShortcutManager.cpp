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
	//shortcut disabled by default, will be changed during turning on/off of editors
	shortcut->setEnabled(false);
	
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

void LandscapeEditorShortcutManager::SetHeightMapEditorShortcutsEnabled(bool enabled)
{
	shortcutsMap[ResourceEditor::SHORTCUT_SET_ABSOLUTE]->setEnabled(enabled);
	shortcutsMap[ResourceEditor::SHORTCUT_SET_RELATIVE]->setEnabled(enabled);
	shortcutsMap[ResourceEditor::SHORTCUT_SET_AVERAGE]->setEnabled(enabled);
	shortcutsMap[ResourceEditor::SHORTCUT_SET_ABS_DROP]->setEnabled(enabled);
	shortcutsMap[ResourceEditor::SHORTCUT_SET_DROPPER]->setEnabled(enabled);
	shortcutsMap[ResourceEditor::SHORTCUT_SET_COPY_PASTE]->setEnabled(enabled);
}

void LandscapeEditorShortcutManager::SetTileMaskEditorShortcutsEnabled(bool enabled)
{
	shortcutsMap[ResourceEditor::SHORTCUT_NORMAL_DRAW_TILEMASK]->setEnabled(enabled);
	shortcutsMap[ResourceEditor::SHORTCUT_COPY_PASTE_TILEMASK]->setEnabled(enabled);
}

void LandscapeEditorShortcutManager::SetBrushSizeShortcutsEnabled(bool enabled)
{
	shortcutsMap[ResourceEditor::SHORTCUT_BRUSH_SIZE_INCREASE_SMALL]->setEnabled(enabled);
	shortcutsMap[ResourceEditor::SHORTCUT_BRUSH_SIZE_DECREASE_SMALL]->setEnabled(enabled);
	shortcutsMap[ResourceEditor::SHORTCUT_BRUSH_SIZE_INCREASE_LARGE]->setEnabled(enabled);
	shortcutsMap[ResourceEditor::SHORTCUT_BRUSH_SIZE_DECREASE_LARGE]->setEnabled(enabled);
}

void LandscapeEditorShortcutManager::SetBrushImageSwitchingShortcutsEnabled(bool enabled)
{
	shortcutsMap[ResourceEditor::SHORTCUT_BRUSH_IMAGE_NEXT]->setEnabled(enabled);
	shortcutsMap[ResourceEditor::SHORTCUT_BRUSH_IMAGE_PREV]->setEnabled(enabled);
}

void LandscapeEditorShortcutManager::SetTextureSwitchingShortcutsEnabled(bool enabled)
{
	shortcutsMap[ResourceEditor::SHORTCUT_TEXTURE_NEXT]->setEnabled(enabled);
	shortcutsMap[ResourceEditor::SHORTCUT_TEXTURE_PREV]->setEnabled(enabled);
}

void LandscapeEditorShortcutManager::SetStrengthShortcutsEnabled(bool enabled)
{
	shortcutsMap[ResourceEditor::SHORTCUT_STRENGTH_INCREASE_SMALL]->setEnabled(enabled);
	shortcutsMap[ResourceEditor::SHORTCUT_STRENGTH_DECREASE_SMALL]->setEnabled(enabled);
	shortcutsMap[ResourceEditor::SHORTCUT_STRENGTH_INCREASE_LARGE]->setEnabled(enabled);
	shortcutsMap[ResourceEditor::SHORTCUT_STRENGTH_DECREASE_LARGE]->setEnabled(enabled);
}

void LandscapeEditorShortcutManager::SetAvgStrengthShortcutsEnabled(bool enabled)
{
	shortcutsMap[ResourceEditor::SHORTCUT_AVG_STRENGTH_INCREASE_SMALL]->setEnabled(enabled);
	shortcutsMap[ResourceEditor::SHORTCUT_AVG_STRENGTH_DECREASE_SMALL]->setEnabled(enabled);
	shortcutsMap[ResourceEditor::SHORTCUT_AVG_STRENGTH_INCREASE_LARGE]->setEnabled(enabled);
	shortcutsMap[ResourceEditor::SHORTCUT_AVG_STRENGTH_DECREASE_LARGE]->setEnabled(enabled);
}

void LandscapeEditorShortcutManager::SetVisibilityToolShortcutsEnabled(bool enabled)
{
	shortcutsMap[ResourceEditor::SHORTCUT_VISIBILITY_TOOL_SET_POINT]->setEnabled(enabled);
	shortcutsMap[ResourceEditor::SHORTCUT_VISIBILITY_TOOL_SET_AREA]->setEnabled(enabled);
}