#pragma once


// send buffer 용
class OutputMemoryStream
{
private:
	char* m_buffer;
	size_t m_head;
	size_t m_capacity;

	void ReAllocBuffer(size_t inNewLength);
public:
	OutputMemoryStream()
		:m_buffer(nullptr), m_head(0), m_capacity(0)
	{
		m_buffer = new char[BUFSIZE];
	}
	OutputMemoryStream(const OutputMemoryStream& inOther)
		:m_buffer(nullptr),
		m_head(inOther.m_head),
		m_capacity(inOther.m_head)
	{
		m_buffer = new char[BUFSIZE];
		memcpy(m_buffer, inOther.m_buffer, inOther.m_head);
	}
	OutputMemoryStream& operator=(const OutputMemoryStream& inOther)
	{
		delete[] m_buffer;
		m_head = inOther.m_head;
		m_capacity = inOther.m_capacity;
		m_buffer = new char[BUFSIZE];
		memcpy(m_buffer, inOther.m_buffer, inOther.m_head);
	}

	~OutputMemoryStream() { delete[] m_buffer; }

public:
	const char* GetBufferPtr() { return m_buffer; }
	size_t GetLength() const { return m_head; }

	void Write(const void* inData, size_t inByteCount);
	// primitive 형식만 가능
	template<typename T>
	void Write(const T& inData);
	void Wirte(const std::string& inString);
};

template<typename T>
void OutputMemoryStream::Write(const T& inData)
{
	static_assert(
		std::is_arithmetic<T>::value ||
		std::is_enum<T>::value,
		"Generi Write only supports primitive data type"
		);

	Write(&inData, sizeof(inData));
}

// recv buffer 용
class InputMemoryStream
{
private:
	char* m_buffer;
	bool  m_isBufferOwner;

	size_t m_head;
	size_t m_capacity;
public:
	InputMemoryStream(char* inBuffer, size_t inByteCount, bool inIsOwner = false)
		: m_buffer(inBuffer),
		m_isBufferOwner(inIsOwner),
		m_capacity(inByteCount),
		m_head(0)
	{

	}
	InputMemoryStream(const InputMemoryStream& inOther)
		: m_buffer(nullptr),
		m_isBufferOwner(true),
		m_capacity(inOther.m_capacity),
		m_head(inOther.m_head)
	{
		m_buffer = new char[m_capacity];
		memcpy(m_buffer, inOther.m_buffer, m_capacity);
	}
	InputMemoryStream& operator=(const InputMemoryStream& inOther)
	{
		if (m_isBufferOwner)
			delete[] m_buffer;
		m_capacity = inOther.m_capacity;
		m_head = inOther.m_head;
		m_isBufferOwner = true;
		m_buffer = new char[m_capacity];
		memcpy(m_buffer, inOther.m_buffer, m_capacity);
	}
	~InputMemoryStream()
	{
		if (m_isBufferOwner)
			delete[] m_buffer;
	}

public:
	size_t GetRemainDataSize() const { return (m_capacity - m_head); }

	void Read(void* outData, size_t inByteCount);
	template<typename T>
	void Read(T& inData);
};

template<typename T>
void InputMemoryStream::Read(T& outData)
{
	static_assert(
		std::is_arithmetic<T>::value ||
		std::is_enum<T>::value,
		"Generi Write only supports primitive data type"
		);

	Read(&outData, sizeof(T));
}