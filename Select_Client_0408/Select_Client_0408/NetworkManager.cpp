#include "base.h"

// ���� �ʿ�
std::unique_ptr<NetworkManager> NetworkManager::sInstance;

bool NetworkManager::StaticInit(const char* inIp, u_int inPort)
{
	sInstance.reset(new NetworkManager());
	if (false == SocketUtil::Init())
		return false;
	return sInstance->Init(inIp, inPort);
}
bool NetworkManager::Init(const char* inIp, u_int inPort)
{
	m_sock = SocketUtil::CreateTCPSocket();
	if (m_sock == nullptr)
	{
		return false;
	}
	// blocking
	if (m_sock->SetNonBlockingMode(false) == false)
	{
		return false;
	}
	SocketAddress serveraddr(inet_addr(inIp), htons(inPort));
	if (false == m_sock->Connect(serveraddr))
		return false;

	printf("Connecting...\n\n");
	return true;
}


bool NetworkManager::DoFrame()
{
	switch (m_myInfo.GetState())
	{
	case myState::Wait:
		// ������ ���� ��� �޴��� ���� ��
		WaitForSingleObject(m_myInfo.hSelectRoom, INFINITE);
		break;
	case myState::SelectRoom:
	{
		printf("�� ��ȣ ���� : ");
 		int select = 0;		
		scanf("%d", &select);
		while (getchar() != '\n')
		{	// ���� ����
		}


		switch (select)
		{
		case -1:
		default:
		{
			// ���� ��Ŷ ������
			OutputMemoryStreamPtr _stream = std::make_shared<OutputMemoryStream>();
			PacketUtil::PackPacket(*_stream, PROTOCOL::Disconnect);
			SendPacketPtr _sendpacket = std::make_shared<SendPacket>(_stream);
			PacketUtil::PacketSend(m_sock, _sendpacket);
			m_myInfo.SetState(myState::End);
		}
		break;
		case 0:
		case 1:
		case 2:
		{
			char buf[BUFSIZE];
			ZeroMemory(buf, BUFSIZE);			
			printf("�г��� �Է� : ");
			gets_s(buf, BUFSIZE);
			

			OutputMemoryStreamPtr _stream = std::make_shared<OutputMemoryStream>();
			PacketUtil::PackPacket(*_stream, PROTOCOL::EnterChatRoom, select, buf);
			SendPacketPtr _sendpacket = std::make_shared<SendPacket>(_stream);
			PacketUtil::PacketSend(m_sock, _sendpacket);

			WaitForSingleObject(m_myInfo.hChatting, INFINITE);
		}
		break;
		}
	}
	break;
	case myState::Chatting:
	{
		char buf[BUFSIZE];
		ZeroMemory(buf, BUFSIZE);
		scanf("%s", buf);		// ä�ó��� �Է�

		if (strcmp(buf, "exit") == 0)
		{
			// end packet
			OutputMemoryStreamPtr _stream = std::make_shared<OutputMemoryStream>();
			PacketUtil::PackPacket(*_stream, PROTOCOL::ExitChatRoom);
			SendPacketPtr _sendpacket = std::make_shared<SendPacket>(_stream);
			PacketUtil::PacketSend(m_sock, _sendpacket);
			m_myInfo.SetState(myState::Wait);
			ResetEvent(m_myInfo.hChatting);
		}
		else
		{
			// ä�ó��� ����
			OutputMemoryStreamPtr _stream = std::make_shared<OutputMemoryStream>();
			PacketUtil::PackPacket(*_stream, PROTOCOL::Chat, buf);
			SendPacketPtr _sendpacket = std::make_shared<SendPacket>(_stream);
			PacketUtil::PacketSend(m_sock, _sendpacket);
		}

	}
	break;
	case myState::End:
		return false;
	default:
		break;
	}









	return true;
}

