bool UWidget_CheckingAttandance_Slot::Set_Slot_Implementation(int SlotCount, bool IsGet)
{

	UMyGameInstance* MyInstance = Cast<UMyGameInstance>(GetGameInstance());
	class UDataTable* Reward_Table = MyInstance->GetCheckingRewardTables();
	FName SlotCoin(*FString::FromInt(SlotCount + 1));
	FChecking_Reward Slot_Reward = *Reward_Table->FindRow<FChecking_Reward>(SlotCoin, "");

	//슬롯당 보상 세팅 // 블루프린트에서 이미지 등 세팅예정
	Slot_Coin = Slot_Reward.Gold;
	Slot_BTN->SetIsEnabled(false);

	return false;
}

bool UWidget_CheckingAttandance_Slot::Today_Slot_Implementation()
{
	Slot_BTN->SetIsEnabled(true);

	return false;
}