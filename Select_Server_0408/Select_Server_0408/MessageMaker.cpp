#include "base.h"

#ifdef CONSOLE
const char* MessageMaker::m_initMsg = "�������� ����\n";

#endif // CONSOLE

#ifdef WINDOW
const char* MessageMaker::m_initMsg = "�������� ����\r\n";
#endif // WINDOWS