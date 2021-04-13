#pragma once



class MyInfo
{
public:
	char m_sendbuf[BUFSIZE];
	myState m_state;
	HANDLE hSelectRoom;
	HANDLE hChatting;
public:
	MyInfo():
		m_state(myState::Wait)
	{}
	~MyInfo()
	{
		CloseHandle(hSelectRoom);
		CloseHandle(hChatting);
	}
public:
	void InitEvents()
	{
		hSelectRoom = CreateEvent(0, false, false, nullptr);	// ÀÚµ¿			
		hChatting = CreateEvent(0, true, false, nullptr);	
	}
	

	void SetState(myState inState) { m_state = inState; }
	myState GetState() const { return m_state; }

};