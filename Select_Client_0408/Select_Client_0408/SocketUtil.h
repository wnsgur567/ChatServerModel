#pragma once

class SocketUtil
{	
public:
	static bool Init();			// wsa start
	static void CleanUp();		// wsa clean up
	static int	GetLastError();
	static void ReportError(const char* inOperationDesc);

	static TCPSocketPtr CreateTCPSocket();

	static int Select(const vector< TCPSocketPtr >* inReadSet,
		vector< TCPSocketPtr >* outReadSet,
		const vector< TCPSocketPtr >* inWriteSet,
		vector< TCPSocketPtr >* outWriteSet
		/*const vector< TCPSocketPtr >* inExceptSet,
		vector< TCPSocketPtr >* outExceptSet*/);
	// set에 집어넣기
	static fd_set* FillSetFromVector(fd_set& outSet, const vector<TCPSocketPtr>* inSockets);
	// select 된 소켓들 걸러내기
	static void FillVectorFromSet(vector< TCPSocketPtr >* outSockets, const vector< TCPSocketPtr >* inSockets, const fd_set& inSet);
};