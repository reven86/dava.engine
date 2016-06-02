#include "Core/Core.h"

#include "Platform/Qt5/QtLayer.h"

#include "FrameworkMain.h"
#include "MainWindow.h"
#include "Application.h"

#include "NgtTools/Common/GlobalContext.h"
#include <core_ui_framework/i_ui_framework.hpp>
#include <core_ui_framework/i_window.hpp>
#include <core_qt_common/qt_window.hpp>
#include <core_qt_common/i_qt_framework.hpp>
#include <core_qt_common/i_qt_view.hpp>
#include <core_ui_framework/i_ui_application.hpp>
#include <core_ui_framework/i_view.hpp>

#include "QtTools/DavaGLWidget/DavaGLWidget.h"
#include <QApplication>
#include <QtCore>
#include <QtGui>
#include <QtWidgets>

//GL widget NGT wrapper
class TestWidget : public IQtView
{
public:
    TestWidget()
    {
        button = new DavaGLWidget();
    }
    ~TestWidget() override = default;

    const char* id() const override
    {
        return "pushBUtton";
    }
    const char* title() const override
    {
        return "button title";
    }
    const char* windowId() const override
    {
        return "";
    }
    const LayoutHint& hint() const override
    {
        static LayoutHint hint("default");
        return hint;
    };
    void update() override
    {
        button->update();
    };

    void focusInEvent() override{};
    void focusOutEvent() override{};

    virtual void registerListener(IViewEventListener* listener) override
    {
    }
    virtual void deregisterListener(IViewEventListener* listener) override
    {
    }

    QWidget* releaseView() override
    {
        return button;
    };
    void retainView() override{};
    QWidget* view() const override
    {
        return button;
    };

    DavaGLWidget* button;
};

int main(int argc, char* argv[])
{
    //helper class which connect Qt openGL and DAVA framework together
    DAVA::QtLayer qtLayer;

    //DAVA Framework require to launch Core first
    DAVA::Core::Run(argc, argv);

    int retCode = 0;
    {
        //our application wrapper. Here it only load plugins
        Application a(argc, argv);
        a.LoadPlugins();

        //the only way to display something in a mainWindow - is to create in manually
        std::unique_ptr<QMainWindow> mainWindow(new QMainWindow());
        mainWindow->setCentralWidget(new QWidget());
        //also we need to add layout manually
        mainWindow->centralWidget()->setLayout(new QHBoxLayout());

        //unfortunatly i need to use tabWidget and i can not just place GLWidget to the centralWidget of mainWindow
        QTabWidget* tabWidget = new QTabWidget(mainWindow.get());
        //also i need to register property manually, so i can not use createWindow("MainWindow.ui")
        tabWidget->setProperty("layoutTags", "default");
        //add tabWidget to the mainWindow layout manually. Without this GLWidget will be small as possible
        mainWindow->centralWidget()->layout()->addWidget(tabWidget);

        //just test QDockWidget
        QDockWidget* dockWidget = new QDockWidget(mainWindow.get());
        dockWidget->setProperty("layoutTags", "dock");
        mainWindow->addDockWidget(Qt::LeftDockWidgetArea, dockWidget);

        IUIFramework* framework = Context::queryInterface<IUIFramework>();
        IQtFramework* qtFramework = dynamic_cast<IQtFramework*>(framework);
        //create mainWindow manually from Qt QMainWindow object
        std::unique_ptr<IWindow> window(new QtWindow(*qtFramework, std::move(mainWindow)));
        window->show();

        //now create GLWidget and add it to the IUIApplication instance.
        TestWidget testWidget;
        IUIApplication* app = Context::queryInterface<IUIApplication>();
        app->addWindow(*window.get());
        app->addView(testWidget);
        DVASSERT(app != nullptr);
        retCode = app->startApplication();
    }
    return retCode;
}
