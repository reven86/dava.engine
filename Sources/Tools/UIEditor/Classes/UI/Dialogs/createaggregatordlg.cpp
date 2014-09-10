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


#include "Classes/UI/Dialogs/createaggregatordlg.h"
#include "ui_createaggregatordlg.h"
#include "HierarchyTreeController.h"

#include <QMessageBox>

CreateAggregatorDlg::CreateAggregatorDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateAggregatorDlg)
{
    ui->setupUi(this);
	ui->lineEdit->setValidator(new QRegExpValidator(HierarchyTreeNode::GetNameRegExp()));

	const HierarchyTreeNode::HIERARCHYTREENODESLIST& platforms = HierarchyTreeController::Instance()->GetTree().GetPlatforms();
	for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter = platforms.begin();
		 iter != platforms.end();
		 ++iter)
	{
		ui->platformsCombo->addItem((*iter)->GetName());
		ui->platformsCombo->setItemData(ui->platformsCombo->count() - 1, QVariant((*iter)->GetId()));
	}
}

CreateAggregatorDlg::~CreateAggregatorDlg()
{
    delete ui;
}

QString CreateAggregatorDlg::GetName() const
{
	return ui->lineEdit->text();
}

Rect CreateAggregatorDlg::GetRect() const
{
	return Rect(0, 0, ui->width->value(), ui->height->value());
}

HierarchyTreeNode::HIERARCHYTREENODEID CreateAggregatorDlg::GetPlatformId() const
{
	return ui->platformsCombo->itemData(ui->platformsCombo->currentIndex()).toInt();
}

void CreateAggregatorDlg::SetDefaultPlatform(HierarchyTreeNode::HIERARCHYTREENODEID platformId)
{
	HierarchyTreePlatformNode* node = dynamic_cast<HierarchyTreePlatformNode*>(HierarchyTreeController::Instance()->GetTree().GetNode(platformId));
	if (!node)
	{
		node = HierarchyTreeController::Instance()->GetActivePlatform();
		if (node)
		{
			platformId = node->GetId();
		}
	}

	int id = ui->platformsCombo->findData(QVariant(platformId));
	ui->platformsCombo->setCurrentIndex(id);

	if (node)
	{
		ui->width->setValue(node->GetWidth());
		ui->height->setValue(node->GetHeight());
	}
}

void CreateAggregatorDlg::accept()
{
	const QString name = GetName();
	QString errorMsg("");
	if (!name.isNull() && !name.isEmpty())
	{
		// Get current selected patform in combobox. It could differ from active one.
		int platformId = GetPlatformId();
		HierarchyTreePlatformNode* paltfromNode = dynamic_cast<HierarchyTreePlatformNode*>(HierarchyTreeController::Instance()->GetTree().GetNode(platformId));
		
		if(paltfromNode && !paltfromNode->IsAggregatorOrScreenNamePresent(name))
		{
			QDialog::accept();
		}
		else
		{
			errorMsg = "Please fill aggregator name field with unique value.\r\n The same name with any of screen is forbidden.";
		}
	}
	else
	{
		errorMsg = "Please fill aggregator name field with value. It can't be empty.";
	}
	
	if(!errorMsg.isEmpty())
	{
		QMessageBox msgBox;
		msgBox.setText(errorMsg);
		msgBox.exec();
	}
	
}