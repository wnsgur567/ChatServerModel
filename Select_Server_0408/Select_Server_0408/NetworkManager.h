#pragma once


class NetworkManager
{
	using clientsMap_iter = unordered_map<int, ClientInfoPtr>::iterator;
	using clientsMap_constIter = unordered_map<int, ClientInfoPtr>::const_iterator;
public:
	static std::unique_ptr<NetworkManager> sInstance;
private:
	TCPSocketPtr m_listenSock;

	int m_new_clientID;	 // ������ Ŭ���̾�Ʈ���� �Ҵ�� id value

	unordered_map<int, ClientInfoPtr>			m_IdToClient_map;		// id -> client ptr
	unordered_map<TCPSocketPtr, ClientInfoPtr>	m_SockToClient_map;		// socket -> client ptr
private:
	NetworkManager()
		:m_new_clientID(0), m_writeset_mask(0) { }
	bool Init(u_int inPort);
	void HandleNewClient();								// ���ο� Ŭ���̾�Ʈ ���� �� map �� ���	
	void HandleDisconnectedClient(int inClientID);		// ���� ���� Ŭ���̾�Ʈ ����
	ClientInfoPtr GetClientInfo(int inClientID);		// map ���� id�� �˻�
public:
	NetworkManager(const NetworkManager&) = delete;
	NetworkManager& operator=(const NetworkManager&) = delete;
	~NetworkManager() { SocketUtil::CleanUp(); }
	static bool StaticInit(u_int inPort);
public:
	bool DoFrame();

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

	queue<std::pair<ClientInfoPtr, RecvPacketPtr>> m_recvQueue;		// recv �� complete �� ��鸸 �־���
	queue<std::pair<ClientInfoPtr, SendPacketPtr>> m_sendQueue;		// send ���� ��� ť
	void HandleRecvQueue();	// queue �� ��ϵ� ��Ŷ���� ó��
	void HandleSendQueue();	// queue �� ��ϵ� ��Ŷ���� ó��
public:
	void SetSelectMask(StateType inMask) { m_writeset_mask = inMask; }
};