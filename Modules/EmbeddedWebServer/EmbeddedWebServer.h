#pragma once

extern "C"
{
struct mg_connection;
}

namespace DAVA
{
/**
	Called when mongoose has received new HTTP request.
    If callback returns non-zero,
    callback must process the request by sending valid HTTP headers and body,
    and mongoose will not do any further processing.
    If callback returns 0, mongoose processes the request itself. In this case,
    callback must not send any data to the client.
*/
using OnRequestHandler = int (*)(mg_connection* connection);

/**
	Start in separate thread web server.
	Example:
	StartEmbeddedWebServer(
		"/var/www",
		"80"
	)
	On error return false
*/
bool StartEmbeddedWebServer(const char* documentRoot, const char* listeningPorts, OnRequestHandler callback = nullptr);

/**
	Stop web server. Wait till all job threads finished.
*/
void StopEmbeddedWebServer();
}
