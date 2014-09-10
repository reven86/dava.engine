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


#include "importdialog.h"
#include "ui_importdialog.h"
#include "importdialogfilewidget.h"
#include <QPushButton>
#include "HierarchyTreeController.h"

ImportDialog::ImportDialog(eImportType type, QWidget *parent, const FilePath& platformPath)
:	QDialog(parent)
,	ui(new Ui::ImportDialog)
,	importType(type)
,	controlWidget(0)
,	platformId(HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY)
,	platformPath(platformPath)
{
	ui->setupUi(this);

	switch (type)
	{
		case IMPORT_PLATFORM:
			ui->widthSpinBox->setValue(0);
			ui->heightSpinBox->setValue(0);
			connect(ui->widthSpinBox, SIGNAL(valueChanged(int)), this, SLOT(UpdateOkButtonState()));
			connect(ui->heightSpinBox, SIGNAL(valueChanged(int)), this, SLOT(UpdateOkButtonState()));

			ui->screenImportPropertiesWidget->hide();
			break;

		case IMPORT_SCREEN:
			ui->platformImportPropertiesWidget->hide();
			break;

		default:
			break;
	}

	ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

	this->setAttribute(Qt::WA_AlwaysShowToolTips);

	if (type == IMPORT_SCREEN)
	{
		InitScreensImport();
	}
	else if (type == IMPORT_PLATFORM)
	{
		InitPlatformImport();
	}

	UpdateOkButtonState();
}

ImportDialog::~ImportDialog()
{
	ClearCurrentList();
	delete ui;
}

void ImportDialog::InitScreensImport()
{
	this->setWindowTitle(tr("Import screens or aggregators"));
	HierarchyTreePlatformNode* platform = HierarchyTreeController::Instance()->GetActivePlatform();
	if (platform)
	{
		platformId = platform->GetId();
	}
	else
	{
		return;
	}

	Vector<String> platformFileList = GetDirectoryContent(platform->GetPlatformFolder(), true, ".yaml");
	Vector<String> platformChildrenNames = GetChildrenNames(platform);
	Vector<String> nodesNotInProject = GetStringNotInList(platformFileList, platformChildrenNames);

	InitWithFileList(nodesNotInProject);
}

void ImportDialog::InitPlatformImport()
{
	this->setWindowTitle(tr("Import platform"));

	Vector<String> platformFileList = GetDirectoryContent(platformPath, true, ".yaml");
	InitWithFileList(platformFileList);
}

void ImportDialog::ClearCurrentList()
{
	for (uint32 i = 0; i < fileWidgets.size(); ++i)
	{
		ImportDialogFileWidget* widget = fileWidgets[i];
		SafeDelete(widget);
	}
	SafeDelete(controlWidget);

	fileWidgets.clear();
}

void ImportDialog::UpdateOkButtonState()
{
	bool enabled = true;

	if (importType == IMPORT_PLATFORM)
	{
		enabled &= (ui->widthSpinBox->value() > 0);
		enabled &= (ui->heightSpinBox->value() > 0);
	}
	enabled &= !fileWidgets.empty();
	enabled &= CheckAggregatorsSize();

	ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(enabled);
}

bool ImportDialog::CheckAggregatorsSize()
{
	bool res = true;

	Vector<ImportDialogFileWidget*>::const_iterator it;
	for (it = fileWidgets.begin(); it != fileWidgets.end(); ++it)
	{
		if ((*it)->GetSelectedAction() != ACTION_AGGREGATOR)
		{
			continue;
		}

		QSize size = (*it)->GetSize();

		res &= (size.width() != 0);
		res &= (size.height() != 0);

		if (!res)
		{
			break;
		}
	}

	return res;
}

void ImportDialog::InitWithFileList(const Vector<String> &fileList)
{
	ClearCurrentList();

	int32 filesCount = fileList.size();
	if (filesCount > 1)
	{
		controlWidget = new ImportDialogFileWidget(CONTROL_WIDGET_ID);
		controlWidget->InitWithFilenameAndAction("", DEFAULT_ACTION);
		controlWidget->SetSizeWidgetShowable(false);
		controlWidget->SetUpperIconsShowable(true);
		ui->filesLayout->addWidget(controlWidget);

		connect(controlWidget, SIGNAL(ActionChanged(uint32)), this, SLOT(OnActionChanged(uint32)));
	}

	for (int32 i = 0; i < filesCount; ++i)
	{
		String fileName = fileList[i];

		ImportDialogFileWidget* widget = new ImportDialogFileWidget(i);
		widget->InitWithFilenameAndAction(QString::fromStdString(fileName), DEFAULT_ACTION);
		ui->filesLayout->addWidget(widget);
		fileWidgets.push_back(widget);

		connect(widget, SIGNAL(ActionChanged(uint32)), this, SLOT(OnActionChanged(uint32)));
		connect(ui->resetAllButtonPlatform, SIGNAL(clicked()), widget, SLOT(ResetAggregatorSize()));
		connect(ui->resetAllButtonScreen, SIGNAL(clicked()), widget, SLOT(ResetAggregatorSize()));
		connect(widget, SIGNAL(SizeChanged()), this, SLOT(UpdateOkButtonState()));
	}

	if (filesCount == 1)
	{
		fileWidgets[0]->SetUpperIconsShowable(true);
	}
}

void ImportDialog::OnActionChanged(uint32 id)
{
	if (id == CONTROL_WIDGET_ID)
	{
		eAction controlAction = controlWidget->GetSelectedAction();
		for (uint32 i = 0; i < fileWidgets.size(); ++i)
		{
			ImportDialogFileWidget* widget = fileWidgets[i];
			widget->SetAction(controlAction);
		}
	}
	else
	{
		eAction firstAction = fileWidgets[0]->GetSelectedAction();

		bool allActionsAreSame = true;

		for (uint32 i = 1; i < fileWidgets.size(); ++i)
		{
			allActionsAreSame &= (fileWidgets[i]->GetSelectedAction() == firstAction);
		}

		if (allActionsAreSame && controlWidget)
		{
			controlWidget->SetAction(firstAction);
		}
	}

	UpdateOkButtonState();
}

Vector<ImportDialog::FileItem> ImportDialog::GetFiles()
{
	Vector<FileItem> result;

	QSize platformSize = GetPlatformSize();
	for (uint32 i = 0; i < fileWidgets.size(); ++i)
	{
		ImportDialogFileWidget* widget = fileWidgets[i];

		FileItem item;
		item.fileName = widget->GetFilename();
		item.action = widget->GetSelectedAction();
		item.size = widget->GetSize();

		if (item.action == ACTION_AGGREGATOR)
		{
			if (item.size.width() == -1)
			{
				item.size.setWidth(platformSize.width());
			}

			if (item.size.height() == -1)
			{
				item.size.setHeight(platformSize.height());
			}
		}

		if (item.action != ACTION_IGNORE)
		{
			result.push_back(item);
		}
	}

	return result;
}

QSize ImportDialog::GetPlatformSize()
{
	if (importType == IMPORT_PLATFORM)
	{
		return QSize(ui->widthSpinBox->value(), ui->heightSpinBox->value());
	}
	else if (importType == IMPORT_SCREEN)
	{
		HierarchyTreePlatformNode* platform = dynamic_cast<HierarchyTreePlatformNode*>(HierarchyTreeController::Instance()->GetTree().GetNode(platformId));
		if (!platform)
		{
			return QSize(0, 0);
		}

		return QSize(platform->GetWidth(), platform->GetHeight());
	}

	return QSize(0, 0);
}

HierarchyTreeNode::HIERARCHYTREENODEID ImportDialog::GetPlatformId()
{
	if (importType == IMPORT_SCREEN)
	{
		return platformId;
	}

	return HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;
}

Vector<String> ImportDialog::GetDirectoryContent(const FilePath& path, bool getFiles, const String& fileMask, bool removeExtension)
{
	ScopedPtr<FileList> fileList( new FileList(path) );
	fileList->Sort();
	
	Vector<String> list;
	for (int32 i = 0; i < fileList->GetCount(); ++i)
	{
		String s = fileList->GetFilename(i);
		bool isDirectory = fileList->IsDirectory(i);
		
		if (isDirectory && !getFiles)
		{
			if (!fileList->IsNavigationDirectory(i))
			{
				list.push_back(s);
			}
		}
		else if (!isDirectory && getFiles)
		{
			String ext = fileList->GetPathname(i).GetExtension();
			if (ext == fileMask)
			{
				if (removeExtension)
				{
					s = fileList->GetPathname(i).GetBasename();
				}
				list.push_back(s);
			}
		}
	}
	
	return list;
}

Vector<String> ImportDialog::GetChildrenNames(HierarchyTreeNode* node)
{
	Vector<String> list;
	
	HierarchyTreeNode::HIERARCHYTREENODESCONSTITER it;
	for (it = node->GetChildNodes().begin(); it != node->GetChildNodes().end(); ++it)
	{
		list.push_back((*it)->GetName().toStdString());
	}
	
	return list;
}

Vector<String> ImportDialog::GetStringNotInList(const Vector<String>& strings, const Vector<String>& listToSearchIn)
{
	Vector<String> result;
	for (Vector<String>::const_iterator it = strings.begin(); it != strings.end(); ++it)
	{
		if (std::find(listToSearchIn.begin(), listToSearchIn.end(), *it) == listToSearchIn.end())
		{
			result.push_back(*it);
		}
	}
	
	return result;
}