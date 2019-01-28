#pragma once

#include "Defines.h"
#include "Platform.h"

#include <cstdint>
#include <mutex>
#include <atomic>

//SOCKET is defined on Windows to be UINT_PTR, which is then defined as unsigned __int64 on
//Win64 and as unsigned int on Win32. Seems like the same objective as size_t in standard C++
//Also, this standardizes the invalid socket check on both BSD sockets and winsockets, Except
//in cases when size_t is defined as unsigned char or unsigned short, but those small systems
//with such small memory (maximum 66k) would not be using standard BSD sockets and C++ for 
//development anyway.

class NetworkManager;

class TCPServer {
public:
	friend NetworkManager;
	TCPServer() = delete;
	TCPServer(const TCPServer&) = delete;
	TCPServer& operator=(const TCPServer&) = delete;
	TCPServer(TCPServer&&) = default;
	TCPServer& operator=(TCPServer&&) = default;
	~TCPServer();
	bool isValid() {
		return ListenSocket!=size_t(-1) 
			&& service
			&& addr && max_queue;
	}
	bool EnableNonBlockAccept(bool state = true);
	bool EnableNonBlockIO(size_t index, bool state = true);

	struct Connection_Info {
		size_t index = -1;
		size_t socket = -1;
		//for keeping track of disconnecting and connecting
		std::atomic<size_t> revision = 0;
		//Connection can only be made form one thread at a time
		std::mutex mtx;
		bool blocking_mode = true;
		bool in_use = false;
		//other data specific to one connection stored here.
	};
	struct DataBlob {
		size_t size = 0;
		const char* data = nullptr;
	};

	//Flags:
	//0x00: Not enable non-block io
	//0x01: read
	//0x02: write
	//Other bits set but not the previous:
	//Enable non-block but not register within the server class
	Connection_Info* AcceptConnection(sockaddr* addr = nullptr, int* addrlen = nullptr, int rw_flag = 0x03);
	bool CloseConnection(size_t index);
	bool ReadConnection(size_t index, DataBlob& buffer);
	bool WriteConnection(size_t index, DataBlob& buffer);
	//calls select() with internally set "to_read_fd" and "to_write_fd", 
	//writes to the location given by user, other parameters are used directly
	bool PollConnections(fd_set* read_ready, fd_set* write_ready, fd_set* exception, timeval* timeout);
	fd_set to_read_fd;
	fd_set to_write_fd;
	size_t max_fd;
protected:
	TCPServer(size_t LSocket, const char* service, const addrinfo* const addri, const size_t queue_size);
private:
	const size_t ListenSocket;
	const char* const service;
	const addrinfo* addr;
	const size_t max_queue;
	//mutex on connection info read/write is needed if the Server is used on multiple threads
	std::mutex mtx;
	Connection_Info* Connections = nullptr;
	size_t connect_count = 0;
	size_t connect_cache = 0;
	size_t* disconnected = nullptr;
	size_t disconnect_count = 0;
};

class TCPClient {
public:
	friend NetworkManager;
	TCPClient(const TCPClient&) = delete;
	TCPClient& operator=(const TCPClient&) = delete;
	TCPClient(TCPClient&&) = default;
	TCPClient& operator=(TCPClient&&) = default;
	size_t ConnectServer(addrinfo& hint, const char* domain, const char* service);
	struct Connection_Info {
		size_t index = -1;
		size_t socket = -1;
		//other data specific to one connection stored here.
	};
protected:
	TCPClient() = default;
private:
	Connection_Info* Connections = nullptr;
	size_t connect_count = 0;
	size_t connect_cache = 0;
	size_t* disconnected = nullptr;
	size_t disconnect_count = 0;
};


class NetworkManager {
private:
	NetworkManager(char version_major, char version_minor);
	~NetworkManager();
	WSADATA WinSocketInfo;
public:
	static TCPServer CreateServer(addrinfo& hint, const char* domain, const char* service, size_t queue_size = SOMAXCONN);
	static TCPClient CreateClient();
	static NetworkManager& GetInstance(char version_major = 2, char version_minor = 0) {
		static NetworkManager manager = NetworkManager(version_major,version_minor);
		return manager;
	}
};


