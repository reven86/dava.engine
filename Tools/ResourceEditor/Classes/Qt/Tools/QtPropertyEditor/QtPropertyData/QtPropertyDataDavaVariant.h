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



#ifndef __QT_PROPERTY_DATA_DAVA_VARIANT_H__
#define __QT_PROPERTY_DATA_DAVA_VARIANT_H__

#include "Base/Introspection.h"
#include "Base/EnumMap.h"

#include "../QtPropertyData.h"

class QToolButton;

class QtPropertyDataDavaVariant : public QtPropertyData
{
	Q_OBJECT

	friend class QtPropertyDataDavaVariantSubValue;

public:
	QtPropertyDataDavaVariant(const DAVA::VariantType &value);
	virtual ~QtPropertyDataDavaVariant();

	const DAVA::VariantType& GetVariantValue() const;
	void SetVariantValue(const DAVA::VariantType& value);

	void AddAllowedValue(const DAVA::VariantType& realValue, const QVariant& visibleValue = QVariant());
	void ClearAllowedValues();

	virtual void SetIcon(const QIcon &icon);
	virtual QIcon GetIcon();

	QVariant FromDavaVariant(const DAVA::VariantType &variant);

protected:
	DAVA::VariantType curVariantValue;

	virtual QVariant GetValueInternal();
	virtual QVariant GetValueAlias();
	virtual void SetValueInternal(const QVariant &value);

	virtual QWidget* CreateEditorInternal(QWidget *parent, const QStyleOptionViewItem& option);
	virtual bool SetEditorDataInternal(QWidget *editor);
	virtual bool EditorDoneInternal(QWidget *editor);

protected slots:
	void ColorOWPressed();
	void FilePathOWPressed();
	void AllowedOWPressed();
	void AllowedSelected(int index);

private:
	struct AllowedValue
	{
		DAVA::VariantType realValue;
		QVariant visibleValue;
	};

	QVector<AllowedValue> allowedValues;
	bool allowedValuesLocked;
	QToolButton *allowedButton;

	bool iconCacheIsValid;
	QIcon iconCache;

	void InitFlags();
	void ChildsCreate();
	void ChildsSetFromMe();
	void MeSetFromChilds();

	QVariant FromKeyedArchive(DAVA::KeyedArchive *archive);
	QVariant FromVector4(const DAVA::Vector4 &vector);
	QVariant FromVector3(const DAVA::Vector3 &vector);
	QVariant FromVector2(const DAVA::Vector2 &vector);
	QVariant FromMatrix4(const DAVA::Matrix4 &matrix);
	QVariant FromMatrix3(const DAVA::Matrix3 &matrix);
	QVariant FromMatrix2(const DAVA::Matrix2 &matrix);
	QVariant FromColor(const DAVA::Color &color);
	QVariant FromAABBox3(const DAVA::AABBox3 &aabbox);

	void ToKeyedArchive(const QVariant &value);
	void ToVector4(const QVariant &value);
	void ToVector3(const QVariant &value);
	void ToVector2(const QVariant &value);
	void ToMatrix4(const QVariant &value);
	void ToMatrix3(const QVariant &value);
	void ToMatrix2(const QVariant &value);
	void ToColor(const QVariant &value);
	void ToAABBox3(const QVariant &value);

	void SubValueAdd(const QString &key, const QVariant &value);
	void SubValueSet(const QString &key, const QVariant &value);
	QVariant SubValueGet(const QString &key);

	QWidget* CreateAllowedValuesEditor(QWidget *parent);
	void SetAllowedValueEditorData(QWidget *editorWidget);
	void ApplyAllowedValueFromEditor(QWidget *editorWidget);
};

class QtPropertyDataDavaVariantSubValue : public QtPropertyData
{
public:
	QtPropertyDataDavaVariantSubValue(QtPropertyDataDavaVariant *_parentVariant, const QVariant &value)
		: QtPropertyData(value)
		, parentVariant(_parentVariant)
		, trackParent(true)
	{ }

	QtPropertyDataDavaVariant *parentVariant;
	bool trackParent;

	virtual void SetValueInternal(const QVariant &value)
	{
		QtPropertyData::SetValueInternal(value);
		if(NULL != parentVariant && trackParent)
		{
			parentVariant->MeSetFromChilds();
		}

		trackParent = true;
	}
};

#endif // __QT_PROPERTY_DATA_DAVA_VARIANT_H__
