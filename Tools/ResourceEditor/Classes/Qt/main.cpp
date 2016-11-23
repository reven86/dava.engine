#include "Debug/DVAssertDefaultHandlers.h"
#include "Classes/Application/REApplication.h"

int DAVAMain(DAVA::Vector<DAVA::String> cmdline)
{
    DAVA::Assert::SetupDefaultHandlers();

    REApplication app(std::move(cmdline));
    return app.Run();
}
