#include "base.h"

int main()
{
	if (Client::StaticInit())
	{
		Client::sInstance->Run();
		return 0;
	}
	return -1;
}