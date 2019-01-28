#include "NetworkManager.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>

//Functions in class NetworkManager

NetworkManager::NetworkManager(char version_major, char version_minor) {
	int err = WSAStartup(MAKEWORD(version_major, version_minor), &WinSocketInfo);
	if (err) {
		HandleSystemError(err);
		WSACleanup();
		throw std::runtime_error(
		std::string("Failed to initialize Winsocket with Version")+
		char('0'+version_major)+'.'+char('0'+version_minor));
	}
	std::clog << "Initialized Winsocket with version " << int(LOBYTE(WinSocketInfo.wVersion)) << '.' << int(HIBYTE(WinSocketInfo.wVersion)) << '\n'
		<< "Highest version available: " << int(LOBYTE(WinSocketInfo.wHighVersion)) << '.' << int(HIBYTE(WinSocketInfo.wHighVersion)) << '\n'
		<< "Description:\n" << WinSocketInfo.szDescription << '\n'
		<< "System status:\n" << WinSocketInfo.szSystemStatus << '\n';
	if (LOBYTE(WinSocketInfo.wVersion) < 2) {
		std::clog<<"Maximum Sockets: "<<WinSocketInfo.iMaxSockets
			<<"\nMaximum UDP Datagram Size: "<<WinSocketInfo.iMaxUdpDg<<'\n';
		if(WinSocketInfo.lpVendorInfo)
			std::clog << "\nVendor Info:\n"<<WinSocketInfo.lpVendorInfo<<'\n';
	}
	std::clog << std::endl;
}

NetworkManager::~NetworkManager() {
	WSACleanup();
}

TCPServer NetworkManager::CreateServer(addrinfo& hint, const char* domain, const char* service, size_t queue_size) {
	hint.ai_socktype = SOCK_STREAM;
	hint.ai_protocol = IPPROTO_TCP;
	hint.ai_flags = AI_PASSIVE;
	addrinfo* result = nullptr;
	int err = getaddrinfo(domain, service, &hint, &result);
	if (err) {
		HandleSocketError(err);
		return TCPServer(-1, nullptr, nullptr, 0);
	}
	size_t ListenSocket = socket(result->ai_family,result->ai_socktype,result->ai_protocol);
	if (ListenSocket == size_t(-1)) {
		HandleSocketError();
		freeaddrinfo(result);
		return TCPServer(-1, nullptr, nullptr, 0);
	}
	err = bind(ListenSocket, result->ai_addr, result->ai_addrlen);
	if (err) {
		HandleSocketError(err);
		closesocket(ListenSocket);
		freeaddrinfo(result);
		return TCPServer(-1, nullptr, nullptr, 0);
	}
	if (listen(ListenSocket, queue_size) == -1) {
		HandleSocketError();
		closesocket(ListenSocket);
		freeaddrinfo(result);
		return TCPServer(-1, nullptr, nullptr, 0);
	}
	return TCPServer(ListenSocket, service, result, queue_size);
}

TCPClient NetworkManager::CreateClient() {
	return TCPClient();
}

//Functions in class TCPServer

TCPServer::TCPServer(size_t LSocket, const char* iservice, const addrinfo* const addri, const size_t queue_size):
	ListenSocket(LSocket), service(iservice), addr(addri), max_queue(queue_size) {
	FD_ZERO(&to_read_fd);
	FD_ZERO(&to_write_fd);
	FD_SET(LSocket, &to_read_fd);
	max_fd = ListenSocket;
}


TCPServer::~TCPServer() {
	closesocket(ListenSocket);
}

bool TCPServer::EnableNonBlockAccept(bool state) {
	if (ioctlsocket(ListenSocket, FIONBIO, (u_long*)&state) == -1) {
		HandleSocketError();
		return false;
	}
	return true;
}

TCPServer::Connection_Info* TCPServer::AcceptConnection(sockaddr* addr, int* addrlen, int rw_flag) {
	size_t incoming_socket=accept(ListenSocket, addr, addrlen);
	if (incoming_socket == size_t(-1)) {
		HandleSocketError();
		return nullptr;
	}
	if (ioctlsocket(incoming_socket, FIONBIO, (u_long*)&rw_flag) == -1) {
		HandleSocketError();
		closesocket(incoming_socket);
		return nullptr;
	}
	//push the socket into connection info and set the fields correctly

	//Set the incoming client to read and write polling
	if(rw_flag & 0x01) FD_SET(incoming_socket, &to_read_fd);
	if(rw_flag & 0x02) FD_SET(incoming_socket, &to_write_fd);
}

bool TCPServer::PollConnections(fd_set* read_ready, fd_set* write_ready, fd_set* exception, timeval * timeout) {
	//Can optimize by partial copy, not implemented by now
	if (read_ready) *read_ready = to_read_fd;
	if (write_ready) *write_ready = to_write_fd;
	if (select(max_fd, read_ready, write_ready, exception, timeout) == -1) {
		HandleSocketError();
		return false;
	}
	return true;
}


//Functions in class TCPClient
size_t TCPClient::ConnectServer(addrinfo& hint, const char * domain, const char * service) {
	return size_t();
}
