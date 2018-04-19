#ifndef MS_UNIX_STREAM_SOCKET_HPP
#define MS_UNIX_STREAM_SOCKET_HPP

#include "common.hpp"
#include <uv.h>
#include "uv_callback.h"
// /home/globik/kore-mediasoup/training/luvi/l/libuv_callback.so
#include <string>
struct fucker{int a;};
class UnixStreamSocket
{
public:
	/* Struct for the data field of uv_req_t when writing data. */
	struct UvWriteData
	{
		UnixStreamSocket* socket{ nullptr };
		uv_write_t req;
		uint8_t store[1];
		int m;
	};

public:
	UnixStreamSocket(int fd, size_t bufferSize);
	UnixStreamSocket& operator=(const UnixStreamSocket&) = delete;
	UnixStreamSocket(const UnixStreamSocket&)            = delete;

protected:
	virtual ~UnixStreamSocket();

public:
	void Destroy();
	bool IsClosing() const;
	void Write(const uint8_t* data, size_t len);
	void Write(const std::string& data);
	uv_callback_t to_cpp;
	//static 
		void * on_to_cpp(uv_callback_t *callback,void*data);
	//char*dt;

	/* Callbacks fired by UV events. */
public:
	void OnUvReadAlloc(size_t suggestedSize, uv_buf_t* buf);
	void OnUvRead(ssize_t nread, const uv_buf_t* buf);
	void fuck(int);
	void OnUvWriteError(int error);
	void OnUvShutdown(uv_shutdown_t* req, int status);
	void OnUvClosed();
	int b;
	struct fucker*mina{ nullptr };

	/* Pure virtual methods that must be implemented by the subclass. */
protected:
	//virtual
		void UserOnUnixStreamRead();//                            = 0;
	virtual void UserOnUnixStreamSocketClosed(bool isClosedByPeer) = 0;

private:
	// Allocated by this.
	uv_pipe_t* uvHandle{ nullptr };
	//struct fucker*mina{ nullptr };
	// Others.
	bool isClosing{ false };
	bool isClosedByPeer{ false };
	bool hasError{ false };
	//uv_callback_t to_cpp;

protected:
	// Passed by argument.
	//struct fucker*mina{ nullptr };
	size_t bufferSize{ 0 };
	// Allocated by this.
	uint8_t* buffer{ nullptr };
	// Others.
	size_t bufferDataLen{ 0 };
	char*dt=NULL;
	int a;
};

/* Inline methods. */

inline bool UnixStreamSocket::IsClosing() const
{
	return this->isClosing;
}

inline void UnixStreamSocket::Write(const std::string& data)
{
	Write(reinterpret_cast<const uint8_t*>(data.c_str()), data.size());
}

#endif
