#include "base.h"

#ifdef CONSOLE
const char* MessageMaker::m_initMsg = "서버접속 성공\n";
const char* MessageMaker::m_UnStableConnectionMsg = "접속이 불안정합니다\n";
const char* MessageMaker::m_disconnectedMsg = "서버접속 종료\n";
const char* MessageMaker::m_errorDisconnectedMsg = "서버와 연결이 끊겼습니다...\n";

const char* MessageMaker::m_waitingRoomMsg = "대기실에 입장하였습니다.\n";

#endif // CONSOLE

#ifdef WINDOW
const char* MessageMaker::m_initMsg = "서버접속 성공\r\n";
#endif // WINDOWS