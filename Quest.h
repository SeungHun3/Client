// ����Ʈ 
UENUM(BlueprintType)
enum class enum_Quest_Condition : uint8
{
	NotStart, // ���۰���
	IsDoing, // ������
	LastFinished,// ������ ����ޱ��ư Ŭ����������
	IsComplete, // �Ϸ�

};

UPROPERTY(EditAnywhere, BlueprintReadWrite)
TArray<FQuest_Info> MyQuest_Info;	// ���� ���� ����Ʈ�� ���
// ����Ʈ ����ü�迭���� ���ǿ� �´� �ε��� ��ȯ
int FindQuestInfo_Index(const FString& QuestName);
// PlayFab_Statistics Ž�� -> ����Ʈ ������ Title������ Ž�� �� ����ü Add
void CheckQuestInfo();
// Json������ �״�� ������ ����ü�����
FQuest_Info MakeQuestInfo(const FString& QuestName, const FString& JsonParseData);
// ����Ʈ ���̺� ��������
UDataTable* FindQuestTable(const FString& QuestName);
// ����Ʈ Playfab TitleData������Ʈ
void Quest_Update_Title(const FString& QuestName);
// ����Ʈ Playfab Statistic������Ʈ
void Quest_Update_Statistic(const FString& QuestName, enum_Quest_Update Update);

FQuest_Info SetQuestInfo(const FString& QuestName, int Step);
//���� ���൵�� ���� ���̺� �� ���ڻ̾Ƽ� �迭�������� // ������Ƽ Quest_Step �� ���൵�� Ž�� (FQuest_Info Quest; Quest.Quest_Step;)
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