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



#ifndef __LIBRARY_VIEW_H__
#define __LIBRARY_VIEW_H__

#include <QTreeView>
#include <QAction>
#include "Scene/SceneDataManager.h"
#include "Commands/LibraryCommands.h"
#include "Commands/CommandsManager.h"
#include "DockLibrary/LibraryModel.h"

class ContextMenuAction : public QAction
{
	Q_OBJECT

public:
	ContextMenuAction(const QString &text, Command *command)
		: QAction(text, NULL)
		, curCommand(command)
	{ }

	~ContextMenuAction()
	{
		if(NULL != curCommand)
		{
			delete curCommand;
		}
	}

	void Exec()
	{
		if(NULL != curCommand)
		{
			Scene* scene = SceneDataManager::Instance()->SceneGetActive()->GetScene();
			CommandsManager::Instance()->Execute(curCommand, scene);
		}
	}

protected:
	Command *curCommand;
};

class LibraryView : public QTreeView
{
	Q_OBJECT

public:
	LibraryView(QWidget *parent = 0);
	~LibraryView();

public slots:
	void ProjectOpened(const QString &path);
	void ProjectClosed();
	void LibraryFileTypesChanged(bool showDAEFiles, bool showSC2Files);
	void OnModelEdit();
	void OnModelAdd();
	void OnDAEConvert();
	void OnDAEConvertWithSavingOfSettings();

	void ShowDAE(bool show);
	void ShowSC2(bool show);

protected slots:
	void ModelRootPathChanged(const QString & newPath);
	void ShowContextMenu(const QPoint &point);
	void FileSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

private:
	LibraryModel *libModel;
};

#endif // __LIBRARY_VIEW_H__ 
