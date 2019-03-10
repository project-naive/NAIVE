#include "Platform.h"

#include <iostream>

void HandleSocketError(int SocketError) {
	if (!SocketError) SocketError = WSAGetLastError();
	HandleSystemError(SocketError);
}

void HandleSystemError(int SystemError) {
	if(!SystemError) SystemError = GetLastError();
	LPSTR buffer;
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				   nullptr,
				   SystemError,
				   LANG_NEUTRAL,
				   (LPSTR)&buffer,
				   0,
				   nullptr);
	std::cerr << buffer << std::endl;
	LocalFree(buffer);
}
