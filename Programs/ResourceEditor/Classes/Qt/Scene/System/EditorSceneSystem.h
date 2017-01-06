#pragma once

class RECommandNotificationObject;
class EditorSceneSystem
{
    friend class SceneEditor2;

public:
    virtual ~EditorSceneSystem() = default;

    virtual void EnableSystem()
    {
        systemIsEnabled = true;
    }

protected:
    virtual void Draw()
    {
    }
    virtual void ProcessCommand(const RECommandNotificationObject& commandNotification)
    {
    }

    bool systemIsEnabled = false;
};
