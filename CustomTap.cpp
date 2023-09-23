void UWidget_CustomizingTab::Change_Slot_Broadcast(int TabNumber)
{
	Change_Slot_Delegate.Broadcast(TabNumber);
}

bool UWidget_CustomizingTab::Begin_Tab_Bind_Slot_Implementation(int TabNumber)
{
	UMyGameInstance* MyInstance = Cast<UMyGameInstance>(GetGameInstance());
	DataTable = MyInstance->GetDataTables();


	// �޽� ����Ʈ ó��
	Costume_Pawn->BeginDefalutMesh();
	// scene���� �Լ� ���ε�ó��
	Costume_Pawn->Bind_Init(this);

	// �� ���Ե� init �Լ� ����, ȣ��� ���ε��Լ� ó��
	for (auto CustomSlot : Slot_Container->GetAllChildren())
	{
		if (CustomSlot)
		{
			Cast<UWidget_CustomizingSlot>(CustomSlot)->Init_Slot(Costume_Pawn, DataTable, this);
		}
	}
	Gender_Visible(true);
	//�� ���Ե� ���ε� �� �������� ȣ��
	Change_Slot_Broadcast(TabNumber);

	return true;
}