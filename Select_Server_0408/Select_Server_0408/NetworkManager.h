#pragma once


class NetworkManager
{
	using clientsMap_iter = unordered_map<int, ClientInfoPtr>::iterator;
	using clientsMap_constIter = unordered_map<int, ClientInfoPtr>::const_iterator;
public:
	static std::unique_ptr<NetworkManager> sInstance;
private:
	TCPSocketPtr m_listenSock;	
private:
	NetworkManager()
		:m_new_clientID(0), m_writeset_mask(0), m_new_chatRoomID(0) { }
	bool Init(u_int inPort);	
public:
	NetworkManager(const NetworkManager&) = delete;
	NetworkManager& operator=(const NetworkManager&) = delete;
	~NetworkManager() { SocketUtil::CleanUp(); }
	static bool StaticInit(u_int inPort);
public:
	bool DoFrame();	

	//
	// select ���� ���� �� �޼ҵ�
private:
	StateType			 m_writeset_mask;			// write set�� ����� state mask

	vector<TCPSocketPtr> read_sockets;				// input  read
	vector<TCPSocketPtr> result_read_sockets;		// output read
	vector<TCPSocketPtr> write_sockets;				// input  write
	vector<TCPSocketPtr> result_write_sockets;		// ouput  write
	void ClearSocketVecs();
	void UpdateReadSockets();
	void UpdateWriteSockets();

	// TODO : ��Ŷ���� Ŭ���̾�Ʈ���� ID ���� �ο��ؼ� ������ üũ �� �� �ֵ��� �սô�
	// ������ ���ҵ�

	queue<std::pair<ClientInfoPtr, RecvPacketPtr>> m_recvQueue;		// recv �� complete �� ��鸸 �־���
	queue<std::pair<ClientInfoPtr, SendPacketPtr>> m_sendQueue;		// send ���� ��� ť
	void HandleRecvQueue();	// queue �� ��ϵ� ��Ŷ���� ó��
	void HandleSendQueue();	// queue �� ��ϵ� ��Ŷ���� ó��
public:
	void SetSelectMask(StateType inMask) { m_writeset_mask = inMask; }

	//
	// Client ���� ���� �� �޼ҵ�
private:
	int m_new_clientID;													// ������ Ŭ���̾�Ʈ���� �Ҵ�� id value
	unordered_map<int, ClientInfoPtr>			m_IdToClient_map;		// id -> client ptr
	unordered_map<TCPSocketPtr, ClientInfoPtr>	m_SockToClient_map;		// socket -> client ptr
private:
	void HandleNewClient();												// ���ο� Ŭ���̾�Ʈ ���� �� map �� ���	
	void HandleDisconnectedClient(int inClientID);						// ���� ���� Ŭ���̾�Ʈ ����
public:
	ClientInfoPtr GetClientInfo(int inClientID);						// map ���� id�� �˻�
	bool		  ReqSend(ClientInfoPtr inClient,OutputMemoryStreamPtr inStream);	// �ش� client���� ���� ��Ŷ�� ����� ť�� ���
	//
	// chatting room ���� ���� �� �޼ҵ�
private:
	int m_new_chatRoomID;	// ������ ä�÷뿡�� �Ҵ�� id

	unordered_map<string, int>		m_RoomnameToId_map;		// name -> id
	unordered_map<int, ChatRoomPtr> m_IdToChatRoom_map;		// id -> chatroom ptr
private:
	bool HandleNewChatRoom(const string& inName);			// ���ο� ä�ù� ���� �� ���
															// �̹� �����ϴ� ���̸� false return
	void HandleDestroyRoom(const int inChatRoomId);			// ä�ù� ���� �� �ı�

	void SendChatRoomList(ClientInfoPtr inClient);

	// void SendAll();													// ��� �� �����ڿ��� send	
	void SendEnterRoomMsg(ChatRoomPtr inChatRoom, const char* inName);	// �ش� �� �����ڿ��� send
	void SendExitRoomMsg(ChatRoomPtr inChatRoom, const char* inName);	// �ش� �� �����ڿ��� send
	void SendChatMsg(ChatRoomPtr inChatRoom, const char* inName, const char* inMsg);
	// void SendRoom(const int inChatRoomId, PROTOCOL inProtocol, const char* msg);	// �ش� �� �����ڿ��� send
public:
	ChatRoomPtr GetChatRoom(const int inChatRoomID);
	bool TestInitializeChatRooms();		// ���Ƿ� ���� ����
};