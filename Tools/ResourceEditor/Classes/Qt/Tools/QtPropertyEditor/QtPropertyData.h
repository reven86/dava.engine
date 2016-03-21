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


#ifndef __QT_PROPERTY_DATA_H__
#define __QT_PROPERTY_DATA_H__

#include "Base/BaseTypes.h"
#include "Base/FastName.h"
#include "Base/Meta.h"

#include "Functional/Signal.h"
#include "Functional/Function.h"

#include "Commands2/Base/Command2.h"

#include <QToolButton>
#include <QVariant>

class QEvent;
class QIcon;
class QStyleOptionViewItem;

class QtPropertyModel;
class QtPropertyData;
class QtPropertyDataValidator;
class Command2;

class QtPropertyToolButton : public QToolButton
{
    friend class QtPropertyData;

public:
    enum StateVariant
    {
        ACTIVE_ALWAYS,
        ACTIVE_WHEN_ITEM_IS_ENABLED,
        ACTIVE_WHEN_ITEM_IS_EDITABLE,
        ACTIVE_WHEN_ITEM_IS_EDITABLE_OR_ENABLED,
        ACTIVE_WHEN_ITEM_IS_EDITABLE_AND_ENABLED
    };

    QtPropertyToolButton(QtPropertyData* data, QWidget* parent = 0);
    ~QtPropertyToolButton();

    QtPropertyData* GetPropertyData() const;
    virtual bool event(QEvent* event);

    void SetStateVariant(StateVariant state);
    StateVariant GetStateVariant() const;

    bool eventsPassThrought;
    bool overlayed;

protected:
    QtPropertyData* propertyData;
    StateVariant stateVariant;

    void UpdateState(bool itemIsEnabled, bool itemIsEditable);
};

class QtPropertyData
{
    friend class QtPropertyModel;
    friend class QtPropertyItemDelegate;

public:
    enum ValueChangeReason
    {
        VALUE_SOURCE_CHANGED,
        VALUE_SET,
        VALUE_EDITED,
        STATE_CHANGED,
    };

    struct UserData
    {
        virtual ~UserData()
        {
        }
    };

    QtPropertyData(const DAVA::FastName& name);
    QtPropertyData(const DAVA::FastName& name, const QVariant& value);
    virtual ~QtPropertyData();

    QVariant data(int role) const;
    bool setData(const QVariant& value, int role);

    const DAVA::FastName& GetName() const;

    QString GetPath() const;

    QVariant GetValue() const;
    void SetValue(const QVariant& value, ValueChangeReason reason = QtPropertyData::VALUE_SET);
    bool UpdateValue(bool force = false);

    QVariant GetAlias() const;

    QIcon GetIcon() const;
    void SetIcon(const QIcon& icon);

    QFont GetFont() const;
    void SetFont(const QFont& font);

    QBrush GetBackground() const;
    void SetBackground(const QBrush& brush);

    QBrush GetForeground() const;
    void SetForeground(const QBrush& brush);

    Qt::ItemFlags GetFlags() const;
    void SetFlags(Qt::ItemFlags flags);

    virtual UserData* GetUserData() const;
    virtual void SetUserData(UserData* userdata);

    virtual void SetToolTip(const QVariant& toolTip);
    virtual QVariant GetToolTip() const;

    virtual const DAVA::MetaInfo* MetaInfo() const;

    // reset background/foreground/font settings
    void ResetStyle();

    void SetCheckable(bool checkable);
    bool IsCheckable() const;
    void SetChecked(bool checked);
    bool IsChecked() const;

    void SetEditable(bool editable);
    bool IsEditable() const;

    void SetEnabled(bool enabled);
    bool IsEnabled() const;

    QtPropertyModel* GetModel() const;

    QtPropertyDataValidator* GetValidator() const;
    void SetValidator(QtPropertyDataValidator*);

    // editor
    QWidget* CreateEditor(QWidget* parent, const QStyleOptionViewItem& option) const;
    bool EditorDone(QWidget* editor);
    bool SetEditorData(QWidget* editor);

    // childs
    QtPropertyData* Parent() const;
    void ChildAdd(std::unique_ptr<QtPropertyData>&& data);
    void ChildrenAdd(DAVA::Vector<std::unique_ptr<QtPropertyData>>&& data);
    void ChildInsert(std::unique_ptr<QtPropertyData>&& data, int pos);
    int ChildCount() const;
    QtPropertyData* ChildGet(int i) const;
    QtPropertyData* ChildGet(const DAVA::FastName& key) const;
    int ChildIndex(const QtPropertyData* data) const;
    void ChildrenExtract(DAVA::Vector<std::unique_ptr<QtPropertyData>>& children);
    void ChildRemove(const QtPropertyData* data);
    void ChildRemoveAll();
    void ResetChildren();

    virtual void FinishTreeCreation();

    // Optional widgets
    int GetButtonsCount() const;
    QtPropertyToolButton* GetButton(int index = 0);
    QtPropertyToolButton* AddButton(QtPropertyToolButton::StateVariant stateVariant = QtPropertyToolButton::ACTIVE_ALWAYS);
    void RemButton(QtPropertyToolButton* button);

    void EmitDataChanged(ValueChangeReason reason);

    // edit command
    virtual Command2::Pointer CreateLastCommand() const;

    // Merging
    bool IsMergedDataEqual() const;
    void ForeachMergedItem(DAVA::Function<bool(QtPropertyData*)> const& functor) const;
    int GetMergedItemCount() const;
    void Merge(std::unique_ptr<QtPropertyData>&& data);
    void MergeChild(std::unique_ptr<QtPropertyData>&& data);
    virtual bool IsMergable() const;

protected:
    mutable QVariant curValue;
    mutable bool isValuesMerged = true;

    DAVA::FastName name;
    Qt::ItemFlags curFlags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;

    QMap<int, QVariant> style;
    bool updatingValue = false;

    QtPropertyModel* model = nullptr;
    QtPropertyData* parent = nullptr;
    std::unique_ptr<UserData> userData;

    struct ChildKey
    {
        ChildKey() = default;
        ChildKey(const QtPropertyData* child);

        bool operator==(const ChildKey& other) const;
        bool operator!=(const ChildKey& other) const;
        bool operator<(const ChildKey& other) const;

        const QtPropertyData* child = nullptr;
    };

    using TChildMap = DAVA::Map<ChildKey, size_t>;
    TChildMap keyToDataMap;
    DAVA::Vector<std::unique_ptr<QtPropertyData>> childrenData;
    DAVA::Vector<std::unique_ptr<QtPropertyData>> mergedData;

    QWidget* optionalButtonsViewport = nullptr;
    QVector<QtPropertyToolButton*> optionalButtons;

    std::unique_ptr<QtPropertyDataValidator> validator;

    QVariant tooltipValue;

    void SetModel(QtPropertyModel* model);
    void SetColorButtonIcon(const QIcon& icon);

    void BuildCurrentValue();
    void SetTempValue(const QVariant& value);

    virtual void UpdateUp();
    virtual void UpdateDown();

    // Functions should be re-implemented by sub-class
    virtual QVariant GetValueInternal() const;
    virtual QVariant GetValueAlias() const;
    virtual void SetValueInternal(const QVariant& value);
    virtual void SetTempValueInternal(const QVariant& value);
    virtual bool UpdateValueInternal();
    virtual QWidget* CreateEditorInternal(QWidget* parent, const QStyleOptionViewItem& option) const;
    virtual bool EditorDoneInternal(QWidget* editor);
    virtual bool SetEditorDataInternal(QWidget* editor);

    // viewport, where optional toolbuttons should be drawn
    QWidget* GetOWViewport() const;
    void SetOWViewport(QWidget* viewport);

    // optional widgets state update
    void UpdateOWState();

    void RefillSearchIndex();
};

#endif // __QT_PROPERTY_DATA_H__
