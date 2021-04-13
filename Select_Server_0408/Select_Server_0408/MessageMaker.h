#pragma once

class MessageMaker
{
private:
#ifdef CONSOLE
	static const char* m_initMsg;
	static const char* m_UnStableConnectionMsg;
	static const char* m_disconnectedMsg;
	static const char* m_errorDisconnectedMsg;
	static const char* m_waitingRoomMsg;
#endif // CONSOLE

#ifdef WINDOW
	static const char* m_initMsg;
#endif // WINDOWS



public:
	static const char* Get_InitMSG() { return m_initMsg; }
	static const char* Get_UnStableConnection() { return m_UnStableConnectionMsg; }
	static const char* Get_Disconnected() { return m_disconnectedMsg; }
	static const char* Get_ErrorDisconnected() { return m_disconnectedMsg; }
	static const char* Get_WaitingRoomMsg() { return m_waitingRoomMsg; }

	static void Get_EnterRoomMsg(char* outBuf, const char* inClientName)
	{
		sprintf(outBuf, "[%s] 님이 입장하셨습니다.\n", inClientName);
	}

	static void Get_ChatMsg(char* outBuf, const char* inClientName, const char* inMsg)
	{
		sprintf(outBuf, "[%s] : %s\n", inClientName, inMsg);
	}

	static void Get_ExitRoomMsg(char* outBuf, const char* inClientName)
	{
		sprintf(outBuf, "[%s] 님이 퇴장하셨습니다.\n", inClientName);
	}

	static void Get_ChatRoomList(char* outBuf, vector<const char*> roomnames)
	{
		char buf[256];
		int count = 0;

		for (auto item : roomnames)
		{
			ZeroMemory(buf, 256);
			itoa(count, buf, 10);

			strcat(outBuf, buf);		// 00
			strcat(outBuf, "번방 : ");	// 00번방 : 
			strcat(outBuf, "[ ");		// 00번방 : [
			strcat(outBuf, item);		// 00번방 : [ item
			strcat(outBuf, " ]\n");		// 00번방 : [ item ]\n

			++count;
		}
	}

	static const char* Get_SignMSG(SIGN_RESULT _result)
	{
		// switch
	}


};

