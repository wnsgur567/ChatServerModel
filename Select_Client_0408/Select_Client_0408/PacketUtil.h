#pragma once

class RecvPacket
{
	friend class PacketUtil;
private:
	char m_buf[BUFSIZE];

	RecvState m_state;
	int				m_sizebytes;				// size part / ���� recv ��ġ
	const int		m_target_sizebytes;			// size part / ��ǥ recv ��ġ
	int				m_recvbytes;				// ���� recv ��ġ
	int				m_target_recvbytes;			// ��ǥ recv ��ġ
public:
	RecvPacket() :
		m_state(RecvState::Idle),
		m_sizebytes(0),
		m_target_sizebytes(sizeof(int)),
		m_recvbytes(0),
		m_target_recvbytes(0)
	{ }

	RecvPacket(const RecvPacket& _inSrc) :
		m_state(_inSrc.m_state),
		m_sizebytes(_inSrc.m_sizebytes),
		m_target_sizebytes(_inSrc.m_target_sizebytes),
		m_recvbytes(_inSrc.m_recvbytes),
		m_target_recvbytes(_inSrc.m_target_recvbytes)
	{
		memcpy(m_buf, _inSrc.m_buf, static_cast<size_t> (m_recvbytes));
	}

	const char* GetBuf() const
	{
		return m_buf;
	}
	RecvState GetState() const
	{
		return m_state;
	}
	void SetState(RecvState inState)
	{
		m_state = inState;
	}

	void Clear()
	{
		m_state = RecvState::Idle;
		m_sizebytes = 0;
		m_recvbytes = 0;
		m_target_recvbytes = 0;
	}

	InputMemoryStreamPtr ToStreamPtr()
	{
		if (m_state != RecvState::Completed)
			return nullptr;
		return std::make_shared<InputMemoryStream>(m_buf, m_recvbytes);
	}
	// m_buf �� memcpy �ؼ� ��Ʈ������ ��ȯ
	InputMemoryStreamPtr ToCopyStreamPtr()
	{
		if (m_state != RecvState::Completed)
			return nullptr;
		char* tmpbuf = new char[m_recvbytes];
		memcpy(tmpbuf, m_buf, m_recvbytes);
		return std::make_shared<InputMemoryStream>(m_buf, m_recvbytes, true);
	}
};

class SendPacket
{
	friend class PacketUtil;
private:
	OutputMemoryStreamPtr streamPtr;		// buffer�� ���� �ȿ� 

	SendState		m_state;
	int				m_sendbytes;				// ���� send ��ġ
	int				m_target_sendbytes;			// ��ǥ send ��ġ

public:
	SendPacket(OutputMemoryStreamPtr inStreamPtr)
		:streamPtr(inStreamPtr),
		m_state(SendState::Idle),
		m_sendbytes(0),
		m_target_sendbytes(inStreamPtr->GetLength())
	{ }

	SendState GetState() const
	{
		return m_state;
	}
	void SetState(SendState inState)
	{
		m_state = inState;
	}
};

// TODO : ���߿� ���÷��� �����ؼ� �ۼ�
class PacketUtil
{
public:
	static SendState PacketSend(const TCPSocketPtr inSock, SendPacketPtr inoutPacket);
	static RecvState PacketRecv(const TCPSocketPtr inSock, RecvPacketPtr inoutPacket);

	static void PackPacket(OutputMemoryStream& outOutputStream, const PROTOCOL inProtocol);
	static void PackPacket(OutputMemoryStream& outOutputStream, const PROTOCOL inProtocol, const char* str1);
	static void PackPacket(OutputMemoryStream& outOutputStream, const PROTOCOL inProtocol, const int inNum);
	static void PackPacket(OutputMemoryStream& outOutputStream, const PROTOCOL inProtocol, const int inNum, const char* str1);
	static void PackPacket(OutputMemoryStream& outOutputStream, const PROTOCOL inProtocol, const char* str1, const char* str2);
	static void PackPacket(OutputMemoryStream& outOutputStream, const PROTOCOL inProtocol, const char* str1, const char* str2, const char* str3);
	static void PackPacket(OutputMemoryStream& outOutputStream, const PROTOCOL inProtocol, const SIGN_RESULT inResult, const char* str1);


	static PROTOCOL GetProtocol(InputMemoryStream& inInputStream);
	static void UnPackPacket(InputMemoryStream& inInputStream, char* str);
	static void UnPackPacket(InputMemoryStream& inInputStream, int& outNum);
	static void UnPackPacket(InputMemoryStream& inInputStream, int& outNum, char* str1);
	static void UnPackPacket(InputMemoryStream& inInputStream, char* str1, char* str2);
	static void UnPackPacket(InputMemoryStream& inInputStream, char* str1, char* str2, char* str3);
	static void UnPackPacket(InputMemoryStream& inInputStream, SIGN_RESULT& outResult, char* str1);
};