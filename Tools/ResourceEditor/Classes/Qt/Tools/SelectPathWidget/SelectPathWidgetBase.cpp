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


#include "SelectPathWidgetBase.h"
#include "Tools/MimeDataHelper/MimeDataHelper.h"
#include "Qt/Settings/SettingsManager.h"
#include "Project/ProjectManager.h"

#include "QtTools/FileDialog/FileDialog.h"

#include <QFileInfo>
#include <QKeyEvent>
#include <QUrl>
#include <QStyle>
#include <QMessageBox>

SelectPathWidgetBase::SelectPathWidgetBase(QWidget* _parent, bool _checkForProjectPath, DAVA::String _openDialogDefualtPath, DAVA::String _relativPath, DAVA::String _openFileDialogTitle, DAVA::String _fileFormatDescriotion)
:	QLineEdit(_parent),
    checkForProjectPath(_checkForProjectPath)
{
	Init(_openDialogDefualtPath, _relativPath, _openFileDialogTitle, _fileFormatDescriotion);
}

SelectPathWidgetBase::~SelectPathWidgetBase()
{
	delete openButton;
	delete clearButton;
}

void SelectPathWidgetBase::Init(DAVA::String& _openDialogDefualtPath, DAVA::String& _relativPath,DAVA::String _openFileDialogTitle, DAVA::String _fileFormatFilter)
{
	setAcceptDrops(true);
	
	relativePath = DAVA::FilePath(_relativPath);
	openDialogDefaultPath = _openDialogDefualtPath;
	openFileDialogTitle = _openFileDialogTitle;
	fileFormatFilter = _fileFormatFilter;
	
	clearButton = CreateToolButton(":/QtIcons/ccancel.png");
	openButton = CreateToolButton(":/QtIcons/openscene.png");
		
	connect(clearButton, SIGNAL(clicked()), this, SLOT(EraseClicked()));
	connect(openButton, SIGNAL(clicked()), this, SLOT(OpenClicked()));
	connect(this, SIGNAL(editingFinished()), this, SLOT(acceptEditing()));
}

void SelectPathWidgetBase::resizeEvent(QResizeEvent *)
{
	QSize sz = clearButton->sizeHint();
	int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
	clearButton->move(rect().right() - frameWidth - sz.width(),(rect().bottom() + 1 - sz.height())/2);
	
	QSize szOpenBtn = openButton->sizeHint();
    int offsetFromRight = clearButton->isVisible() ? sz.width() : 0;
	openButton->move(rect().right() - offsetFromRight - frameWidth - szOpenBtn.width(),(rect().bottom() + 1 - szOpenBtn.height())/2);
}

QToolButton* SelectPathWidgetBase::CreateToolButton(const DAVA::String& iconPath)
{
	QToolButton* retButton;
	
	retButton = new QToolButton(this);
	QIcon icon(iconPath.c_str());
	retButton->setIcon(icon);
	retButton->setCursor(Qt::ArrowCursor);
	retButton->setStyleSheet("QToolButton { border: none; padding: 0px; }");
	int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
	QSize msz = minimumSizeHint();
	setStyleSheet(QString("QLineEdit { padding-right: %1px; } ").arg(retButton->sizeHint().width() * 2 + frameWidth));
	setMinimumSize(qMax(msz.width(), retButton->sizeHint().height() + frameWidth * 2 + 2),
				   qMax(msz.height(), retButton->sizeHint().height() + frameWidth * 2 + 2));
	
	return retButton;
}

void SelectPathWidgetBase::EraseWidget()
{
	setText(QString(""));
	mimeData.clear();
}


void SelectPathWidgetBase::EraseClicked()
{
	EraseWidget();
}

void SelectPathWidgetBase::acceptEditing()
{
	this->setText(getText());
}

void SelectPathWidgetBase::setVisible(bool value)
{
    QLineEdit::setVisible(value);
    SelectPathWidgetBase::resizeEvent(NULL);
}

void SelectPathWidgetBase::OpenClicked()
{
	DAVA::FilePath presentPath(text().toStdString());
	DAVA::FilePath dialogString(openDialogDefaultPath);
    if (DAVA::FileSystem::Instance()->Exists(presentPath.GetDirectory())) //check if file text box clean
    {
        dialogString = presentPath.GetDirectory();
	}
	this->blockSignals(true);
	DAVA::String retString = FileDialog::getOpenFileName(this, openFileDialogTitle.c_str(), QString(dialogString.GetAbsolutePathname().c_str()), fileFormatFilter.c_str()).toStdString();
	this->blockSignals(false);

    if(retString.empty())
    {
        return;
    }

    DAVA::String projectPath = ProjectManager::Instance()->GetProjectPath().GetAbsolutePathname();

    if(checkForProjectPath && DAVA::String::npos == retString.find(projectPath))
    {
        QMessageBox::warning(NULL, "Wrong file selected", QString(DAVA::Format("Path %s doesn't belong to project.", retString.c_str()).c_str() ), QMessageBox::Ok);
        return;
    }
    
	HandlePathSelected(retString);
}

void SelectPathWidgetBase::HandlePathSelected(DAVA::String name)
{
	DAVA::FilePath fullPath(name);

    DVASSERT(DAVA::FileSystem::Instance()->Exists(fullPath));

    setText(name);

    DAVA::List<DAVA::FilePath> urls;
	urls.push_back(fullPath);
	DAVA::MimeDataHelper::ConvertToMimeData(urls, &mimeData);
}

void SelectPathWidgetBase::setText(const QString& filePath)
{
    QLineEdit::setText(filePath);
    setToolTip(filePath);
    emit PathSelected(filePath.toStdString());
}

void SelectPathWidgetBase::setText(const DAVA::String &filePath)
{
    SelectPathWidgetBase::setText(QString(filePath.c_str()));
}

DAVA::String SelectPathWidgetBase::getText()
{
	return text().toStdString();
}


DAVA::String SelectPathWidgetBase::ConvertToRelativPath(const DAVA::String& path)
{
	DAVA::FilePath fullPath(path);
    if (DAVA::FileSystem::Instance()->Exists(fullPath))
    {
        return fullPath.GetRelativePathname(relativePath);
	}
	else
	{
		return path;
	}
}

void SelectPathWidgetBase::dropEvent(QDropEvent* event)
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
    if (DAVA::FileSystem::Instance()->Exists(filePath)) // check is it item form scene tree or file system
    {
        setText(filePath.GetAbsolutePathname());
	}
	else
	{
		setText(itemName);
	}
	
    event->setDropAction(Qt::LinkAction);
    event->accept();
}

void SelectPathWidgetBase::dragEnterEvent(QDragEnterEvent* event)
{
    event->setDropAction(Qt::LinkAction);
	if(DAVA::MimeDataHelper::IsMimeDataTypeSupported(event->mimeData()))
	{
        event->accept();
	}
}

void SelectPathWidgetBase::dragMoveEvent(QDragMoveEvent* event)
{
    event->setDropAction(Qt::LinkAction);
    event->accept();
}

bool SelectPathWidgetBase::IsOpenButtonVisible() const
{
    return openButton->isVisible();
}

void SelectPathWidgetBase::SetOpenButtonVisible(bool value)
{
    openButton->setVisible(value);
}

bool SelectPathWidgetBase::IsClearButtonVisible() const
{
    return clearButton->isVisible();
}

void SelectPathWidgetBase::SetClearButtonVisible(bool value)
{
    clearButton->setVisible(value);
}

DAVA::String SelectPathWidgetBase::GetOpenDialogDefaultPath() const
{
    return openDialogDefaultPath;
}

void SelectPathWidgetBase::SetOpenDialogDefaultPath(const DAVA::FilePath& path)
{
    openDialogDefaultPath = path.GetAbsolutePathname();
}

DAVA::String SelectPathWidgetBase::GetFileFormatFilter() const
{
    return fileFormatFilter;
}

void SelectPathWidgetBase::SetFileFormatFilter(const DAVA::String& filter)
{
    fileFormatFilter = filter;
}
