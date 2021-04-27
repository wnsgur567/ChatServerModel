#include "base.h"

// ���� �ʿ�
std::unique_ptr<NetworkManager> NetworkManager::sInstance;

bool NetworkManager::StaticInit(u_int inPort)
{
	sInstance.reset(new NetworkManager());
	if (false == SocketUtil::Init())
		return false;
	return sInstance->Init(inPort);
}
bool NetworkManager::Init(u_int inPort)
{
	m_listenSock = SocketUtil::CreateTCPSocket();
	if (m_listenSock == nullptr)
	{
		return false;
	}

	SocketAddress myAddress(htonl(INADDR_ANY), htons(inPort));
	if (false == m_listenSock->Bind(myAddress))
		return false;

	printf("Initializing NetworkManager at port %d", inPort);

	if (false == m_listenSock->Listen(SOMAXCONN))
		return false;

	if (m_listenSock->SetNonBlockingMode(true) == false)
	{
		return false;
	}
	return true;
}


// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //


bool NetworkManager::DoFrame()
{
	// socket set �ʱ�ȭ
	ClearSocketVecs();

	// socket set ������Ʈ
	UpdateReadSockets();
	UpdateWriteSockets();

	// select()
	if (false == SocketUtil::Select(&read_sockets, &result_read_sockets, &write_sockets, &result_write_sockets))
		return false;


#pragma region send_recv_ó����

	// recv ó�� (result_read_sockets)
	// recv() �� �����մϴ�
	// ������� recvpacket�� queue �� ����մϴ�
	for (auto& sock : result_read_sockets)
	{
		if (sock == m_listenSock)
		{	// listen socket �̸�
			// Ŭ���̾�Ʈ ���� ���μ���
			HandleNewClient();
		}
		else
		{	// ������ ó��			
			auto _clientinfo_ptr = m_SockToClient_map[sock];
			RecvState ret_state;

			RecvPacketPtr _recvpacket;
			if (nullptr == _clientinfo_ptr->GetRecvPacketPtr().get())
			{	// ������ ó���ϴ��� ������... 
				// ��Ŷ ���� �İ�
				_recvpacket = std::make_shared<RecvPacket>(_clientinfo_ptr);
				_recvpacket->SetState(RecvState::Size);

				// recv
				ret_state = PacketUtil::PacketRecv(_clientinfo_ptr, _recvpacket);
			}
			else
			{	// �ϴ� �۾��� ������
				_recvpacket = _clientinfo_ptr->GetRecvPacketPtr();

				// recv
				ret_state = PacketUtil::PacketRecv(_clientinfo_ptr, _recvpacket);
			}

			// complete �ȳ�鸸 ť�� �־�
			if (ret_state == RecvState::Completed)
			{
				// ��Ŷ ó�� ��� ť�� �ְ�
				m_recvQueue.push(
					std::pair<ClientInfoPtr, RecvPacketPtr>(_clientinfo_ptr, _recvpacket)
				);

				// Ŭ���̾�Ʈ ������ �ִ� ���� ����
				_clientinfo_ptr->GetRecvPacketPtr().reset();
			}
			else if (ret_state == RecvState::ClientEnd)
			{	// ���� ���������� ������
				// TODO : send queue �� ����ֱ�
				_clientinfo_ptr->SetState(ClientState::Disconnected_send);
			}
		}
	}

	// send ó�� (result_write_sockets)
	// Sendpacket �� ����� queue �� ����մϴ�
	for (auto& sock : result_write_sockets)
	{	// send queue �� ���
		// ��Ŷ ��ü�� recv queue ó���������� �̹� �� ������� ����			
		auto _clientinfo_ptr = m_SockToClient_map[sock];
		SendPacketPtr _sendpacket = _clientinfo_ptr->GetSendPacketPtr();
		if (nullptr != _sendpacket.get())
		{	// ���� frame ���� �������� ���� �ִ� ���
			return false;
		}

		// �� state �� send�� Packet �����
		_sendpacket.reset();
		OutputMemoryStreamPtr _stream = std::make_shared<OutputMemoryStream>();
		switch (_clientinfo_ptr->GetState())
		{
		case ClientState::Standby_send:
			PacketUtil::PackPacket(*_stream, PROTOCOL::Init, MessageMaker::Get_InitMSG());
			_sendpacket = std::make_shared<SendPacket>(_stream, _clientinfo_ptr);
			m_sendQueue.push(
				std::pair<ClientInfoPtr, SendPacketPtr>(_clientinfo_ptr, _sendpacket)
			);

			break;

		case ClientState::UnstableConnection_send:
			// 
			break;

		case ClientState::Disconnected_send:
		{
			const char* msg = MessageMaker::Get_Disconnected();
			PacketUtil::PackPacket(*_stream, PROTOCOL::Disconnect, msg);
			// stream -> packet
			_sendpacket = std::make_shared<SendPacket>(_stream, _clientinfo_ptr);
			// ���� ��Ŷ queue �� ����ֱ�
			m_sendQueue.push(
				std::pair<ClientInfoPtr, SendPacketPtr>(_clientinfo_ptr, _sendpacket)
			);
		}
		break;

		case ClientState::WaitingRoom_send:
		{
			SendChatRoomList(_clientinfo_ptr);
		}
		break;

		case ClientState::EnterChatRoom_send:
		{
			ChatRoomPtr _room_ptr = GetChatRoom(_clientinfo_ptr->GetChatRoomID());
			SendEnterRoomMsg(_room_ptr, _clientinfo_ptr->GetName().c_str());
		}
		break;

		case ClientState::JoinedChatRoom_send:

			break;

		case ClientState::ExitChatRoom_send:
		{
			ChatRoomPtr _room_ptr = GetChatRoom(_clientinfo_ptr->GetChatRoomID());
			SendExitRoomMsg(_room_ptr, _clientinfo_ptr->GetName().c_str());
			_room_ptr->ExitRoom(_clientinfo_ptr);
		}
		break;

		default:
			printf("for : result write sockets // client state switch�� Ȯ��\n");
			break;
		}


	}

#pragma endregion

	// recv queue ó��
	HandleRecvQueue();
	// send queue ó��
	HandleSendQueue();


	return true;
}

// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //

#pragma region Client

ClientInfoPtr NetworkManager::GetClientInfo(int inClientID)
{
	if (m_IdToClient_map.end() == m_IdToClient_map.find(inClientID))
	{	// �˻� ����
		return nullptr;
	}
	// �˻� ����
	return m_IdToClient_map[inClientID];
}

bool NetworkManager::ReqSend(ClientInfoPtr inClient, OutputMemoryStreamPtr inStream)
{
	// TODO : 

	return true;
}

void NetworkManager::HandleNewClient()
{
	// ������ Ŭ���̾�Ʈ ���� ����
	SocketAddress _newAddress;
	TCPSocketPtr _newSock = m_listenSock->Accept(_newAddress);
	if (nullptr == _newSock)
	{
		printf("���������� Ŭ���̾�Ʈ accept ����");
		return;
	}

	// �񵿱� �������� ��ȯ
	if (false == _newSock->SetNonBlockingMode(true))
		return;

	// ���ο� Ŭ���̾�Ʈ ����
	ClientInfoPtr _newClient = std::make_shared<ClientInfo>(_newSock, _newAddress, std::string("None"), m_new_clientID);
	_newAddress.Print("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n");

	// map ���
	m_IdToClient_map.insert(
		std::pair<int, ClientInfoPtr>(m_new_clientID, _newClient)
	);
	m_SockToClient_map.insert(
		std::pair<TCPSocketPtr, ClientInfoPtr>(_newSock, _newClient)
	);

	// Ŭ���̾�Ʈ ���� ����
	_newClient->SetIsConnected(true);
	_newClient->SetState(ClientState::Standby_send);	// �ٷ� �޼��� ��������


	++m_new_clientID;
}

void NetworkManager::HandleDisconnectedClient(int inClientID)
{
	auto iter = m_IdToClient_map.find(inClientID);

	// �˻� ���� (id ����)
	if (m_IdToClient_map.end() == iter)
		return;

	// ���� ���� ����
	iter->second->GetAddress().Print("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n");
	//


	iter->second->SetIsConnected(false);
	iter->second->SetIsSignIn(false);
	iter->second->SetIsJoinChatRoom(false);

	m_SockToClient_map.erase(iter->second->GetTCPSocket());
	m_IdToClient_map.erase(inClientID);
}



#pragma endregion


// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //

#pragma region Select

void NetworkManager::ClearSocketVecs()
{
	read_sockets.clear();
	result_read_sockets.clear();
	write_sockets.clear();
	result_write_sockets.clear();
}

void NetworkManager::UpdateReadSockets()
{
	// accept �� ���� ���
	read_sockets.push_back(m_listenSock);

	// ������ Ŭ���̾�Ʈ�� ��� ���
	for (auto& item : m_IdToClient_map)
	{
		read_sockets.push_back(item.second->GetTCPSocket());
	}

}
void NetworkManager::UpdateWriteSockets()
{	// state �� ���� ���

	// ������ Ŭ���̾�Ʈ �� state �� �´� ��
	for (auto& item : m_IdToClient_map)
	{
		auto masked = (static_cast<StateType>(item.second->GetState()) & m_writeset_mask);
		if (masked)
		{	// send state �̴� writeset�� ����ϵ��� �սô�
			write_sockets.push_back(item.second->GetTCPSocket());
		}
	}
}

// �̹� recv�� �����Ͽ� ������� RecvPacket�� ó���մϴ�
// ��� ó���� �P���� state�� �����մϴ�
void NetworkManager::HandleRecvQueue()
{
	while (false == m_recvQueue.empty())
	{
		// deque
		auto item = m_recvQueue.front();
		auto _clientinfo_ptr = item.first;
		auto _packet = item.second;
		m_recvQueue.pop();

		// ��Ŷ -> stream
		auto _stream = _packet->ToStreamPtr();

		// �������� ���� �з�
		PROTOCOL _protocol = PacketUtil::GetProtocol(*_stream);
		switch (_protocol)
		{
			// TODO : protocol ���� 
		case PROTOCOL::Init:
			// ����
			break;

		case PROTOCOL::EnterChatRoom:
		{
			int selected;
			char buf[BUFSIZE];
			ZeroMemory(buf, BUFSIZE);
			PacketUtil::UnPackPacket(*_stream, selected, buf);
			ChatRoomPtr _room_ptr = GetChatRoom(selected);
			if (nullptr == _room_ptr)
			{
				// TODO : �������� �ʴ� ��
			}
			else
			{
				_clientinfo_ptr->SetName(buf);
				_clientinfo_ptr->SetChatRoomID(selected);
				_room_ptr->EnterRoom(_clientinfo_ptr);

				_clientinfo_ptr->SetState(ClientState::EnterChatRoom_send);
			}
		}
		break;
		case PROTOCOL::Chat:
		{
			char buf[BUFSIZE];
			ZeroMemory(buf, BUFSIZE);
			PacketUtil::UnPackPacket(*_stream, buf);
			ChatRoomPtr _room_ptr = GetChatRoom(_clientinfo_ptr->GetChatRoomID());
			if (nullptr == _room_ptr)
			{
				// TODO : �������� �ʴ� ��
			}
			else
			{
				SendChatMsg(_room_ptr, _clientinfo_ptr->GetName().c_str(), buf);

				_clientinfo_ptr->SetState(ClientState::JoinedChatRoom_send);
			}
		}
		break;
		case PROTOCOL::ExitChatRoom:
			_clientinfo_ptr->SetState(ClientState::ExitChatRoom_send);
			break;
		case PROTOCOL::Disconnect:
			_clientinfo_ptr->SetState(ClientState::Disconnected_send);
			break;
		}
	}
}

// Ŭ���̾�Ʈ���� ���� ������ �����ϴ� �� send()
// ��� �����͸� �����ϸ� state�� �����մϴ�
void NetworkManager::HandleSendQueue()
{
	// ó���� ���� ��Ŷ���� ���� �ӽ� ť
	// ��Ŷ�� ��ȣ�� �ο��ϴ°ɷ� ��ü�� ����
	queue<std::pair<ClientInfoPtr, SendPacketPtr>> tmp_queue;

	while (false == m_sendQueue.empty())
	{
		// deque
		auto item = m_sendQueue.front();
		auto _clientinfo_ptr = item.first;
		auto _packet = item.second;
		m_sendQueue.pop();

		// �̹� ������ �ƴϸ�
		if (false == _clientinfo_ptr->IsSendTurn(_packet->GetID()))
		{
			m_sendQueue.push(tmp_queue.front());
			m_sendQueue.pop();
			continue;
		}

		// recv ���� Ŭ���̾�Ʈ�� ���� ����Ȱ� catch �� ��쵵 ������ ��Ŷ �������
		if (_clientinfo_ptr->GetState() == ClientState::Disconnected_send)
		{
			// ���� ��Ŷ �����
			OutputMemoryStreamPtr _stream = std::make_shared<OutputMemoryStream>();
			PacketUtil::PackPacket(*_stream, PROTOCOL::Disconnect);
			_packet = std::make_shared<SendPacket>(_stream, _clientinfo_ptr);
		}
		
		// send send send !!
		SendState _sendResult = PacketUtil::PacketSend(_clientinfo_ptr, _packet);

		// ������ �Ϸ������
		// State �� ���� ���� State�� �ٲٱ�
		if (SendState::Completed == _sendResult)
		{	// ���δ� ��������
			// change state
			switch (_clientinfo_ptr->GetState())
			{
			case ClientState::Standby_send:
				_clientinfo_ptr->SetState(ClientState::WaitingRoom_send);	// TODO : WaitingRoom_send
				break;

				// send ���� ���Ḧ catch �� ���
			case ClientState::Disconnected_send:
				// ������ �����Ŵ
				HandleDisconnectedClient(_clientinfo_ptr->GetID());
				break;

			case ClientState::WaitingRoom_send:
				_clientinfo_ptr->SetState(ClientState::EnterChatRoom);
				break;

			case ClientState::EnterChatRoom_send:
			case ClientState::JoinedChatRoom_send:
			{
				_clientinfo_ptr->SetState(ClientState::JoinedChatRoom_send);
			}
			break;
			case ClientState::ExitChatRoom_send:
				_clientinfo_ptr->SetState(ClientState::Standby_send);
				break;

			default:
				printf("change state �̵��!!!\n");
				break;
			}
		}
		else
		{	// �������� �ӽ� ť�� ��� ��Ŵ
			tmp_queue.push(item);
		}
	}

	// ������� send queue �� ���� , ���� �����ӿ� �ٽ� ó��
	while (false == tmp_queue.empty())
	{
		m_sendQueue.push(tmp_queue.front());
		m_sendQueue.pop();
	}
}


#pragma endregion

// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //

#pragma region ChatRoom

bool NetworkManager::HandleNewChatRoom(const string& inName)
{
	// �ߺ��� ���� �����Ǿ� ������
	if (m_RoomnameToId_map.find(inName) != m_RoomnameToId_map.end())
		return false;

	ChatRoomPtr _newRoom = ChatRoomUtil::CreateRoom(inName, m_new_chatRoomID, MAX_PARTICIPANT);

	m_RoomnameToId_map[inName] = m_new_chatRoomID;
	m_IdToChatRoom_map[m_new_chatRoomID] = _newRoom;

	++m_new_chatRoomID;
	return true;
}

void NetworkManager::HandleDestroyRoom(const int inChatRoomId)
{
	if (m_IdToChatRoom_map.find(inChatRoomId) == m_IdToChatRoom_map.end())
		return;

	ChatRoomPtr _roomptr = m_IdToChatRoom_map[inChatRoomId];


	m_RoomnameToId_map.erase(_roomptr->GetName());
	m_IdToChatRoom_map.erase(inChatRoomId);
}

void NetworkManager::SendChatRoomList(ClientInfoPtr inClient)
{
	OutputMemoryStreamPtr _stream = std::make_shared<OutputMemoryStream>();

	char msg[BUFSIZE];
	ZeroMemory(msg, BUFSIZE);
	vector<const char*> _list;

	for (auto& item : m_RoomnameToId_map)
	{
		_list.push_back(item.first.c_str());
	}

	MessageMaker::Get_ChatRoomList(msg, _list);
	PacketUtil::PackPacket(*_stream, PROTOCOL::WaitingRoom_Menu, msg);
	// _stream -> packet
	SendPacketPtr _sendpacket = std::make_shared<SendPacket>(_stream, inClient);

	// sendqueue�� ���
	m_sendQueue.push(
		std::pair<ClientInfoPtr, SendPacketPtr>(inClient, _sendpacket)
	);

}


void NetworkManager::SendEnterRoomMsg(ChatRoomPtr inChatRoom, const char* inName)
{
	char msg[BUFSIZE];
	ZeroMemory(msg, BUFSIZE);
	MessageMaker::Get_EnterRoomMsg(msg, inName);

	// sendqueue�� ���
	for (auto _clientinfo_ptr : inChatRoom->m_participants)
	{
		OutputMemoryStreamPtr _stream = std::make_shared<OutputMemoryStream>();
		PacketUtil::PackPacket(*_stream, PROTOCOL::EnterChatRoom, msg);
		// _stream -> packet
		SendPacketPtr _sendpacket = std::make_shared<SendPacket>(_stream, _clientinfo_ptr);
		m_sendQueue.push(
			std::pair<ClientInfoPtr, SendPacketPtr>(_clientinfo_ptr, _sendpacket)
		);
	}
}

void NetworkManager::SendExitRoomMsg(ChatRoomPtr inChatRoom, const char* inName)
{
	char msg[BUFSIZE];
	ZeroMemory(msg, BUFSIZE);
	MessageMaker::Get_ExitRoomMsg(msg, inName);


	// sendqueue�� ���

	for (auto _clientinfo_ptr : inChatRoom->m_participants)
	{
		OutputMemoryStreamPtr _stream = std::make_shared<OutputMemoryStream>();
		PacketUtil::PackPacket(*_stream, PROTOCOL::ExitChatRoom, msg);

		// _stream -> packet
		SendPacketPtr _sendpacket = std::make_shared<SendPacket>(_stream,_clientinfo_ptr);
		m_sendQueue.push(
			std::pair<ClientInfoPtr, SendPacketPtr>(_clientinfo_ptr, _sendpacket)
		);
	}
}

void NetworkManager::SendChatMsg(ChatRoomPtr inChatRoom, const char* inName, const char* inMsg)
{
	char msg[BUFSIZE];
	ZeroMemory(msg, BUFSIZE);
	MessageMaker::Get_ChatMsg(msg, inName, inMsg);


	// sendqueue�� ���
	for (auto _clientinfo_ptr : inChatRoom->m_participants)
	{
		OutputMemoryStreamPtr _stream = std::make_shared<OutputMemoryStream>();
		PacketUtil::PackPacket(*_stream, PROTOCOL::Chat, msg);

		// _stream -> packet
		SendPacketPtr _sendpacket = std::make_shared<SendPacket>(_stream, _clientinfo_ptr);
		m_sendQueue.push(
			std::pair<ClientInfoPtr, SendPacketPtr>(_clientinfo_ptr, _sendpacket)
		);
	}
}



ChatRoomPtr NetworkManager::GetChatRoom(const int inChatRoomID)
{
	if (m_IdToChatRoom_map.find(inChatRoomID) == m_IdToChatRoom_map.end())
		return nullptr;
	return m_IdToChatRoom_map[inChatRoomID];
}

bool NetworkManager::TestInitializeChatRooms()
{
	HandleNewChatRoom("���");
	HandleNewChatRoom("����");
	HandleNewChatRoom("���");

	return true;
}

#pragma endregion
