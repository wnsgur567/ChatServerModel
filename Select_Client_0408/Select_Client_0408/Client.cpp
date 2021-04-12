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
	Loop();
}

void Client::Loop()
{
	while (true)
	{
		if (false == NetworkManager::sInstance->DoFrame())
			break;
	}
}