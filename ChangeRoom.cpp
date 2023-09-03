
//채널 뷰 설정 세팅 // channel = 1 ~ 20 // 총 20개의 룸 탐색함수 // RoomMaxSize; 
void PhotonListner::onRoomListUpdate(void) // Room프로퍼티가 변경될때마다 호출 // ex)방생성, 삭제, 인원수변동
{
	TMap<int, int> RoomMap; // channel, PlayerCount;

	// dummy일 경우에만
	if (b_IsDummy)
	{
		int RoomListSize = m_pClient->getRoomList().getSize(); // 룸배열은 먼저 생성된 순서대로 설정됨
		for (int i = 0; i < RoomListSize; i++)
		{
			FString Channel;
			FString RoomName = m_pClient->getRoomList()[i]->getName().UTF8Representation().cstr();
			int Index = RoomName.Len() - sRoomName.Len();
			while (Index--)
			{
				Channel += RoomName[(RoomName.Len() - 1) - Index];				//채널		
			}
			int PlayerCount = m_pClient->getRoomList()[i]->getPlayerCount();	//플레이어수 
			RoomMap.Add(FCString::Atoi(*Channel), PlayerCount);					// 맵에 넣어서
		}
		m_pView->UpdateRoomList(RoomMap);										// 액터로 전송

		return;
	}
	// 더미가 아닌 메인클라이언트가 첫접속시에
	if (b_IsFirstConnect)
	{
		b_IsFirstConnect = false;
		TArray<int> ChannelArray;
		TArray<int> PlayerCountArray;

		int RoomSize = m_pClient->getRoomList().getSize(); // 룸배열은 먼저 생성된 순서대로 설정됨
		for (int i = 0; i < RoomSize; i++)
		{
			FString Channel;
			FString RoomName = m_pClient->getRoomList()[i]->getName().UTF8Representation().cstr();
			int Index = RoomName.Len() - sRoomName.Len();
			while (Index--)
			{
				Channel += RoomName[(RoomName.Len() - 1) - Index];				//채널		
			}
			int PlayerCount = m_pClient->getRoomList()[i]->getPlayerCount();	//플레이어수 
			RoomMap.Add(FCString::Atoi(*Channel), PlayerCount);					// 맵에 넣어서
		}

		for (int i = 1; i <= RoomMaxSize; i++)									// 룸탐색
		{
			if (RoomMap.Contains(i)) // 룸을 찾았다면
			{
				//최대 인원수 체크
				int PlayerCount = *RoomMap.Find(i);
				if (PlayerCount <= PlayerMaxCount)
				{
					sRoomCount = FString::FromInt(i);
					m_pView->ConnectComplete(); // 방 입장
					return;
				}
			}
			else//찾지 못했다면 룸이 없음 ->  해당번호 룸입장
			{
				sRoomCount = FString::FromInt(i);
				m_pView->ConnectComplete(); // 방 입장
				return;
			}
		}
		// 전부다 풀방이라면

		sRoomCount = "Full";
		m_pView->ConnectComplete();
		return;
	}
}