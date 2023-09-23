UWidget_CheckingAttandance_Main::UWidget_CheckingAttandance_Main(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	/*
	//슬롯세팅(블루프린트) => subClass사용해 블루프린트에서 직접 할당하여 넣음
	static ConstructorHelpers::FClassFinder<UWidget_CheckingAttandance_Slot> SlotWidget(TEXT("/Game/Project/Widget/Checking/WB_CheckingAttandance_Slot.WB_CheckingAttandance_Slot_C"));
	if (SlotWidget.Succeeded())
	{
		SlotClass = SlotWidget.Class;
	}*/
}
void UWidget_CheckingAttandance_Main::NativeConstruct()
{
	Get_Reward_BTN->OnClicked.AddDynamic(this, &UWidget_CheckingAttandance_Main::PressReward);
	Button_SpecialReward->OnClicked.AddDynamic(this, &UWidget_CheckingAttandance_Main::PressSpecial);
	Button_SpecialReward->SetIsEnabled(false); // 20일 보상버튼 막기

	Begin_Bind_Setting();

	Super::NativeConstruct();
}

void UWidget_CheckingAttandance_Main::Begin_Bind_Setting()
{
	if (!MyPlayer && !MyPlayer->BP_ActorComponent_Playfab)
		return;

	Main_gridPanel->ClearChildren();
	SlotArray.Empty();
	const int FixView = 4; // 보여지는 슬롯 갯수

	//Reward 테이블 세팅
	UMyGameInstance* MyInstance = Cast<UMyGameInstance>(GetGameInstance());
	class UDataTable* Reward_Table = MyInstance->GetCheckingRewardTables();
	// 변수 세팅
	TableLength = Reward_Table->GetRowNames().Num();
	MyPlayer = Cast<APawn_Player>(GetOwningPlayerPawn());
	IsTodayChecked = MyPlayer->BP_ActorComponent_Playfab->Is_Today_Checked();

	// 출석일 수 확인 후에
	Checking_Count = MyPlayer->BP_ActorComponent_Playfab->Get_Checking_Count();


	if (Checking_Count >= TableLength) // 전체보상을 다 받았다면
	{
		Checking_Count = TableLength;
		IsTodayChecked = true;
	}

	// 총길이만큼 Slot생성 후 출석일에 따라 Array에 담아줌
	for (int i = 0; i < TableLength; i++)
	{
		//위젯생성
		UWidget_CheckingAttandance_Slot* SlotWidget = CreateWidget<UWidget_CheckingAttandance_Slot>(GetWorld(), SlotClass);

		// Slot당  출석 검사
		(i < Checking_Count) ? SlotWidget->Set_Slot(i, true) : SlotWidget->Set_Slot(i, false);
		SlotArray.Add(SlotWidget); // 배열에 담아주고		
	}

	// 출석여부에 따라 보상받기 버튼 활성화
	Get_Reward_BTN->SetIsEnabled(!IsTodayChecked);
	// 오늘기준 출석 남은일수
	if (IsTodayChecked)
	{
		TodayCount(Checking_Count);
	}
	else
	{
		Checking_Count += 1;
		TodayCount(Checking_Count);
		//오늘 보상을 받지 않았다면
		//오늘날짜 보상 세팅
		FName CountSTR(*FString::FromInt(Checking_Count));
		TodayReward = *Reward_Table->FindRow<FChecking_Reward>(CountSTR, "");
		SlotArray[Checking_Count - 1]->Today_Slot();
	}


	//오늘까지 FixView 단위로 보여주게함
	int ViewCount = static_cast<int>((Checking_Count - 1) / FixView);
	for (int i = 0; i < FixView; i++)
	{
		int count = ViewCount * FixView + i;
		Main_gridPanel->AddChildToUniformGrid(SlotArray[count], 0, i);
	}
}
void UWidget_CheckingAttandance_Main::PressReward()
{	// 클릭시 버튼 막아주고
	Get_Reward_BTN->SetIsEnabled(false);
	// 서버에 오늘 체크했다는 표기와 보상 업데이트
	if (MyPlayer && MyPlayer->BP_ActorComponent_Playfab)
	{
		MyPlayer->BP_ActorComponent_Playfab->Update_Check_Change(this, TodayReward.Gold);
	}
}

void UWidget_CheckingAttandance_Main::PressSpecial()
{
	Button_SpecialReward->SetIsEnabled(false);

	//Special Reward 테이블 세팅
	UMyGameInstance* MyInstance = Cast<UMyGameInstance>(GetGameInstance());
	class UDataTable* Special_Table = MyInstance->GetSpecialRewardTables();

	if (Special_Table->FindRow<FChecking_Reward>("Daily_20th_Reward", ""))
	{
		//특별보상 요청
		FChecking_Reward Special_Reward = *Special_Table->FindRow<FChecking_Reward>("Daily_20th_Reward", "");
		MyPlayer->BP_ActorComponent_Playfab->Daily_20th_Reward(this, Special_Reward.Gold);
	}

}

void UWidget_CheckingAttandance_Main::ChangeSlot()
{
	// ==>PressReward()
	if (Checking_Count - 1 < TableLength)
	{
		SlotArray[Checking_Count - 1]->Set_Slot(Checking_Count - 1, true);
		MyPlayer->BP_ActorComponent_Playfab->UpdateCoin();
	}
	if (Checking_Count == TableLength) // 20일 다채우면
	{
		Button_SpecialReward->SetIsEnabled(true); //버튼 오픈
	}
}

void UWidget_CheckingAttandance_Main::ChangeSpecial_Implementation()
{
	if (MyPlayer && MyPlayer->BP_ActorComponent_Playfab)
	{
		MyPlayer->BP_ActorComponent_Playfab->UpdateCoin();
	}
}


//디버깅용
void UWidget_CheckingAttandance_Main::Debug_ClearAttandance()
{
	if (MyPlayer && MyPlayer->BP_ActorComponent_Playfab)
	{
		MyPlayer->BP_ActorComponent_Playfab->Debug_Clear_Attandance(this);
	}
}

void UWidget_CheckingAttandance_Main::Debug_ClearCheck()
{
	if (MyPlayer && MyPlayer->BP_ActorComponent_Playfab)
	{
		MyPlayer->BP_ActorComponent_Playfab->Debug_Clear_Daily(this);
	}
}
//여기까지 디버깅