#pragma once

#pragma warning(disable : 4996)

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

#define SERVERPORT	9000
#define BUFSIZE		512
#define SOCKET_END	0

#define MAX_PARTICIPANT	100	// 한 방 최대 참가자

using StateType = unsigned int;

enum class ClientState : StateType
{
	None = 0,								// not initialize

	Standby					= 1 << 0,		// 서버 접속 후 대기 상태
	Standby_send			= 1 << 1,		// 서버 접속 후 대기 상태
	UnstableConnection		= 1 << 2,		// 연결 상태 불안정 할 때
	UnstableConnection_send	= 1 << 3,		// 연결 상태 불안정 할 때
	Disconnected			= 1 << 4,		// 서버와 연결 종료
	Disconnected_send		= 1 << 5,		// 서버와 연결 종료
#pragma region 로그인 관련 state

#pragma endregion

#pragma region 채팅 관련 state
	WaitingRoom				= 1 << 20,		// 채팅 대기실 (방 선택)
	WaitingRoom_send		= 1 << 21,		// 채팅 대기실 (방 선택)

	EnterChatRoom			= 1 << 22,		// 채팅방 접속 중
	EnterChatRoom_send		= 1 << 23,		// 채팅방 접속 중

	JoinedChatRoom			= 1 << 24,		// 채팅방 접속 완료
	JoinedChatRoom_send		= 1 << 25,		// 채팅방 접속 완료

	ExitChatRoom			= 1<< 26,
	ExitChatRoom_send		= 1 << 27,
#pragma endregion

	
};

enum class ChatRoomState : StateType
{
	None = 0,
	Creating				= 1 << 0,		// 방 생성 중
	Created					= 1 << 2,		// 방 생성 완료
	Destroying				= 1 << 3,		// 방 파괴 중
	Destroyed				= 1 << 4,		// 방 파괴 완료
};

enum class PROTOCOL
{
	None = 0,
	Init = 1,			// 서버 접속시 메세지
	WaitingRoom_Menu,	// server : 방 선택 메세지
						// client : 선택한 방
	EnterChatRoom,		// server : 해당 방에 집어넣기
						// client :	
	Chat,				// 채팅 메세지
	ExitChatRoom,		// server :	해당 클라이언트 참가한 방에서 강제로 나가도록
						// client :	참가한 방에서 나가면
	Disconnect,			// server : 접속 종료 패킷
						// client : 접속 종료 패킷
	Max
};

enum class SIGN_RESULT
{
	None = 0,
};
enum class CHATROOM_RESULT
{
	None = 0,
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

using std::list;
using std::queue;
using std::vector;
using std::unordered_map;
using std::string;
using std::cout;
using std::endl;


class OutputMemoryStream;
class InputMemoryStream;
class RecvPacket;
class SendPacket;
class TCPSocket;
class ClientInfo;
class ChatRoom;

using OutputMemoryStreamPtr = std::shared_ptr<OutputMemoryStream>;
using InputMemoryStreamPtr = std::shared_ptr<InputMemoryStream>;
using RecvPacketPtr = std::shared_ptr<RecvPacket>;
using SendPacketPtr = std::shared_ptr<SendPacket>;
using TCPSocketPtr = std::shared_ptr<TCPSocket>;
using ClientInfoPtr = std::shared_ptr<ClientInfo>;
using ChatRoomPtr = std::shared_ptr<ChatRoom>;


#include "MemoryStream.h"
#include "PacketUtil.h"
#include "MessageMaker.h"
#include "SocketAddress.h"
#include "TCPSocket.h"
#include "SocketUtil.h"
#include "Packet.h"
#include "ClientInfo.h"
#include "NetworkManager.h"
#include "Server.h"

#include "Process.h"
#include "ChatRoom.h"
#include "ChatRoomUtil.h"