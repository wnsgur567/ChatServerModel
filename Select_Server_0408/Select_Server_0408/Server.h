#pragma once



class Server
{	
private:
	Server();	
	void Loop();
public:
	~Server(){}
	static std::unique_ptr<Server> sInstance;
	static bool StaticInit();
	void Run();
};