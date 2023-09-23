void UWidget_Customizing::Begin_Bind_Setting()
{

	// 게임인스턴스에서 DataTable을 가져와 PlayFab에 넣을 ITemID배열 Default 로 채우고
	//Default_Set_ItemIDs();
	TabNumber = 0;
	//하위 슬롯들 바인딩처리
	WB_CustomizingTab->Begin_Tab_Bind_Slot(TabNumber);
	// 버튼 세팅
	Overlay_Previous->SetVisibility(ESlateVisibility::Hidden);
	Button_Next->SetIsEnabled(false);
}

void UWidget_Customizing::Default_Set_ItemIDs()
{
	MyGameInstance* MyInstance = Cast<MyGameInstance>(GetGameInstance());
	TArray<UDataTable*>InstanceDataTables = MyInstance->GetDataTables();

	for (auto Tables : InstanceDataTables)
	{
		//게임인스턴스의 데이터 테이블을 가져와서 첫번째 열의 데이터로 채워넣기
		FCustomizing_Struct CustomStruct = *Tables->FindRow<FCustomizing_Struct>("1", "");
		if (!CustomStruct.ItemID.IsEmpty() && FCString::Atoi(*CustomStruct.ItemID) != 0) // 변환을 제대로 성공했다면
		{
			//ITemID에 넣기
			ITemIDs.Add(FCString::Atoi(*CustomStruct.ItemID));
		}

	}
}