#include "Core/Core.h"

#include "Platform/Qt5/QtLayer.h"

#include "FrameworkMain.h"
#include "MainWindow.h"

#include "NgtTools/Application/NGTApplication.h"
#include "NgtTools/Common/GlobalContext.h"
#include "core_ui_framework/i_ui_framework.hpp"
#include "core_ui_framework/i_window.hpp"
#include <QApplication>
#include <QtCore>
#include <QtGui>
#include <QtWidgets>

class TemplateApplication : public NGTLayer::BaseApplication
{
public:
    TemplateApplication(int argc, char** argv)
        : BaseApplication(argc, argv)
    {
    }

protected:
    void GetPluginsForLoad(DAVA::Vector<DAVA::WideString>& names) const override
    {
        names.push_back(L"plg_reflection");
        names.push_back(L"plg_variant");
        names.push_back(L"plg_command_system");
        names.push_back(L"plg_serialization");
        names.push_back(L"plg_file_system");
        names.push_back(L"plg_editor_interaction");
        names.push_back(L"plg_qt_app");
        names.push_back(L"plg_qt_common");
    }
};

int main(int argc, char* argv[])
{
    DAVA::QtLayer qtLayer;
    DAVA::Core::Run(argc, argv);
    int retCode = 0;
    {
        TemplateApplication a(argc, argv);
        a.LoadPlugins();

        MainWindow w;
        w.show();

        retCode = a.StartApplication(&w);
    }
    return retCode;
}
