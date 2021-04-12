#pragma once

#pragma warning(disable:4996)

#pragma comment(lib,"ws2_32")
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>
#include <memory>
#include <list>
#include <queue>
#include <vector>
#include <unordered_map>
#include <algorithm>

#define DEBUG
#define CONSOLE
//#define WINDOW

#define SERVERIP	"127.0.0.1"
#define SERVERPORT	9000
#define BUFSIZE		512
#define SOCKET_END	0

using std::list;
using std::queue;
using std::vector;
using std::unordered_map;
using std::string;
using std::cout;
using std::endl;

enum class PROTOCOL
{
	None = 0,
	Init = 1,			// 서버 접속시 메세지
	WaitingRoom_Menu,	// server : 방 선택 메세지
						// client : 선택한 방
	JoinChatRoom,		// server : 해당 방에 집어넣기
						// client :	
	OutOfRoom,			// server :	해당 클라이언트 참가한 방에서 강제로 나가도록
						// client :	참가한 방에서 나가면
};

enum class SIGN_RESULT
{

};

enum class RecvState
{
	ClientEnd = -1,	// 통신 종료 (정상 ,비정상 둘다)

	Idle = 0,		// recv 아직 시작 안함

	Size,			// size 받아야 할 차례
	Size_Interrupt,	// size 를 받다 중단된 상태
	Data,			// data 받아야 할 차례
	Data_Interrupt,	// data 를 받다 중단된 상태

	Completed,		// recv 완료

	Max
};
enum class SendState
{
	ClientEnd = -1,	// 통신 종료

	Idle = 0,		// send 시작 안함

	Data,			// data 보낼 차례
	Data_Interrupt,	// data 를 보내다 중단 된 상태

	Completed,		// data 보내기 완료

	Max
};


class OutputMemoryStream;
class InputMemoryStream;
class RecvPacket;
class SendPacket;
class TCPSocket;

using OutputMemoryStreamPtr = std::shared_ptr<OutputMemoryStream>;
using InputMemoryStreamPtr = std::shared_ptr<InputMemoryStream>;
using RecvPacketPtr = std::shared_ptr<RecvPacket>;
using SendPacketPtr = std::shared_ptr<SendPacket>;
using TCPSocketPtr = std::shared_ptr<TCPSocket>;


#include "SocketUtil.h"
#include "MemoryStream.h"
#include "PacketUtil.h"
#include "SocketAddress.h"
#include "TCPSocket.h"
#include "NetworkManager.h"
#include "Client.h"