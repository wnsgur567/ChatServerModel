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
	// critical section �ʱ�ȭ
	InitializeCriticalSection(&g_critical_section);

	// �̺�Ʈ �ʱ�ȭ
	NetworkManager::sInstance->m_myInfo.InitEvents();
	// recv ������ ����
	NetworkManager::sInstance->m_hRecvThread = CreateThread(nullptr,0,RecvThread,nullptr,0,nullptr);
	if (NetworkManager::sInstance->m_hRecvThread == nullptr)	// ����		
		return;
	Loop();	

	// thread ���� ���
	WaitForSingleObject(NetworkManager::sInstance->m_hRecvThread,INFINITE);

	CloseHandle(NetworkManager::sInstance->m_hRecvThread);	// ������ ������ �˾Ƽ� ����	
															// �����쿡�� ���� ����� �����ϱ� ������ ������ ���� close handle �� ȣ���ص� ��� ���ٰ� �ϴµ�
															// �׷��� �ȵǴ� ��찡 �־���...
	// critical section ����
	DeleteCriticalSection(&g_critical_section);
}

// ��Ʈ��ũ ��� ������ ����
void Client::Loop()
{
	while (true)
	{
		if (false == NetworkManager::sInstance->DoFrame())
			break;
	}
}