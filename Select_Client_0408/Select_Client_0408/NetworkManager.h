#pragma once


class NetworkManager
{
public:
	static std::unique_ptr<NetworkManager> sInstance;
private:
	TCPSocketPtr m_sock;
private:
	NetworkManager() { }
	bool Init(const char* inIp, u_int inPort);
public:
	~NetworkManager() { SocketUtil::CleanUp(); }
	static bool StaticInit(const char* inIp, u_int inPort);
public:
	bool DoFrame();
};