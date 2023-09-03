//퀘스트 구조체 찾기 못찾았다면 -1
int UActorComponent_Playfab::FindQuestInfo_Index(const FString& QuestName)
{
	int Index = -1;
	for (int i = 0; i < MyQuest_Info.Num(); i++)
	{	// 정보가지고 있다면 
		if (MyQuest_Info[i].Quest_Name == QuestName) { Index = i;	break; }
	}

	return Index;
}
// 퀘스트 PlayFab_Statistics 없음, value 0 = 시작가능, value 1 = 진행중, value 2 = 완료
void UActorComponent_Playfab::CheckQuestInfo()
{
	TArray<FString> QuestKey;
	PlayFab_Statistics.GetKeys(QuestKey);
	for (auto it : QuestKey)
	{	// 업적데이터중 퀘스트를 추려서  value가 1인것들만 == 진행중인 퀘스트만 담기
		if ((it.Left(5) == "Quest") && (UserTitleData.Contains(it)) && (1 == *PlayFab_Statistics.Find(it)))
		{
			FString val = *UserTitleData.Find(it);
			FQuest_Info Quest = MakeQuestInfo(it, val);
			MyQuest_Info.Add(Quest); //데이터 넣어주기
		}
	}
}
// 퀘스트 JSon구조 == QuestName : ParseData{ 진행단계+index(Key) : 완료여부(Value),  진행단계+index(Key) : 완료여부(Value)... }
FQuest_Info UActorComponent_Playfab::MakeQuestInfo(const FString& QuestName, const FString& JsonParseData) //ex) ParseData = {"1/n0":true,"1/n1":false}
{
	//json데이터 변환
	FQuest_Info Quest;
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonParseData);
	FJsonSerializer::Deserialize(JsonReader, JsonObject);
	TArray<FString> StepKeys;
	JsonObject->Values.GetKeys(StepKeys);


	// 퀘스트 이름세팅
	Quest.Quest_Name = QuestName;
	TArray<FString> StepParse;
	StepKeys[0].ParseIntoArray(StepParse, TEXT("/n"));
	//퀘스트 진행도
	Quest.Quest_Step = FCString::Atoi(*StepParse[0]);

	//진행도 부분 클리어체크
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
// 퀘스트 UserTitleData Update
void UActorComponent_Playfab::Quest_Update_Title(const FString& QuestName)
{
	PlayFabClientPtr ClientAPI = IPlayFabModuleInterface::Get().GetClientAPI();
	TSharedPtr<FJsonObject> JsonObj = MakeShareable(new FJsonObject());
	TSharedPtr<FJsonObject> JsonSubObj = MakeShareable(new FJsonObject());
	TArray< TSharedPtr<FJsonValue> > EntriesArray;

	EntriesArray.Add(MakeShareable(new FJsonValueString(QuestName))); // 퀘스트이름
	int CurrQuest = FindQuestInfo_Index(QuestName);
	if (CurrQuest == -1)
	{
		UE_LOG(LogTemp, Log, TEXT("// Invalid_Struct "));
		return;
	}

	FString Step = FString::FromInt(MyQuest_Info[CurrQuest].Quest_Step);

	for (int i = 0; i < MyQuest_Info[CurrQuest].IsFinished.Num(); i++)
	{
		FString StepIndex = Step + FString("/n") + FString::FromInt(i);			// 퀘스트 진행도 
		JsonSubObj->SetBoolField(StepIndex, MyQuest_Info[CurrQuest].IsFinished[i]);// 퀘스트 완료여부
	}
	EntriesArray.Add(MakeShareable(new FJsonValueObject(JsonSubObj))); // 퀘스트 정보  // Key(진행도 + /n + 퀘스트index ) :Value (완료여부)

	JsonObj->SetArrayField("Quest", EntriesArray);


	//서버로 전송
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

	PlayFab::ClientModels::FExecuteCloudScriptRequest request; // 업적데이터에 퀘스트 Value 2로 넣어주기
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
				case enum_Quest_Update::Drop:// 퀘스트포기

					PlayFab_Statistics.Add(QuestName, ENUM_Val); // 업적데이터 0 으로 변경
					for (int i = 0; i < MyQuest_Info.Num(); i++) // 퀘스트 데이터지우기
					{
						if (PlayFab_Statistics.Contains(QuestName)) { PlayFab_Statistics.Add(QuestName, 0); }
						if (MyQuest_Info[i].Quest_Name == QuestName) { MyQuest_Info.RemoveAt(i);	break; }
					}
					PlayerOwner->Quest_Drop(QuestName);
					break;


				case enum_Quest_Update::Start:	//퀘스트 시작

					PlayFab_Statistics.Add(QuestName, ENUM_Val); // 업적데이터 추가
					checkIndex = FindQuestInfo_Index(QuestName);					// 초기화
					if (checkIndex != -1) { MyQuest_Info.RemoveAt(checkIndex); }	//동일이름의 퀘스트명이 있다면 지우고
					Quest = SetQuestInfo(QuestName, 1); // 퀘스트 시작정보 구조체 담아서
					MyQuest_Info.Add(Quest); // 배열에 추가 후 
					Quest_Update_Title(QuestName); // 타이틀 데이터에 업데이트하기

					PlayerOwner->Quest_Start(Quest);
					break;

				case enum_Quest_Update::Complete:
					// 보상받기 // 받으면 타이틀 데이터 지우기
					for (int i = 0; i < MyQuest_Info.Num(); i++) // 퀘스트 데이터지우기
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
	Quest.Quest_Name = QuestName;	// 퀘스트 이름
	Quest.Quest_Step = Step;		// 퀘스트 진행도
	Quest.IsFinished.Reset();

	UDataTable* MyQuestTable = FindQuestTable(QuestName);
	TArray<int> RowNames = GetQuestRowNames(FString::FromInt(Step), MyQuestTable);
	int IndexSize = RowNames.Num();
	if (IndexSize == 0) { return Quest; } // 진행도가 없다면 Quest.IsFinished.Reset() 만 시키고 탈출

	for (int i = 0; i < IndexSize; i++)
	{
		Quest.IsFinished.Add(false); // 퀘스트 완료여부
	}

	//퀘스트(선택된데이터테이블)가 가지고 있는 데이터테이블(대화시스템)
	FString TablePropName = FString::FromInt(RowNames[0]); // 같은 진행도를 가진 인덱스중 첫번째의 데이터테이블
	FQuest_Info CurrQuestTable = *MyQuestTable->FindRow<FQuest_Info>(FName(*TablePropName), "NoData_Questinfo_Table");
	Quest.QuestTable = CurrQuestTable.QuestTable;

	return Quest;
}

//같은 진행도를 가진 테이블 행 숫자뽑아서 배열가져오기 // 프로퍼티 Quest_Step 을 진행도로 탐색 (FQuest_Info Quest; Quest.Quest_Step;)
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
// 퀘스트 총 진행도구하기
int UActorComponent_Playfab::QuestTotalStepcount(const FString& QuestName)
{
	UDataTable* CheckTable = FindQuestTable(QuestName);
	TArray<FName> checkRows = CheckTable->GetRowNames(); // 모든행을 구해서
	TArray<int> Totalindex;
	for (auto it : checkRows)
	{
		FQuest_Info* checkProp = CheckTable->FindRow<FQuest_Info>(it, "TotalStepCount, Nodata");

		if (checkProp) { Totalindex.AddUnique(checkProp->Quest_Step); }	// 프로퍼티 AddUnique로 추가 (중복없이넣기)
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
	if (CurrQuest == -1) { return false; }	// 잘못된 이름이거나
	if (!MyQuest_Info[CurrQuest].IsFinished.IsValidIndex(index)) { return false; }	// 잘못된 인덱스 접근이라면

	MyQuest_Info[CurrQuest].IsFinished[index] = true;
	Quest_Update_Title(QuestName);

	for (int i = 0; i < MyQuest_Info[CurrQuest].IsFinished.Num(); i++)
	{
		if (!MyQuest_Info[CurrQuest].IsFinished[i]) { return false; } // index가 전부 true가 아니라면
	}

	//Step의 index가 모두 true라면
	return true;
}

void UActorComponent_Playfab::Quest_Next(const FString& QuestName)
{
	int CurrQuest = FindQuestInfo_Index(QuestName);
	if (CurrQuest == -1) { return; }	// 잘못된 이름이라면 탈출

	int NextStep = MyQuest_Info[CurrQuest].Quest_Step + 1;

	FQuest_Info Quest;
	Quest = SetQuestInfo(QuestName, NextStep);
	// 퀘스트가 끝났다면						// 퀘스트 완료 업적 업데이트
	if (!Quest.IsFinished.IsValidIndex(0)) { Quest_Complete(QuestName);	return; }

	MyQuest_Info[CurrQuest] = Quest; //정보담고
	PlayerOwner->Quest_Next(MyQuest_Info[CurrQuest]);
	Quest_Update_Title(QuestName);		// 타이틀 데이터에 업데이트하기
}
//퀘스트포기 == 업적데이터 0로 바꿔주기
void UActorComponent_Playfab::Quest_Drop(const FString& QuestName)
{
	Quest_Update_Statistic(QuestName, enum_Quest_Update::Drop);
}
//퀘스트완료 == 업적데이터 2로 바꿔주기
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
		if ((it == QuestName) && (2 == *PlayFab_Statistics.Find(it))) { return true; }	// 업적데이터중 퀘스트이름을 찾아서  value가 2인것들만
	}

	//못찾으면 false
	return false;
}

bool UActorComponent_Playfab::Quest_Check_IsLastFinished(const FString& QuestName)
{
	int FindQuest = FindQuestInfo_Index(QuestName);
	if (FindQuest == -1) { return  false; }

	int StepTotalCount = QuestTotalStepcount(QuestName);

	if (MyQuest_Info[FindQuest].Quest_Step >= StepTotalCount) // 진행도가 최대테이블을 넘어서는 경우 완료했는지 파악하기
	{
		for (auto it : MyQuest_Info[FindQuest].IsFinished)
		{
			if (!it) { return false; }// 하나라도 퀘스트를 완료하지 못한상태라면 false탈출
		}
		return true; //모두완료했다면 true 탈출
	}

	//진행도가 부족한경우
	return false;
}

enum_Quest_Condition UActorComponent_Playfab::Quest_Check_Condition(const FString& QuestName)
{
	if (Quest_Check_isDoing(QuestName))
	{
		if (Quest_Check_IsLastFinished(QuestName))//마지막 보상받기 까지 왔는지여부
		{
			return enum_Quest_Condition::LastFinished;
		}
		return enum_Quest_Condition::IsDoing;//그냥 진행중일때
	}
	else if (Quest_Check_IsComplete(QuestName)) { return	enum_Quest_Condition::IsComplete; } // 보상까지 받고 끝난상태일때

	return enum_Quest_Condition::NotStart;
}


//업적 데이터 서버로 업데이트하기
void UActorComponent_Playfab::UpdateAchievement(int AchieveNumber)
{
	FString Temp;
	FString AchieveCount = FString::FromInt(AchieveNumber); // AchieveNumber = 001~999 Naming 맞추기용
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
				//서버업데이트 끝날때 호출
				PlayFab_Statistics.Add(FullName, 0);
				PlayerOwner->Finished_UpdateAchieve(AchieveCount);
			}));

}
//업적데이터만 추려서 반환
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