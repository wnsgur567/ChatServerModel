#pragma once

class ChatRoom
{
	using ParticipantsIterator = vector<ClientInfoPtr>::iterator;
	using Const_ParticipantsIterator = vector<ClientInfoPtr>::const_iterator;
private:
	int m_ChatRoomID;						// �� ID
	string m_name;							// �� �̸�
	int m_maxParticipant;					// �ִ� ���� �ο� ��

	ClientInfoPtr		  m_owner;			// ����
public:
	vector<ClientInfoPtr> m_participants;	// �� ������
public:
	ChatRoom(const string& inName, const int inID, int inMaxParticipant)
		: m_ChatRoomID(inID), m_name(inName), m_maxParticipant(inMaxParticipant),
		m_owner(nullptr)
	{
		m_participants.reserve(m_maxParticipant);
	}

	int		 GetID() const { return m_ChatRoomID; }
	string	 GetName() const { return m_name; }
	int		 GetParticipantsCount() const { return m_participants.size(); }
	bool	 IsOwner(ClientInfoPtr inClient) const
	{
		if (m_owner->GetID() == inClient->GetID())
			return true;
		return false;
	}
public:
	void EnterRoom(ClientInfoPtr inClient)
	{
		// �ִ� �ο��� üũ
		if (m_participants.size() >= static_cast<size_t>(m_maxParticipant))
			return;

		// TODO : �����̶�� ���� �ο�

		// �߰�
		m_participants.push_back(inClient);
	}
	void ExitRoom(ClientInfoPtr inClient)
	{
		for (vector< ClientInfoPtr>::iterator it = m_participants.begin(); it != m_participants.end(); it++)
		{
			if ((*it)->GetID() == inClient->GetID())
			{
				// TODO : �������� üũ / �����̸� �絵�ϵ���


				// �濡�� ����
				m_participants.erase(it);
				break;
			}
		}
	}

	void SendAll(const PROTOCOL inProtocol, const char* inName, void (NetworkManager::* Action)(void))
	{


		for (vector<ClientInfoPtr>::iterator it = m_participants.begin(); it != m_participants.end(); it++)
		{

		}



	}

	// ��� ������ ����
	void Clear()
	{
		m_participants.clear();
		m_owner.reset();
	}
};