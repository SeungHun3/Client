//����Ʈ ����ü ã�� ��ã�Ҵٸ� -1
int UActorComponent_Playfab::FindQuestInfo_Index(const FString& QuestName)
{
	int Index = -1;
	for (int i = 0; i < MyQuest_Info.Num(); i++)
	{	// ���������� �ִٸ� 
		if (MyQuest_Info[i].Quest_Name == QuestName) { Index = i;	break; }
	}

	return Index;
}
// ����Ʈ PlayFab_Statistics ����, value 0 = ���۰���, value 1 = ������, value 2 = �Ϸ�
void UActorComponent_Playfab::CheckQuestInfo()
{
	TArray<FString> QuestKey;
	PlayFab_Statistics.GetKeys(QuestKey);
	for (auto it : QuestKey)
	{	// ������������ ����Ʈ�� �߷���  value�� 1�ΰ͵鸸 == �������� ����Ʈ�� ���
		if ((it.Left(5) == "Quest") && (UserTitleData.Contains(it)) && (1 == *PlayFab_Statistics.Find(it)))
		{
			FString val = *UserTitleData.Find(it);
			FQuest_Info Quest = MakeQuestInfo(it, val);
			MyQuest_Info.Add(Quest); //������ �־��ֱ�
		}
	}
}
// ����Ʈ JSon���� == QuestName : ParseData{ ����ܰ�+index(Key) : �ϷῩ��(Value),  ����ܰ�+index(Key) : �ϷῩ��(Value)... }
FQuest_Info UActorComponent_Playfab::MakeQuestInfo(const FString& QuestName, const FString& JsonParseData) //ex) ParseData = {"1/n0":true,"1/n1":false}
{
	//json������ ��ȯ
	FQuest_Info Quest;
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonParseData);
	FJsonSerializer::Deserialize(JsonReader, JsonObject);
	TArray<FString> StepKeys;
	JsonObject->Values.GetKeys(StepKeys);


	// ����Ʈ �̸�����
	Quest.Quest_Name = QuestName;
	TArray<FString> StepParse;
	StepKeys[0].ParseIntoArray(StepParse, TEXT("/n"));
	//����Ʈ ���൵
	Quest.Quest_Step = FCString::Atoi(*StepParse[0]);

	//���൵ �κ� Ŭ����üũ
	int indexSize = JsonObject->Values.Num();

	for (int i = 0; i < indexSize; i++)
	{
		FString KeyIndex = StepParse[0] + FString("/n") + FString::FromInt(i);
		Quest.IsFinished.Add(JsonObject->Values.Find(KeyIndex)->Get()->AsBool());
	}

	return Quest;
}

UDataTable* UActorComponent_Playfab::FindQuestTable(const FString& QuestName)
{
	UMyGameInstance* MyInstance = Cast<UMyGameInstance>(GetWorld()->GetGameInstance());
	UDataTable* Quest_Table = MyInstance->GetQuestTables();
	FQuest_List* CurrQuest = Quest_Table->FindRow<FQuest_List>(FName(*QuestName), "FindQuestTable_NoQuest");
	if (CurrQuest == nullptr) { return nullptr; }

	return CurrQuest->QuestData;
}
// ����Ʈ UserTitleData Update
void UActorComponent_Playfab::Quest_Update_Title(const FString& QuestName)
{
	PlayFabClientPtr ClientAPI = IPlayFabModuleInterface::Get().GetClientAPI();
	TSharedPtr<FJsonObject> JsonObj = MakeShareable(new FJsonObject());
	TSharedPtr<FJsonObject> JsonSubObj = MakeShareable(new FJsonObject());
	TArray< TSharedPtr<FJsonValue> > EntriesArray;

	EntriesArray.Add(MakeShareable(new FJsonValueString(QuestName))); // ����Ʈ�̸�
	int CurrQuest = FindQuestInfo_Index(QuestName);
	if (CurrQuest == -1)
	{
		UE_LOG(LogTemp, Log, TEXT("// Invalid_Struct "));
		return;
	}

	FString Step = FString::FromInt(MyQuest_Info[CurrQuest].Quest_Step);

	for (int i = 0; i < MyQuest_Info[CurrQuest].IsFinished.Num(); i++)
	{
		FString StepIndex = Step + FString("/n") + FString::FromInt(i);			// ����Ʈ ���൵ 
		JsonSubObj->SetBoolField(StepIndex, MyQuest_Info[CurrQuest].IsFinished[i]);// ����Ʈ �ϷῩ��
	}
	EntriesArray.Add(MakeShareable(new FJsonValueObject(JsonSubObj))); // ����Ʈ ����  // Key(���൵ + /n + ����Ʈindex ) :Value (�ϷῩ��)

	JsonObj->SetArrayField("Quest", EntriesArray);


	//������ ����
	PlayFab::ClientModels::FExecuteCloudScriptRequest request;
	request.FunctionName = "UpdateQuest";
	request.FunctionParameter = JsonObj;
	request.GeneratePlayStreamEvent = true;
	ClientAPI->ExecuteCloudScript(
		request,
		UPlayFabClientAPI::FExecuteCloudScriptDelegate::CreateLambda([&](const PlayFab::ClientModels::FExecuteCloudScriptResult& result)
			{

			}),
			PlayFab::FPlayFabErrorDelegate::CreateUObject(this, &UActorComponent_Playfab::ErrorScript)
			);

}

void UActorComponent_Playfab::Quest_Update_Statistic(const FString& QuestName, enum_Quest_Update Update)
{
	const enum_Quest_Update QuestCondition = Update;
	int JsonVal = static_cast<int>(QuestCondition);


	PlayFabClientPtr ClientAPI = IPlayFabModuleInterface::Get().GetClientAPI();
	TArray< TSharedPtr<FJsonValue> > EntriesArray;
	EntriesArray.Add(MakeShareable(new FJsonValueString(QuestName)));
	EntriesArray.Add(MakeShareable(new FJsonValueNumber(JsonVal)));
	TSharedPtr<FJsonObject> JsonObj = MakeShareable(new FJsonObject());
	JsonObj->SetArrayField("statistic", EntriesArray);

	PlayFab::ClientModels::FExecuteCloudScriptRequest request; // ���������Ϳ� ����Ʈ Value 2�� �־��ֱ�
	request.FunctionName = "Update_Statistic";
	request.FunctionParameter = JsonObj;
	request.GeneratePlayStreamEvent = true;
	ClientAPI->ExecuteCloudScript(
		request,
		UPlayFabClientAPI::FExecuteCloudScriptDelegate::CreateLambda([&, QuestName, QuestCondition](const PlayFab::ClientModels::FExecuteCloudScriptResult& result)
			{
				FQuest_Info Quest;
				int checkIndex;
				int ENUM_Val = static_cast<int>(QuestCondition);
				switch (QuestCondition)
				{
				case enum_Quest_Update::Drop:// ����Ʈ����

					PlayFab_Statistics.Add(QuestName, ENUM_Val); // ���������� 0 ���� ����
					for (int i = 0; i < MyQuest_Info.Num(); i++) // ����Ʈ �����������
					{
						if (PlayFab_Statistics.Contains(QuestName)) { PlayFab_Statistics.Add(QuestName, 0); }
						if (MyQuest_Info[i].Quest_Name == QuestName) { MyQuest_Info.RemoveAt(i);	break; }
					}
					PlayerOwner->Quest_Drop(QuestName);
					break;


				case enum_Quest_Update::Start:	//����Ʈ ����

					PlayFab_Statistics.Add(QuestName, ENUM_Val); // ���������� �߰�
					checkIndex = FindQuestInfo_Index(QuestName);					// �ʱ�ȭ
					if (checkIndex != -1) { MyQuest_Info.RemoveAt(checkIndex); }	//�����̸��� ����Ʈ���� �ִٸ� �����
					Quest = SetQuestInfo(QuestName, 1); // ����Ʈ �������� ����ü ��Ƽ�
					MyQuest_Info.Add(Quest); // �迭�� �߰� �� 
					Quest_Update_Title(QuestName); // Ÿ��Ʋ �����Ϳ� ������Ʈ�ϱ�

					PlayerOwner->Quest_Start(Quest);
					break;

				case enum_Quest_Update::Complete:
					// ����ޱ� // ������ Ÿ��Ʋ ������ �����
					for (int i = 0; i < MyQuest_Info.Num(); i++) // ����Ʈ �����������
					{
						if (PlayFab_Statistics.Contains(QuestName)) { PlayFab_Statistics.Add(QuestName, ENUM_Val); }
						if (MyQuest_Info[i].Quest_Name == QuestName) { MyQuest_Info.RemoveAt(i);	break; }
					}
					PlayerOwner->Quest_Complete(QuestName);
					break;
				}

			}));

}

FQuest_Info UActorComponent_Playfab::SetQuestInfo(const FString& QuestName, int Step)
{
	FQuest_Info Quest;
	Quest.Quest_Name = QuestName;	// ����Ʈ �̸�
	Quest.Quest_Step = Step;		// ����Ʈ ���൵
	Quest.IsFinished.Reset();

	UDataTable* MyQuestTable = FindQuestTable(QuestName);
	TArray<int> RowNames = GetQuestRowNames(FString::FromInt(Step), MyQuestTable);
	int IndexSize = RowNames.Num();
	if (IndexSize == 0) { return Quest; } // ���൵�� ���ٸ� Quest.IsFinished.Reset() �� ��Ű�� Ż��

	for (int i = 0; i < IndexSize; i++)
	{
		Quest.IsFinished.Add(false); // ����Ʈ �ϷῩ��
	}

	//����Ʈ(���õȵ��������̺�)�� ������ �ִ� ���������̺�(��ȭ�ý���)
	FString TablePropName = FString::FromInt(RowNames[0]); // ���� ���൵�� ���� �ε����� ù��°�� ���������̺�
	FQuest_Info CurrQuestTable = *MyQuestTable->FindRow<FQuest_Info>(FName(*TablePropName), "NoData_Questinfo_Table");
	Quest.QuestTable = CurrQuestTable.QuestTable;

	return Quest;
}

//���� ���൵�� ���� ���̺� �� ���ڻ̾Ƽ� �迭�������� // ������Ƽ Quest_Step �� ���൵�� Ž�� (FQuest_Info Quest; Quest.Quest_Step;)
TArray<int> UActorComponent_Playfab::GetQuestRowNames(const FString& QuestStepProp, UDataTable* QuestTable)
{
	TArray<FString> RowNames;
	TArray<int> QuestRowNames;
	RowNames = UDataTableFunctionLibrary::GetDataTableColumnAsString(QuestTable, FName("Quest_Step"));

	for (int i = 0; i < RowNames.Num(); i++)
	{
		if (RowNames[i] == QuestStepProp)
		{
			QuestRowNames.Add(i + 1);
		}
	}
	return QuestRowNames;
}
// ����Ʈ �� ���൵���ϱ�
int UActorComponent_Playfab::QuestTotalStepcount(const FString& QuestName)
{
	UDataTable* CheckTable = FindQuestTable(QuestName);
	TArray<FName> checkRows = CheckTable->GetRowNames(); // ������� ���ؼ�
	TArray<int> Totalindex;
	for (auto it : checkRows)
	{
		FQuest_Info* checkProp = CheckTable->FindRow<FQuest_Info>(it, "TotalStepCount, Nodata");

		if (checkProp) { Totalindex.AddUnique(checkProp->Quest_Step); }	// ������Ƽ AddUnique�� �߰� (�ߺ����ֱ̳�)
	}

	return Totalindex.Num();
}

void UActorComponent_Playfab::Quest_Start(const FString& QuestName)
{
	Quest_Update_Statistic(QuestName, enum_Quest_Update::Start);
}

bool UActorComponent_Playfab::Quest_Finish(const FString& QuestName, int index)
{
	int CurrQuest = FindQuestInfo_Index(QuestName);
	if (CurrQuest == -1) { return false; }	// �߸��� �̸��̰ų�
	if (!MyQuest_Info[CurrQuest].IsFinished.IsValidIndex(index)) { return false; }	// �߸��� �ε��� �����̶��

	MyQuest_Info[CurrQuest].IsFinished[index] = true;
	Quest_Update_Title(QuestName);

	for (int i = 0; i < MyQuest_Info[CurrQuest].IsFinished.Num(); i++)
	{
		if (!MyQuest_Info[CurrQuest].IsFinished[i]) { return false; } // index�� ���� true�� �ƴ϶��
	}

	//Step�� index�� ��� true���
	return true;
}

void UActorComponent_Playfab::Quest_Next(const FString& QuestName)
{
	int CurrQuest = FindQuestInfo_Index(QuestName);
	if (CurrQuest == -1) { return; }	// �߸��� �̸��̶�� Ż��

	int NextStep = MyQuest_Info[CurrQuest].Quest_Step + 1;

	FQuest_Info Quest;
	Quest = SetQuestInfo(QuestName, NextStep);
	// ����Ʈ�� �����ٸ�						// ����Ʈ �Ϸ� ���� ������Ʈ
	if (!Quest.IsFinished.IsValidIndex(0)) { Quest_Complete(QuestName);	return; }

	MyQuest_Info[CurrQuest] = Quest; //�������
	PlayerOwner->Quest_Next(MyQuest_Info[CurrQuest]);
	Quest_Update_Title(QuestName);		// Ÿ��Ʋ �����Ϳ� ������Ʈ�ϱ�
}
//����Ʈ���� == ���������� 0�� �ٲ��ֱ�
void UActorComponent_Playfab::Quest_Drop(const FString& QuestName)
{
	Quest_Update_Statistic(QuestName, enum_Quest_Update::Drop);
}
//����Ʈ�Ϸ� == ���������� 2�� �ٲ��ֱ�
void UActorComponent_Playfab::Quest_Complete(const FString& QuestName)
{
	Quest_Update_Statistic(QuestName, enum_Quest_Update::Complete);
}

bool UActorComponent_Playfab::Quest_Check_isDoing(const FString& QuestName)
{
	for (auto it : MyQuest_Info)
	{
		if (it.Quest_Name == QuestName) { return true; }
	}
	return false;
}

bool UActorComponent_Playfab::Quest_Check_IsComplete(const FString& QuestName)
{
	TArray<FString> QuestKey;
	PlayFab_Statistics.GetKeys(QuestKey);
	for (auto it : QuestKey)
	{
		if ((it == QuestName) && (2 == *PlayFab_Statistics.Find(it))) { return true; }	// ������������ ����Ʈ�̸��� ã�Ƽ�  value�� 2�ΰ͵鸸
	}

	//��ã���� false
	return false;
}

bool UActorComponent_Playfab::Quest_Check_IsLastFinished(const FString& QuestName)
{
	int FindQuest = FindQuestInfo_Index(QuestName);
	if (FindQuest == -1) { return  false; }

	int StepTotalCount = QuestTotalStepcount(QuestName);

	if (MyQuest_Info[FindQuest].Quest_Step >= StepTotalCount) // ���൵�� �ִ����̺��� �Ѿ�� ��� �Ϸ��ߴ��� �ľ��ϱ�
	{
		for (auto it : MyQuest_Info[FindQuest].IsFinished)
		{
			if (!it) { return false; }// �ϳ��� ����Ʈ�� �Ϸ����� ���ѻ��¶�� falseŻ��
		}
		return true; //��οϷ��ߴٸ� true Ż��
	}

	//���൵�� �����Ѱ��
	return false;
}

enum_Quest_Condition UActorComponent_Playfab::Quest_Check_Condition(const FString& QuestName)
{
	if (Quest_Check_isDoing(QuestName))
	{
		if (Quest_Check_IsLastFinished(QuestName))//������ ����ޱ� ���� �Դ�������
		{
			return enum_Quest_Condition::LastFinished;
		}
		return enum_Quest_Condition::IsDoing;//�׳� �������϶�
	}
	else if (Quest_Check_IsComplete(QuestName)) { return	enum_Quest_Condition::IsComplete; } // ������� �ް� ���������϶�

	return enum_Quest_Condition::NotStart;
}


//���� ������ ������ ������Ʈ�ϱ�
void UActorComponent_Playfab::UpdateAchievement(int AchieveNumber)
{
	FString Temp;
	FString AchieveCount = FString::FromInt(AchieveNumber); // AchieveNumber = 001~999 Naming ���߱��
	for (int i = 0; i < 3 - AchieveCount.Len(); i++)
	{
		Temp += "0";
	}
	AchieveCount = Temp + AchieveCount;

	FString FullName = AchieveName + AchieveCount;
	PlayFabClientPtr ClientAPI = IPlayFabModuleInterface::Get().GetClientAPI();
	TSharedPtr<FJsonObject> JsonObj = MakeShareable(new FJsonObject());
	JsonObj->SetStringField("name", FullName);
	PlayFab::ClientModels::FExecuteCloudScriptRequest request;
	request.FunctionName = "Upload_StatisticName";
	request.FunctionParameter = JsonObj;
	request.GeneratePlayStreamEvent = true;
	ClientAPI->ExecuteCloudScript(
		request,
		UPlayFabClientAPI::FExecuteCloudScriptDelegate::CreateLambda([&, FullName, AchieveCount](const PlayFab::ClientModels::FExecuteCloudScriptResult& result)
			{
				//����������Ʈ ������ ȣ��
				PlayFab_Statistics.Add(FullName, 0);
				PlayerOwner->Finished_UpdateAchieve(AchieveCount);
			}));

}
//���������͸� �߷��� ��ȯ
TArray<FString> UActorComponent_Playfab::GetAchievement()
{
	TArray<FString> AchieveNumber;
	TArray<FString> Keys;
	PlayFab_Statistics.GetKeys(Keys);
	for (auto it : Keys)
	{
		if (it.Find(AchieveName) == 0)
		{
			AchieveNumber.Add(it.Right(it.Len() - AchieveName.Len()));
		}
	}
	return AchieveNumber;
}