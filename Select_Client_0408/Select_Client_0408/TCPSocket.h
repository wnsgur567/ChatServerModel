#pragma once

class TCPSocket
{
private:
	SOCKET m_socket;
public:
	TCPSocket(SOCKET inSocket);
	~TCPSocket();

	SOCKET			GetSock() const;

	bool			Bind(const SocketAddress& inAddress);
	bool			Listen(int inClients);
	TCPSocketPtr	Accept(SocketAddress& outAddress);
	bool			Connect(const SocketAddress& inServerAddr);
	bool			SetNonBlockingMode(bool inIsNonBlock);
public:
	int				Send(const void* inData, int inLen);
	int				Recv(void* outData, int inLen);
	bool			operator==(const TCPSocket& inOther);
};