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



#include "GeneralSettingsDialog.h"
#include "Main/mainwindow.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataIntrospection.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataKeyedArchiveMember.h"


#define EDITOR_TAB_WIDTH 400

const SettingsManager::eSettingsGroups GeneralSettingsDialog::groupsTodisplay[] =
{
	SettingsManager::GENERAL,
	SettingsManager::DEFAULT
};

GeneralSettingsDialog::GeneralSettingsDialog( QWidget* parent)
		:QDialog(parent)
{
	setWindowTitle("Settings");
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	tabWidget = new QTabWidget;
	btnOK = new QPushButton("OK", this);
	connect(btnOK, SIGNAL(clicked(bool)), this, SLOT(accept()));

	mainLayout = new QVBoxLayout;
	mainLayout->addWidget(tabWidget);
	mainLayout->addWidget(btnOK, 0, Qt::AlignRight );
	setLayout(mainLayout);
	InitSettings();
}

GeneralSettingsDialog::~GeneralSettingsDialog()
{
	foreach (QtPropertyEditor* editor, settingGroupsEditors)
	{
		SafeDelete(editor);
	}
	delete btnOK;
	delete tabWidget;
}

void GeneralSettingsDialog::InitSettings()
{
	SettingsManager* manager = SettingsManager::Instance();
	for (uint32 groupID = 0; groupID < (sizeof(groupsTodisplay) / sizeof(SettingsManager::eSettingsGroups)) ; ++groupID)
	{
		KeyedArchive* settingsGroup = manager->GetSettingsGroup(groupsTodisplay[groupID]);
		if(NULL == settingsGroup)
		{
			continue;
		}
		String groupName = manager->GetNameOfGroup(groupsTodisplay[groupID]);
		
		QtPropertyEditor* groupEditor = new QtPropertyEditor(this);
		settingGroupsEditors.push_back(groupEditor);
		groupEditor->SetEditTracking(true);
		DAVA::Map<DAVA::String, DAVA::VariantType*> data = settingsGroup->GetArchieveData();
		DAVA::Map<DAVA::String, DAVA::VariantType*>::iterator it = data.begin();

		for(; it != data.end(); ++it)
		{
			QtPropertyData*  propData = new QtPropertyKeyedArchiveMember(settingsGroup, it->first);
			groupEditor->AppendProperty( it->first.c_str(), propData);
		}

		groupEditor->expandAll();
		groupEditor->setMinimumWidth(EDITOR_TAB_WIDTH);
		tabWidget->addTab(groupEditor, groupName.c_str());
		groupEditor->resizeColumnToContents(0);
	}
}