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


#include "Classes/UI/librarywidget.h"
#include "ui_librarywidget.h"
#include "LibraryController.h"
#include "IconHelper.h"

using namespace DAVA;

#define TEXT_ID 0
#define ITEM_ID 0, Qt::UserRole

LibraryWidget::LibraryWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LibraryWidget)
{
    ui->setupUi(this);
	ui->treeWidget->clear();
	LibraryController::Instance()->Init(this);
}

LibraryWidget::~LibraryWidget()
{
    delete ui;
}

QTreeWidgetItem* LibraryWidget::AddControl(const QString& name, const QString& iconPath, HierarchyTreeNode::HIERARCHYTREENODEID itemId)
{
	QTreeWidgetItem* control = new QTreeWidgetItem();
	control->setText(TEXT_ID, name);
	control->setIcon(TEXT_ID, QIcon(iconPath));
    control->setData(ITEM_ID, itemId);
	ui->treeWidget->addTopLevelItem(control);
	return control;
}

void LibraryWidget::RemoveControl(QTreeWidgetItem* item)
{
	int index = ui->treeWidget->indexOfTopLevelItem(item);
	if (index != -1)
	{
		delete item;
	}
}

void LibraryWidget::ResetSelection()
{
	if (ui->treeWidget->currentItem())
	{
		ui->treeWidget->reset();
	}
}