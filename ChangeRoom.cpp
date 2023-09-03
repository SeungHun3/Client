
//ä�� �� ���� ���� // channel = 1 ~ 20 // �� 20���� �� Ž���Լ� // RoomMaxSize; 
void PhotonListner::onRoomListUpdate(void) // Room������Ƽ�� ����ɶ����� ȣ�� // ex)�����, ����, �ο�������
{
	TMap<int, int> RoomMap; // channel, PlayerCount;

	// dummy�� ��쿡��
	if (b_IsDummy)
	{
		int RoomListSize = m_pClient->getRoomList().getSize(); // ��迭�� ���� ������ ������� ������
		for (int i = 0; i < RoomListSize; i++)
		{
			FString Channel;
			FString RoomName = m_pClient->getRoomList()[i]->getName().UTF8Representation().cstr();
			int Index = RoomName.Len() - sRoomName.Len();
			while (Index--)
			{
				Channel += RoomName[(RoomName.Len() - 1) - Index];				//ä��		
			}
			int PlayerCount = m_pClient->getRoomList()[i]->getPlayerCount();	//�÷��̾�� 
			RoomMap.Add(FCString::Atoi(*Channel), PlayerCount);					// �ʿ� �־
		}
		m_pView->UpdateRoomList(RoomMap);										// ���ͷ� ����

		return;
	}
	// ���̰� �ƴ� ����Ŭ���̾�Ʈ�� ù���ӽÿ�
	if (b_IsFirstConnect)
	{
		b_IsFirstConnect = false;
		TArray<int> ChannelArray;
		TArray<int> PlayerCountArray;

		int RoomSize = m_pClient->getRoomList().getSize(); // ��迭�� ���� ������ ������� ������
		for (int i = 0; i < RoomSize; i++)
		{
			FString Channel;
			FString RoomName = m_pClient->getRoomList()[i]->getName().UTF8Representation().cstr();
			int Index = RoomName.Len() - sRoomName.Len();
			while (Index--)
			{
				Channel += RoomName[(RoomName.Len() - 1) - Index];				//ä��		
			}
			int PlayerCount = m_pClient->getRoomList()[i]->getPlayerCount();	//�÷��̾�� 
			RoomMap.Add(FCString::Atoi(*Channel), PlayerCount);					// �ʿ� �־
		}

		for (int i = 1; i <= RoomMaxSize; i++)									// ��Ž��
		{
			if (RoomMap.Contains(i)) // ���� ã�Ҵٸ�
			{
				//�ִ� �ο��� üũ
				int PlayerCount = *RoomMap.Find(i);
				if (PlayerCount <= PlayerMaxCount)
				{
					sRoomCount = FString::FromInt(i);
					m_pView->ConnectComplete(); // �� ����
					return;
				}
			}
			else//ã�� ���ߴٸ� ���� ���� ->  �ش��ȣ ������
			{
				sRoomCount = FString::FromInt(i);
				m_pView->ConnectComplete(); // �� ����
				return;
			}
		}
		// ���δ� Ǯ���̶��

		sRoomCount = "Full";
		m_pView->ConnectComplete();
		return;
	}
}