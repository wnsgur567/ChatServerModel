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

ClientInfoPtr NetworkManager::GetClientInfo(int inClientID)
{
	if (m_IdToClient_map.end() == m_IdToClient_map.find(inClientID))
	{	// 검색 실패
		return nullptr;
	}
	// 검색 성공
	return m_IdToClient_map[inClientID];
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
	// 서버 접속 메세지 처리
	OutputMemoryStreamPtr _stream = std::make_shared<OutputMemoryStream>();
	auto msg = MessageMaker::Get_InitMSG();
	PacketUtil::PackPacket(*_stream, PROTOCOL::Init, MessageMaker::Get_InitMSG());
	SendPacketPtr _sendpacket = std::make_shared<SendPacket>(_stream);
	m_sendQueue.push(std::pair<ClientInfoPtr, SendPacketPtr>(_newClient, _sendpacket));

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

// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //

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
				_recvpacket = std::make_shared<RecvPacket>();
				_recvpacket->SetState(RecvState::Size);

				// recv
				ret_state = PacketUtil::PacketRecv(sock, _recvpacket);				
			}
			else
			{	// 하던 작업이 있으면
				_recvpacket = _clientinfo_ptr->GetRecvPacketPtr();

				// recv
				ret_state = PacketUtil::PacketRecv(sock, _recvpacket);
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
				HandleDisconnectedClient(_clientinfo_ptr->GetID());
			}
		}
	}

	// send 처리 (result_write_sockets)
	// 사실상 등록한 모든 소켓이 나옴
	for (auto& sock : result_write_sockets)
	{	// send queue 에 등록
		// 패킷 자체는 recv queue 처리과정에서 이미 다 만들어져 있음

		// TODO : send 할거 처리하기
		/*auto _clientinfo_ptr = m_SockToClient_map[sock];
		SendPacketPtr _sendpacket = _clientinfo_ptr->GetSendPacketPtr();
		m_sendQueue.push(
			std::pair<ClientInfoPtr, SendPacketPtr>(_clientinfo_ptr, _sendpacket)
		);*/
	}

#pragma endregion

	
#pragma region queue 처리
	// recv queue 처리
	HandleRecvQueue();
	// send queue 처리
	HandleSendQueue();
#pragma endregion	

	return true;
}

void NetworkManager::HandleRecvQueue()
{
	while (false == m_recvQueue.empty())
	{
		// deque
		auto& item = m_recvQueue.front();
		auto& _clientinfo_ptr = item.first;
		auto& _packet = item.second;
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
		}		
	}
}

void NetworkManager::HandleSendQueue()
{
	// 처리가 덜된 패킷들을 담을 임시 큐
	queue<std::pair<ClientInfoPtr, SendPacketPtr>> tmp_queue;
	
	while (false == m_sendQueue.empty())
	{
		// deque
		auto item = m_sendQueue.front();
		auto _clientinfo_ptr = item.first;
		auto _packet = item.second;
		m_sendQueue.pop();

		// send send send !!
		SendState _sendResult = PacketUtil::PacketSend(_clientinfo_ptr->GetTCPSocket(), _packet);
		
		if (SendState::Completed == _sendResult)
		{	// 전부다 보냈으면
			// change state
			switch (_clientinfo_ptr->GetState())
			{
			case ClientState::Standby_send:
				_clientinfo_ptr->SetState(ClientState::None);	// TODO : WaitingRoom_send
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