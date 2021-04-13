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
	if (NetworkManager::sInstance->m_hRecvThread == nullptr)	// ����		
		return;
	CloseHandle(NetworkManager::sInstance->m_hRecvThread);	// ������ ������ �˾Ƽ� ����	
	Loop();	

	// thread ���� ���
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