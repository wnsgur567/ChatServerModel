#include "base.h"

bool SocketUtil::Init()
{
	WSADATA _wsa;
	if (WSAStartup(MAKEWORD(2, 2), &_wsa) != 0)
	{
		ReportError("SocketUtil::Init");
		return false;
	}
	return true;
}
void SocketUtil::CleanUp()
{
	WSACleanup();
}

int SocketUtil::GetLastError()
{
	return WSAGetLastError();
}

void SocketUtil::ReportError(const char* inOperationDesc)
{
	LPVOID lpMsgBuf;
	DWORD errorNum = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorNum,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	printf("Error %s: %d- %s", inOperationDesc, errorNum, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

TCPSocketPtr SocketUtil::CreateTCPSocket()
{
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == SOCKET_ERROR)
	{
		ReportError("SocketUtil::CreateTCPSocket");
		return nullptr;
	}
	return std::make_shared<TCPSocket>(sock);
}

int SocketUtil::Select(
	const vector< TCPSocketPtr >* inReadVec,	//	recv�� �����ؾ��� socket��
	vector< TCPSocketPtr >* outReadVec,			//	select �� ���� �� socket��
	const vector< TCPSocketPtr >* inWriteVec,	//	send�� �����ؾ��� socket��
	vector< TCPSocketPtr >* outWriteVec			//	select �� ���� �� socket��
	/*const vector< TCPSocketPtr >* inExceptSet,
	vector< TCPSocketPtr >* outExceptSet*/
)
{
	fd_set read_set;
	fd_set write_set;
	//fd_set except_set;	

	FD_ZERO(&read_set);
	FD_ZERO(&write_set);
	//FD_ZERO(&except_set);

	// �ش� ���Ϳ� �����ϴ� ���ϵ��� �� set�� �����
	fd_set* readPtr = FillSetFromVector(read_set, inReadVec);
	fd_set* writePtr = FillSetFromVector(write_set, inWriteVec);
	//fd_set* exceptPtr = FillSetFromVector(except_set, inExceptSet);

	// select()
	int retval = select(0, readPtr, writePtr, nullptr, nullptr);

	if (retval == SOCKET_ERROR)
	{
		return false;
	}

	
	// �ش� set�� �����ִ� ���ϵ��� ���ͷ� ����
	FillVectorFromSet(outReadVec, inReadVec, read_set);
	FillVectorFromSet(outWriteVec, inWriteVec, write_set);
	//FillVectorFromSet(outExceptSet, inExceptSet, except_set);

	return true;
}

// socket���� outSet�� �����
fd_set* SocketUtil::FillSetFromVector(
	fd_set& outSet,		// ����� set (read or write)
	const vector<TCPSocketPtr>* inSockets	// set�� ���� socket vec
)
{
	if (inSockets)
	{	// not null
		FD_ZERO(&outSet);
		// ��� socket�� ���
		for (const TCPSocketPtr& socket : *inSockets)
		{
			FD_SET(socket->GetSock(), &outSet);
		}
		return &outSet;
	}
	return nullptr;
}

// inSet �� �˻��ؼ� �����ִ� ���ϵ��� �˻�
// inSockets : ó�� �����Ϸ��� ���� ���ϵ��� ISSET ���� �˻��Ͽ� 
// �����ִ� ���ϵ��� outSokcets�� ���
void SocketUtil::FillVectorFromSet(
	vector< TCPSocketPtr >* outSockets, 
	const vector< TCPSocketPtr >* inSockets,
	const fd_set& inSet
)
{
	if (inSockets && outSockets)
	{
		outSockets->clear();
		for (const TCPSocketPtr& socket : *inSockets)
		{
			if (FD_ISSET(socket->GetSock(), &inSet))
			{
				outSockets->push_back(socket);
			}
		}
	}
}