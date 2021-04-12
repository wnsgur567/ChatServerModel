#pragma once

class SocketAddress
{
private:
	sockaddr_in m_sockaddr;
public:
	sockaddr_in& GetSockAddr()
	{
		return m_sockaddr;
	}
	const sockaddr_in& GetSockAddr() const
	{
		return m_sockaddr;
	}
	int GetAddrSize()
	{
		return sizeof(m_sockaddr);
	}
	int GetAddrSize() const
	{
		return sizeof(m_sockaddr);
	}
public:
	SocketAddress() 
	{ 
		ZeroMemory(&m_sockaddr, sizeof(m_sockaddr));
		m_sockaddr.sin_family = AF_INET;
	}
	// port 는 htons 처리한 값을 넣기 (내부서 따로 처리 안함)
	SocketAddress(const u_long inAddress, const u_short inPort)
	{
		ZeroMemory(&m_sockaddr, sizeof(m_sockaddr));
		m_sockaddr.sin_family = AF_INET;
		m_sockaddr.sin_port = inPort;
		m_sockaddr.sin_addr.s_addr = inAddress;
	}
	SocketAddress(const SocketAddress& inOtherAddr)
	{
		ZeroMemory(&m_sockaddr, sizeof(m_sockaddr));
		m_sockaddr.sin_family = inOtherAddr.m_sockaddr.sin_family;
		m_sockaddr.sin_port = inOtherAddr.m_sockaddr.sin_port;
		m_sockaddr.sin_addr.s_addr = inOtherAddr.m_sockaddr.sin_addr.s_addr;
	}

	bool operator==(const SocketAddress& inOther) const
	{
		if (m_sockaddr.sin_family == inOther.m_sockaddr.sin_family &&
			m_sockaddr.sin_addr.s_addr == inOther.m_sockaddr.sin_addr.s_addr &&
			m_sockaddr.sin_port == inOther.m_sockaddr.sin_port)
			return true;

		return false;
	}

	// addr 과 port 출력
	// ex ) "\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n"
	void Print(const char* format) const
	{
		printf(format,
			inet_ntoa(m_sockaddr.sin_addr), ntohs(m_sockaddr.sin_port));
	}
};