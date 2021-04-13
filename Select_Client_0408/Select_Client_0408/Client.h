#pragma once


class Client
{
private:
	Client();
	void Loop();
public:
	~Client() {}
	static std::unique_ptr< Client> sInstance;
	static bool StaticInit();
	void Run();	
};