// I want all settings and build options to set in code
// to keep CMakeLists.txt as simple as posible,
// so include directly mongoose.c in current cpp file.

#define _CRT_SECURE_NO_WARNINGS 1

#include "EmbeddedWebServer.h"

#include <stdexcept>

#ifndef __DAVAENGINE_WIN_UAP__

#include "mongoose.h"
extern "C"
{
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#endif

#include "mongoose.c"

#ifdef __clang__
#pragma clang diagnostic pop
#endif
}

namespace DAVA
{
	struct mg_context* ctxEmbeddedWebServer = nullptr;

	static int LogMessage(const struct mg_connection* conn, const char* message) {
		(void)conn;
		fprintf(stderr, "%s\n", message);
		return 0;
	}

	void StartEmbeddedWebServer(const char* documentRoot, const char* listeningPorts)
	{
		if (ctxEmbeddedWebServer != nullptr)
		{
			throw std::runtime_error("second attemp to start embedded web server");
		}

		if (documentRoot == nullptr)
		{
			documentRoot = ".";
		}

		if (listeningPorts == nullptr)
		{
			listeningPorts = "80,443s";
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
			throw std::runtime_error("can't start mongoose");
		}
	}

	void StopEmbeddedWebServer()
	{
		if (ctxEmbeddedWebServer == nullptr)
		{
			throw std::runtime_error("nothing to stop");
		}

		mg_stop(ctxEmbeddedWebServer);
		ctxEmbeddedWebServer = nullptr;
	}
}
#else
namespace DAVA
{
	void StartEmbeddedWebServer(const char*, const char*)
	{
		throw std::runtime_error("not implemented on Win10 UAP");
	}

	void StopEmbeddedWebServer()
	{
		throw std::runtime_error("not implemented on Win10 UAP");
	}
}
#endif // !__DAVAE






