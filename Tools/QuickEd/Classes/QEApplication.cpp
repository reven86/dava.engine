#include "QEApplication.h"

QEApplication::QEApplication(int argc, char** argv)
    : BaseApplication(argc, argv)
{
}

void QEApplication::GetPluginsForLoad(DAVA::Vector<DAVA::WideString>& names) const
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

void QEApplication::OnPostLoadPugins()
{
    qApp->setOrganizationName("DAVA");
    qApp->setApplicationName("QuickEd");
}
