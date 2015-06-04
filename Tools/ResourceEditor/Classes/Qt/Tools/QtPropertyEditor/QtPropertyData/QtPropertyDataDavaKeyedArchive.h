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


#ifndef __QT_PROPERTY_DATA_DAVA_KEYEDARCHIVE_H__
#define __QT_PROPERTY_DATA_DAVA_KEYEDARCHIVE_H__

#include "Base/Introspection.h"
#include "FileSystem/KeyedArchive.h"
#include "../QtPropertyData.h"
#include "Commands2/KeyedArchiveCommand.h"

#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>

class QtPropertyDataDavaKeyedArcive : public QtPropertyData
{
	Q_OBJECT;

public:
	QtPropertyDataDavaKeyedArcive(DAVA::KeyedArchive *archive);
	virtual ~QtPropertyDataDavaKeyedArcive();

	virtual const DAVA::MetaInfo * MetaInfo() const;
	virtual void* CreateLastCommand() const;

	DAVA::KeyedArchive* archive;

protected:
	mutable Command2 *lastCommand;
	int lastAddedType;

	virtual QVariant GetValueInternal() const;
	virtual bool UpdateValueInternal();

private:
	void ChildCreate(const QString &key, DAVA::VariantType *value);

protected slots:
	void AddKeyedArchiveField();
	void RemKeyedArchiveField();
	void NewKeyedArchiveFieldReady(const DAVA::String &key, const DAVA::VariantType &value);
};

class KeyedArchiveItemWidget : public QWidget
{
	Q_OBJECT;

public:
	KeyedArchiveItemWidget(DAVA::KeyedArchive *arch, int defaultType = DAVA::VariantType::TYPE_STRING, QWidget *parent = NULL);
	~KeyedArchiveItemWidget();

signals:
	void ValueReady(const DAVA::String &key, const DAVA::VariantType &value);

protected:
	DAVA::KeyedArchive *arch;

	QLineEdit *keyWidget;
	QComboBox *valueWidget;
	QComboBox *presetWidget;
	QPushButton *defaultBtn;

	virtual void showEvent(QShowEvent * event);
	virtual void keyPressEvent(QKeyEvent *event);

protected slots:
	void OkKeyPressed();
	void PreSetSelected(int index);
};

#endif // __QT_PROPERTY_DATA_DAVA_KEYEDARCHIVE_H__
