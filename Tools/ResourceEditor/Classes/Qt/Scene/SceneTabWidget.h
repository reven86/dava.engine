#pragma once

#include "UI/UI3DView.h"

#include "FileSystem/FilePath.h"

#include <QMap>
#include <QTabBar>
#include <QWidget>
#include <QMetaType>
#include <QMimeData>
#include <QUrl>

#include <memory>

namespace DAVA
{
class UIEvent;
class UIScreen;
class UI3DView;
class RenderWidget;
}

class SceneEditor2;
class MainTabBar;
class ScenePreviewDialog;
class Request;
class SelectableGroup;
class GlobalOperations;

class SceneTabWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SceneTabWidget(QWidget* parent);
    ~SceneTabWidget();
};
