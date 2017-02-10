#pragma once

#include <TArc/DataProcessing/DataNode.h>

#include <QPointer>

class MainWindow;

class LegacySupportData : public DAVA::TArc::DataNode
{
public:
    LegacySupportData();
    ~LegacySupportData() override;

private:
    friend class LegacySupportModule;
    MainWindow* GetMainWindow() const;

    QPointer<MainWindow> mainWindow;
    DAVA_VIRTUAL_REFLECTION(LegacySupportData, DAVA::TArc::DataNode);
};
