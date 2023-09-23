void UWidget_CustomizingTab::Change_Slot_Broadcast(int TabNumber)
{
	Change_Slot_Delegate.Broadcast(TabNumber);
}

bool UWidget_CustomizingTab::Begin_Tab_Bind_Slot_Implementation(int TabNumber)
{
	UMyGameInstance* MyInstance = Cast<UMyGameInstance>(GetGameInstance());
	DataTable = MyInstance->GetDataTables();


	// 메쉬 디폴트 처리
	Costume_Pawn->BeginDefalutMesh();
	// scene변경 함수 바인드처리
	Costume_Pawn->Bind_Init(this);

	// 각 슬롯들 init 함수 실행, 호출될 바인드함수 처리
	for (auto CustomSlot : Slot_Container->GetAllChildren())
	{
		if (CustomSlot)
		{
			Cast<UWidget_CustomizingSlot>(CustomSlot)->Init_Slot(Costume_Pawn, DataTable, this);
		}
	}
	Gender_Visible(true);
	//각 슬롯들 바인딩 다 끝났으면 호출
	Change_Slot_Broadcast(TabNumber);

	return true;
}