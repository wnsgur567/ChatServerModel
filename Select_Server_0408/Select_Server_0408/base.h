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

#define MAX_PARTICIPANT	100	// �� �� �ִ� ������

using StateType = unsigned int;

enum class ClientState : StateType
{
	None = 0,								// not initialize

	Standby					= 1 << 0,		// ���� ���� �� ��� ����
	Standby_send			= 1 << 1,		// ���� ���� �� ��� ����
	UnstableConnection		= 1 << 2,		// ���� ���� �Ҿ��� �� ��
	UnstableConnection_send	= 1 << 3,		// ���� ���� �Ҿ��� �� ��
	Disconnected			= 1 << 4,		// ������ ���� ����
	Disconnected_send		= 1 << 5,		// ������ ���� ����
#pragma region �α��� ���� state

#pragma endregion

#pragma region ä�� ���� state
	WaitingRoom				= 1 << 20,		// ä�� ���� (�� ����)
	WaitingRoom_send		= 1 << 21,		// ä�� ���� (�� ����)

	EnterChatRoom			= 1 << 22,		// ä�ù� ���� ��
	EnterChatRoom_send		= 1 << 23,		// ä�ù� ���� ��

	JoinedChatRoom			= 1 << 24,		// ä�ù� ���� �Ϸ�
	JoinedChatRoom_send		= 1 << 25,		// ä�ù� ���� �Ϸ�

	ExitChatRoom			= 1<< 26,
	ExitChatRoom_send		= 1 << 27,
#pragma endregion

	
};

enum class ChatRoomState : StateType
{
	None = 0,
	Creating				= 1 << 0,		// �� ���� ��
	Created					= 1 << 2,		// �� ���� �Ϸ�
	Destroying				= 1 << 3,		// �� �ı� ��
	Destroyed				= 1 << 4,		// �� �ı� �Ϸ�
};

enum class PROTOCOL
{
	None = 0,
	Init = 1,			// ���� ���ӽ� �޼���
	WaitingRoom_Menu,	// server : �� ���� �޼���
						// client : ������ ��
	EnterChatRoom,		// server : �ش� �濡 ����ֱ�
						// client :	
	Chat,				// ä�� �޼���
	ExitChatRoom,		// server :	�ش� Ŭ���̾�Ʈ ������ �濡�� ������ ��������
						// client :	������ �濡�� ������
	Disconnect,			// server : ���� ���� ��Ŷ
						// client : ���� ���� ��Ŷ
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
	ClientEnd = -1,	// ��� ���� (���� ,������ �Ѵ�)

	Idle = 0,		// recv ���� ���� ����

	Size,			// size �޾ƾ� �� ����
	Size_Interrupt,	// size �� �޴� �ߴܵ� ����
	Data,			// data �޾ƾ� �� ����
	Data_Interrupt,	// data �� �޴� �ߴܵ� ����

	Completed,		// recv �Ϸ�

	Max
};
enum class SendState
{
	ClientEnd = -1,	// ��� ����

	Idle = 0,		// send ���� ����

	Data,			// data ���� ����
	Data_Interrupt,	// data �� ������ �ߴ� �� ����

	Completed,		// data ������ �Ϸ�

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