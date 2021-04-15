#include "base.h"

// 선언 필요
std::unique_ptr<NetworkManager> NetworkManager::sInstance;

// 네트워크 통신 클래스 싱글톤 생성
bool NetworkManager::StaticInit(const char* inIp, u_int inPort)
{
	sInstance.reset(new NetworkManager());
	if (false == SocketUtil::Init())
		return false;
	return sInstance->Init(inIp, inPort);
}

// 네트워크 통신 클래스 초기화
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

	// 서버와 connect 완료 accept 대기중
	printf("Connecting...\n\n");
	return true;
}

// 루프문에서 한 프레임에 처리할 내용
bool NetworkManager::DoFrame()
{
	switch (m_myInfo.GetState())
	{
	case myState::Wait:
		// 서버로 부터 출력 메뉴가 들어온 후
		WaitForSingleObject(m_myInfo.hSelectRoom, INFINITE);
		break;
	case myState::SelectRoom:
	{
		printf("방 번호 선택 : ");
 		int select = 0;		
		scanf("%d", &select);
		while (getchar() != '\n')
		{	// 버퍼 비우기
		}

		// send 순서
		// 1. _stream 에 쓰기
		// 2. Packing
		// 3. packing 한 sendpacket을 서버로 전송 (packetsend)

		switch (select)
		{
		case -1:
		default:
		{
			// 종료 패킷 보내기
			OutputMemoryStreamPtr _stream = std::make_shared<OutputMemoryStream>();
			PacketUtil::PackPacket(*_stream, PROTOCOL::Disconnect);
			SendPacketPtr _sendpacket = std::make_shared<SendPacket>(_stream);
			PacketUtil::PacketSend(m_sock, _sendpacket);
			m_myInfo.SetState(myState::End);
		}
		break;

		// 현재 3개의 방을 기준으로 함
		case 0:
		case 1:
		case 2:
		{
			char buf[BUFSIZE];
			ZeroMemory(buf, BUFSIZE);			
			printf("닉네임 입력 : ");
			gets_s(buf, BUFSIZE);
			

			OutputMemoryStreamPtr _stream = std::make_shared<OutputMemoryStream>();
			PacketUtil::PackPacket(*_stream, PROTOCOL::EnterChatRoom, select, buf);
			SendPacketPtr _sendpacket = std::make_shared<SendPacket>(_stream);
			PacketUtil::PacketSend(m_sock, _sendpacket);

			// 서버로 부터 해당 룸의 입장패킷을 전달받을 때기지 무한정 이벤트 대기
			WaitForSingleObject(m_myInfo.hChatting, INFINITE);
		}
		break;
		}
	}
	break;

	// 실제 채팅 부분
	case myState::Chatting:
	{
		char buf[BUFSIZE];
		ZeroMemory(buf, BUFSIZE);
		scanf("%s", buf);		// 채팅내용 입력

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
			// 채팅내용 전송
			OutputMemoryStreamPtr _stream = std::make_shared<OutputMemoryStream>();
			PacketUtil::PackPacket(*_stream, PROTOCOL::Chat, buf);
			SendPacketPtr _sendpacket = std::make_shared<SendPacket>(_stream);
			PacketUtil::PacketSend(m_sock, _sendpacket);
		}

	}
	break;

	// 클라이언트 종료
	case myState::End:
		return false;

	default:
		break;
	}

	// 정상종료
	// 다음 프레임을 준비함
	return true;
}

