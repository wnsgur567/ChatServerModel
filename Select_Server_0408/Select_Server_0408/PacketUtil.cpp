#include "base.h"

#pragma region PACKET
SendState PacketUtil::PacketSend(ClientInfoPtr inClient, SendPacketPtr inoutPacket)
{
	int _sendbytes = 0;
	switch (inoutPacket->GetState())
	{
	case SendState::Idle:
	case SendState::Data:
	case SendState::Data_Interrupt:
	{
		// send()
		_sendbytes = send(
			inClient->GetTCPSocket()->GetSock(),								// sock
			inoutPacket->streamPtr->GetBufferPtr() + inoutPacket->m_sendbytes,	// buf
			inoutPacket->m_target_sendbytes - inoutPacket->m_sendbytes,			// len
			0
		);

		if (_sendbytes == SOCKET_ERROR)
		{
			return SendState::ClientEnd;
		}

		// ������ŭ ��� �̵�
		inoutPacket->m_sendbytes += _sendbytes;

		// �� ������ ���
		if (inoutPacket->m_target_sendbytes > inoutPacket->m_sendbytes)
		{
			printf("socket send buf Ȯ��");
			inoutPacket->m_state = SendState::Data_Interrupt;
			return inoutPacket->GetState();
		}

		// �� ������ ���
		inoutPacket->m_state = SendState::Completed;
		// ���� ��Ŷ ī����
		inClient->IncreaseNextSendPacketID();
	}
	break;
	}

	return inoutPacket->GetState();
}


// packet �� buf �� recv
// return value �� complete �� �Ǹ�
// buffer �� inputstream �� �������� ��
RecvState PacketUtil::PacketRecv(ClientInfoPtr inClient, RecvPacketPtr inoutPacket)
{
	int _recvbytes = 0;
#pragma region SIZE
	switch (inoutPacket->GetState())
	{
		// ���Ŀ� �и� �ʿ��ϸ�....
	case RecvState::Idle:
	case RecvState::Size:
	case RecvState::Size_Interrupt:
	{
		// recv() _ sizepart
		_recvbytes = recv(
			inClient->GetTCPSocket()->GetSock(),									// sock
			((char*)&(inoutPacket->m_target_recvbytes)) + inoutPacket->m_sizebytes,	// buf
			(inoutPacket->m_target_sizebytes - inoutPacket->m_sizebytes),			// len
			0);	// flag		
				
		if (_recvbytes == SOCKET_ERROR)
		{
			return RecvState::ClientEnd;
		}

		// �о�帰 ��ŭ head �̵�
		inoutPacket->m_sizebytes += _recvbytes;

		// size part �� ���� �� �޾�����
		if (inoutPacket->m_target_sizebytes > inoutPacket->m_sizebytes)
		{
			printf("socket recv buf Ȯ��");
			inoutPacket->m_state = RecvState::Size_Interrupt;
			return inoutPacket->m_state;		// return
		}

		// size part �� ���� �޾�����
		// data part ��
		inoutPacket->m_state = RecvState::Data;
	}
	break;
	}
#pragma endregion 

#pragma region DATA
	// target recv bytes ����ŭ recv
	switch (inoutPacket->m_state)
	{
	case RecvState::Data:
	case RecvState::Data_Interrupt:
	{
		// recv() _ datapart
		_recvbytes = recv(
			inClient->GetTCPSocket()->GetSock(),			// sock
			inoutPacket->m_buf + inoutPacket->m_recvbytes,	// buf
			inoutPacket->m_target_recvbytes - inoutPacket->m_recvbytes,	// len
			0	// flag
		);

		if (_recvbytes == SOCKET_ERROR)
		{
			return RecvState::ClientEnd;
		}

		// �о�帰 ��ŭ head �̵�
		inoutPacket->m_recvbytes += _recvbytes;

		// data part �� ���� �� �޾�����
		if (inoutPacket->m_target_recvbytes > inoutPacket->m_recvbytes)
		{
			printf("socket recv buf Ȯ��");
			inoutPacket->m_state = RecvState::Data_Interrupt;
			return inoutPacket->m_state;		// return
		}
		inoutPacket->m_state = RecvState::Completed;
	}
	break;
	}
#pragma endregion
	return inoutPacket->m_state;
}
#pragma endregion

#pragma region PACK
void PacketUtil::PackPacket(OutputMemoryStream& outOutputStream, const PROTOCOL inProtocol)
{
	int size = 0;
	size = sizeof(inProtocol);
	outOutputStream.Write(size);
	outOutputStream.Write(&inProtocol, sizeof(inProtocol));
}

void PacketUtil::PackPacket(OutputMemoryStream& outOutputStream, const PROTOCOL inProtocol, const char* str1)
{
	int size = 0;
	int strsize1 = strlen(str1);
	size = sizeof(inProtocol) + sizeof(int) + strsize1;
	outOutputStream.Write(size);
	outOutputStream.Write(&inProtocol, sizeof(inProtocol));
	outOutputStream.Write(strsize1);
	outOutputStream.Write(str1, strsize1);
}
void PacketUtil::PackPacket(OutputMemoryStream& outOutputStream, const PROTOCOL inProtocol, const int inNum)
{
	int size = 0;
	size = sizeof(inProtocol) + sizeof(inNum);
	outOutputStream.Write(size);
	outOutputStream.Write(&inProtocol, sizeof(inProtocol));
	outOutputStream.Write(inNum);
}
void PacketUtil::PackPacket(OutputMemoryStream& outOutputStream, const PROTOCOL inProtocol, const int inNum, const char* str1)
{
	int size = 0;
	int strsize1 = strlen(str1);
	size = sizeof(inProtocol) + sizeof(inNum) + sizeof(int) + strsize1;
	outOutputStream.Write(size);
	outOutputStream.Write(&inProtocol, sizeof(inProtocol));
	outOutputStream.Write(inNum);
	outOutputStream.Write(strsize1);
	outOutputStream.Write(str1, strsize1);
}
void PacketUtil::PackPacket(OutputMemoryStream& outOutputStream, const PROTOCOL inProtocol, const char* str1, const char* str2)
{
	int size = 0;
	int strsize1 = strlen(str1);
	int strsize2 = strlen(str2);
	size = sizeof(inProtocol) + sizeof(int) + strsize1 + sizeof(int) + strsize2;
	outOutputStream.Write(size);
	outOutputStream.Write(&inProtocol, sizeof(inProtocol));
	outOutputStream.Write(strsize1);
	outOutputStream.Write(str1, strsize1);
	outOutputStream.Write(strsize2);
	outOutputStream.Write(str2, strsize2);
}
void PacketUtil::PackPacket(OutputMemoryStream& outOutputStream, const PROTOCOL inProtocol, const char* str1, const char* str2, const char* str3)
{
	int size = 0;
	int strsize1 = strlen(str1);
	int strsize2 = strlen(str2);
	int strsize3 = strlen(str3);
	size = sizeof(inProtocol) + sizeof(int) + strsize1 + sizeof(int) + strsize2 + sizeof(int) + strsize3;
	outOutputStream.Write(size);
	outOutputStream.Write(&inProtocol, sizeof(inProtocol));
	outOutputStream.Write(strsize1);
	outOutputStream.Write(str1, strsize1);
	outOutputStream.Write(strsize2);
	outOutputStream.Write(str2, strsize2);
	outOutputStream.Write(strsize3);
	outOutputStream.Write(str3, strsize3);
}
void PacketUtil::PackPacket(OutputMemoryStream& outOutputStream, const PROTOCOL inProtocol, const SIGN_RESULT inResult, const char* str1)
{
	int size = 0;
	int strsize1 = strlen(str1);
	size = sizeof(inProtocol) + sizeof(inResult) + sizeof(int) + strsize1;
	outOutputStream.Write(size);
	outOutputStream.Write(&inProtocol, sizeof(inProtocol));
	outOutputStream.Write(&inResult, sizeof(inResult));
	outOutputStream.Write(strsize1);
	outOutputStream.Write(str1, strsize1);
}
#pragma endregion

#pragma region UNPACK
PROTOCOL PacketUtil::GetProtocol(InputMemoryStream& inInputStream)
{
	PROTOCOL _proto;
	inInputStream.Read(&_proto, sizeof(_proto));
	return _proto;
}
void PacketUtil::UnPackPacket(InputMemoryStream& inInputStream, char* str1)
{
	int strsize1;
	inInputStream.Read(strsize1);
	inInputStream.Read(str1, strsize1);
}
void PacketUtil::UnPackPacket(InputMemoryStream& inInputStream, int& outNum)
{
	inInputStream.Read(outNum);
}
void PacketUtil::UnPackPacket(InputMemoryStream& inInputStream, int& outNum, char* str1)
{
	int strsize1;
	inInputStream.Read(outNum);
	inInputStream.Read(strsize1);
	inInputStream.Read(str1, strsize1);
}
void PacketUtil::UnPackPacket(InputMemoryStream& inInputStream, char* str1, char* str2)
{
	int strsize1, strsize2;
	inInputStream.Read(strsize1);
	inInputStream.Read(str1, strsize1);
	inInputStream.Read(strsize2);
	inInputStream.Read(str2, strsize2);
}
void PacketUtil::UnPackPacket(InputMemoryStream& inInputStream, char* str1, char* str2, char* str3)
{
	int strsize1, strsize2, strsize3;
	inInputStream.Read(strsize1);
	inInputStream.Read(str1, strsize1);
	inInputStream.Read(strsize2);
	inInputStream.Read(str2, strsize2);
	inInputStream.Read(strsize3);
	inInputStream.Read(str3, strsize3);
}
void PacketUtil::UnPackPacket(InputMemoryStream& inInputStream, SIGN_RESULT& outResult, char* str1)
{
	int strsize1;
	inInputStream.Read(&outResult, sizeof(SIGN_RESULT));
	inInputStream.Read(strsize1);
	inInputStream.Read(str1, strsize1);
}
#pragma endregion
