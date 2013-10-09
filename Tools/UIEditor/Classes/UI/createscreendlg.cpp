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


#include "Classes/UI/createscreendlg.h"
#include "ui_createscreendlg.h"
#include "HierarchyTreeController.h"

#include <QMessageBox>

CreateScreenDlg::CreateScreenDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateScreenDlg)
{
    ui->setupUi(this);
	
	const HierarchyTreeNode::HIERARCHYTREENODESLIST& platforms = HierarchyTreeController::Instance()->GetTree().GetPlatforms();
	for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter = platforms.begin();
		 iter != platforms.end();
		 ++iter)
	{
		ui->platformsCombo->addItem((*iter)->GetName());
		ui->platformsCombo->setItemData(ui->platformsCombo->count() - 1, QVariant((*iter)->GetId()));
	}
}

CreateScreenDlg::~CreateScreenDlg()
{
    delete ui;
}

QString CreateScreenDlg::GetScreenName() const
{
	return ui->lineEdit->text();
}

HierarchyTreeNode::HIERARCHYTREENODEID CreateScreenDlg::GetPlatformId() const
{
	return ui->platformsCombo->itemData(ui->platformsCombo->currentIndex()).toInt();
}

void CreateScreenDlg::SetDefaultPlatform(HierarchyTreeNode::HIERARCHYTREENODEID platformId)
{
	int id = ui->platformsCombo->findData(QVariant(platformId));
	if (id < 0) id = 0;
	ui->platformsCombo->setCurrentIndex(id);
}

void CreateScreenDlg::accept()
{
	const QString screenName = GetScreenName();
	QString errorMsg("");
	if (!screenName.isNull() && !screenName.isEmpty())
	{
		if(!HierarchyTreeController::Instance()->GetActivePlatform()->IsAggregatorOrScreenNamePresent(screenName))
		{
			QDialog::accept();
		}
		else
		{
			errorMsg = "Please fill screen name field with unique value.\r\nThe same name with any of aggregators is forbidden.";
		}
	}
	else
	{
		errorMsg = ("Please fill screen name field with value. It can't be empty.");
	}
	if(!errorMsg.isEmpty())
	{
		QMessageBox msgBox;
		msgBox.setText(errorMsg);
		msgBox.exec();
	}
}