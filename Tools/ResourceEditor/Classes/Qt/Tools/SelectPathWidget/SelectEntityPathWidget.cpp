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


#include "SelectEntityPathWidget.h"
#include "Tools/MimeDataHelper/MimeDataHelper.h"
#include "Scene/SceneEditor2.h"
#include <QFileInfo>
#include <QKeyEvent>
#include <QUrl>
#include <QStyle>

#define MIME_ENTITY_NAME "application/dava.entity"
#define MIME_EMITER_NAME "application/dava.emitter"
#define MIME_URI_LIST_NAME "text/uri-list"

SelectEntityPathWidget::SelectEntityPathWidget(QWidget* _parent, DAVA::String _openDialogDefualtPath, DAVA::String _relativPath)
:	SelectPathWidgetBase(_parent, false,_openDialogDefualtPath, _relativPath, "Open Scene File", "Scene File (*.sc2)")
{
	allowedFormatsList.push_back(".sc2");
}

SelectEntityPathWidget::~SelectEntityPathWidget()
{
	Q_FOREACH(DAVA::Entity* item, entitiesToHold)
	{
		SafeRelease(item);
	}
}

void SelectEntityPathWidget::dragEnterEvent(QDragEnterEvent* event)
{
	if(!DAVA::MimeDataHelper::IsMimeDataTypeSupported(event->mimeData()))
	{
		return;
	}

    event->setDropAction(Qt::LinkAction);

	bool isFormatSupported = true;
	
	if(event->mimeData()->hasFormat(MIME_URI_LIST_NAME))
	{
		isFormatSupported= false;
		DAVA::FilePath path(event->mimeData()->urls().first().toLocalFile().toStdString());
		Q_FOREACH(DAVA::String item, allowedFormatsList)
		{
			if(path.IsEqualToExtension(item))
			{
				isFormatSupported = true;
				break;
			}
		}
	}
	if(isFormatSupported)
	{
        event->accept();
	}
}


DAVA::Entity* SelectEntityPathWidget::GetOutputEntity(SceneEditor2* editor)
{
	DAVA::List<DAVA::Entity*> retList;
	ConvertFromMimeData(&mimeData, retList, editor);
	DAVA::Entity* retEntity = retList.size() > 0 ? *retList.begin(): NULL;
	return retEntity;
}

void SelectEntityPathWidget::ConvertFromMimeData(const QMimeData* mimeData, DAVA::List<DAVA::Entity*>& retList, SceneEditor2* sceneEditor)
{
	if(mimeData->hasFormat(MIME_ENTITY_NAME) || mimeData->hasFormat(MIME_EMITER_NAME))
	{
		ConvertQMimeDataFromSceneTree(mimeData, retList);
	}
	else if(mimeData->hasFormat(MIME_URI_LIST_NAME))
	{
		ConvertQMimeDataFromFilePath(mimeData, retList, sceneEditor);
	}
}

void SelectEntityPathWidget::ConvertQMimeDataFromSceneTree(const QMimeData* mimeData,
														   DAVA::List<DAVA::Entity*>& retList)
{
	retList = MimeDataHelper::GetPointersFromSceneTreeMime(mimeData);
	SetEntities(retList, true);
}

void SelectEntityPathWidget::ConvertQMimeDataFromFilePath(const QMimeData* mimeData,
														  DAVA::List<DAVA::Entity*>& retList,
														  SceneEditor2* sceneEditor)
{
	if(mimeData == NULL || sceneEditor == NULL || !mimeData->hasUrls())
	{
		return;
	}
	
	retList.clear();
	
	QList<QUrl> droppedUrls = mimeData->urls();
	
	Q_FOREACH(QUrl url, droppedUrls)
	{
		DAVA::FilePath filePath( url.toLocalFile().toStdString());
        if (!(FileSystem::Instance()->Exists(filePath) && filePath.GetExtension() == ".sc2"))
        {
            continue;
		}
		
		DAVA::Entity * entity = sceneEditor->structureSystem->Load(filePath);
		
		if(NULL != entity)
		{
			retList.push_back(entity);
		}
	}
	// for just created entities no need to increase refCouner
	// it will be released in ~SelectEntityPathWidget()
	SetEntities(retList, false);
}

void SelectEntityPathWidget::SetEntities(const DAVA::List<DAVA::Entity*>& list, bool perfromRertain)
{
	Q_FOREACH(DAVA::Entity* item, entitiesToHold)
	{
		SafeRelease(item);
	}
	entitiesToHold = list;
	if(perfromRertain)
	{
		Q_FOREACH(DAVA::Entity* item, entitiesToHold)
		{
			SafeRetain(item);
		}
	}
		
}