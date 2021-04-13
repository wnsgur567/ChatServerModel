#pragma once
#include "base.h"

DWORD WINAPI RecvThread(LPVOID arg)
{
	TCPSocketPtr m_sock = NetworkManager::sInstance->GetSocketPtr();
	char msg[BUFSIZE];	// 출력 버퍼

	// test
	RecvPacketPtr _recvpacket = std::make_shared<RecvPacket>();

	bool endflag = false;
	while (false == endflag)
	{
		_recvpacket->Clear();
		PacketUtil::PacketRecv(m_sock, _recvpacket);
		auto _stream = _recvpacket->ToStreamPtr();
		PROTOCOL protocol = PacketUtil::GetProtocol(*_stream);
		switch (protocol)
		{
		case PROTOCOL::Init:
			ZeroMemory(msg, 512);
			PacketUtil::UnPackPacket(*_stream, msg);
			printf(msg);
			break;
		case PROTOCOL::WaitingRoom_Menu:
			ZeroMemory(msg, 512);
			PacketUtil::UnPackPacket(*_stream, msg);
			printf(msg);															// 채팅방 메뉴 출력
			NetworkManager::sInstance->m_myInfo.SetState(myState::SelectRoom);		// 채팅방 선택 하도록 state 변경
			SetEvent(NetworkManager::sInstance->m_myInfo.hSelectRoom);				// select menu event on (auto)
			break;
		case PROTOCOL::EnterChatRoom:
			ZeroMemory(msg, 512);
			PacketUtil::UnPackPacket(*_stream, msg);
			printf(msg);															// 입장 메세지 출력
			NetworkManager::sInstance->m_myInfo.SetState(myState::Chatting);		// 채팅 state
			SetEvent(NetworkManager::sInstance->m_myInfo.hChatting);				// chatting event on	
			break;
		case PROTOCOL::Chat:
			ZeroMemory(msg, 512);
			PacketUtil::UnPackPacket(*_stream, msg);
			printf(msg);															// chatting message 출력
			break;
		case PROTOCOL::ExitChatRoom:
			ZeroMemory(msg, 512);
			PacketUtil::UnPackPacket(*_stream, msg);								
			printf(msg);															// 방 나감 메세지 출력
			NetworkManager::sInstance->m_myInfo.SetState(myState::Wait);			// 대기 state
			ResetEvent(NetworkManager::sInstance->m_myInfo.hChatting);				// chatting event off
			break;
		case PROTOCOL::Disconnect:
			ZeroMemory(msg, 512);
			PacketUtil::UnPackPacket(*_stream, msg);
			printf(msg);
			endflag = true;
			NetworkManager::sInstance->m_myInfo.SetState(myState::End);
			break;

		case PROTOCOL::None:
		default:
			printf("RecvThread Protocol check");
			break;
		}
	}

	return 0;
}