#include "Classes/Qt/Application/REApplication.h"
#include "Debug/DVAssertDefaultHandlers.h"

int DAVAMain(DAVA::Vector<DAVA::String> cmdline)
{
    DAVA::Assert::SetupDefaultHandlers();

    REApplication app(std::move(cmdline));
    return app.Run();
}
