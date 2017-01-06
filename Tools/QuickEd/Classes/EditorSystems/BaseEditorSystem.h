#pragma once

namespace DAVA
{
class UIEvent;
}

class EditorSystemsManager;

class BaseEditorSystem
{
public:
    explicit BaseEditorSystem(EditorSystemsManager* parent);
    virtual ~BaseEditorSystem() = default;

    virtual bool OnInput(DAVA::UIEvent* currentInput);

protected:
    EditorSystemsManager* systemsManager = nullptr;
};
