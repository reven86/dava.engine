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



#include "SelectPathWidget.h"
#include "./../Qt/Tools/MimeDataHelper/MimeDataHelper.h"


#include <QFileInfo>
#include <QKeyEvent>
#include <QUrl>
#include <QFileDialog>
#include <QStyle>

SelectPathWidget::SelectPathWidget(QWidget* _parent, DAVA::String _openDialogDefualtPath, DAVA::String _relativPath)
:	QLineEdit(_parent)
{
	Init(_openDialogDefualtPath, _relativPath);
}

SelectPathWidget::~SelectPathWidget()
{
	delete openButton;
	delete clearButton;
}

void SelectPathWidget::Init(DAVA::String& _openDialogDefualtPath, DAVA::String& _relativPath)
{
	setAcceptDrops(true);
	
	relativePath = DAVA::FilePath(_relativPath);
	openDialogDefaultPath = _openDialogDefualtPath;
	
	clearButton = CreateToolButton(":/QtIcons/ccancel.png");
	openButton = CreateToolButton(":/QtIcons/openscene.png");
		
	connect(clearButton, SIGNAL(clicked()), this, SLOT(EraseClicked()));
	connect(openButton, SIGNAL(clicked()), this, SLOT(OpenClicked()));
}

void SelectPathWidget::resizeEvent(QResizeEvent *)
{
	QSize sz = clearButton->sizeHint();
	int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
	clearButton->move(rect().right() - frameWidth - sz.width(),(rect().bottom() + 1 - sz.height())/2);
	
	QSize szOpenBtn = openButton->sizeHint();
	openButton->move(rect().right() - sz.width() - frameWidth - szOpenBtn.width(),(rect().bottom() + 1 - szOpenBtn.height())/2);
}

QToolButton* SelectPathWidget::CreateToolButton(const DAVA::String& iconPath)
{
	QToolButton* retButton;
	
	retButton = new QToolButton(this);
	QIcon icon(iconPath.c_str());
	retButton->setIcon(icon);
	retButton->setCursor(Qt::ArrowCursor);
	retButton->setStyleSheet("QToolButton { border: none; padding: 0px; }");
	int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
	setStyleSheet(QString("QLineEdit { padding-right: %1px; } ").arg(retButton->sizeHint().width() + frameWidth + 1));
	QSize msz = minimumSizeHint();
	setMinimumSize(qMax(msz.width(), retButton->sizeHint().height() + frameWidth * 2 + 2),
				   qMax(msz.height(), retButton->sizeHint().height() + frameWidth * 2 + 2));
	
	return retButton;
}

void SelectPathWidget::dragEnterEvent(QDragEnterEvent* event)
{
	if(DAVA::MimeDataHelper::IsMimeDataTypeSupported(event->mimeData()))
	{
		event->acceptProposedAction();
	}
}

void SelectPathWidget::dropEvent(QDropEvent* event)
{
	const QMimeData* sendedMimeData = event->mimeData();
		
	DAVA::List<DAVA::String> nameList;
	
	DAVA::MimeDataHelper::GetItemNamesFromMimeData(sendedMimeData, nameList);
	if(nameList.size() == 0)
	{
		return;
	}
	
	mimeData.clear();

	
	foreach(const QString & format, event->mimeData()->formats())
	{
		if(DAVA::MimeDataHelper::IsMimeDataTypeSupported(format.toStdString()))
		{
			mimeData.setData(format, event->mimeData()->data(format));
		}
	}
	
	DAVA::String itemName = *nameList.begin();
	DAVA::FilePath filePath(itemName);
	if(filePath.Exists())// check is it item form scene tree or file system
	{
		SetPathText(filePath.GetAbsolutePathname().c_str());
	}
	else
	{
		SetPathText(itemName.c_str());
	}
	
	event->acceptProposedAction();
}

void SelectPathWidget::EraseWidget()
{
	setText("");
	mimeData.clear();
}

void SelectPathWidget::EraseClicked()
{
	EraseWidget();
}

void SelectPathWidget::OpenClicked()
{
	DAVA::FilePath presentPath(text().toStdString());
	DAVA::FilePath dialogString(openDialogDefaultPath);
	if(presentPath.GetDirectory().Exists())//check if file text box clean
	{
		dialogString = presentPath.GetDirectory();
	}
	
	DAVA::String retString = QFileDialog::getOpenFileName(this, "Open Scene File",
                                                    QString(dialogString.GetAbsolutePathname().c_str()),
                                                    "Scene File (*.sc2)").toStdString();

	if(!retString.empty())
	{
		SetPathText(retString);
		
		DAVA::FilePath fullPath(retString);
		
		DVASSERT(fullPath.Exists());
		DAVA::List<DAVA::FilePath> urls;
		urls.push_back(fullPath);
		DAVA::MimeDataHelper::ConvertToMimeData(urls, &mimeData);
	}
}

void SelectPathWidget::SetPathText(const DAVA::String& filePath)
{
	setText(QString(ConvertToRelativPath(filePath).c_str()));
}

DAVA::String SelectPathWidget::GetPathText()
{
	return text().toStdString();
}

void SelectPathWidget::SetRelativePath(const DAVA::String& newRelativPath)
{
	relativePath = DAVA::FilePath(newRelativPath);
	DAVA::String existingPath = text().toStdString();
	if(!existingPath.empty())
	{
		setText(QString(ConvertToRelativPath(existingPath).c_str()));
	}
}

DAVA::Entity* SelectPathWidget::GetOutputEntity(SceneEditor2* editor)
{
	DAVA::List<DAVA::Entity*> retList;
	DAVA::MimeDataHelper::ConvertFromMimeData(&mimeData, retList, editor);
	DAVA::Entity* retEntity = retList.size() > 0 ? *retList.begin(): NULL;
	return retEntity;
}

DAVA::String SelectPathWidget::ConvertToRelativPath(const DAVA::String& path)
{
	DAVA::FilePath fullPath(path);
	if(fullPath.Exists())
	{
		return fullPath.GetRelativePathname(relativePath);
	}
	else
	{
		return path;
	}
}

