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



#include "SceneSettingsDialog.h"
#include "Tools/QtPropertyEditor/QtPropertyEditor.h"
#include "Main/mainwindow.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataIntrospection.h"

#define EDITOR_TAB_WIDTH 400

SceneSettingsDialog::SceneSettingsDialog( QWidget* parent)
		:QDialog(parent)
{
	setWindowTitle("Scene Settings");
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	sceneSettingsEditor = new QtPropertyEditor(this);
	InitSceneSettingsEditor();
	sceneSettingsEditor->setMinimumWidth(EDITOR_TAB_WIDTH);
	sceneSettingsEditor->resizeColumnToContents(0);
	btnOk = new QPushButton("OK", this);
	btnOk->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	connect(btnOk, SIGNAL(clicked()), this, SLOT(close()));

	mainLayout = new QVBoxLayout;
	mainLayout->addWidget(sceneSettingsEditor);
	mainLayout->addWidget(btnOk, 0, Qt::AlignRight);
	setLayout(mainLayout);
}

SceneSettingsDialog::~SceneSettingsDialog()
{
	SafeDelete(sceneSettingsEditor);
	SafeDelete(btnOk);
}

void SceneSettingsDialog::InitSceneSettingsEditor()
{
	SceneEditor2* sceneEditor = QtMainWindow::Instance()->GetCurrentScene();
	if(!sceneEditor)
	{
		return;
	}
	const InspInfo* inspInfo = sceneEditor->GetTypeInfo();

	uint32 membCount = inspInfo->MembersCount();
	for(uint32 i = 0; i < membCount; i++)
	{
		const InspMember* member = inspInfo->Member(i);//get to systems layer
		const MetaInfo* memberInfo = member->Type();
		if(NULL != memberInfo->GetIntrospection())
		{
			QtPropertyData* propData = QtPropertyDataIntrospection::CreateMemberData(sceneEditor, member);
			sceneSettingsEditor->AppendProperty(member->Desc().text, propData);
		}
	}
	
	sceneSettingsEditor->expandAll();
}

