#include <winsock2.h>
#include <Ws2tcpip.h>

#include <iostream>

int main() {
	WSADATA socketdata;
	int err = WSAStartup(MAKEWORD(2, 0), &socketdata);
	if (err) {
		std::cerr << "Failed to initialize WinSocket!\nError code:\t" << err << '\n' << std::endl;
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	addrinfo info;
	ZeroMemory(&info,sizeof(info));
	info.ai_family = AF_INET;
	info.ai_socktype = SOCK_STREAM;;
	info.ai_protocol = IPPROTO_TCP;
	info.ai_flags = AI_PASSIVE;
	addrinfo* result = nullptr;
	err = getaddrinfo("127.0.0.1", "80", &info, &result);
	if (err) {
		std::cerr << "Failed to getaddrinfo!\nError code:\t" << err << '\n' << std::endl;
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	SOCKET ListenSocket = INVALID_SOCKET;
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		std::cerr << "Error at socket(), error code:\t" << WSAGetLastError() << '\n' << std::endl;
		freeaddrinfo(result);
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	err = bind(ListenSocket, result->ai_addr, int(result->ai_addrlen));
	if (err) {
		std::cerr << "Failed to Bind socket!\nError code:\t" << err << '\n' << std::endl;
		closesocket(ListenSocket);
		freeaddrinfo(result);
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	freeaddrinfo(result);
	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
	std::cerr<<"Listen failed with error code:\t"<< WSAGetLastError()<<'\n'<<std::endl;
		closesocket(ListenSocket);
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	SOCKET ClientSocket=INVALID_SOCKET;
	ClientSocket = accept(ListenSocket, nullptr, nullptr);
	if (ClientSocket == INVALID_SOCKET) {
		std::cerr<<"Accepting connection failed!\nError code:\t"<<WSAGetLastError()<<'\n'<<std::endl;
		closesocket(ListenSocket);
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	system("pause");
	WSACleanup();
	return 0;
}