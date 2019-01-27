#include <winsock2.h>
#include <Ws2tcpip.h>

#include <iostream>
#include <string>


void HandleSystemError(int SocketError) {
	LPSTR buffer;
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				   nullptr,
				   SocketError,
				   LANG_NEUTRAL,
				   (LPSTR)&buffer,
				   0,
				   nullptr);
	std::cerr << buffer << std::endl;
	LocalFree(buffer);
}


int main() {
	WSADATA socketdata;
	int err = WSAStartup(MAKEWORD(2, 0), &socketdata);
	if (err) {
		HandleSystemError(err);
		exit(EXIT_FAILURE);
	}
	addrinfo info;
	ZeroMemory(&info,sizeof(addrinfo));
	info.ai_family = AF_UNSPEC;
	info.ai_socktype = SOCK_STREAM;;
	info.ai_protocol = IPPROTO_TCP;
	addrinfo* result = nullptr;
	err = getaddrinfo("127.0.0.1", "80", &info, &result);
	if (err) {
		HandleSystemError(err);
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	SOCKET ConnectSocket = INVALID_SOCKET;
	ConnectSocket = socket(result->ai_family,result->ai_socktype,result->ai_protocol);
	if (ConnectSocket == INVALID_SOCKET) {
		HandleSystemError(WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	err = connect(ConnectSocket,result->ai_addr,int(result->ai_addrlen));
	if (err == SOCKET_ERROR) {
		HandleSystemError(WSAGetLastError());
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
		freeaddrinfo(result);
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	freeaddrinfo(result);
	if (ConnectSocket == INVALID_SOCKET) {
		HandleSystemError(WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	bool connection_active = true;
	while (std::cin && connection_active) {
		std::string input;
		if (std::cin.peek()=='\n') {
			connection_active = false;
		}
		else {
			getline(std::cin,input);
		}
		if (connection_active) {
			int send_result = send(ConnectSocket, input.c_str(), input.size(), 0);
			if (send_result == SOCKET_ERROR) {
				HandleSystemError(WSAGetLastError());
				closesocket(ConnectSocket);
				WSACleanup();
				exit(EXIT_FAILURE);
			}
			std::clog << "Sent data of size " << send_result << "!\n\n" << input << '\n' << std::endl;
#define BUFFER_SIZE 1024
			char input_buffer[BUFFER_SIZE]{};
			int recieve_result = recv(ConnectSocket, input_buffer, BUFFER_SIZE, 0);
			if (recieve_result == SOCKET_ERROR) {
				HandleSystemError(WSAGetLastError());
				closesocket(ConnectSocket);
				WSACleanup();
				exit(EXIT_FAILURE);
			} else if (recieve_result) {
				std::clog << "Recieved data of size " << recieve_result << "!\n\n" << input_buffer << '\n' << std::endl;
			} else {
				std::clog << "Connection closed...\n" << std::endl;
			}
		}
	} 
	err = shutdown(ConnectSocket, SD_SEND);
	if (err == SOCKET_ERROR) {
		HandleSystemError(WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	system("pause");
	closesocket(ConnectSocket);
	WSACleanup();
	return 0;
}

