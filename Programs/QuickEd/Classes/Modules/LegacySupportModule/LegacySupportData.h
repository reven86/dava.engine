#pragma once

#include <TArc/DataProcessing/DataNode.h>

#include <QPointer>

class MainWindow;

class LegacySupportData : public DAVA::TArc::DataNode
{
public:
    LegacySupportData();
    ~LegacySupportData() override;
    MainWindow* GetMainWindow() const;

private:
    QPointer<MainWindow> mainWindow;
    DAVA_VIRTUAL_REFLECTION(LegacySupportData, DAVA::TArc::DataNode);
};
