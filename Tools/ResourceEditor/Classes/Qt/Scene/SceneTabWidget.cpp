#include "Scene/SceneTabWidget.h"

#include "UI/Focus/UIFocusComponent.h"

#include "Main/mainwindow.h"
#include "Scene/SceneEditor2.h"
#include "Classes/Qt/GlobalOperations.h"
#include "Tools/QtLabelWithActions/QtLabelWithActions.h"
#include "Tools/MimeData/MimeDataHelper2.h"
#include "Deprecated/ScenePreviewDialog.h"
#include "MaterialEditor/MaterialAssignSystem.h"

#include "Platform/SystemTimer.h"
#include "Engine/Qt/RenderWidget.h"

#include <QVBoxLayout>
#include <QResizeEvent>
#include <QFileInfo>
#include <QMessageBox>
#include <QShortcut>
#include <QDebug>
#include <QTimer>
#include <QtGlobal>

SceneTabWidget::SceneTabWidget(QWidget* parent)
    : QWidget(parent)
{
    // TODO UVR
    /* auto moveToSelectionHandler = [&]
    {
        if (curScene == nullptr)
            return;
        curScene->cameraSystem->MoveToSelection();
    };
    auto moveToSelectionHandlerHotkey = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_D), this);
    connect(moveToSelectionHandlerHotkey, &QShortcut::activated, moveToSelectionHandler);*/
}

SceneTabWidget::~SceneTabWidget()
{
}