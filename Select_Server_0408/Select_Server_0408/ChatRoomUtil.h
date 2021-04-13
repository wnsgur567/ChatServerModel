#pragma once

class ChatRoomUtil
{
public:
	static ChatRoomPtr CreateRoom(const string& inRoomName, const int inRoomID,const int inMaxParticipant)
	{
		return std::make_shared<ChatRoom>(inRoomName, inRoomID, inMaxParticipant);
	}

	
};