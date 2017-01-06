#pragma once

#include "Classes/Qt/Scene/ActiveSceneHolder.h"

#include <QObject>

class DeveloperTools : public QObject
{
    Q_OBJECT

public:
    explicit DeveloperTools(QWidget* parent = 0);

public slots:

    void OnDebugFunctionsGridCopy();
    void OnDebugCreateTestSkinnedObject();
    void OnImageSplitterNormals();
    void OnReplaceTextureMipmap();
    void OnToggleLandscapeInstancing();

private:
    QWidget* GetParentWidget();

private:
    ActiveSceneHolder sceneHolder;
};
