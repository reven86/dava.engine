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


#ifndef __TEXTURE_LIST_MODEL_H__
#define __TEXTURE_LIST_MODEL_H__

#include <QAbstractListModel>
#include <QVector>

#include "DAVAEngine.h"
#include "Scene/SceneHelper.h"
#include "Scene/EntityGroup.h"

class TextureListModel : public QAbstractListModel
{
	Q_OBJECT

public:
	enum TextureListSortMode
	{
		SortByName,
		SortByFileSize,
		SortByImageSize,
		SortByDataSize
	};

	TextureListModel(QObject *parent = 0);
	~TextureListModel();

	void setScene(DAVA::Scene *scene);
	void setHighlight(const EntityGroup *nodes);
	void setFilter(QString filter);
	void setFilterBySelectedNode(bool enabled);
	void setSortMode(TextureListModel::TextureListSortMode sortMode);

	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

	void dataReady(const DAVA::TextureDescriptor *descriptor);

	DAVA::Texture* getTexture(const QModelIndex &index) const;
	DAVA::Texture* getTexture(const DAVA::TextureDescriptor* descriptor) const;
	DAVA::TextureDescriptor* getDescriptor(const QModelIndex &index) const;

	bool isHighlited(const QModelIndex &index) const;

private:
	QVector<DAVA::TextureDescriptor *> textureDescriptorsAll;
	QVector<DAVA::TextureDescriptor *> textureDescriptorsFiltredSorted;
	QVector<DAVA::TextureDescriptor *> textureDescriptorsHighlight;
	QMap<const DAVA::TextureDescriptor *, DAVA::Texture *> texturesAll;

	TextureListSortMode curSortMode;
	QString	curFilter;
	bool curFilterBySelectedNode;

	void clear();
	void applyFilterAndSort();
    
    DAVA::Scene *activeScene;
};

struct SortFnByName
{
	bool operator()(const DAVA::TextureDescriptor* t1, const DAVA::TextureDescriptor* t2);
};

struct SortFnByFileSize
{
	bool operator()(const DAVA::TextureDescriptor* t1, const DAVA::TextureDescriptor* t2);
};

struct SortFnByImageSize
{
	SortFnByImageSize(const TextureListModel *m) : model(m) { }
	bool operator()(const DAVA::TextureDescriptor* t1, const DAVA::TextureDescriptor* t2);

protected:
	const TextureListModel *model;
};

struct SortFnByDataSize
{
	SortFnByDataSize(const TextureListModel *m) : model(m) { }
	bool operator()(const DAVA::TextureDescriptor* t1, const DAVA::TextureDescriptor* t2);

protected:
	const TextureListModel *model;
};

#endif // __TEXTURE_LIST_MODEL_H__
