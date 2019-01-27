#include "NetworkManager.h"

#include <iostream>

int main() {
	addrinfo hints;
	memset(&hints,0,sizeof(addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	TCPServer server = NetworkManager::GetInstance().CreateServer(hints,"127.0.0.1", "HTTP");
	if (!server.isValid()) {
		std::cerr<<"Server creation failed!\nEnding execution!\n"<<std::endl;
		exit(EXIT_FAILURE);
	}
	if (!server.EnableNonBlockAccept()) {
		std::cerr << "Server non-block listening failed!\nEnding execution!\n" << std::endl;
		exit(EXIT_FAILURE);
	}
	system("pause");
	return 0;
}



















