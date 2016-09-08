#include "Engine/Public/Engine.h"
#include "EditorCore.h"

int GameMain(DAVA::Vector<DAVA::String> cmdline)
{
    DAVA::Engine engine;

    EditorCore* editorCore = new EditorCore(&engine);

    engine.windowCreated.Connect(editorCore, &EditorCore::OnWindowCreated);

    //we need to do it after QApplication will be created
    engine.gameLoopStarted.Connect(editorCore, &EditorCore::OnGameLoopStarted);

    engine.gameLoopStopped.Connect([editorCore]() {
        delete editorCore;
    });

    return engine.Run();
}

