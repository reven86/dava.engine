#pragma once

class RECommandNotificationObject;
class EditorSceneSystem
{
public:
    virtual ~EditorSceneSystem() = default;

    virtual void Draw()
    {
    }
    virtual void ProcessCommand(const RECommandNotificationObject& commandNotification)
    {
    }

    virtual void EnableSystem()
    {
        systemIsEnabled = true;
    }

protected:
    bool systemIsEnabled = false;
};
