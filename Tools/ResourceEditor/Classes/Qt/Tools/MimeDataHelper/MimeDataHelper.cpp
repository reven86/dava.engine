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



#include "MimeDataHelper.h"
#include "Scene/SceneEditor2.h"

#include <QList>
#include <QUrl>
#include <QFileInfo>
#include <QStringList>

namespace DAVA
{

#define BEGIM_MIME_HANDLER_MAP const MimeDataHelper::MimeHandler MimeDataHelper::mimeHandlerMap[] = {

#define MIME_HANDLER(formatName, getNameFuncPtr)\
	MimeDataHelper::MimeHandler(formatName, &getNameFuncPtr),

#define END_MIME_HNADLER_MAP };

#define MIME_HANDLERS_COUNT sizeof(MimeDataHelper::mimeHandlerMap) / sizeof(*MimeDataHelper::mimeHandlerMap)

BEGIM_MIME_HANDLER_MAP
	MIME_HANDLER("application/dava.entity",	MimeDataHelper::GetItemNamesFromSceneTreeMime)
	MIME_HANDLER("application/dava.emitter",MimeDataHelper::GetItemNamesFromSceneTreeMime)
	MIME_HANDLER("text/uri-list",			MimeDataHelper::GetItemNamesFromFilePathMime)
END_MIME_HNADLER_MAP

bool MimeDataHelper::IsMimeDataTypeSupported(const QMimeData* mimeData)
{
	for(int32 i = 0; i < MIME_HANDLERS_COUNT; ++i)
	{
		if(mimeData->hasFormat(QString(mimeHandlerMap[i].format.c_str())))
		{			
			return true;
		}
	}
	return false;
}

bool MimeDataHelper::IsMimeDataTypeSupported(const String& mimeType)
{
	for(int32 i = 0; i < MIME_HANDLERS_COUNT; ++i)
	{
		if(mimeType == mimeHandlerMap[i].format.c_str())
		{
			return true;
		}
	}
	return false;
}

void MimeDataHelper::GetItemNamesFromMimeData(const QMimeData* mimeData, List<String>& nameList)
{
	for(int32 i = 0; i < MIME_HANDLERS_COUNT; ++i)
	{
		if(mimeData->hasFormat(QString(mimeHandlerMap[i].format.c_str())))
		{
			mimeHandlerMap[i].getNameFuncPtr(mimeData, nameList);
		}
	}
}

void MimeDataHelper::ConvertToMimeData(List<FilePath>& filePathList, QMimeData* mimeData)
{
	if(mimeData == NULL)
	{
		return;
	}
	
	mimeData->clear();
	QList<QUrl> list;
	for(List<FilePath>::const_iterator it = filePathList.begin(); it != filePathList.end(); ++it)
	{
		list.append(QUrl::fromLocalFile(it->GetAbsolutePathname().c_str()));
	}
	mimeData->setUrls(list);
}

void MimeDataHelper::ConvertToMimeData(List<Entity*>& entityList, QMimeData* mimeData)
{
	if(mimeData == NULL)
	{
		return;
	}
	
	mimeData->clear();
	QByteArray encodedData;
	
	QDataStream stream(&encodedData, QIODevice::WriteOnly);
	foreach(Entity* entity, entityList)
	{
		stream.writeRawData((char *) &entity, sizeof(DAVA::Entity*));
	}
	
	mimeData->setData(QString(mimeHandlerMap[0].format.c_str()), encodedData);
}

	
List<Entity*> MimeDataHelper::GetPointersFromSceneTreeMime(const QMimeData* mimeData)
{
	List<Entity*> retList;
	if(mimeData == NULL)
	{
		return retList;
	}
	QByteArray encodedData = mimeData->data(mimeData->formats().first());
	QDataStream stream(&encodedData, QIODevice::ReadOnly);
	
	while(!stream.atEnd())
	{
		DAVA::Entity *entity = NULL;
		stream.readRawData((char *) &entity, sizeof(DAVA::Entity*));
		if(NULL != entity)
		{
			retList.push_back(entity);
		}
	}
	return retList;
}

void MimeDataHelper::GetItemNamesFromSceneTreeMime(const QMimeData* mimeData, List<String> & nameList)
{
	List<Entity*> entityList = GetPointersFromSceneTreeMime(mimeData);
	if(entityList.size() == 0)
	{
		return;
	}
	
	nameList.clear();
	Q_FOREACH(Entity* entity, entityList)
	{
		nameList.push_back(entity->GetName());
	}
}

void MimeDataHelper::GetItemNamesFromFilePathMime(const QMimeData* mimeData, List<String> & nameList)
{
	if(NULL == mimeData)
	{
		return;
	}
	nameList.clear();
	
	QList<QUrl> droppedUrls = mimeData->urls();
	Q_FOREACH(QUrl url, droppedUrls)
	{
		nameList.push_back(url.toLocalFile().toStdString());
	}
}

}//namespace DAVA