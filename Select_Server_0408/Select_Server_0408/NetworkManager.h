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
	// select 관련 변수 및 메소드
private:
	StateType			 m_writeset_mask;			// write set에 등록할 state mask

	vector<TCPSocketPtr> read_sockets;				// input  read
	vector<TCPSocketPtr> result_read_sockets;		// output read
	vector<TCPSocketPtr> write_sockets;				// input  write
	vector<TCPSocketPtr> result_write_sockets;		// ouput  write
	void ClearSocketVecs();
	void UpdateReadSockets();
	void UpdateWriteSockets();

	// TODO : 패킷에도 클라이언트별로 ID 값을 부여해서 순서를 체크 할 수 있도록 합시다
	// 지금은 못할듯

	queue<std::pair<ClientInfoPtr, RecvPacketPtr>> m_recvQueue;		// recv 후 complete 된 놈들만 넣었음
	queue<std::pair<ClientInfoPtr, SendPacketPtr>> m_sendQueue;		// send 보낼 대기 큐
	void HandleRecvQueue();	// queue 에 등록된 패킷들을 처리
	void HandleSendQueue();	// queue 에 등록된 패킷들을 처리
public:
	void SetSelectMask(StateType inMask) { m_writeset_mask = inMask; }

	//
	// Client 관련 변수 및 메소드
private:
	int m_new_clientID;													// 다음번 클라이언트에게 할당될 id value
	unordered_map<int, ClientInfoPtr>			m_IdToClient_map;		// id -> client ptr
	unordered_map<TCPSocketPtr, ClientInfoPtr>	m_SockToClient_map;		// socket -> client ptr
private:
	void HandleNewClient();												// 새로운 클라이언트 생성 및 map 에 등록	
	void HandleDisconnectedClient(int inClientID);						// 연결 끝난 클라이언트 정리
public:
	ClientInfoPtr GetClientInfo(int inClientID);						// map 에서 id로 검색
	bool		  ReqSend(ClientInfoPtr inClient,OutputMemoryStreamPtr inStream);	// 해당 client에게 보낼 패킷을 만들어 큐에 등록
	//
	// chatting room 관련 변수 및 메소드
private:
	int m_new_chatRoomID;	// 다음번 채팅룸에게 할당될 id

	unordered_map<string, int>		m_RoomnameToId_map;		// name -> id
	unordered_map<int, ChatRoomPtr> m_IdToChatRoom_map;		// id -> chatroom ptr
private:
	bool HandleNewChatRoom(const string& inName);			// 새로운 채팅방 개설 및 등록
															// 이미 존재하는 방이면 false return
	void HandleDestroyRoom(const int inChatRoomId);			// 채팅방 정리 및 파괴

	void SendChatRoomList(ClientInfoPtr inClient);

	// void SendAll();													// 모든 방 참가자에게 send	
	void SendEnterRoomMsg(ChatRoomPtr inChatRoom, const char* inName);	// 해당 방 참가자에게 send
	void SendExitRoomMsg(ChatRoomPtr inChatRoom, const char* inName);	// 해당 방 참가자에게 send
	void SendChatMsg(ChatRoomPtr inChatRoom, const char* inName, const char* inMsg);
	// void SendRoom(const int inChatRoomId, PROTOCOL inProtocol, const char* msg);	// 해당 방 참가자에게 send
public:
	ChatRoomPtr GetChatRoom(const int inChatRoomID);
	bool TestInitializeChatRooms();		// 임의로 방을 개설
};