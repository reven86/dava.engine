#ifndef __DEBUG_TOOLS__
#define __DEBUG_TOOLS__

#include <QObject>

class DeveloperTools : public QObject
{
    Q_OBJECT

public:
    explicit DeveloperTools(QObject* parent = 0);

public slots:

    void OnDebugFunctionsGridCopy();
    void OnDebugCreateTestSkinnedObject(); //creates
    void OnImageSplitterNormals();
    void OnReplaceTextureMipmap();
    void OnToggleLandscapeInstancing();
};
#endif /* defined(__DEBUG_TOOLS__) */