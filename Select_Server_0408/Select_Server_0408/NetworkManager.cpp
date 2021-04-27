#include "base.h"

// 선언 필요
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
	// socket set 초기화
	ClearSocketVecs();

	// socket set 업데이트
	UpdateReadSockets();
	UpdateWriteSockets();

	// select()
	if (false == SocketUtil::Select(&read_sockets, &result_read_sockets, &write_sockets, &result_write_sockets))
		return false;


#pragma region send_recv_처리부

	// recv 처리 (result_read_sockets)
	// recv() 를 수행합니다
	// 만들어진 recvpacket을 queue 에 등록합니다
	for (auto& sock : result_read_sockets)
	{
		if (sock == m_listenSock)
		{	// listen socket 이면
			// 클라이언트 생성 프로세스
			HandleNewClient();
		}
		else
		{	// 데이터 처리			
			auto _clientinfo_ptr = m_SockToClient_map[sock];
			RecvState ret_state;

			RecvPacketPtr _recvpacket;
			if (nullptr == _clientinfo_ptr->GetRecvPacketPtr().get())
			{	// 이전에 처리하던게 없으면... 
				// 패킷 새로 파고
				_recvpacket = std::make_shared<RecvPacket>(_clientinfo_ptr);
				_recvpacket->SetState(RecvState::Size);

				// recv
				ret_state = PacketUtil::PacketRecv(_clientinfo_ptr, _recvpacket);
			}
			else
			{	// 하던 작업이 있으면
				_recvpacket = _clientinfo_ptr->GetRecvPacketPtr();

				// recv
				ret_state = PacketUtil::PacketRecv(_clientinfo_ptr, _recvpacket);
			}

			// complete 된놈들만 큐에 넣어
			if (ret_state == RecvState::Completed)
			{
				// 패킷 처리 대기 큐에 넣고
				m_recvQueue.push(
					std::pair<ClientInfoPtr, RecvPacketPtr>(_clientinfo_ptr, _recvpacket)
				);

				// 클라이언트 정보에 있는 버퍼 비우기
				_clientinfo_ptr->GetRecvPacketPtr().reset();
			}
			else if (ret_state == RecvState::ClientEnd)
			{	// 연결 끊어졌으니 제거해
				// TODO : send queue 에 집어넣기
				_clientinfo_ptr->SetState(ClientState::Disconnected_send);
			}
		}
	}

	// send 처리 (result_write_sockets)
	// Sendpacket 을 만들어 queue 에 등록합니다
	for (auto& sock : result_write_sockets)
	{	// send queue 에 등록
		// 패킷 자체는 recv queue 처리과정에서 이미 다 만들어져 있음			
		auto _clientinfo_ptr = m_SockToClient_map[sock];
		SendPacketPtr _sendpacket = _clientinfo_ptr->GetSendPacketPtr();
		if (nullptr != _sendpacket.get())
		{	// 지난 frame 에서 보내던게 남아 있는 경우
			return false;
		}

		// 각 state 별 send할 Packet 만들기
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
			// 만든 패킷 queue 에 집어넣기
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
			printf("for : result write sockets // client state switch문 확인\n");
			break;
		}


	}

#pragma endregion

	// recv queue 처리
	HandleRecvQueue();
	// send queue 처리
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
	{	// 검색 실패
		return nullptr;
	}
	// 검색 성공
	return m_IdToClient_map[inClientID];
}

bool NetworkManager::ReqSend(ClientInfoPtr inClient, OutputMemoryStreamPtr inStream)
{
	// TODO : 

	return true;
}

void NetworkManager::HandleNewClient()
{
	// 접속한 클라이언트 소켓 생성
	SocketAddress _newAddress;
	TCPSocketPtr _newSock = m_listenSock->Accept(_newAddress);
	if (nullptr == _newSock)
	{
		printf("비정상적인 클라이언트 accept 실패");
		return;
	}

	// 비동기 소켓으로 전환
	if (false == _newSock->SetNonBlockingMode(true))
		return;

	// 새로운 클라이언트 생성
	ClientInfoPtr _newClient = std::make_shared<ClientInfo>(_newSock, _newAddress, std::string("None"), m_new_clientID);
	_newAddress.Print("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n");

	// map 등록
	m_IdToClient_map.insert(
		std::pair<int, ClientInfoPtr>(m_new_clientID, _newClient)
	);
	m_SockToClient_map.insert(
		std::pair<TCPSocketPtr, ClientInfoPtr>(_newSock, _newClient)
	);

	// 클라이언트 정보 셋팅
	_newClient->SetIsConnected(true);
	_newClient->SetState(ClientState::Standby_send);	// 바로 메세지 보내도록


	++m_new_clientID;
}

void NetworkManager::HandleDisconnectedClient(int inClientID)
{
	auto iter = m_IdToClient_map.find(inClientID);

	// 검색 실패 (id 없음)
	if (m_IdToClient_map.end() == iter)
		return;

	// 정리 정리 정리
	iter->second->GetAddress().Print("\n[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n");
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
	// accept 용 소켓 등록
	read_sockets.push_back(m_listenSock);

	// 접속한 클라이언트들 모두 등록
	for (auto& item : m_IdToClient_map)
	{
		read_sockets.push_back(item.second->GetTCPSocket());
	}

}
void NetworkManager::UpdateWriteSockets()
{	// state 에 따라 등록

	// 접속한 클라이언트 중 state 에 맞는 놈만
	for (auto& item : m_IdToClient_map)
	{
		auto masked = (static_cast<StateType>(item.second->GetState()) & m_writeset_mask);
		if (masked)
		{	// send state 이니 writeset에 등록하도록 합시다
			write_sockets.push_back(item.second->GetTCPSocket());
		}
	}
}

// 이미 recv를 수행하여 만들어진 RecvPacket을 처리합니다
// 모든 처리가 긑나면 state를 변경합니다
void NetworkManager::HandleRecvQueue()
{
	while (false == m_recvQueue.empty())
	{
		// deque
		auto item = m_recvQueue.front();
		auto _clientinfo_ptr = item.first;
		auto _packet = item.second;
		m_recvQueue.pop();

		// 패킷 -> stream
		auto _stream = _packet->ToStreamPtr();

		// 프로토콜 별로 분류
		PROTOCOL _protocol = PacketUtil::GetProtocol(*_stream);
		switch (_protocol)
		{
			// TODO : protocol 정리 
		case PROTOCOL::Init:
			// 없음
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
				// TODO : 존재하지 않는 룸
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
				// TODO : 존재하지 않는 룸
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

// 클라이언트에게 실제 데이터 전송하는 곳 send()
// 모든 데이터를 전송하면 state를 변경합니다
void NetworkManager::HandleSendQueue()
{
	// 처리가 덜된 패킷들을 담을 임시 큐
	// 패킷에 번호를 부여하는걸로 대체할 예정
	queue<std::pair<ClientInfoPtr, SendPacketPtr>> tmp_queue;

	while (false == m_sendQueue.empty())
	{
		// deque
		auto item = m_sendQueue.front();
		auto _clientinfo_ptr = item.first;
		auto _packet = item.second;
		m_sendQueue.pop();

		// 이번 순번이 아니면
		if (false == _clientinfo_ptr->IsSendTurn(_packet->GetID()))
		{
			m_sendQueue.push(tmp_queue.front());
			m_sendQueue.pop();
			continue;
		}

		// recv 에서 클라이언트가 강제 종료된걸 catch 한 경우도 있으니 패킷 만들겠음
		if (_clientinfo_ptr->GetState() == ClientState::Disconnected_send)
		{
			// 종료 패킷 만들기
			OutputMemoryStreamPtr _stream = std::make_shared<OutputMemoryStream>();
			PacketUtil::PackPacket(*_stream, PROTOCOL::Disconnect);
			_packet = std::make_shared<SendPacket>(_stream, _clientinfo_ptr);
		}
		
		// send send send !!
		SendState _sendResult = PacketUtil::PacketSend(_clientinfo_ptr, _packet);

		// 전송이 완료됬으니
		// State 에 따라 다음 State로 바꾸기
		if (SendState::Completed == _sendResult)
		{	// 전부다 보냈으면
			// change state
			switch (_clientinfo_ptr->GetState())
			{
			case ClientState::Standby_send:
				_clientinfo_ptr->SetState(ClientState::WaitingRoom_send);	// TODO : WaitingRoom_send
				break;

				// send 에서 종료를 catch 한 경우
			case ClientState::Disconnected_send:
				// 접속을 종료시킴
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
				printf("change state 미등록!!!\n");
				break;
			}
		}
		else
		{	// 남았으면 임시 큐에 등록 시킴
			tmp_queue.push(item);
		}
	}

	// 남은놈들 send queue 에 재등록 , 다음 프레임에 다시 처리
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
	// 중복된 방이 개설되어 있으면
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

	// sendqueue에 등록
	m_sendQueue.push(
		std::pair<ClientInfoPtr, SendPacketPtr>(inClient, _sendpacket)
	);

}


void NetworkManager::SendEnterRoomMsg(ChatRoomPtr inChatRoom, const char* inName)
{
	char msg[BUFSIZE];
	ZeroMemory(msg, BUFSIZE);
	MessageMaker::Get_EnterRoomMsg(msg, inName);

	// sendqueue에 등록
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


	// sendqueue에 등록

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


	// sendqueue에 등록
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
	HandleNewChatRoom("사과");
	HandleNewChatRoom("포도");
	HandleNewChatRoom("멜론");

	return true;
}

#pragma endregion
