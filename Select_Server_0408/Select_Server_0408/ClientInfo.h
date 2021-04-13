#pragma once

class ClientInfo
{
private:
	ClientState				 m_state;

	TCPSocketPtr			 m_sock;
	SocketAddress			 m_sockaddr;

	string					 m_name;
	int						 m_clientID;	// client 고유 id값

	int						 m_chatRoomID;	// 현재 접속해있는 채팅 room id

	bool m_isSignIn;			// 현재 로그인 되어있는 상태인지
	bool m_isConnected;			// 현재 연결되어있는 상태인지 (Accept() 이후)
	bool m_isJoinChatRoom;		// 채팅방에 입장한 상태인지		

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
	RecvPacketPtr m_recv_buf;	// 지난 프레임에서 다 처리 못했을 경우 여기다 임시 보관함
	SendPacketPtr m_send_buf;	// 지난 프레임에서 다 처리 못했을 경우 여기다 임시 보관함
public:
	RecvPacketPtr		GetRecvPacketPtr()					{ return m_recv_buf; }
	RecvPacketPtr		GetRecvPacketPtr() const			{ return m_recv_buf; }
	bool				IsRemainPreviousRecv() const;		// 지난 프레임에서 다 못보냈냐?
	SendPacketPtr		GetSendPacketPtr()					{ return m_send_buf; }
	SendPacketPtr		GetSendPacketPtr() const			{ return m_send_buf; }
	bool				IsRemainPreviousSend() const;		// 지난 프레임에서 다 못보냈냐?
};