#pragma once

#include "Base/Introspection.h"
#include "Preferences/PreferencesRegistrator.h"
#include <QDialog>

class QTreeView;
class PreferencesModel;

class PreferencesDialog : public QDialog, public DAVA::InspBase
{
public:
    PreferencesDialog(QWidget* parent = nullptr, Qt::WindowFlags flags = 0);
    ~PreferencesDialog();

private slots:
    void OnButtonBoxAccepted();
    void OnButtonBoxRejected();

private:
    DAVA::String GetGeometry() const;
    void SetGeometry(const DAVA::String& str);

    DAVA::String GetHeaderState() const;
    void SetHeaderState(const DAVA::String& str);

    QTreeView* treeView = nullptr;
    PreferencesModel* preferencesModel = nullptr;

public:
    INTROSPECTION(PreferencesDialog,
                  PROPERTY("currentGeometry", "PreferencesDialogInternal/Current Geometry", GetGeometry, SetGeometry, DAVA::I_PREFERENCE)
                  PROPERTY("headerState", "PreferencesDialogInternal/Header State", GetHeaderState, SetHeaderState, DAVA::I_PREFERENCE)
                  )
};