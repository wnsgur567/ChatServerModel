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
	// port �� htons ó���� ���� �ֱ� (���μ� ���� ó�� ����)
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

	// addr �� port ���
	// ex ) "\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n"
	void Print(const char* format) const
	{
		printf(format,
			inet_ntoa(m_sockaddr.sin_addr), ntohs(m_sockaddr.sin_port));
	}
};