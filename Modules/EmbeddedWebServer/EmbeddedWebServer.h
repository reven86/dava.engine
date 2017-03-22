#pragma once

namespace DAVA
{
/**
	Start in separete thread web server.
	Example:
	StartEmbeddedWebServer(
		"/var/www",
		"80"
	)
	On error return false
*/
bool StartEmbeddedWebServer(const char* documentRoot, const char* listeningPorts);

/**
	Stop web server. Wait till all job threads finished.
*/
void StopEmbeddedWebServer();
}
