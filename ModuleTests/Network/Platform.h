#pragma once

#include <winsock2.h>
#include <Ws2tcpip.h>

#undef max
#undef min

void HandleSocketError(int SocketError = 0);
void HandleSystemError(int SystemError = 0);

