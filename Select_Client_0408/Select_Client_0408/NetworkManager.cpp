#include "base.h"

// 선언 필요
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
	// 처리부

	// test
	RecvPacketPtr _recvpacket = std::make_shared<RecvPacket>();
	PacketUtil::PacketRecv(m_sock, _recvpacket);
	auto _stream = _recvpacket->ToStreamPtr();
	PROTOCOL protocol = PacketUtil::GetProtocol(*_stream);
	switch (protocol)
	{
	case PROTOCOL::None:
		break;
	case PROTOCOL::Init:
		char msg[512];
		ZeroMemory(msg, 512);
		PacketUtil::UnPackPacket(*_stream, msg);
		printf(msg);
		break;
	case PROTOCOL::WaitingRoom_Menu:
		break;
	case PROTOCOL::JoinChatRoom:
		break;
	case PROTOCOL::OutOfRoom:
		break;
	default:
		break;
	}
	


	return true;
}