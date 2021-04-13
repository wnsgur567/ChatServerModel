#include "base.h"

std::unique_ptr<Client> Client::sInstance;

Client::Client()
{

}

bool Client::StaticInit()
{
	sInstance.reset(new Client());

	if (NetworkManager::StaticInit(SERVERIP, SERVERPORT) == false)
	{
		return false;
	}


	return true;
}

void Client::Run()
{
	NetworkManager::sInstance->m_myInfo.InitEvents();
	NetworkManager::sInstance->m_hRecvThread = CreateThread(nullptr,0,RecvThread,nullptr,0,nullptr);
	if (NetworkManager::sInstance->m_hRecvThread == nullptr)	// 종료		
		return;
	CloseHandle(NetworkManager::sInstance->m_hRecvThread);	// 스레드 끝나면 알아서 삭제	
	Loop();	

	// thread 종료 대기
	WaitForSingleObject(NetworkManager::sInstance->m_hRecvThread,INFINITE);
}

void Client::Loop()
{
	while (true)
	{
		if (false == NetworkManager::sInstance->DoFrame())
			break;
	}
}