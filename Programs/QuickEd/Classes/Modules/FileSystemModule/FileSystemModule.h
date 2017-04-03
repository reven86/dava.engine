#pragma once

#include <TArc/Core/ClientModule.h>

class FileSystemWidget;

class FileSystemModule : public DAVA::TArc::ClientModule
{
    void PostInit() override;
    void InitUI();
    void RegisterOperations();

    void OnOpenFile(const QString& filePath);

    FileSystemWidget* widget = nullptr;

    DAVA_VIRTUAL_REFLECTION(FileSystemModule, DAVA::TArc::ClientModule);
};
