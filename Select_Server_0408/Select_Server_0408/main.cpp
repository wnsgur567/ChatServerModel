#include "base.h"

int main()
{
	if (Server::StaticInit() == true)
	{
		Server::sInstance->Run();
		return 0;
	}


	return -1;
}