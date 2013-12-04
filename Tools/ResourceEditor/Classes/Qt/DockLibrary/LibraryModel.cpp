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



#include <QTreeView>
#include "../SceneEditor/EditorSettings.h"
#include "Main/QtUtils.h"
#include "DockLibrary/LibraryModel.h"
#include "DAVAEngine.h"
#include "Classes/Qt/Scene/SceneTabWidget.h"
#include "Classes/Qt/Main/mainwindow.h"

using namespace DAVA;

// If you need to specify different color backgrounds for different files extensions,
// expand this list.
ExtensionToColorMap LibraryModel::extensionToBackgroundColorMap[] =
{
	{"dae", QColor(222, 239, 254) },
	{ NULL, Qt::black }
};
	
LibraryModel::LibraryModel(QObject *parent)
    :   QFileSystemModel(parent)
{
	SetFileNameFilters(true, true);
    setNameFilterDisables(false);
}

LibraryModel::~LibraryModel()
{ }

void LibraryModel::SetLibraryPath(const QString &path)
{
	setRootPath(QDir(path).canonicalPath());
}

QVariant LibraryModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
	{
		return QFileSystemModel::data(index, role);
	}

	if ((Qt::BackgroundColorRole == role))
    {
        QFileInfo info = fileInfo(index);
        
        if(info.isFile())
        {
			return GetColorForExtension(info.suffix(), extensionToBackgroundColorMap, index, role);
        }
    }

    return QFileSystemModel::data(index, role);
}

void LibraryModel::SetFileNameFilters(bool showDAEFiles, bool showSC2Files)
{
	QStringList nameFilters;

	showDAE = showDAEFiles;
	showSC2 = showSC2Files;

	if (showDAEFiles)
	{
	    nameFilters << QString("*.dae");
	}
	if (showSC2Files)
	{
		nameFilters << QString("*.sc2");
	}

	QDir::Filters filterForNoneSelected = QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Dirs | QDir::Drives;
	if (nameFilters.empty())
	{
		// Show directories only in case nothing is selected.
		this->setFilter(filterForNoneSelected);
	}
	else
	{
		// Show directories and files in case file tyoes are selected.
		this->setFilter(filterForNoneSelected | QDir::Files);
		setNameFilters(nameFilters);
	}
}

bool LibraryModel::IsDAEShown()
{
	return showDAE;
}

bool LibraryModel::IsSC2Shown()
{
	return showSC2;
}

QVariant LibraryModel::GetColorForExtension(const QString& extension, const ExtensionToColorMap* colorMap, const QModelIndex &index, int role) const
{
	int pos = 0;
	while (colorMap[pos].extension != NULL)
	{
		if (colorMap[pos].extension.compare(extension,  Qt::CaseInsensitive) == 0)
		{
			return colorMap[pos].color;
		}
		
		pos ++;
	}

    return QFileSystemModel::data(index, role);
}

QMimeData * LibraryModel::mimeData( const QModelIndexList & indexes ) const
{
	SceneTabWidget *widget = QtMainWindow::Instance()->GetSceneWidget();
	widget->HideScenePreview();

	return QFileSystemModel::mimeData(indexes);
}
