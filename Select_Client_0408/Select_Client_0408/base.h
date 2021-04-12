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
	Init = 1,			// ���� ���ӽ� �޼���
	WaitingRoom_Menu,	// server : �� ���� �޼���
						// client : ������ ��
	JoinChatRoom,		// server : �ش� �濡 ����ֱ�
						// client :	
	OutOfRoom,			// server :	�ش� Ŭ���̾�Ʈ ������ �濡�� ������ ��������
						// client :	������ �濡�� ������
};

enum class SIGN_RESULT
{

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