#include <cef/include/cef_app.h>

int main(int argc, char* argv[])
{
    CefMainArgs mainArgs(::GetModuleHandle(nullptr));
    return CefExecuteProcess(mainArgs, nullptr, nullptr);
}