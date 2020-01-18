#pragma once

#include <cstdint>
#include <fstream>

class InStream {
public:
	//read bytes in the stream, returns the actual read bytes
	//return value's first byte indicate stream ended.
	//the stream source is responsible for good buffering
	//(typically set at runtime) and composing the results.
	virtual uint64_t readBytes(void* dest, uint64_t bytes) = 0;
	//may fill the first bit to indicate out of range(file on disk, known length) 
	//or unavailable (network not recieved or discarded from cache)
	virtual uint64_t seekReadBytes(void* dest, int64_t seek, uint64_t bytes) = 0;
	//The number of bytes available without blocking or ending
	virtual uint64_t bytesAvail() = 0;
	//The previous number of bytes available without blocking or ending
	virtual uint64_t bytesCached() = 0;
	//These operations may block due to locks
	virtual uint64_t setMaxAvail(uint64_t bytes) = 0;
	virtual uint64_t setMaxCached(uint64_t bytes) = 0;
	//Read operation to network and disk should only be done here.
	virtual uint64_t fetchData(uint64_t bytes) = 0;
protected:
	//Typically, implement a pointer array cyclic-lock-free-queue to
	//4k aligned buffers that holds the actual data (also ensure allocator
	//allocates from ram pool, if desired)
	uint64_t MaxAvailBytes;
	uint64_t MaxCachedBytes;
	uint64_t CurAvailBytes;
	uint64_t CurCachedBytes;
	InStream(uint64_t MaxAvail, uint64_t MaxCached):
		MaxAvailBytes(MaxAvail), MaxCachedBytes(MaxCached),
		CurAvailBytes(0), CurCachedBytes(0)
	{}
	virtual ~InStream() {}
};

//simplest implementation, only implement all the interfaces, but
//does not conform to the realtime specifications
class SlowFileStream: public InStream {
public:
	SlowFileStream(char* filename, size_t& err):InStream(~0, ~0)
	{
		err = 0;
		ifs.open(filename, std::ios::binary);
		if (!ifs) {
			err = 1;
			return;
		}
		ifs.seekg(0, std::ios::end);
		MaxAvailBytes = ifs.tellg();
		MaxCachedBytes = MaxAvailBytes;
		CurAvailBytes = MaxAvailBytes;
		CurCachedBytes = 0;
		ifs.seekg(0);
	}
	~SlowFileStream()
	{
		ifs.close();
	}
	virtual uint64_t readBytes(void* dest, uint64_t bytes) override
	{
		uint64_t left = bytes;
		do {
			uint64_t read = ifs.readsome((char*)dest, left);
			if (read != left) left -= read;
		} while (left && ifs);
		CurAvailBytes -= bytes - left;
		CurCachedBytes += bytes - left;
		return bytes - left;
	}
	virtual uint64_t seekReadBytes(void* dest, int64_t seek, uint64_t bytes) override
	{
		size_t cur = ifs.tellg();
		ifs.seekg(seek, std::ios::cur);
		uint64_t left = bytes;
		do {
			uint64_t read = ifs.readsome((char*)dest, left);
			if (read != left) left -= read;
		} while (left && ifs);
		ifs.seekg(-seek - (bytes - left), std::ios::cur);
		return bytes - left;
	}
	virtual uint64_t setMaxAvail(uint64_t bytes) override {};
	virtual uint64_t setMaxCached(uint64_t bytes) override {};
	virtual uint64_t bytesAvail() override
	{
		return CurAvailBytes;
	};
	virtual uint64_t bytesCached() override
	{
		return CurCachedBytes;
	};
	virtual uint64_t fetchData(uint64_t bytes) override {};
	std::ifstream ifs;
};

