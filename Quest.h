// 퀘스트 
UENUM(BlueprintType)
enum class enum_Quest_Condition : uint8
{
	NotStart, // 시작가능
	IsDoing, // 진행중
	LastFinished,// 마지막 보상받기버튼 클릭안했을때
	IsComplete, // 완료

};

UPROPERTY(EditAnywhere, BlueprintReadWrite)
TArray<FQuest_Info> MyQuest_Info;	// 진행 중인 퀘스트만 담기
// 퀘스트 구조체배열에서 조건에 맞는 인덱스 반환
int FindQuestInfo_Index(const FString& QuestName);
// PlayFab_Statistics 탐색 -> 퀘스트 있으면 Title데이터 탐색 후 구조체 Add
void CheckQuestInfo();
// Json데이터 그대로 가져와 구조체만들기
FQuest_Info MakeQuestInfo(const FString& QuestName, const FString& JsonParseData);
// 퀘스트 테이블 가져오기
UDataTable* FindQuestTable(const FString& QuestName);
// 퀘스트 Playfab TitleData업데이트
void Quest_Update_Title(const FString& QuestName);
// 퀘스트 Playfab Statistic업데이트
void Quest_Update_Statistic(const FString& QuestName, enum_Quest_Update Update);

FQuest_Info SetQuestInfo(const FString& QuestName, int Step);
//같은 진행도를 가진 테이블 행 숫자뽑아서 배열가져오기 // 프로퍼티 Quest_Step 을 진행도로 탐색 (FQuest_Info Quest; Quest.Quest_Step;)
UFUNCTION(BlueprintCallable, BlueprintPure)
TArray<int> GetQuestRowNames(const FString& QuestStepProp, class UDataTable* QuestTable);

int QuestTotalStepcount(const FString& QuestName);


UFUNCTION(BlueprintCallable)
void Quest_Start(const FString& QuestName);
UFUNCTION(BlueprintCallable)
bool Quest_Finish(const FString& QuestName, int index);
UFUNCTION(BlueprintCallable)
void Quest_Next(const FString& QuestName);
UFUNCTION(BlueprintCallable)
void Quest_Drop(const FString& QuestName);

void Quest_Complete(const FString& QuestName);

bool Quest_Check_isDoing(const FString& QuestName);
bool Quest_Check_IsComplete(const FString& QuestName);
bool Quest_Check_IsLastFinished(const FString& QuestName);

UFUNCTION(BlueprintCallable)
enum_Quest_Condition Quest_Check_Condition(const FString& QuestName);