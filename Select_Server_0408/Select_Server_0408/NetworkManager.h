#pragma once


class NetworkManager
{
	using clientsMap_iter = unordered_map<int, ClientInfoPtr>::iterator;
	using clientsMap_constIter = unordered_map<int, ClientInfoPtr>::const_iterator;
public:
	static std::unique_ptr<NetworkManager> sInstance;
private:
	TCPSocketPtr m_listenSock;

	int m_new_clientID;	 // 다음번 클라이언트에게 할당될 id value

	unordered_map<int, ClientInfoPtr>			m_IdToClient_map;		// id -> client ptr
	unordered_map<TCPSocketPtr, ClientInfoPtr>	m_SockToClient_map;		// socket -> client ptr
private:
	NetworkManager()
		:m_new_clientID(0), m_writeset_mask(0) { }
	bool Init(u_int inPort);
	void HandleNewClient();								// 새로운 클라이언트 생성 및 map 에 등록	
	void HandleDisconnectedClient(int inClientID);		// 연결 끝난 클라이언트 정리
	ClientInfoPtr GetClientInfo(int inClientID);		// map 에서 id로 검색
public:
	NetworkManager(const NetworkManager&) = delete;
	NetworkManager& operator=(const NetworkManager&) = delete;
	~NetworkManager() { SocketUtil::CleanUp(); }
	static bool StaticInit(u_int inPort);
public:
	bool DoFrame();

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

	queue<std::pair<ClientInfoPtr, RecvPacketPtr>> m_recvQueue;		// recv 후 complete 된 놈들만 넣었음
	queue<std::pair<ClientInfoPtr, SendPacketPtr>> m_sendQueue;		// send 보낼 대기 큐
	void HandleRecvQueue();	// queue 에 등록된 패킷들을 처리
	void HandleSendQueue();	// queue 에 등록된 패킷들을 처리
public:
	void SetSelectMask(StateType inMask) { m_writeset_mask = inMask; }
};