#include <winsock2.h>
#include <Ws2tcpip.h>

#include <iostream>

int main() {
	WSADATA socketdata;
	int err = WSAStartup(MAKEWORD(2, 0), &socketdata);
	if (err) {
		std::cerr<<"Failed to initialize WinSocket!\nError code:\t"<<err<<'\n'<<std::endl;
		exit(EXIT_FAILURE);
	}
	addrinfo info;
	ZeroMemory(&info,sizeof(addrinfo));
	info.ai_family = AF_UNSPEC;
	info.ai_socktype = SOCK_STREAM;;
	info.ai_protocol = IPPROTO_TCP;
	addrinfo* result = nullptr;
	while (!result) {
		err = getaddrinfo("127.0.0.1", "80", &info, &result);
	}
	if (err) {
		std::cerr << "Failed to getaddrinfo!\nError code:\t" << err << '\n' << std::endl;
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	SOCKET ConnectSocket = INVALID_SOCKET;
	ConnectSocket = socket(result->ai_family,result->ai_socktype,result->ai_protocol);
	if (ConnectSocket == INVALID_SOCKET) {
		std::cerr<<"Error at socket(), error code:\t"<<WSAGetLastError()<<'\n'<<std::endl;
		freeaddrinfo(result);
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	err = connect(ConnectSocket,result->ai_addr,int(result->ai_addrlen));
	if (err == SOCKET_ERROR) {
		std::cerr << "Error at conect(), error code:\t" << WSAGetLastError() << '\n' << std::endl;		
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
		freeaddrinfo(result);
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	freeaddrinfo(result);
	if (ConnectSocket == INVALID_SOCKET) {
		std::cerr<<"Unable to connect to server!\n"<<std::endl;
		closesocket(ConnectSocket);
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	system("pause");
	WSACleanup();
	return 0;
}

