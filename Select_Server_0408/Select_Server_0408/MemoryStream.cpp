#include "base.h"

#undef max

void OutputMemoryStream::ReAllocBuffer(size_t inNewLength)
{
	if (inNewLength < m_capacity)
		return;

	char* tmpBuffer = new char[inNewLength];
	for (size_t i = 0; i < m_capacity; i++)
	{
		tmpBuffer[i] = m_buffer[i];
	}
	delete[] m_buffer;
	m_buffer = tmpBuffer;
	m_capacity = inNewLength;
}

void OutputMemoryStream::Write(const void* inData, size_t inByteCount)
{
	size_t result_head = m_head + inByteCount;
	
	if (result_head > m_capacity)
		ReAllocBuffer(std::max(m_capacity * 2, result_head));

	std::memcpy(m_buffer + m_head, inData, inByteCount);

	m_head = result_head;
}

void OutputMemoryStream::Wirte(const std::string& inString)
{
	int length = static_cast<int>(inString.length());
	Write<int>(length);
	memcpy(m_buffer + m_head, inString.c_str(), length);
	m_head = m_head + sizeof(int) + length;
}


void InputMemoryStream::Read(void* outData, size_t inByteCount)
{
	memcpy(outData, m_buffer + m_head, inByteCount);
	m_head = m_head + inByteCount;
}