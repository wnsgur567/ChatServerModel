#include "base.h"

#ifdef CONSOLE
const char* MessageMaker::m_initMsg = "�������� ����\n";
const char* MessageMaker::m_UnStableConnectionMsg = "������ �Ҿ����մϴ�\n";
const char* MessageMaker::m_disconnectedMsg = "�������� ����\n";
const char* MessageMaker::m_errorDisconnectedMsg = "������ ������ ������ϴ�...\n";

const char* MessageMaker::m_waitingRoomMsg = "���ǿ� �����Ͽ����ϴ�.\n";

#endif // CONSOLE

#ifdef WINDOW
const char* MessageMaker::m_initMsg = "�������� ����\r\n";
#endif // WINDOWS