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



#include "DAVAEngine.h"
#include "Debug/DVAssert.h"
#include "Main/QtUtils.h"
#include "QtPropertyDataDavaVariant.h"
#include "../QtPropertyWidgets/QtColorLineEdit.h"
#include "../QtPropertyWidgets/QtComboFake.h"
#include "Tools/QtFileDialog/QtFileDialog.h"

#include <QColorDialog>
#include <QListWidget>
#include <QComboBox>
#include <QPushButton>
#include <QToolButton>
#include <QPainter>
#include <QLineEdit>
#include <QCoreApplication>
#include <QKeyEvent>

#include "SceneEditor/EditorSettings.h"

QtPropertyDataDavaVariant::QtPropertyDataDavaVariant(const DAVA::VariantType &value)
	: curVariantValue(value)
	, iconCacheIsValid(false)
	, allowedValuesLocked(false)
	, allowedButton(NULL)
{
	InitFlags();
	ChildsCreate();

	// ensure data is fully initialized (icons set)
	GetValueInternal();
}

QtPropertyDataDavaVariant::~QtPropertyDataDavaVariant()
{ }

const DAVA::VariantType& QtPropertyDataDavaVariant::GetVariantValue() const
{
	return curVariantValue;
}

void QtPropertyDataDavaVariant::InitFlags()
{
	// set special flags
	switch(curVariantValue.type)
	{
	case DAVA::VariantType::TYPE_BOOLEAN:
		SetFlags(FLAG_IS_CHECKABLE | FLAG_IS_NOT_EDITABLE);
		break;
	case DAVA::VariantType::TYPE_KEYED_ARCHIVE:
	case DAVA::VariantType::TYPE_BYTE_ARRAY:
		SetFlags(FLAG_IS_DISABLED);
		break;

	case DAVA::VariantType::TYPE_MATRIX2:
	case DAVA::VariantType::TYPE_MATRIX3:
	case DAVA::VariantType::TYPE_MATRIX4:
	case DAVA::VariantType::TYPE_AABBOX3:
		SetFlags(FLAG_IS_NOT_EDITABLE);
		break;

	case DAVA::VariantType::TYPE_FLOAT:
	case DAVA::VariantType::TYPE_INT32:
	case DAVA::VariantType::TYPE_INT64:
	case DAVA::VariantType::TYPE_UINT32:
	case DAVA::VariantType::TYPE_UINT64:
	case DAVA::VariantType::TYPE_STRING:
	case DAVA::VariantType::TYPE_VECTOR2:
	case DAVA::VariantType::TYPE_VECTOR3:
	case DAVA::VariantType::TYPE_VECTOR4:
	case DAVA::VariantType::TYPE_COLOR:
	case DAVA::VariantType::TYPE_FASTNAME:
	case DAVA::VariantType::TYPE_FILEPATH:
	default:
		break;
	}
}

void QtPropertyDataDavaVariant::SetVariantValue(const DAVA::VariantType& value)
{
	DVASSERT(curVariantValue.type == DAVA::VariantType::TYPE_NONE || curVariantValue.type == value.type);

	bool needChildCreate = false;
	if(curVariantValue.type == DAVA::VariantType::TYPE_NONE)
	{
		needChildCreate = true;
	}

	curVariantValue = value;

	if(needChildCreate)
	{
		InitFlags();
		ChildsCreate();
	}
	else
	{
		ChildsSetFromMe();
	}
}

void QtPropertyDataDavaVariant::AddAllowedValue(const DAVA::VariantType& realValue, const QVariant& visibleValue /*= QVariant()*/)
{
	AllowedValue av;

	if(NULL == allowedButton)
	{
		allowedButton = new QtComboFake();
		allowedButton->setArrowType(Qt::DownArrow);
		allowedButton->setAutoRaise(true);

		AddOW(QtPropertyOW(allowedButton, true));
		QObject::connect(allowedButton, SIGNAL(pressed()), this, SLOT(AllowedOWPressed()));	
	}

	av.realValue = realValue;
	av.visibleValue = visibleValue;

	allowedValues.push_back(av);
}

void QtPropertyDataDavaVariant::ClearAllowedValues()
{
	allowedValues.clear();

	if(NULL != allowedButton)
	{
		RemOW(allowedButton);
		allowedButton = NULL;
	}
}

QVariant QtPropertyDataDavaVariant::GetValueInternal()
{
	return FromDavaVariant(curVariantValue);
}

QVariant QtPropertyDataDavaVariant::GetValueAlias()
{
	QVariant ret;

	if(allowedValues.size() > 0)
	{
		for (int i = 0; i < allowedValues.size(); ++i)
		{
			DAVA::VariantType v = DAVA::VariantType::Convert(allowedValues[i].realValue, curVariantValue.type);
			if(v == curVariantValue)
			{
				ret = allowedValues[i].visibleValue;
				break;
			}
		}

		if(!ret.isValid())
		{
			// if we have allowed value, but current value isn't in set
			// print this value as unknown
			// 
			QString s("Unknown - ");
			s += FromDavaVariant(curVariantValue).toString();

			ret = s;
		}
	}

	return ret;
}

void QtPropertyDataDavaVariant::SetValueInternal(const QVariant &value)
{
	switch(curVariantValue.type)
	{
	case DAVA::VariantType::TYPE_BOOLEAN:
		curVariantValue.SetBool(value.toBool());
		break;
	case DAVA::VariantType::TYPE_FLOAT:
		curVariantValue.SetFloat(value.toFloat());
		break;
	case DAVA::VariantType::TYPE_INT32:
		curVariantValue.SetInt32(value.toInt());
		break;
	case DAVA::VariantType::TYPE_INT64:
		curVariantValue.SetInt64(value.toLongLong());
		break;
	case DAVA::VariantType::TYPE_UINT32:
		curVariantValue.SetUInt32(value.toUInt());
		break;
	case DAVA::VariantType::TYPE_UINT64:
		curVariantValue.SetUInt64(value.toULongLong());
		break;
	case DAVA::VariantType::TYPE_KEYED_ARCHIVE:
		ToKeyedArchive(value);
		break;
	case DAVA::VariantType::TYPE_STRING:
		curVariantValue.SetString(value.toString().toStdString());
		break;
	case DAVA::VariantType::TYPE_MATRIX2:
		ToMatrix2(value);
		break;
	case DAVA::VariantType::TYPE_MATRIX3:
		ToMatrix3(value);
		break;
	case DAVA::VariantType::TYPE_MATRIX4:
		ToMatrix4(value);
		break;
	case DAVA::VariantType::TYPE_VECTOR2:
		ToVector2(value);
		break;
	case DAVA::VariantType::TYPE_VECTOR3:
		ToVector3(value);
		break;
	case DAVA::VariantType::TYPE_VECTOR4:
		ToVector4(value);
		break;
    case DAVA::VariantType::TYPE_COLOR:
        ToColor(value);
        break;
    case DAVA::VariantType::TYPE_FASTNAME:
        curVariantValue.SetFastName(DAVA::FastName(value.toString().toStdString().c_str()));
        break;
	case DAVA::VariantType::TYPE_AABBOX3:
		ToAABBox3(value);
		break;
	case DAVA::VariantType::TYPE_FILEPATH:
		curVariantValue.SetFilePath(value.toString().toStdString());
		break;

	case DAVA::VariantType::TYPE_BYTE_ARRAY:
	default:
		break;
	}
}

void QtPropertyDataDavaVariant::SubValueAdd(const QString &key, const QVariant &value)
{
	ChildAdd(key, new QtPropertyDataDavaVariantSubValue(this, value));
}

void QtPropertyDataDavaVariant::SubValueSet(const QString &key, const QVariant &value)
{
	QtPropertyDataDavaVariantSubValue *subValue = (QtPropertyDataDavaVariantSubValue *) ChildGet(key);
	if(NULL != subValue)
	{
		subValue->trackParent = false;
		subValue->SetValueInternal(value);
	}
}

void QtPropertyDataDavaVariant::ChildsCreate()
{
	switch(curVariantValue.type)
	{
	case DAVA::VariantType::TYPE_VECTOR2:
		{
			DAVA::Vector2 vec = curVariantValue.AsVector2();
			SubValueAdd("X", vec.x);
			SubValueAdd("Y", vec.y);
		}
		break;
	case DAVA::VariantType::TYPE_VECTOR3:
		{
			DAVA::Vector3 vec = curVariantValue.AsVector3();
			SubValueAdd("X", vec.x);
			SubValueAdd("Y", vec.y);
			SubValueAdd("Z", vec.z);
		}
		break;
	case DAVA::VariantType::TYPE_VECTOR4:
		{
			DAVA::Vector4 vec = curVariantValue.AsVector4();
			SubValueAdd("X", vec.x);
			SubValueAdd("Y", vec.y);
			SubValueAdd("Z", vec.z);
			SubValueAdd("W", vec.w);
		}
		break;
	case DAVA::VariantType::TYPE_COLOR:
		{
			QPushButton *colorBtn = new QPushButton(QIcon(":/QtIcons/color.png"), "");
			colorBtn->setIconSize(QSize(12, 12));
			colorBtn->setFlat(true);
			AddOW(QtPropertyOW(colorBtn));
			QObject::connect(colorBtn, SIGNAL(pressed()), this, SLOT(ColorOWPressed()));
		}
		break;
	case DAVA::VariantType::TYPE_AABBOX3:
		{
			DAVA::AABBox3 box = curVariantValue.AsAABBox3();

			SubValueAdd("min X", box.min.x);
			SubValueAdd("min Y", box.min.y);
			SubValueAdd("min Z", box.min.z);
			SubValueAdd("max X", box.max.x);
			SubValueAdd("max Y", box.max.y);
			SubValueAdd("max Z", box.max.z);
		}
		break;
	case DAVA::VariantType::TYPE_FILEPATH:
		{
			QPushButton *filePathBtn = new QPushButton(QIcon(":/QtIcons/openscene.png"), "");
			filePathBtn->setIconSize(QSize(14, 14));
			filePathBtn->setFlat(true);
			AddOW(QtPropertyOW(filePathBtn));
			QObject::connect(filePathBtn, SIGNAL(pressed()), this, SLOT(FilePathOWPressed()));
		}
		break;
	case DAVA::VariantType::TYPE_KEYED_ARCHIVE:
	case DAVA::VariantType::TYPE_MATRIX2:
	case DAVA::VariantType::TYPE_MATRIX3:
	case DAVA::VariantType::TYPE_MATRIX4:
	default:
		break;
	}
}

void QtPropertyDataDavaVariant::ChildsSetFromMe()
{
	switch(curVariantValue.type)
	{
	case DAVA::VariantType::TYPE_VECTOR2:
		{
			DAVA::Vector2 vec = curVariantValue.AsVector2();
			SubValueSet("X", vec.x);
			SubValueSet("Y", vec.y);
		}
		break;
	case DAVA::VariantType::TYPE_VECTOR3:
		{
			DAVA::Vector3 vec = curVariantValue.AsVector3();
			SubValueSet("X", vec.x);
			SubValueSet("Y", vec.y);
			SubValueSet("Z", vec.z);
		}
		break;
	case DAVA::VariantType::TYPE_VECTOR4:
		{
			DAVA::Vector4 vec = curVariantValue.AsVector4();
			SubValueSet("X", vec.x);
			SubValueSet("Y", vec.y);
			SubValueSet("Z", vec.z);
			SubValueSet("W", vec.w);
		}
		break;
	case DAVA::VariantType::TYPE_AABBOX3:
        {
            DAVA::AABBox3 box = curVariantValue.AsAABBox3();
			SubValueSet("min X", box.min.x);
			SubValueSet("min Y", box.min.y);
			SubValueSet("min Z", box.min.z);
			SubValueSet("max X", box.max.x);
			SubValueSet("max Y", box.max.y);
			SubValueSet("max Z", box.max.z);
        }
        break;
	case DAVA::VariantType::TYPE_KEYED_ARCHIVE:
	case DAVA::VariantType::TYPE_MATRIX2:
	case DAVA::VariantType::TYPE_MATRIX3:
	case DAVA::VariantType::TYPE_MATRIX4:
	default:
		break;
	}
}

void QtPropertyDataDavaVariant::MeSetFromChilds()
{
	switch(curVariantValue.type)
	{
	case DAVA::VariantType::TYPE_VECTOR2:
		{
			DAVA::Vector2 vec;
			vec.x = ChildGet("X")->GetValue().toFloat();
			vec.y = ChildGet("Y")->GetValue().toFloat();

			if(curVariantValue.AsVector2() != vec)
			{
				curVariantValue.SetVector2(vec);
				SetValue(FromVector2(vec), QtPropertyData::VALUE_EDITED);
			}
		}
		break;
	case DAVA::VariantType::TYPE_VECTOR3:
		{
			DAVA::Vector3 vec;
			vec.x = ChildGet("X")->GetValue().toFloat();
			vec.y = ChildGet("Y")->GetValue().toFloat();
			vec.z = ChildGet("Z")->GetValue().toFloat();

			if(curVariantValue.AsVector3() != vec)
			{
				curVariantValue.SetVector3(vec);
				SetValue(FromVector3(vec), QtPropertyData::VALUE_EDITED);
			}
		}
		break;
	case DAVA::VariantType::TYPE_VECTOR4:
		{
			DAVA::Vector4 vec;
			vec.x = ChildGet("X")->GetValue().toFloat();
			vec.y = ChildGet("Y")->GetValue().toFloat();
			vec.z = ChildGet("Z")->GetValue().toFloat();
			vec.w = ChildGet("W")->GetValue().toFloat();

			if(curVariantValue.AsVector4() != vec)
			{
				curVariantValue.SetVector4(vec);
				SetValue(FromVector4(vec), QtPropertyData::VALUE_EDITED);
			}
		}
		break;
    case DAVA::VariantType::TYPE_AABBOX3:
        {
            DAVA::AABBox3 box;
            
            box.min.x = ChildGet("min X")->GetValue().toFloat();
            box.min.y = ChildGet("min Y")->GetValue().toFloat();
            box.min.z = ChildGet("min Z")->GetValue().toFloat();
            box.max.x = ChildGet("max X")->GetValue().toFloat();
            box.max.y = ChildGet("max Y")->GetValue().toFloat();
            box.max.z = ChildGet("max Z")->GetValue().toFloat();
            
			if(curVariantValue.AsAABBox3().min != box.min || curVariantValue.AsAABBox3().max != box.max)
			{
				curVariantValue.SetAABBox3(box);
				SetValue(FromAABBox3(box), QtPropertyData::VALUE_EDITED);
			}
        }
        break;
	case DAVA::VariantType::TYPE_KEYED_ARCHIVE:
	case DAVA::VariantType::TYPE_MATRIX2:
	case DAVA::VariantType::TYPE_MATRIX3:
	case DAVA::VariantType::TYPE_MATRIX4:
	default:
		break;
	}
}

QVariant QtPropertyDataDavaVariant::FromDavaVariant(const DAVA::VariantType &variant)
{
	QVariant v;

	switch(variant.type)
	{
	case DAVA::VariantType::TYPE_BOOLEAN:
		v = variant.AsBool();
		break;
	case DAVA::VariantType::TYPE_FLOAT:
		v = variant.AsFloat();
		break;
	case DAVA::VariantType::TYPE_INT32:
		v = variant.AsInt32();
		break;
	case DAVA::VariantType::TYPE_INT64:
		v = variant.AsInt64();
		break;
	case DAVA::VariantType::TYPE_UINT32:
		v = variant.AsUInt32();
		break;
	case DAVA::VariantType::TYPE_UINT64:
		v = variant.AsUInt64();
		break;
	case DAVA::VariantType::TYPE_KEYED_ARCHIVE:
		v = FromKeyedArchive(variant.AsKeyedArchive());
		break;
	case DAVA::VariantType::TYPE_STRING:
		v = variant.AsString().c_str();
		break;
	case DAVA::VariantType::TYPE_MATRIX2:
		v = FromMatrix2(variant.AsMatrix2());
		break;
	case DAVA::VariantType::TYPE_MATRIX3:
		v = FromMatrix3(variant.AsMatrix3());
		break;
	case DAVA::VariantType::TYPE_MATRIX4:
		v = FromMatrix4(variant.AsMatrix4());
		break;
	case DAVA::VariantType::TYPE_VECTOR2:
		v = FromVector2(variant.AsVector2());
		break;
	case DAVA::VariantType::TYPE_VECTOR3:
		v = FromVector3(variant.AsVector3());
		break;
	case DAVA::VariantType::TYPE_VECTOR4:
		v = FromVector4(variant.AsVector4());
		break;
	case DAVA::VariantType::TYPE_COLOR:
		v = FromColor(variant.AsColor());
		break;
	case DAVA::VariantType::TYPE_FASTNAME:
		v = QString(variant.AsFastName().c_str());
		break;
	case DAVA::VariantType::TYPE_AABBOX3:
		v = FromAABBox3(variant.AsAABBox3());
		break;
	case DAVA::VariantType::TYPE_FILEPATH:
		v = variant.AsFilePath().GetAbsolutePathname().c_str();
		break;

	case DAVA::VariantType::TYPE_BYTE_ARRAY:
	default:
		break;
	}

	return v;
}

QVariant QtPropertyDataDavaVariant::FromKeyedArchive(DAVA::KeyedArchive *archive)
{
	QVariant v;

	if(NULL != archive)
	{
		v = QString("KeyedArchive");
	}

	return v;
}

QVariant QtPropertyDataDavaVariant::FromVector4(const DAVA::Vector4 &vector)
{
	QVariant v;

	v = QString().sprintf("[%8.2f, %8.2f, %8.2f, %8.2f]", vector.x, vector.y, vector.z, vector.w);

	return v;
}

QVariant QtPropertyDataDavaVariant::FromVector3(const DAVA::Vector3 &vector)
{
	QVariant v;

	v = QString().sprintf("[%8.2f, %8.2f, %8.2f]", vector.x, vector.y, vector.z);

	return v;
}

QVariant QtPropertyDataDavaVariant::FromVector2(const DAVA::Vector2 &vector)
{
	QVariant v;

	v = QString().sprintf("[%8.2f, %8.2f]", vector.x, vector.y);

	return v;
}

QVariant QtPropertyDataDavaVariant::FromMatrix4(const DAVA::Matrix4 &matrix)
{
	QVariant v;

	v = QString().sprintf("[%8.2f, %8.2f, %8.2f, %8.2f]\n[%8.2f, %8.2f, %8.2f, %8.2f]\n[%8.2f, %8.2f, %8.2f, %8.2f]\n[%8.2f, %8.2f, %8.2f, %8.2f]",
                          matrix._00, matrix._10, matrix._20, matrix._30,
                          matrix._01, matrix._11, matrix._21, matrix._31,
                          matrix._02, matrix._12, matrix._22, matrix._32,
                          matrix._03, matrix._13, matrix._23, matrix._33
                          );

	return v;
}

QVariant QtPropertyDataDavaVariant::FromMatrix3(const DAVA::Matrix3 &matrix)
{
	QVariant v;

	v = QString().sprintf("[%8.2f, %8.2f, %8.2f]\n[%8.2f, %8.2f, %8.2f]\n[%8.2f, %8.2f, %8.2f]",
                          matrix._00, matrix._10, matrix._20,
                          matrix._01, matrix._11, matrix._21,
                          matrix._02, matrix._12, matrix._22
                          );

	return v;
}

QVariant QtPropertyDataDavaVariant::FromMatrix2(const DAVA::Matrix2 &matrix)
{
	QVariant v;

	v = QString().sprintf("([%8.2f, %8.2f]\n[%8.2f, %8.2f])",
		matrix._00, matrix._01,
		matrix._10, matrix._11);

	return v;
}

QVariant QtPropertyDataDavaVariant::FromColor(const DAVA::Color &color)
{
	QVariant v;
	QColor c = ColorToQColor(color);

	//v = QString().sprintf("#%02x%02x%02x%02x", c.red(), c.green(), c.blue(), c.alpha());

	v.setValue(c);
	return v;
}

QVariant QtPropertyDataDavaVariant::FromAABBox3(const DAVA::AABBox3 &aabbox)
{
	QVariant v;

	v = QString().sprintf("[%8.2f, %8.2f, %8.2f]\n[%8.2f, %8.2f, %8.2f]",
		aabbox.min.x, aabbox.min.y, aabbox.min.z,
		aabbox.max.x, aabbox.max.y, aabbox.max.z
		);

	return v;
}

void QtPropertyDataDavaVariant::ToKeyedArchive(const QVariant &value)
{
	// No way to set whole archive
}

void QtPropertyDataDavaVariant::ToVector4(const QVariant &value)
{
	// TODO:
	// ...
}

void QtPropertyDataDavaVariant::ToVector3(const QVariant &value)
{
	// TODO:
	// ...
}

void QtPropertyDataDavaVariant::ToVector2(const QVariant &value)
{
	// TODO:
	// ...
}

void QtPropertyDataDavaVariant::ToMatrix4(const QVariant &value)
{
	// TODO:
	// ...
}

void QtPropertyDataDavaVariant::ToMatrix3(const QVariant &value)
{
	// TODO:
	// ...
}

void QtPropertyDataDavaVariant::ToMatrix2(const QVariant &value)
{
	// TODO:
	// ...
}

void QtPropertyDataDavaVariant::ToColor(const QVariant &value)
{
	curVariantValue.SetColor(QColorToColor(value.value<QColor>()));
}

void QtPropertyDataDavaVariant::ToAABBox3(const QVariant &value)
{
	// TODO:
	// ...
}

QWidget* QtPropertyDataDavaVariant::CreateEditorInternal(QWidget *parent, const QStyleOptionViewItem& option)
{
	QWidget* ret = NULL;

	// user shouldn't add allowedValues during edit process
	allowedValuesLocked = true;

	// if there is allowedValues we should show them in combobox
	// user will only be able to select values from combobox
	if(allowedValues.size() > 0)
	{
		ret = CreateAllowedValuesEditor(parent);
	}
	// check types and create our own widgets for edit
	// if we don't create - Qt will create standard editing widget for QVariant
	else
	{
		if(curVariantValue.type == DAVA::VariantType::TYPE_COLOR)
		{
			ret = new QtColorLineEdit(parent);
		}
	}
    
    return ret;
}

bool QtPropertyDataDavaVariant::SetEditorDataInternal(QWidget *editor)
{
	bool ret = false;

	// if we have valueItems, so that means combobox was created
	if(allowedValues.size())
	{
		SetAllowedValueEditorData(editor);
	}
	else
	{
		if(curVariantValue.type == DAVA::VariantType::TYPE_COLOR)
		{
			QtColorLineEdit *colorLineEdit = (QtColorLineEdit *) editor;

			colorLineEdit->SetColor(ColorToQColor(curVariantValue.AsColor()));
			ret = true;
		}
	}

	return ret;
}

bool QtPropertyDataDavaVariant::EditorDoneInternal(QWidget *editor)
{
	bool ret = false;

	// if we have allowedValues - that means combobox was created
	if(allowedValues.size())
	{
		ApplyAllowedValueFromEditor(editor);
		ret = true;
	}
	else
	{
		if(curVariantValue.type == DAVA::VariantType::TYPE_COLOR)
		{
			QtColorLineEdit *colorLineEdit = (QtColorLineEdit *) editor;
			SetValue(colorLineEdit->GetColor(), QtPropertyData::VALUE_EDITED);
			ret = true;
		}
	}

	// reset icon cache. icon will be recreated on next icon request
	iconCacheIsValid = false;

	// allow modify valueItems list
	allowedValuesLocked = false;

	return ret;
}

void QtPropertyDataDavaVariant::ColorOWPressed()
{
	QColor c = QColorDialog::getColor(ColorToQColor(curVariantValue.AsColor()), NULL, "Select color", QColorDialog::ShowAlphaChannel);
	if(c.isValid())
	{
		SetValue(c, QtPropertyData::VALUE_EDITED);
		iconCacheIsValid = false;
	}
}

void QtPropertyDataDavaVariant::FilePathOWPressed()
{
	QString path = QtFileDialog::getOpenFileName(GetOWViewport(), "Select file", QString(curVariantValue.AsFilePath().GetAbsolutePathname().c_str()));
	if(!path.isEmpty())
	{
		SetValue(path, QtPropertyData::VALUE_EDITED);
	}
}

void QtPropertyDataDavaVariant::AllowedOWPressed()
{

}

void QtPropertyDataDavaVariant::AllowedSelected(int index)
{
	QComboBox *allowedWidget = dynamic_cast<QComboBox *>(QObject::sender());
	if(NULL != allowedWidget)
	{
		QCoreApplication::postEvent(allowedWidget, new QKeyEvent(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier));
	}
}

QIcon QtPropertyDataDavaVariant::GetIcon()
{
	if(!iconCacheIsValid)
	{
		iconCacheIsValid = true;

		if(curVariantValue.type == DAVA::VariantType::TYPE_COLOR)
		{
			QPixmap pix(16,16);
			QPainter p(&pix);
			QColor c = ColorToQColor(curVariantValue.AsColor());

			if(c.alpha() < 255)
			{
				p.setBrush(QColor(250, 250, 250));
				p.drawRect(QRect(0, 0, 15, 15));
				p.setPen(QColor(200, 200, 200));
				p.setBrush(QColor(150, 150, 150));
				p.drawRect(QRect(0, 0, 7, 7));
				p.drawRect(QRect(8, 8, 15, 15));
			}

			p.setPen(QColor(0, 0, 0));
			p.setBrush(QBrush(c));
			p.drawRect(QRect(0,0,15,15));

			iconCache = QIcon(pix);
		}
		else if(curVariantValue.type == DAVA::VariantType::TYPE_FILEPATH)
		{
			iconCache = QIcon(":/QtIcons/file.png");
		}
		else
		{
			iconCache = QtPropertyData::GetIcon();
		}
	}

	return iconCache;
}

void QtPropertyDataDavaVariant::SetIcon(const QIcon &icon)
{
	QtPropertyData::SetIcon(icon);
}

QWidget* QtPropertyDataDavaVariant::CreateAllowedValuesEditor(QWidget *parent)
{
	QComboBox *allowedWidget = NULL;

	if(allowedValues.size() > 0)
	{
		allowedWidget = new QComboBox(parent);

		for(int i = 0; i < allowedValues.size(); ++i)
		{
			QString text;

			// if we have valid representation for "visible" items,
			// we should add it into combobox
			if(allowedValues[i].visibleValue.isValid())
			{
				text = allowedValues[i].visibleValue.toString();
			}
			// if not - we will create it from dava::varianttype
			else
			{
				text = FromDavaVariant(curVariantValue).toString();
			}

			allowedWidget->addItem(text);
		}

		QObject::connect(allowedWidget, SIGNAL(activated(int)), this, SLOT(AllowedSelected(int)));
	}

	return allowedWidget;
}

void QtPropertyDataDavaVariant::SetAllowedValueEditorData(QWidget *editorWidget)
{
	QComboBox *allowedWidget = dynamic_cast<QComboBox*>(editorWidget);

	if(NULL != allowedWidget)
	{
		int index = -1;

		// we should set combobox current index,
		// that matches current value
		for(int i = 0; i < allowedValues.size(); ++i)
		{
			DAVA::VariantType v = DAVA::VariantType::Convert(allowedValues[i].realValue, curVariantValue.type);
			if(v == curVariantValue)
			{
				index = i;
				break;
			}
		}

		allowedWidget->setCurrentIndex(index);
		allowedWidget->showPopup();
	}
}

void QtPropertyDataDavaVariant::ApplyAllowedValueFromEditor(QWidget *editorWidget)
{
	QComboBox *allowedWidget = dynamic_cast<QComboBox*>(editorWidget);

	if(NULL != allowedWidget)
	{
		int index = allowedWidget->currentIndex();
		if(index >= 0 && index < allowedValues.size())
		{
			DAVA::VariantType v = DAVA::VariantType::Convert(allowedValues[index].realValue, curVariantValue.type);
			if(curVariantValue != v)
			{
				SetValue(FromDavaVariant(allowedValues[index].realValue), QtPropertyData::VALUE_EDITED);
			}
		}
	}
}
