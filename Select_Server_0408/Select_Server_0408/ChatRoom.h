#pragma once

class ChatRoom
{
	using ParticipantsIterator = vector<ClientInfoPtr>::iterator;
	using Const_ParticipantsIterator = vector<ClientInfoPtr>::const_iterator;
private:
	int m_ChatRoomID;						// 방 ID
	string m_name;							// 방 이름
	int m_maxParticipant;					// 최대 참가 인원 수

	ClientInfoPtr		  m_owner;			// 방장
public:
	vector<ClientInfoPtr> m_participants;	// 방 참가자
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
		// 최대 인원수 체크
		if (m_participants.size() >= static_cast<size_t>(m_maxParticipant))
			return;

		// TODO : 방장이라면 권한 부여

		// 추가
		m_participants.push_back(inClient);
	}
	void ExitRoom(ClientInfoPtr inClient)
	{
		for (vector< ClientInfoPtr>::iterator it = m_participants.begin(); it != m_participants.end(); it++)
		{
			if ((*it)->GetID() == inClient->GetID())
			{
				// TODO : 방장인지 체크 / 방장이면 양도하도록


				// 방에서 제거
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

	// 모든 유저들 정리
	void Clear()
	{
		m_participants.clear();
		m_owner.reset();
	}
};