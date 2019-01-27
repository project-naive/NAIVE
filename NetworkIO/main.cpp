#include <iostream>
#include <streambuf>

#include <winsock2.h>
#include <Ws2tcpip.h>

#define PING_PORT           7
#define TIME_PORT           13
#define NETSTAT_PORT        15
#define SSH_PORT            22
#define TELNET_DEFAULT_PORT 23
#define SMTP_PORT           25
#define WHOIS_PORT          43
#define FINGER_PORT         79
#define HTTP_PORT           80
#define POP_PORT            110
#define NNTP_PORT           119
#define CLOGIN_PORT         513

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
		HandleSystemError(err);
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	SOCKET ListenSocket = INVALID_SOCKET;
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		HandleSystemError(WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	err = bind(ListenSocket, result->ai_addr, int(result->ai_addrlen));
	if (err) {
		HandleSystemError(err);
		closesocket(ListenSocket);
		freeaddrinfo(result);
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	freeaddrinfo(result);
	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		HandleSystemError(WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	SOCKET ClientSocket=INVALID_SOCKET;
	ClientSocket = accept(ListenSocket, nullptr, nullptr);
	if (ClientSocket == INVALID_SOCKET) {
		HandleSystemError(WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		exit(EXIT_FAILURE);
	}
#define BUFFER_SIZE 1024
	int send_result=0;
	int recieve_result=0;
	do {
		char input_buffer[BUFFER_SIZE]{};
		recieve_result=recv(ClientSocket, input_buffer, BUFFER_SIZE, 0);
		if (recieve_result == SOCKET_ERROR) {
			HandleSystemError(WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			exit(EXIT_FAILURE);
		}
		else if (recieve_result) {
			char output_buffer[BUFFER_SIZE]{};
			std::clog<<"Bytes recieved:\t"<<recieve_result<<'\n'<<std::endl;
			std::clog<<"Recieved message:\t"<<input_buffer<<'\n'<<std::endl;
			memcpy(output_buffer,input_buffer, recieve_result);
			std::clog << "Sending message:\t" << output_buffer << '\n' << std::endl;
			send_result=send(ClientSocket,output_buffer, recieve_result,0);
			if (send_result == SOCKET_ERROR) {
				HandleSystemError(WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				exit(EXIT_FAILURE);
			}
			std::clog << "Bytes sent:\t" << send_result << '\n' << std::endl;
		} else if (recieve_result == 0) {
			std::clog<<"Connection closing...\n"<<std::endl;
		}
	}while(recieve_result);
	system("pause");
	err = shutdown(ClientSocket, SD_SEND);
	if (err == SOCKET_ERROR) {
		HandleSystemError(WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	closesocket(ClientSocket);
	closesocket(ListenSocket);
	WSACleanup();
	return 0;
}