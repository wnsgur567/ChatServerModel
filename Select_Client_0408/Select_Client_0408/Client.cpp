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
	// critical section 초기화
	InitializeCriticalSection(&g_critical_section);

	// 이벤트 초기화
	NetworkManager::sInstance->m_myInfo.InitEvents();
	// recv 스레드 생성
	NetworkManager::sInstance->m_hRecvThread = CreateThread(nullptr,0,RecvThread,nullptr,0,nullptr);
	if (NetworkManager::sInstance->m_hRecvThread == nullptr)	// 종료		
		return;
	Loop();	

	// thread 종료 대기
	WaitForSingleObject(NetworkManager::sInstance->m_hRecvThread,INFINITE);

	CloseHandle(NetworkManager::sInstance->m_hRecvThread);	// 스레드 끝나면 알아서 삭제	
															// 윈도우에서 참조 계수로 관리하기 때문에 만들자 마자 close handle 을 호출해도 상관 없다고 하는데
															// 그러면 안되는 경우가 있었음...
	// critical section 해제
	DeleteCriticalSection(&g_critical_section);
}

// 네트워크 통신 루프문 시작
void Client::Loop()
{
	while (true)
	{
		if (false == NetworkManager::sInstance->DoFrame())
			break;
	}
}