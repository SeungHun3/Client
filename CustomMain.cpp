void UWidget_Customizing::Begin_Bind_Setting()
{

	// �����ν��Ͻ����� DataTable�� ������ PlayFab�� ���� ITemID�迭 Default �� ä���
	//Default_Set_ItemIDs();
	TabNumber = 0;
	//���� ���Ե� ���ε�ó��
	WB_CustomizingTab->Begin_Tab_Bind_Slot(TabNumber);
	// ��ư ����
	Overlay_Previous->SetVisibility(ESlateVisibility::Hidden);
	Button_Next->SetIsEnabled(false);
}

void UWidget_Customizing::Default_Set_ItemIDs()
{
	MyGameInstance* MyInstance = Cast<MyGameInstance>(GetGameInstance());
	TArray<UDataTable*>InstanceDataTables = MyInstance->GetDataTables();

	for (auto Tables : InstanceDataTables)
	{
		//�����ν��Ͻ��� ������ ���̺��� �����ͼ� ù��° ���� �����ͷ� ä���ֱ�
		FCustomizing_Struct CustomStruct = *Tables->FindRow<FCustomizing_Struct>("1", "");
		if (!CustomStruct.ItemID.IsEmpty() && FCString::Atoi(*CustomStruct.ItemID) != 0) // ��ȯ�� ����� �����ߴٸ�
		{
			//ITemID�� �ֱ�
			ITemIDs.Add(FCString::Atoi(*CustomStruct.ItemID));
		}

	}
}