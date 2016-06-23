#include "Core/Core.h"

#include "Platform/Qt5/QtLayer.h"

#include "FrameworkMain.h"
#include "Application.h"
#include "GLView.h"

#include "NgtTools/Common/GlobalContext.h"
#include <core_ui_framework/i_ui_framework.hpp>
#include <core_ui_framework/i_ui_application.hpp>
#include <core_ui_framework/i_window.hpp>
#include <core_ui_framework/i_view.hpp>

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

        IUIFramework* framework = Context::queryInterface<IUIFramework>();

        IUIApplication* app = Context::queryInterface<IUIApplication>();
        DVASSERT(app != nullptr);

        std::unique_ptr<IWindow> window = framework->createWindow(":/MainWindow.ui", IUIFramework::ResourceType::File);
        app->addWindow(*window);

        GLView glView;
        app->addView(glView);
        window->show();
        retCode = app->startApplication();
    }
    return retCode;
}
