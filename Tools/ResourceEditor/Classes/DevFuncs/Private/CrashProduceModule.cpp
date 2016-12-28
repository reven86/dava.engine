#include "Classes/DevFuncs/CrashProduceModule.h"
#include "Classes/Application/REGlobal.h"

#include "TArc/WindowSubSystem/UI.h"
#include "TArc/WindowSubSystem/ActionUtils.h"

void CrashProduceModule::PostInit()
{
    using namespace DAVA::TArc;
    UI* ui = GetUI();
    ActionPlacementInfo placementInfo(CreateMenuPoint(QList<QString>() << "menuDebug_Functions"
                                                                       << "Crash Produce"));

    QAction* assertAction = new QAction("Generate Assert", nullptr);
    connections.AddConnection(assertAction, &QAction::triggered, []()
                              {
                                  DVASSERT(false);
                              });

    ui->AddAction(REGlobal::MainWindowKey, placementInfo, assertAction);

    QAction* sequencalsAssert = new QAction("Generate sequencal asserts", nullptr);
    connections.AddConnection(sequencalsAssert, &QAction::triggered, []()
                              {
                                  DVASSERT(false && "First assert");
                                  DVASSERT(false && "Second assert");
                              });

    ui->AddAction(REGlobal::MainWindowKey, placementInfo, sequencalsAssert);

    QAction* dumpAction = new QAction("Generate dump", nullptr);
    connections.AddConnection(dumpAction, &QAction::triggered, []()
                              {
                                  int* p = reinterpret_cast<int*>(0xDEADBEAD);
                                  int x = *p;
                                  DAVA::Logger::Info("Crash value %d", x);
                              });

    ui->AddAction(REGlobal::MainWindowKey, placementInfo, dumpAction);
}
