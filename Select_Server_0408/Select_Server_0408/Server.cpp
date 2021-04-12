#include "base.h"



// 선언 필요
std::unique_ptr<Server> Server::sInstance;

Server::Server()
{
	
}

bool Server::StaticInit()
{
	sInstance.reset(new Server());

	if (NetworkManager::StaticInit(SERVERPORT) == false)
	{
		return false;
	}


	return true;
}

void Server::Run()
{
	Loop();
}

void Server::Loop()
{		
	StateType mask =
		static_cast<StateType>(ClientState::Standby_send) |
		static_cast<StateType>(ClientState::UnstableConnection_send) |
		static_cast<StateType>(ClientState::Disconnected_send) |
		static_cast<StateType>(ClientState::WaitingRoom_send) |
		static_cast<StateType>(ClientState::JoinningChatRoom)
		;

	// select 마스크 적용
	NetworkManager::sInstance->SetSelectMask(mask);
	while (true)
	{		
		if (false ==  NetworkManager::sInstance->DoFrame())
			break;
	}
}