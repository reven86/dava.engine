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
#include "Tools/QtPropertyEditor/QtPropertyWidgets/FlagSelectorCombo.h"
#include "Tools/ColorPicker/ColorPicker.h"
#include "Tools/Widgets/MultilineEditor.h"

#include "QtTools/FileDialog/FileDialog.h"


#include <QListWidget>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QToolButton>
#include <QPainter>
#include <QLineEdit>
#include <QCoreApplication>
#include <QKeyEvent>

#define FLOAT_PRINTF_FORMAT1 "% .7f"
#define FLOAT_PRINTF_FORMAT2 "% .7f; % .7f"
#define FLOAT_PRINTF_FORMAT3 "% .7f; % .7f; % .7f"
#define FLOAT_PRINTF_FORMAT4 "% .7f; % .7f; % .7f; % .7f"

QtPropertyDataDavaVariant::QtPropertyDataDavaVariant(const DAVA::VariantType &value)
	: curVariantValue(value)
	, allowedValuesLocked(false)
	, allowedButton(NULL)
	, allowedValueType(Default)
	, isSettingMeFromChilds(false)
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
		SetCheckable(true);
		break;
	case DAVA::VariantType::TYPE_KEYED_ARCHIVE:
	case DAVA::VariantType::TYPE_BYTE_ARRAY:
		SetEnabled(false);
		break;

	case DAVA::VariantType::TYPE_MATRIX2:
	case DAVA::VariantType::TYPE_MATRIX3:
	case DAVA::VariantType::TYPE_MATRIX4:
	case DAVA::VariantType::TYPE_AABBOX3:
		SetEditable(false);
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
	case DAVA::VariantType::TYPE_FASTNAME:
    case DAVA::VariantType::TYPE_FILEPATH:
		break;

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

	// more specific actions
	switch(curVariantValue.type)
	{
		case DAVA::VariantType::TYPE_COLOR:
			UpdateColorButtonIcon();
			break;

		default:
			break;
	}
}

void QtPropertyDataDavaVariant::AddAllowedValue(const DAVA::VariantType& realValue, const QVariant& visibleValue /*= QVariant()*/)
{
	AllowedValue av;

	if(NULL == allowedButton)
	{
		allowedButton = AddButton(QtPropertyToolButton::ACTIVE_WHEN_ITEM_IS_EDITABLE_AND_ENABLED);
		allowedButton->setArrowType(Qt::DownArrow);
		allowedButton->setAutoRaise(true);
		//allowedButton->setEnabled(false);
		allowedButton->eventsPassThrought = true;
		allowedButton->overlayed = true;

		QObject::connect(allowedButton, &QtPropertyToolButton::released, this, &QtPropertyDataDavaVariant::AllowedOWPressed);
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
		RemButton(allowedButton);
		allowedButton = NULL;
	}
}

void QtPropertyDataDavaVariant::SetAllowedValueType(AllowedValueType type)
{
    allowedValueType = type;
}

QtPropertyDataDavaVariant::AllowedValueType QtPropertyDataDavaVariant::GetAllowedValueType( ) const
{
    return allowedValueType;
}

void QtPropertyDataDavaVariant::SetInspDescription(const DAVA::InspDesc &desc)
{
    ClearAllowedValues();

	if(NULL != desc.enumMap)
	{
		for(size_t i = 0; i < desc.enumMap->GetCount(); ++i)
		{
			int v;
			if(desc.enumMap->GetValue(i, v))
			{
				AddAllowedValue(DAVA::VariantType(v), desc.enumMap->ToString(v));
			}
		}
	}

    const bool isFlags = (desc.type == DAVA::InspDesc::T_FLAGS);
    if(isFlags)
    {
        SetAllowedValueType(QtPropertyDataDavaVariant::TypeFlags);
    }
}

QStringList QtPropertyDataDavaVariant::GetFlagsList() const
{
    QStringList values;

    switch (allowedValueType)
    {
    case TypeFlags:
        {
            const int flags = FromDavaVariant(curVariantValue).toInt();
            for (int i = 0; i < allowedValues.size(); ++i)
            {
                const int flag = FromDavaVariant(allowedValues[i].realValue).toInt();
                if ((flag & flags) == flag)
                {
                    const QString visible = allowedValues[i].visibleValue.toString( );
                    const QString real = QString::number(FromDavaVariant(allowedValues[i].realValue).toInt());
                    values << (allowedValues[i].visibleValue.isValid()?visible : real);
                }
            }
        }
        break;
    default:
        break;
    }

    return values;
}

QVariant QtPropertyDataDavaVariant::GetToolTip() const
{
    QVariant ret;

    if (allowedValues.size() > 0)
    {
        switch (allowedValueType)
        {
        case TypeFlags:
            {
                const int flags = FromDavaVariant( curVariantValue ).toInt();
                QStringList values;
                values << QString("Flags: %1").arg(flags);
                values << GetFlagsList();
                ret = values.join( "\n" );
            }
            break;
        default:
            break;
        } // end switch
    }
    if (!ret.isValid())
    {
        ret = GetValueAlias();
    }
    if (!ret.isValid())
    {
        ret = GetValue();
    }

    return ret;
}

QVariant QtPropertyDataDavaVariant::GetValueInternal() const
{
	return FromDavaVariant(curVariantValue);
}

QVariant QtPropertyDataDavaVariant::GetValueAlias() const
{
	QVariant ret;

	if(allowedValues.size() > 0)
	{
        if (allowedValueType == TypeFlags)
        {
            const QString alias = GetFlagsList().join(" | ");
            ret = alias;
        }
        else
        {
		    for (int i = 0; i < allowedValues.size(); ++i)
		    {
			    DAVA::VariantType v = DAVA::VariantType::Convert(allowedValues[i].realValue, curVariantValue.type);
			    if(v == curVariantValue)
			    {
                    if(allowedValues[i].visibleValue.isValid())
                    {
                        ret = allowedValues[i].visibleValue;
                    }
                    else
                    {
                        ret = FromDavaVariant(allowedValues[i].realValue);
                    }
				    break;
			    }
		    }

		    if(!ret.isValid())
		    {
			    // if we have allowed value, but current value isn't in set
			    // print this value as unknown
			    const QString s = QString("Unknown - %1").arg(FromDavaVariant(curVariantValue).toString());
			    ret = s;
		    }
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
		ToFloat(value);
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
        if(value.isValid())
            curVariantValue.SetFastName(DAVA::FastName(value.toString().toStdString().c_str()));
        else
            curVariantValue.SetFastName(DAVA::FastName());
        break;
	case DAVA::VariantType::TYPE_AABBOX3:
		ToAABBox3(value);
		break;
	case DAVA::VariantType::TYPE_FILEPATH:
		curVariantValue.SetFilePath(value.toString().toStdString());
		break;
	case DAVA::VariantType::TYPE_BYTE_ARRAY:
		break;
	default:
		break;
	}

    ChildsSetFromMe();
}

void QtPropertyDataDavaVariant::SubValueAdd(const QString &key, const DAVA::VariantType &subvalue)
{
	ChildAdd(key, new QtPropertyDataDavaVariantSubValue(this, subvalue));
}

void QtPropertyDataDavaVariant::SubValueSetToMe(const QString &key, const QVariant &subvalue)
{
	QtPropertyDataDavaVariantSubValue *child = (QtPropertyDataDavaVariantSubValue *) ChildGet(key);
	if(NULL != child)
	{
		child->trackParent = false;
		child->SetValue(subvalue);
	}
}

void QtPropertyDataDavaVariant::SubValueSetFromMe(const QString &key, const QVariant &subvalue)
{
	QtPropertyDataDavaVariantSubValue *child = (QtPropertyDataDavaVariantSubValue *) ChildGet(key);
	if(NULL != child)
	{
		child->trackParent = false;
		child->SetValueInternal(subvalue);
	}
}

QVariant QtPropertyDataDavaVariant::SubValueGet(const QString &key)
{
    QVariant ret;

    QtPropertyDataDavaVariantSubValue *child = (QtPropertyDataDavaVariantSubValue *) ChildGet(key);
    if(NULL != child)
    {
        ret = child->GetValueInternal();
    }

    return ret;
}

void QtPropertyDataDavaVariant::ChildsCreate()
{
	switch(curVariantValue.type)
	{
    case DAVA::VariantType::TYPE_VECTOR2:
		{
			DAVA::Vector2 vec = curVariantValue.AsVector2();
			SubValueAdd("X", DAVA::VariantType(vec.x));
			SubValueAdd("Y", DAVA::VariantType(vec.y));
		}
		break;
	case DAVA::VariantType::TYPE_VECTOR3:
		{
			DAVA::Vector3 vec = curVariantValue.AsVector3();
			SubValueAdd("X", DAVA::VariantType(vec.x));
			SubValueAdd("Y", DAVA::VariantType(vec.y));
			SubValueAdd("Z", DAVA::VariantType(vec.z));
		}
		break;
	case DAVA::VariantType::TYPE_VECTOR4:
		{
			DAVA::Vector4 vec = curVariantValue.AsVector4();
			SubValueAdd("X", DAVA::VariantType(vec.x));
			SubValueAdd("Y", DAVA::VariantType(vec.y));
			SubValueAdd("Z", DAVA::VariantType(vec.z));
			SubValueAdd("W", DAVA::VariantType(vec.w));
		}
		break;
	case DAVA::VariantType::TYPE_COLOR:
		{
			QToolButton *colorBtn = AddButton(QtPropertyToolButton::ACTIVE_WHEN_ITEM_IS_EDITABLE_AND_ENABLED);
			colorBtn->setIcon(QIcon(":/QtIcons/color.png"));
			colorBtn->setIconSize(QSize(12, 12));
			colorBtn->setAutoRaise(true);
            colorBtn->setObjectName("colorButton");
            QObject::connect(colorBtn, &QToolButton::clicked, this, &QtPropertyDataDavaVariant::ColorOWPressed);

            DAVA::Color color = curVariantValue.AsColor();
            SubValueAdd("R", DAVA::VariantType(color.r));
            SubValueAdd("G", DAVA::VariantType(color.g));
            SubValueAdd("B", DAVA::VariantType(color.b));
            SubValueAdd("A", DAVA::VariantType(color.a));
        }
		break;
	case DAVA::VariantType::TYPE_AABBOX3:
		{
			DAVA::AABBox3 box = curVariantValue.AsAABBox3();

			SubValueAdd("min X", DAVA::VariantType(box.min.x));
			SubValueAdd("min Y", DAVA::VariantType(box.min.y));
			SubValueAdd("min Z", DAVA::VariantType(box.min.z));
			SubValueAdd("max X", DAVA::VariantType(box.max.x));
			SubValueAdd("max Y", DAVA::VariantType(box.max.y));
			SubValueAdd("max Z", DAVA::VariantType(box.max.z));
		}
		break;
	case DAVA::VariantType::TYPE_FILEPATH:
		{
			QToolButton *filePathBtn = AddButton(QtPropertyToolButton::ACTIVE_WHEN_ITEM_IS_EDITABLE_AND_ENABLED);
			filePathBtn->setIcon(QIcon(":/QtIcons/openscene.png"));
			filePathBtn->setIconSize(QSize(14, 14));
			filePathBtn->setAutoRaise(true);
			connect(filePathBtn, &QToolButton::clicked, this, &QtPropertyDataDavaVariant::FilePathOWPressed);
		}
		break;
    case DAVA::VariantType::TYPE_STRING:
        {
            QToolButton *editMultiline = AddButton( QtPropertyToolButton::ACTIVE_WHEN_ITEM_IS_EDITABLE_AND_ENABLED );
            editMultiline->setIcon( QIcon( ":/QtIcons/pencil.png" ) );
            editMultiline->setIconSize( QSize( 14, 14 ) );
            editMultiline->setAutoRaise( true );
            editMultiline->setToolTip( "Open multiline editor" );
            connect( editMultiline, &QToolButton::clicked, this, &QtPropertyDataDavaVariant::MultilineEditClicked );
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
    if(!isSettingMeFromChilds)
    {
	    switch(curVariantValue.type)
	    {
	    case DAVA::VariantType::TYPE_VECTOR2:
		    {
			    DAVA::Vector2 vec = curVariantValue.AsVector2();
			    SubValueSetFromMe("X", vec.x);
			    SubValueSetFromMe("Y", vec.y);
		    }
		    break;
	    case DAVA::VariantType::TYPE_VECTOR3:
		    {
			    DAVA::Vector3 vec = curVariantValue.AsVector3();
			    SubValueSetFromMe("X", vec.x);
			    SubValueSetFromMe("Y", vec.y);
			    SubValueSetFromMe("Z", vec.z);
		    }
		    break;
	    case DAVA::VariantType::TYPE_VECTOR4:
		    {
			    DAVA::Vector4 vec = curVariantValue.AsVector4();
			    SubValueSetFromMe("X", vec.x);
			    SubValueSetFromMe("Y", vec.y);
			    SubValueSetFromMe("Z", vec.z);
			    SubValueSetFromMe("W", vec.w);
		    }
		    break;
	    case DAVA::VariantType::TYPE_AABBOX3:
            {
                DAVA::AABBox3 box = curVariantValue.AsAABBox3();
			    SubValueSetFromMe("min X", box.min.x);
			    SubValueSetFromMe("min Y", box.min.y);
			    SubValueSetFromMe("min Z", box.min.z);
			    SubValueSetFromMe("max X", box.max.x);
			    SubValueSetFromMe("max Y", box.max.y);
			    SubValueSetFromMe("max Z", box.max.z);
            }
            break;
        case DAVA::VariantType::TYPE_COLOR:
            {
                DAVA::Color color = curVariantValue.AsColor();
                SubValueSetFromMe("R", color.r);
                SubValueSetFromMe("G", color.g);
                SubValueSetFromMe("B", color.b);
                SubValueSetFromMe("A", color.a);
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
}

void QtPropertyDataDavaVariant::MeSetFromChilds()
{
    isSettingMeFromChilds = true;

	switch(curVariantValue.type)
	{
	case DAVA::VariantType::TYPE_VECTOR2:
		{
			DAVA::Vector2 vec;
			vec.x = SubValueGet("X").toFloat();
			vec.y = SubValueGet("Y").toFloat();

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
			vec.x = SubValueGet("X").toFloat();
			vec.y = SubValueGet("Y").toFloat();
			vec.z = SubValueGet("Z").toFloat();

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
			vec.x = SubValueGet("X").toFloat();
			vec.y = SubValueGet("Y").toFloat();
			vec.z = SubValueGet("Z").toFloat();
			vec.w = SubValueGet("W").toFloat();

			if(curVariantValue.AsVector4() != vec)
			{
				curVariantValue.SetVector4(vec);
				SetValue(FromVector4(vec), QtPropertyData::VALUE_EDITED);
			}
		}
		break;
    case DAVA::VariantType::TYPE_COLOR:
        {
            DAVA::Color color;
            color.r = SubValueGet("R").toFloat();
            color.g = SubValueGet("G").toFloat();
            color.b = SubValueGet("B").toFloat();
            color.a = SubValueGet("A").toFloat();

            if(curVariantValue.AsColor() != color)
            {
                curVariantValue.SetColor(color);
                SetValue(FromColor(color), QtPropertyData::VALUE_EDITED);
                UpdateColorButtonIcon();
            }
        }
        break;
    case DAVA::VariantType::TYPE_AABBOX3:
        {
            DAVA::AABBox3 box;
            
            box.min.x = SubValueGet("min X").toFloat();
            box.min.y = SubValueGet("min Y").toFloat();
            box.min.z = SubValueGet("min Z").toFloat();
            box.max.x = SubValueGet("max X").toFloat();
            box.max.y = SubValueGet("max Y").toFloat();
            box.max.z = SubValueGet("max Z").toFloat();
            
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

    isSettingMeFromChilds = false;
}

QVariant QtPropertyDataDavaVariant::FromDavaVariant(const DAVA::VariantType &variant) const
{
	QVariant v;

	switch(variant.type)
	{
	case DAVA::VariantType::TYPE_BOOLEAN:
		v = variant.AsBool();
		break;
	case DAVA::VariantType::TYPE_FLOAT:
		v = FromFloat(variant.AsFloat());
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
        if(variant.AsFastName().IsValid())
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

QVariant QtPropertyDataDavaVariant::FromKeyedArchive(DAVA::KeyedArchive *archive) const
{
	QVariant v;

	if(NULL != archive)
	{
		v = QString("KeyedArchive");
	}

	return v;
}

QVariant QtPropertyDataDavaVariant::FromFloat(DAVA::float32 value) const
{
    QVariant v;

    v = QString().sprintf(FLOAT_PRINTF_FORMAT1, value);

    return v;
}

QVariant QtPropertyDataDavaVariant::FromVector4(const DAVA::Vector4 &vector) const
{
	QVariant v;

	v = QString().sprintf("[" FLOAT_PRINTF_FORMAT4 "]", vector.x, vector.y, vector.z, vector.w);

	return v;
}

QVariant QtPropertyDataDavaVariant::FromVector3(const DAVA::Vector3 &vector) const
{
	QVariant v;

	v = QString().sprintf("[" FLOAT_PRINTF_FORMAT3 "]", vector.x, vector.y, vector.z);

	return v;
}

QVariant QtPropertyDataDavaVariant::FromVector2(const DAVA::Vector2 &vector) const
{
	QVariant v;

	v = QString().sprintf("[" FLOAT_PRINTF_FORMAT2 "]", vector.x, vector.y);

	return v;
}

QVariant QtPropertyDataDavaVariant::FromMatrix4(const DAVA::Matrix4 &matrix) const
{
	QVariant v;

	v = QString().sprintf("[" FLOAT_PRINTF_FORMAT4 "]\n[" FLOAT_PRINTF_FORMAT4 "]\n[" FLOAT_PRINTF_FORMAT4 "]\n[" FLOAT_PRINTF_FORMAT4 "]",
                          matrix._00, matrix._10, matrix._20, matrix._30,
                          matrix._01, matrix._11, matrix._21, matrix._31,
                          matrix._02, matrix._12, matrix._22, matrix._32,
                          matrix._03, matrix._13, matrix._23, matrix._33
                          );

	return v;
}

QVariant QtPropertyDataDavaVariant::FromMatrix3(const DAVA::Matrix3 &matrix) const
{
	QVariant v;

	v = QString().sprintf("[" FLOAT_PRINTF_FORMAT3 "]\n[" FLOAT_PRINTF_FORMAT3 "]\n[" FLOAT_PRINTF_FORMAT3 "]",
                          matrix._00, matrix._10, matrix._20,
                          matrix._01, matrix._11, matrix._21,
                          matrix._02, matrix._12, matrix._22
                          );

	return v;
}

QVariant QtPropertyDataDavaVariant::FromMatrix2(const DAVA::Matrix2 &matrix) const
{
	QVariant v;

	v = QString().sprintf("([" FLOAT_PRINTF_FORMAT2 "]\n[" FLOAT_PRINTF_FORMAT2 "])",
		matrix._00, matrix._01,
		matrix._10, matrix._11);

	return v;
}

QVariant QtPropertyDataDavaVariant::FromColor(const DAVA::Color &color) const
{
	QVariant v;

	v = QString().sprintf("[" FLOAT_PRINTF_FORMAT4 "]", color.r, color.g, color.b, color.a);

	return v;
}

QVariant QtPropertyDataDavaVariant::FromAABBox3(const DAVA::AABBox3 &aabbox) const
{
	QVariant v;

	v = QString().sprintf("[" FLOAT_PRINTF_FORMAT3 "]\n[" FLOAT_PRINTF_FORMAT3 "]",
		aabbox.min.x, aabbox.min.y, aabbox.min.z,
		aabbox.max.x, aabbox.max.y, aabbox.max.z
		);

	return v;
}

void QtPropertyDataDavaVariant::ToKeyedArchive(const QVariant &value)
{
	// No way to set whole archive
}

void QtPropertyDataDavaVariant::ToFloat(const QVariant &value)
{
    curVariantValue.SetFloat(value.toFloat());
}

void QtPropertyDataDavaVariant::ToVector4(const QVariant &value)
{
	DAVA::Vector4 v;
	QString str = value.toString();

	if(4 == ParseFloatList(str, 4, v.data))
	{
		curVariantValue.SetVector4(v);
	}
}

void QtPropertyDataDavaVariant::ToVector3(const QVariant &value)
{
	DAVA::Vector3 v;
	QString str = value.toString();

	if(3 == ParseFloatList(str, 3, v.data))
	{
		curVariantValue.SetVector3(v);
	}
}

void QtPropertyDataDavaVariant::ToVector2(const QVariant &value)
{
	DAVA::Vector2 v;
	QString str = value.toString();

	if(2 == ParseFloatList(str, 2, v.data))
	{
		curVariantValue.SetVector2(v);
	}
}

int QtPropertyDataDavaVariant::ParseFloatList(const QString &str, int maxCount, DAVA::float32 *dest)
{
	int index = 0;

	if(!str.isEmpty() && maxCount > 0 && NULL != dest)
	{
		int pos = 0;
		QRegExp rx("(-?(\\d*)([,\\.])(\\d+){0,1})");

		while(index < maxCount && 
			  (pos = rx.indexIn(str, pos)) != -1)
		{
			QString s = rx.cap();
			pos += rx.matchedLength();

			dest[index] = s.toFloat();
			index++;
		}
	}

	return index;
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
    DAVA::Vector4 v;
    QString str = value.toString();

    if(4 == ParseFloatList(str, 4, v.data))
    {
        curVariantValue.SetColor(DAVA::Color(v.x, v.y, v.z, v.w));
    }

    UpdateColorButtonIcon();
}

void QtPropertyDataDavaVariant::UpdateColorButtonIcon()
{
	if(curVariantValue.type == DAVA::VariantType::TYPE_COLOR)
	{
		QPixmap pix(16, 16);
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
		p.drawRect(QRect(0, 0, 15, 15));

		SetColorButtonIcon(QIcon(pix));
	}
}

void QtPropertyDataDavaVariant::ToAABBox3(const QVariant &value)
{
	// TODO:
	// ...
}

QWidget* QtPropertyDataDavaVariant::CreateEditorInternal(QWidget *parent, const QStyleOptionViewItem& option) const
{
	QWidget* ret = NULL;

	// user shouldn't add allowedValues during edit process
	allowedValuesLocked = true;

	// if there is allowedValues we should show them in combobox
	// user will only be able to select values from combobox
	if(allowedValues.size() > 0)
	{
        if (allowedValueType == TypeFlags)
        {
		    ret = CreateAllowedFlagsEditor(parent);
        }
        else
        {
		    ret = CreateAllowedValuesEditor(parent);
        }
	}
	// check types and create our own widgets for edit
	// if we don't create - Qt will create standard editing widget for QVariant
	else
	{
        switch(curVariantValue.type)
        {
            case DAVA::VariantType::TYPE_FLOAT:
                {
                    QLineEdit *sb = new QLineEdit(parent);
                    sb->setValidator(new QRegExpValidator(QRegExp("\\s*-?\\d*[,\\.]?\\d*\\s*")));
                    ret = sb;
                }
                break;

            default:
                break;
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
		ret = true;
	}
	else
	{
        switch(curVariantValue.type)
        {
            case DAVA::VariantType::TYPE_FLOAT:
                {
                    QLineEdit *sb = (QLineEdit *) editor;
                    QString strValue = FromFloat(curVariantValue.AsFloat()).toString();
                    strValue.replace(" ", "");
                    sb->setText(strValue);
                    ret = true;
                }
                break;

            default:
                break;
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
        switch(curVariantValue.type)
        {
            case DAVA::VariantType::TYPE_FLOAT:
                {
                    QLineEdit *sb = (QLineEdit *) editor;
                    float newValue = sb->text().toFloat();

                    SetValue(QVariant(newValue), QtPropertyData::VALUE_EDITED);
                    ret = true;
                }
                break;

            default:
                break;
        }

	}

	// allow modify valueItems list
	allowedValuesLocked = false;

	return ret;
}

void QtPropertyDataDavaVariant::MultilineEditClicked()
{
    DVASSERT( curVariantValue.type == DAVA::VariantType::TYPE_STRING );

    MultilineEditor editor( GetOWViewport() );
    QEventLoop loop;

    connect( &editor, &MultilineEditor::done, &loop, &QEventLoop::quit );

    editor.setWindowFlags( Qt::Window );
    editor.setWindowModality( Qt::WindowModal );
    editor.SetText( curVariantValue.AsString().c_str() );
    editor.show();
    loop.exec();

    if ( editor.IsAccepted() )
    {
        SetValue( editor.GetText(), VALUE_EDITED );
    }
}

void QtPropertyDataDavaVariant::ColorOWPressed()
{
    const DAVA::Color oldColor = curVariantValue.AsColor();
    
    ColorPicker cp(GetOWViewport());
    cp.SetDavaColor(oldColor);

    connect( &cp, SIGNAL( changing( const QColor& ) ), SLOT( OnColorChanging() ) );
    connect( &cp, SIGNAL( changed( const QColor& ) ), SLOT( OnColorChanging() ) );

    const bool result = cp.Exec();
    const DAVA::Color newColor = cp.GetDavaColor();

    QString str;

    str.sprintf(FLOAT_PRINTF_FORMAT4, oldColor.r, oldColor.g, oldColor.b, oldColor.a);
    SetTempValue(str);  // Restore original value, need for undo/redo

    if (result && newColor != oldColor)
	{
        str.sprintf(FLOAT_PRINTF_FORMAT4, newColor.r, newColor.g, newColor.b, newColor.a);
	    SetValue(str, VALUE_EDITED);
	}

    UpdateColorButtonIcon();
}

void QtPropertyDataDavaVariant::OnColorChanging()
{
    ColorPicker *cp = qobject_cast<ColorPicker *>( sender() );
    if (cp == NULL)
        return;

    const DAVA::Color newColor = cp->GetDavaColor();
    QString str;
    str.sprintf(FLOAT_PRINTF_FORMAT4, newColor.r, newColor.g, newColor.b, newColor.a);

	SetTempValue(str);
}

void QtPropertyDataDavaVariant::FilePathOWPressed()
{
    DAVA::FilePath currPath = curVariantValue.AsFilePath();
    QString openFilePath = currPath.GetAbsolutePathname().c_str();
    if(currPath.IsEmpty() || !currPath.Exists())
    {
        openFilePath = defaultOpenDialogPath;
    }
	QString path = FileDialog::getOpenFileName(GetOWViewport(), "Select file", openFilePath, openDialogFilter);
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

QWidget* QtPropertyDataDavaVariant::CreateAllowedValuesEditor(QWidget *parent) const
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
                text = FromDavaVariant(allowedValues[i].realValue).toString();
			}

			allowedWidget->addItem(text);
		}

		QObject::connect(allowedWidget, SIGNAL(activated(int)), this, SLOT(AllowedSelected(int)));
	}

	return allowedWidget;
}

QWidget* QtPropertyDataDavaVariant::CreateAllowedFlagsEditor(QWidget *parent) const
{
	FlagSelectorCombo *allowedWidget = NULL;

	if(allowedValues.size() > 0)
	{
		allowedWidget = new FlagSelectorCombo(parent);

		for(int i = 0; i < allowedValues.size(); ++i)
		{
            const auto value = allowedValues.at(i);
            const QString text = value.visibleValue.isValid()
                ? value.visibleValue.toString()
                : FromDavaVariant(curVariantValue).toString();
            const DAVA::VariantType real = value.realValue;
            quint64 intVal = 0;
            switch ( real.type )
            {
            case DAVA::VariantType::TYPE_INT32:
                intVal = real.AsInt32();
                break;
            case DAVA::VariantType::TYPE_UINT32:
                intVal = real.AsUInt32();
                break;
            case DAVA::VariantType::TYPE_INT64:
                intVal = real.AsInt64();
                break;
            case DAVA::VariantType::TYPE_UINT64:
                intVal = real.AsUInt64();
                break;
            default:
                DVASSERT( false && "Unsupported type of flags" );
                break;
            }

            allowedWidget->AddFlagItem(intVal, text);
		}
	}

	return allowedWidget;
}

void QtPropertyDataDavaVariant::SetAllowedValueEditorData(QWidget *editorWidget)
{
    if (allowedValueType == TypeFlags)
    {
        FlagSelectorCombo *cb = qobject_cast< FlagSelectorCombo * >( editorWidget );

        if (NULL!=cb)
        {
            const quint64 flags = GetValue().toULongLong();
            cb->SetFlags(flags);
            cb->showPopup();
        }
    }
    else
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
}

void QtPropertyDataDavaVariant::ApplyAllowedValueFromEditor(QWidget *editorWidget)
{
    if (allowedValueType == TypeFlags)
    {
        FlagSelectorCombo *cb = qobject_cast< FlagSelectorCombo * >( editorWidget );

        if (NULL!=cb)
        {
            const quint64 flags = cb->GetFlags();
            SetValue(flags, QtPropertyData::VALUE_EDITED);
        }
    }
    else
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
}

void QtPropertyDataDavaVariant::SetOpenDialogFilter(const QString& value)
{
    openDialogFilter = value;
}

QString QtPropertyDataDavaVariant::GetOpenDialogFilter()
{
    return openDialogFilter;
}

void QtPropertyDataDavaVariant::SetDefaultOpenDialogPath(const QString& value)
{
    defaultOpenDialogPath = value;
}

QString QtPropertyDataDavaVariant::GetDefaultOpenDialogPath()
{
    return defaultOpenDialogPath;
}


QtPropertyDataDavaVariantSubValue::QtPropertyDataDavaVariantSubValue(QtPropertyDataDavaVariant* _parentVariant, DAVA::VariantType const& subvalue)
    : QtPropertyDataDavaVariant(subvalue)
    , parentVariant(_parentVariant)
    , trackParent(true)
{
}

void QtPropertyDataDavaVariantSubValue::SetValueInternal(QVariant const& value)
{
    QtPropertyDataDavaVariant::SetValueInternal(value);
    if(NULL != parentVariant && trackParent)
    {
        parentVariant->MeSetFromChilds();
        parentVariant->ChildsSetFromMe();
    }

    trackParent = true;
}

bool QtPropertyDataDavaVariantSubValue::IsMergable() const
{
    return false;
}