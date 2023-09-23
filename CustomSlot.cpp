int UWidget_CustomizingSlot::GetSlotNumber()
{
	// 부모위젯에서 각 슬롯들의 번호를 매겨
	// 데이터 테이블의 행(인덱스값)으로 나열하기 위한 용도
	return 	GetParent()->GetChildIndex(this);
}

void UWidget_CustomizingSlot::Init_Slot_Implementation(APawn_Player* MyCostumePawn, const TArray<class UDataTable*>& MyDataTable, UWidget_CustomizingTab* CustomizingTab)
{
	//GM에 있는 PlayerState를 가져와서 위젯을 꺼내고
	AMyGameModeBase* MyGM = Cast<AMyGameModeBase>(GetWorld()->GetAuthGameMode());
	PS = MyGM->get_MyPlayerState();

	// 변수세팅
	SlotCount = GetSlotNumber();
	CostumePawn = MyCostumePawn;
	DataTable = MyDataTable;
	Widget_CustomizingTab = CustomizingTab;
	// 커스터마이징 탭 위젯의 델리게이트 함수에 바인딩하기
	Widget_CustomizingTab->Change_Slot_Delegate.AddDynamic(this, &UWidget_CustomizingSlot::Change_Slot);
	// 클릭이벤트 바인딩
	Slot_BTN->OnClicked.AddDynamic(this, &UWidget_CustomizingSlot::ClickEvent);

}
// TabNumber 0 = Gender / 1~4 = 커스터마이징
void UWidget_CustomizingSlot::Change_Slot_Implementation(int TabNumber)
{
	if (TabNumber != 0)
	{
		//TabNumber를 통해 데이터테이블 배열 접근-> Change Image
		FName rowName = FName(*FString::FromInt(SlotCount + 1));
		Customizing_Struct = *DataTable[TabNumber - 1]->FindRow<FCustomizing_Struct>(rowName, "");

		//선택했던 슬롯인지 검사 후
		//블루프린트에서 이미지 바꿀 수 있게 BP 작업으로 넘김
		int itemID = FCString::Atoi(*Customizing_Struct.ItemID);
		if (PS->Widget_Customizing->ITemIDs.IsValidIndex(PS->Widget_Customizing->TabNumber - 1))
		{
			if (PS->Widget_Customizing->ITemIDs[PS->Widget_Customizing->TabNumber - 1] == itemID)
			{
				Change_Image(Customizing_Struct.Image, true);
			}
			else { Change_Image(Customizing_Struct.Image, false); }
		}
		else { Change_Image(Customizing_Struct.Image, false); }
	}
}

void UWidget_CustomizingSlot::ClickEvent()
{
	//인덱스 없으면 추가 //TabNumber 1~4
	if (!PS->Widget_Customizing->ITemIDs.IsValidIndex(PS->Widget_Customizing->TabNumber - 1))
	{
		// 다음버튼 비활성화되어있어 순차적으로 쌓임
		PS->Widget_Customizing->ITemIDs.Add(FCString::Atoi(*Customizing_Struct.ItemID));
		// 다음버튼 켜주기
		PS->Widget_Customizing->Button_Next->SetIsEnabled(true);
	}
	// 있으면 바꾸기
	else { PS->Widget_Customizing->ITemIDs[PS->Widget_Customizing->TabNumber - 1] = FCString::Atoi(*Customizing_Struct.ItemID); }


	// 폰에 있는 메쉬도 구조체의 값으로 채워넣어준다
	CostumePawn->ChangeMesh(Customizing_Struct.ClassName, Customizing_Struct.Mesh);


	//클릭한 슬롯 표시
	Widget_CustomizingTab->Change_Slot_Broadcast(PS->Widget_Customizing->TabNumber);


}