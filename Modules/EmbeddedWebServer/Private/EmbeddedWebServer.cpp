// I want all settings and build options to set in code
// to keep CMakeLists.txt as simple as posible,
// so include directly mongoose.c in current cpp file.

#define _CRT_SECURE_NO_WARNINGS 1

#include "EmbeddedWebServer.h"

#ifndef __DAVAENGINE_WIN_UAP__

#include "mongoose.h"
extern "C" {
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4244 4267)
#endif

#include "mongoose.c"

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#ifdef __clang__
#pragma clang diagnostic pop
#endif
}

namespace DAVA
{
struct mg_context* ctxEmbeddedWebServer = nullptr;

static int LogMessage(const struct mg_connection* /*conn*/, const char* message)
{
    fprintf(stderr, "%s\n", message);
    return 0;
}

bool StartEmbeddedWebServer(const char* documentRoot, const char* listeningPorts)
{
    if (ctxEmbeddedWebServer != nullptr)
    {
        return false;
    }

    if (documentRoot == nullptr)
    {
        return false;
    }

    if (listeningPorts == nullptr)
    {
        return false;
    }
    const char* options[] = {
        "document_root", documentRoot, // "/var/www"
        "listening_ports", listeningPorts, // "80,443s",
        nullptr
    };

    struct mg_callbacks callbacks;
    callbacks.log_message = &LogMessage;
    memset(&callbacks, 0, sizeof(callbacks));

    ctxEmbeddedWebServer = mg_start(&callbacks, nullptr, options);

    if (ctxEmbeddedWebServer == nullptr)
    {
        return false;
    }

    return true;
}

void StopEmbeddedWebServer()
{
    if (ctxEmbeddedWebServer != nullptr)
    {
        mg_stop(ctxEmbeddedWebServer);
        ctxEmbeddedWebServer = nullptr;
    }
}
}
#else
namespace DAVA
{
bool StartEmbeddedWebServer(const char*, const char*)
{
    // not supported platform
    return false;
}

void StopEmbeddedWebServer()
{
}
}
#endif // !__DAVAE
