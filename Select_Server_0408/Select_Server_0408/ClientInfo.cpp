#include "base.h"


ClientInfo::ClientInfo(TCPSocketPtr inSocket, const SocketAddress& inSocketAddress, const string& inName, const int inClientID)
	: m_state(ClientState::None),
	m_sock(inSocket),
	m_sockaddr(inSocketAddress),
	m_new_SendPacketID(0),
	m_next_SendPacketID(0),
	m_new_RecvPacketID(0),
	m_next_RecvPacketID(0),
	m_name(inName),
	m_clientID(inClientID),
	m_isSignIn(false), m_isConnected(false), m_isJoinChatRoom(false),
	m_recv_buf(nullptr), m_send_buf(nullptr)
{

}


bool ClientInfo::IsRemainPreviousRecv() const
{
	if (nullptr == m_recv_buf.get())
		return false;
	return true;
}
bool ClientInfo::IsRemainPreviousSend() const
{
	if (nullptr == m_send_buf.get())
		return false;
	return true;
}

bool ClientInfo::IsSendTurn(unsigned int inPacketID)
{
	if (inPacketID == m_next_SendPacketID)
		return true;
	return false;
}
bool ClientInfo::IsRecvTurn(unsigned int inPacketID)
{
	if (inPacketID == m_next_RecvPacketID)
		return true;
	return false;
}