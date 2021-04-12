#include "base.h"

#ifdef CONSOLE
const char* MessageMaker::m_initMsg = "辑滚立加 己傍\n";

#endif // CONSOLE

#ifdef WINDOW
const char* MessageMaker::m_initMsg = "辑滚立加 己傍\r\n";
#endif // WINDOWS