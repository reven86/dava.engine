#include "Modules/LegacySupportModule/LegacySupportData.h"
#include "UI/mainwindow.h"

DAVA_VIRTUAL_REFLECTION_IMPL(LegacySupportData)
{
}

LegacySupportData::LegacySupportData()
{
    mainWindow = new MainWindow();
}

LegacySupportData::~LegacySupportData() = default;

MainWindow* LegacySupportData::GetMainWindow() const
{
    return mainWindow.data();
}
