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

ClientInfoPtr NetworkManager::GetClientInfo(int inClientID)
{
	if (m_IdToClient_map.end() == m_IdToClient_map.find(inClientID))
	{	// �˻� ����
		return nullptr;
	}
	// �˻� ����
	return m_IdToClient_map[inClientID];
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
	// ���� ���� �޼��� ó��
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
				_recvpacket = std::make_shared<RecvPacket>();
				_recvpacket->SetState(RecvState::Size);

				// recv
				ret_state = PacketUtil::PacketRecv(sock, _recvpacket);				
			}
			else
			{	// �ϴ� �۾��� ������
				_recvpacket = _clientinfo_ptr->GetRecvPacketPtr();

				// recv
				ret_state = PacketUtil::PacketRecv(sock, _recvpacket);
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
				HandleDisconnectedClient(_clientinfo_ptr->GetID());
			}
		}
	}

	// send ó�� (result_write_sockets)
	// ��ǻ� ����� ��� ������ ����
	for (auto& sock : result_write_sockets)
	{	// send queue �� ���
		// ��Ŷ ��ü�� recv queue ó���������� �̹� �� ������� ����

		// TODO : send �Ұ� ó���ϱ�
		/*auto _clientinfo_ptr = m_SockToClient_map[sock];
		SendPacketPtr _sendpacket = _clientinfo_ptr->GetSendPacketPtr();
		m_sendQueue.push(
			std::pair<ClientInfoPtr, SendPacketPtr>(_clientinfo_ptr, _sendpacket)
		);*/
	}

#pragma endregion

	
#pragma region queue ó��
	// recv queue ó��
	HandleRecvQueue();
	// send queue ó��
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
		}		
	}
}

void NetworkManager::HandleSendQueue()
{
	// ó���� ���� ��Ŷ���� ���� �ӽ� ť
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
		{	// ���δ� ��������
			// change state
			switch (_clientinfo_ptr->GetState())
			{
			case ClientState::Standby_send:
				_clientinfo_ptr->SetState(ClientState::None);	// TODO : WaitingRoom_send
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