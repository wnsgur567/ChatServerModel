#pragma once

class ClientInfo
{
private:
	ClientState				 m_state;

	TCPSocketPtr			 m_sock;
	SocketAddress			 m_sockaddr;

	string					 m_name;
	int						 m_clientID;	// client ���� id��

	int						 m_chatRoomID;	// ���� �������ִ� ä�� room id

	bool m_isSignIn;			// ���� �α��� �Ǿ��ִ� ��������
	bool m_isConnected;			// ���� ����Ǿ��ִ� �������� (Accept() ����)
	bool m_isJoinChatRoom;		// ä�ù濡 ������ ��������		

public:
	ClientInfo(TCPSocketPtr inSocket, const SocketAddress& inSocketAddress, const string& inName, const int inPlayerID);
	TCPSocketPtr		GetTCPSocket()						{ return m_sock; }
	SocketAddress		GetAddress() const					{ return m_sockaddr; }
	SocketAddress&		GetAddress()						{ return m_sockaddr; }

	int					GetID() const						{ return m_clientID; }
	string				GetName() const						{ return m_name; }
	void				SetName(string inNmae)				{ m_name = inNmae; } 
	int					GetChatRoomID() const				{ return m_chatRoomID; }
	void				SetChatRoomID(int inID)				{ m_chatRoomID = inID; }
	bool				IsSignIn() const					{ return m_isSignIn; }
	void				SetIsSignIn(bool isSignIn)			{ m_isSignIn = isSignIn; }
	bool				IsConnected() const					{ return m_isConnected; }
	void				SetIsConnected(bool isConnected)	{ m_isConnected = isConnected; }
	bool				IsJoinChatRoom() const				{ return m_isJoinChatRoom; }
	void				SetIsJoinChatRoom(bool isJoin)		{ m_isJoinChatRoom = isJoin; }

	ClientState			GetState()							{ return m_state; }
	void				SetState(ClientState inState)		{ m_state = inState; }

private:
	RecvPacketPtr m_recv_buf;	// ���� �����ӿ��� �� ó�� ������ ��� ����� �ӽ� ������
	SendPacketPtr m_send_buf;	// ���� �����ӿ��� �� ó�� ������ ��� ����� �ӽ� ������
public:
	RecvPacketPtr		GetRecvPacketPtr()					{ return m_recv_buf; }
	RecvPacketPtr		GetRecvPacketPtr() const			{ return m_recv_buf; }
	bool				IsRemainPreviousRecv() const;		// ���� �����ӿ��� �� �����³�?
	SendPacketPtr		GetSendPacketPtr()					{ return m_send_buf; }
	SendPacketPtr		GetSendPacketPtr() const			{ return m_send_buf; }
	bool				IsRemainPreviousSend() const;		// ���� �����ӿ��� �� �����³�?
};