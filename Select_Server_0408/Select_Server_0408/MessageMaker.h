#pragma once

class MessageMaker
{
private:
#ifdef CONSOLE
	static const char* m_initMsg;

#endif // CONSOLE

#ifdef WINDOW
	static const char* m_initMsg;
#endif // WINDOWS


	
public:
	static const char* Get_InitMSG() { return m_initMsg; }

	static const char* Get_SignMSG(SIGN_RESULT _result)
	{
		// switch
	}

	
};

