#pragma once

#include "Classes/Settings/SettingsManager.h"

#include "Tools/QtPosSaver/QtPosSaver.h"
#include "Tools/QtPropertyEditor/QtPropertyEditor.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataDavaVariant.h"

#include <QDialog>

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = 0);
    ~SettingsDialog();

protected slots:
    void OnResetPressed();
    void InitProperties();

protected:
    QtPosSaver posSaver;
    QtPropertyEditor* editor;
};

class QtPropertyDataSettingsNode : public QtPropertyDataDavaVariant
{
public:
    QtPropertyDataSettingsNode(const DAVA::FastName& path, const DAVA::FastName& name);
    ~QtPropertyDataSettingsNode();

private:
    DAVA::FastName settingPath;

    virtual void SetValueInternal(const QVariant& value);
    virtual bool UpdateValueInternal();
    virtual bool EditorDoneInternal(QWidget* editor);
};
