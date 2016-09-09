#include "Engine/Public/Engine.h"
#include "EditorCore.h"

int GameMain(DAVA::Vector<DAVA::String> cmdline)
{
    DAVA::Engine engine;

    EditorCore* editorCore = new EditorCore(&engine);

    engine.gameLoopStopped.Connect([editorCore]() {
        delete editorCore;
    });

    return engine.Run();
}

