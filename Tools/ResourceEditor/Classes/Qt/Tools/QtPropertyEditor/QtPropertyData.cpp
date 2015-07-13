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


#include "QtPropertyData.h"
#include "QtPropertyDataValidator.h"

QtPropertyData::QtPropertyData()
	: isValuesMerged( true )
	, curFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable)
	, updatingValue(false)
	, model(NULL)
	, parent(NULL)
	, userData(NULL)
	, optionalButtonsViewport(NULL)
	, validator(NULL)
{ }

QtPropertyData::QtPropertyData(const QVariant &value, Qt::ItemFlags flags)
	: curValue(value)
	, isValuesMerged( true )
	, curFlags(flags)
	, updatingValue(false)
	, model(NULL)
	, parent(NULL)
	, userData(NULL)
	, optionalButtonsViewport(NULL)
	, validator(NULL)
{ }

QtPropertyData::~QtPropertyData()
{
	DVASSERT(!updatingValue && "Property can't be removed during it update process");

	for(int i = 0; i < childrenData.size(); ++i)
	{
		delete childrenData.at(i);
	}
	childrenData.clear();

	for (int i = 0; i < optionalButtons.size(); i++)
	{
		optionalButtons.at(i)->deleteLater();
	}

    DAVA::SafeDelete(userData);
    DAVA::SafeDelete(validator);
}

QVariant QtPropertyData::data(int role) const
{
	QVariant ret;

	switch(role)
	{
	case Qt::EditRole:
	case Qt::DisplayRole:
		ret = GetAlias();
		if(!ret.isValid())
		{
			ret = GetValue();
		}
		break;
    case Qt::ToolTipRole:
        ret = GetToolTip();
        if (!ret.isValid())
        {
            ret = data(Qt::DisplayRole);
        }
        break;
	case Qt::CheckStateRole:
		if(GetFlags() & Qt::ItemIsUserCheckable)
		{
            ret = GetValue();
            if ( !ret.isValid() )
            {
                ret = Qt::PartiallyChecked;
            }
            else
            {
			    ret = GetValue().toBool() ? Qt::Checked : Qt::Unchecked;
            }
		}
		break;
	case Qt::FontRole:
	case Qt::DecorationRole:
	case Qt::BackgroundRole:
	case Qt::ForegroundRole:
		ret = style.value(role);
		break;
	default:
		break;
	}

	return ret;
}

bool QtPropertyData::setData(const QVariant & value, int role)
{
	bool ret = false;

	switch(role)
	{
	case Qt::EditRole:
		SetValue(value, QtPropertyData::VALUE_EDITED);
		ret = true;
		break;
	case Qt::CheckStateRole:
		(value.toInt() == Qt::Unchecked) ? SetValue(false, QtPropertyData::VALUE_EDITED) : SetValue(true, QtPropertyData::VALUE_EDITED);
		ret = true;
		break;
	case Qt::FontRole:
	case Qt::DecorationRole:
	case Qt::BackgroundRole:
	case Qt::ForegroundRole:
		style.insert(role, value);
		ret = true;
		break;
	default:
		break;
	}

	return ret;
}

void QtPropertyData::BuildCurrentValue()
{
    // Build value
    const QVariant master = GetValueInternal();
    bool isAllEqual = true;

    isValuesMerged = false;
    for ( int i = 0; i < mergedData.size(); i++ )
    {
        QtPropertyData *item = mergedData.at(i);
        const QVariant slave = item->GetValue();
        if (master != slave)
        {
            isAllEqual = false;
            break;
        }
    }

    curValue = isAllEqual ? master : QVariant();
    isValuesMerged = isAllEqual;

    // Update Qt MVC properties
    if ( !isAllEqual )
    {
        QList<int> roles;
        roles << Qt::DecorationRole;
        for ( int iRole = 0; iRole < roles.size(); iRole++ )
        {
            const int role = roles.at(iRole);
            auto it = style.find( role );
            if ( it != style.end() )
            {
                *it = QVariant();
            }
        }
    }

}

QVariant QtPropertyData::GetValue() const
{
    QtPropertyData *self = const_cast<QtPropertyData*>(this);
    
    if(curValue.isValid() || !curValue.isNull())
	{
		self->UpdateValue();
	}

    self->BuildCurrentValue();
	return curValue;
}

bool QtPropertyData::IsMergedDataEqual() const
{
    return isValuesMerged;
}

void QtPropertyData::SetValue(const QVariant &value, ValueChangeReason reason)
{
	QVariant oldValue = curValue;

    updatingValue = true;

    foreach ( QtPropertyData *merged, mergedData )
    {
        QtPropertyDataValidator *mergedValidator = merged->validator;
        QVariant validatedValue = value;

        if(reason == VALUE_EDITED && NULL != mergedValidator)
        {
            if(!mergedValidator->Validate(validatedValue))
            {
                continue;
            }
        }

        merged->SetValueInternal(validatedValue);
    }

    // set value
    bool valueIsValid = true;
    QVariant validatedValue = value;

    if(reason == VALUE_EDITED && NULL != validator)
    {
        valueIsValid = validator->Validate(validatedValue);
    }
   
    if(valueIsValid)
    {
        SetValueInternal(validatedValue);
    }

    updatingValue = false;

	// and get what was really set
	// it can differ from input "value"
	// (example: we are trying to set 10, but accepted range is 0-5
	//   value is 10
	//   curValue becomes 0-5)
    UpdateValue();
	curValue = GetValueInternal();

	if(curValue != oldValue)
	{
		updatingValue = true;

		UpdateDown();
		UpdateUp();

		EmitDataChanged(reason);

		updatingValue = false;
	}
}

bool QtPropertyData::UpdateValue(bool force)
{
	bool ret = false;

	if(!updatingValue)
	{
		updatingValue = true;

		if( UpdateValueInternal() || force )
		{
			EmitDataChanged(VALUE_SOURCE_CHANGED);
			ret = true;
		}

		updatingValue = false;
	}

	return ret;
}

QVariant QtPropertyData::GetAlias() const
{
	// this will force update internalValue if 
	// it source was changed 
	GetValue();
    const QVariant alias = IsMergedDataEqual() ? GetValueAlias() : QVariant();

	return alias;
}

void QtPropertyData::SetName(const QString &name)
{
	if(NULL != parent)
	{
		int i = parent->ChildIndex(this);
		if(i >= 0)
		{
			// update name in parent list
			parent->childrenNames[i] = name;
		}
	}

	curName = name;
    foreach ( QtPropertyData *merged, mergedData )
    {
        merged->SetName( curName );
    }
}

QString QtPropertyData::GetName() const
{
	return curName;
}

QString QtPropertyData::GetPath() const
{
	QString path = curName;

	// search top level parent
	const QtPropertyData *parent = this;
	while(NULL != parent->Parent())
	{
		parent = parent->Parent();
		path = parent->curName + "/" + path;
	}

	return path;
}

void QtPropertyData::SetColorButtonIcon(const QIcon &icon)
{
    auto it = std::find_if(optionalButtons.begin(), optionalButtons.end(), [](const QtPropertyToolButton* btn){
        return btn->objectName() == "colorButton";
    });
    if(it != optionalButtons.end())
    {
        (*it)->setIcon(icon);
    }
}

void QtPropertyData::SetIcon(const QIcon &icon)
{
    setData(QVariant(icon), Qt::DecorationRole);
}

QIcon QtPropertyData::GetIcon() const
{
	return qvariant_cast<QIcon>(data(Qt::DecorationRole));
}

QFont QtPropertyData::GetFont() const
{
	return qvariant_cast<QFont>(data(Qt::FontRole));
}

void QtPropertyData::SetFont(const QFont &font)
{
	setData(QVariant(font), Qt::FontRole);
}

QBrush QtPropertyData::GetBackground() const
{
	return qvariant_cast<QBrush>(data(Qt::BackgroundRole));
}

void QtPropertyData::SetBackground(const QBrush &brush)
{
	setData(QVariant(brush), Qt::BackgroundRole);
}

QBrush QtPropertyData::GetForeground() const
{
	return qvariant_cast<QBrush>(data(Qt::ForegroundRole));
}

void QtPropertyData::SetForeground(const QBrush &brush)
{
	setData(QVariant(brush), Qt::ForegroundRole);
}

void QtPropertyData::ResetStyle()
{
	style.remove(Qt::ForegroundRole);
	style.remove(Qt::BackgroundRole);
	style.remove(Qt::FontRole);
}

Qt::ItemFlags QtPropertyData::GetFlags() const
{
	return curFlags;
}

void QtPropertyData::SetFlags(Qt::ItemFlags flags)
{
	curFlags = flags;
}

void QtPropertyData::SetCheckable(bool checkable)
{
	(checkable) ? (curFlags |= Qt::ItemIsUserCheckable) : (curFlags &= ~Qt::ItemIsUserCheckable);
}

bool QtPropertyData::IsCheckable() const
{
	return (curFlags & Qt::ItemIsUserCheckable);
}

void QtPropertyData::SetChecked(bool checked)
{
	setData(QVariant(checked), Qt::CheckStateRole);
}

bool QtPropertyData::IsChecked() const
{
	return data(Qt::CheckStateRole).toBool();
}

void QtPropertyData::SetEditable(bool editable)
{
	(editable) ? (curFlags |= Qt::ItemIsEditable) : (curFlags &= ~Qt::ItemIsEditable);
    UpdateOWState();

    EmitDataChanged(STATE_CHANGED);
}

bool QtPropertyData::IsEditable() const
{
	return (curFlags & Qt::ItemIsEditable);
}

void QtPropertyData::SetEnabled(bool enabled)
{
	(enabled) ? (curFlags |= Qt::ItemIsEnabled) : (curFlags &= ~Qt::ItemIsEnabled);
    UpdateOWState();

    EmitDataChanged(STATE_CHANGED);
}

void QtPropertyData::UpdateOWState()
{
    bool isItemEditable = IsEditable();
    bool isItemEnabled = IsEnabled();

	for(int i = 0; i < optionalButtons.size(); ++i)
	{
		optionalButtons[i]->UpdateState(isItemEnabled, isItemEditable);
	}
}

void QtPropertyData::SetUserData(UserData *data)
{
	if(NULL != userData)
	{
		delete userData;
	}

	userData = data;

	if(NULL != model)
	{
		model->DataChanged(this, VALUE_SET);
	}
}

QtPropertyData::UserData* QtPropertyData::GetUserData() const
{
	return userData;
}

void QtPropertyData::SetToolTip(const QVariant& toolTip)
{
    tooltipValue = toolTip;
}

QVariant QtPropertyData::GetToolTip() const
{
    return tooltipValue;
}

const DAVA::MetaInfo* QtPropertyData::MetaInfo() const
{
	return NULL;
}

bool QtPropertyData::IsEnabled() const
{
	return (curFlags & Qt::ItemIsEnabled);
}

QtPropertyModel* QtPropertyData::GetModel() const
{
	return model;
}

void QtPropertyData::Merge(QtPropertyData* data)
{
    DVASSERT(data);

    if ( !data->IsMergable() )
    {
        // Non-mergable data should be deleted
        data->deleteLater();
        return ;
    }

    data->parent = NULL;
    mergedData << data;

    for ( int i = 0; i < data->childrenData.size(); i++ )
    {
        QtPropertyData* childToMerge = data->childrenData.at(i);
        MergeChild(childToMerge);
    }

    // Do not free/delete
    data->childrenData.clear();
    data->childrenNames.clear();

    UpdateValue(true);
}

void QtPropertyData::MergeChild(QtPropertyData* data, const QString& key)
{
    DVASSERT(data);

    const QString childMergeName = key.isEmpty() ? data->curName : key;
    bool needMerge = true;

    // Looking for child item to merge: name and enabled state should be same
    int childIndex = -1;
    for (int i = 0; i < childrenNames.size(); i++)
    {
        if (childrenData.at(i)->IsEnabled() == data->IsEnabled() &&
            childrenNames.at(i) == childMergeName)
        {
            childIndex = i;
            break;
        }
    }

    const QtPropertyData* child = childrenData.value(childIndex);

    // On change of enabled/disabled states, it is necessary
    // to re-merge child items

    if (child != NULL)
    {
        needMerge &= ( child->MetaInfo() == data->MetaInfo() );
        needMerge &= ( child->IsEnabled() == data->IsEnabled() );
    }
    else
    {
        needMerge = false;
    }

    if (needMerge)
    {
        QtPropertyData *child = childrenData.at(childIndex);
        data->SetName( childMergeName );
        child->Merge( data );
    }
    else
    {
        const int insertIndex = childrenNames.indexOf(childMergeName);
        ChildInsert(childMergeName, data, insertIndex); // insert items with same key in same place
    }
}

bool QtPropertyData::IsMergable() const
{
    // Must be overrided, if data should not be merged
    // For example, if child data modification will affect parent value

    return true;
}

void QtPropertyData::SetModel(QtPropertyModel *_model)
{
	model = _model;

	for(int i = 0; i < childrenData.size(); ++i)
	{
		QtPropertyData *child = childrenData.at(i);
		if(NULL != child)
		{
			child->SetModel(model);
		}
	}
}

void QtPropertyData::SetValidator(QtPropertyDataValidator* value)
{
    DAVA::SafeDelete(validator);
    validator = value;
}

QtPropertyDataValidator* QtPropertyData::GetValidator() const
{
    return validator;
}

QWidget* QtPropertyData::CreateEditor(QWidget *parent, const QStyleOptionViewItem& option) const
{ 
	return CreateEditorInternal(parent, option);
}

bool QtPropertyData::EditorDone(QWidget *editor)
{
    return EditorDoneInternal(editor);
}

bool QtPropertyData::SetEditorData(QWidget *editor)
{
    return SetEditorDataInternal(editor);
}

void QtPropertyData::EmitDataChanged(ValueChangeReason reason)
{
	if(NULL != model)
	{
		model->DataChanged(this, reason);
	}
}

void QtPropertyData::UpdateUp()
{
	if(NULL != parent)
	{
		parent->UpdateValue();
		parent->UpdateUp();
	}
}

void QtPropertyData::UpdateDown()
{
	for(int i = 0; i < childrenData.size(); ++i)
	{
		QtPropertyData *child = childrenData.at(i);
		if(NULL != child)
		{
			child->UpdateValue();
			child->UpdateDown();
		}
	}
}

QtPropertyData* QtPropertyData::Parent() const
{
	return parent;
}

void QtPropertyData::ChildAdd(const QString &key, QtPropertyData *data)
{
	ChildInsert(key, data, ChildCount());
}

void QtPropertyData::ChildAdd(const QString &key, const QVariant &value)
{
	ChildInsert(key, new QtPropertyData(value), ChildCount());
}

void QtPropertyData::ChildInsert(const QString &key, QtPropertyData *data, int pos)
{
	if(NULL != data && !key.isEmpty())
	{
		if(NULL != model)
		{
			model->DataAboutToBeAdded(this, childrenData.size(), childrenData.size());
		}

		if(pos >= 0 && pos < childrenData.size())
		{
			childrenData.insert(pos, data);
			childrenNames.insert(pos, key);
		}
		else
		{
			childrenData.append(data);
			childrenNames.append(key);
		}

		data->curName = key;
		data->parent = this;
		data->SetModel(model);
		data->SetOWViewport(optionalButtonsViewport);

		if(NULL != model)
		{
			model->DataAdded();
		}
	}
}

void QtPropertyData::ChildInsert(const QString &key, const QVariant &value, int pos)
{
	ChildInsert(key, new QtPropertyData(value), pos);
}

int QtPropertyData::ChildCount() const
{
	return childrenData.size();
}

QtPropertyData* QtPropertyData::ChildGet(int i) const
{
	QtPropertyData *ret = NULL;

	if(i >= 0 && i < childrenData.size())
	{
		ret = childrenData.at(i);
	}

	return ret;
}

QtPropertyData* QtPropertyData::ChildGet(const QString &key) const
{
	QtPropertyData *data = NULL;

	int index = childrenNames.indexOf(key);
	if(-1 != index)
	{
		data = childrenData.at(index);
	}

	return data;
}

int QtPropertyData::ChildIndex(QtPropertyData *data) const
{
	return childrenData.indexOf(data);
}

void QtPropertyData::ChildExtract(QtPropertyData *data)
{
	int index = childrenData.indexOf(data);
	ChildRemoveInternal(index, false);
}

void QtPropertyData::ChildRemove(QtPropertyData *data)
{
	int index = childrenData.indexOf(data);
	ChildRemoveInternal(index, true);
}

void QtPropertyData::ChildRemove(const QString &key)
{
	int index = childrenNames.indexOf(key);
	ChildRemoveInternal(index, true);
}

void QtPropertyData::ChildRemove(int index)
{
	ChildRemoveInternal(index, true);
}

void QtPropertyData::ChildRemoveInternal(int index, bool del)
{
	if(index >= 0 && index < childrenData.size())
	{
		if(NULL != model)
		{
			model->DataAboutToBeRemoved(this, index, index);
		}

		QtPropertyData *data = childrenData.at(index);

		childrenData.removeAt(index);
		childrenNames.removeAt(index);

		if(del)
		{
			delete data;
		}

		if(NULL != model)
		{
			model->DataRemoved();
		}
	}
}

void QtPropertyData::ChildRemoveAll()
{
	if(childrenData.size() > 0)
	{
		if(NULL != model)
		{
			model->DataAboutToBeRemoved(this, 0, childrenData.size() - 1);
		}

		for(int i = 0; i < childrenData.size(); ++i)
		{
			delete childrenData.at(i);
		}

		childrenData.clear();
		childrenNames.clear();

		if(NULL != model)
		{
			model->DataRemoved();
		}
	}
}

QtPropertyData * QtPropertyData::GetMergedData( int idx ) const
{
    return mergedData.value(idx);
}

int QtPropertyData::GetMergedCount() const
{
    return mergedData.size();
}

int QtPropertyData::GetButtonsCount() const
{
	return optionalButtons.size();
}

QtPropertyToolButton* QtPropertyData::GetButton(int index)
{
	QtPropertyToolButton *ret = NULL;

	if(index >= 0 && index < optionalButtons.size())
	{
		ret = optionalButtons.at(index);
	}

	return ret;
}

QtPropertyToolButton* QtPropertyData::AddButton(QtPropertyToolButton::StateVariant stateVariant /* = QtPropertyToolButton::ACTIVE_ALWAYS */)
{
	QtPropertyToolButton *button = new QtPropertyToolButton(this, optionalButtonsViewport);

	optionalButtons.append(button);
    button->stateVariant = stateVariant;
	button->setGeometry(0, 0, 18, 18);
	button->setAttribute(Qt::WA_NoSystemBackground, true);
	button->hide();

    UpdateOWState();

	return button;
}

void QtPropertyData::RemButton(int index)
{
	if(index >= 0 && index < optionalButtons.size())
	{
		if(NULL != optionalButtons.at(index))
		{
			delete optionalButtons.at(index);
		}

		optionalButtons.remove(index);
	}
}

void QtPropertyData::RemButton(QtPropertyToolButton *button)
{
	for(int i = 0; i < optionalButtons.size(); ++i)
	{
		if(optionalButtons[i] == button)
		{
			RemButton(i);
			break;
		}
	}
}

QWidget* QtPropertyData::GetOWViewport() const
{
	return optionalButtonsViewport;
}

void QtPropertyData::SetOWViewport(QWidget *viewport)
 {
	optionalButtonsViewport = viewport;

	for(int i = 0; i < optionalButtons.size(); ++i)
	{
		if(NULL != optionalButtons[i])
		{
			optionalButtons[i]->setParent(viewport);
		}
	}

	for(int i = 0; i < childrenData.size(); i++)
	{
		childrenData.at(i)->SetOWViewport(viewport);
	}
}

void* QtPropertyData::CreateLastCommand() const
{
	// can be re-implemented by sub-class

	return NULL;
}

QVariant QtPropertyData::GetValueInternal() const
{
	// should be re-implemented by sub-class

	return curValue;
}

bool QtPropertyData::UpdateValueInternal()
{
	return false;
}

QVariant QtPropertyData::GetValueAlias() const
{
	// should be re-implemented by sub-class

	return QVariant();
}

void QtPropertyData::SetTempValue(const QVariant &value)
{
    foreach ( QtPropertyData *merged, mergedData )
    {
        QtPropertyDataValidator *mergedValidator = merged->validator;
        QVariant validatedValue = value;

        if (NULL != mergedValidator)
        {
            if(!mergedValidator->Validate(validatedValue))
            {
                continue;
            }
        }

        merged->SetTempValueInternal(validatedValue);
    }

    // set value
    bool valueIsValid = true;
    QVariant validatedValue = value;

    if (NULL != validator)
    {
        valueIsValid = validator->Validate(validatedValue);
    }
   
    if(valueIsValid)
    {
        SetTempValueInternal(validatedValue);
    }
}

void QtPropertyData::SetValueInternal(const QVariant &value)
{
	// should be re-implemented by sub-class

	curValue = value;
}

void QtPropertyData::SetTempValueInternal(QVariant const& value)
{
    // should be re-implemented by sub-class
    Q_UNUSED(value);
}

QWidget* QtPropertyData::CreateEditorInternal(QWidget *parent, const QStyleOptionViewItem& option) const
{
	// should be re-implemented by sub-class

	return NULL;
}

bool QtPropertyData::EditorDoneInternal(QWidget *editor)
{
	// should be re-implemented by sub-class
	return false;
}

bool QtPropertyData::SetEditorDataInternal(QWidget *editor)
{
	// should be re-implemented by sub-class
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//
// QtPropertyToolButton
//
//////////////////////////////////////////////////////////////////////////////////////////////////////

QtPropertyToolButton::QtPropertyToolButton(QtPropertyData* data, QWidget * parent /* = 0 */)
	: QToolButton(parent)
	, eventsPassThrought(false)
	, overlayed(false)
	, propertyData(data)
	, stateVariant(ACTIVE_ALWAYS)
{}

QtPropertyToolButton::~QtPropertyToolButton()
{}

QtPropertyData* QtPropertyToolButton::GetPropertyData() const
{
	return propertyData;
}

bool QtPropertyToolButton::event(QEvent * event)
{
	int type = event->type();

	if(eventsPassThrought)
	{
		if(type != QEvent::Enter &&
			type != QEvent::Leave &&
			type != QEvent::MouseMove)
		{
			QToolButton::event(event);
		}

		return false;
	}

	return QToolButton::event(event);
}

QtPropertyToolButton::StateVariant QtPropertyToolButton::GetStateVariant() const
{
    return stateVariant;
}

void QtPropertyToolButton::SetStateVariant(StateVariant state)
{
    stateVariant = state;
}

void QtPropertyToolButton::UpdateState(bool itemIsEnabled, bool itemIsEditable)
{
    bool enabled = false;
    switch(stateVariant)
    {
        case QtPropertyToolButton::ACTIVE_ALWAYS:
            enabled = true;
            break;
        case QtPropertyToolButton::ACTIVE_WHEN_ITEM_IS_ENABLED:
            enabled = itemIsEnabled;
            break;
        case QtPropertyToolButton::ACTIVE_WHEN_ITEM_IS_EDITABLE:
            enabled = itemIsEditable;
            break;
        case QtPropertyToolButton::ACTIVE_WHEN_ITEM_IS_EDITABLE_OR_ENABLED:
            enabled = (itemIsEnabled || itemIsEditable);
            break;
        case QtPropertyToolButton::ACTIVE_WHEN_ITEM_IS_EDITABLE_AND_ENABLED:
            enabled = (itemIsEnabled && itemIsEditable);
            break;
        default:
            break;
    }

    setEnabled(enabled);
}