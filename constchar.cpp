TArray<FString> DataList;

FString str = "myData";

const char* Temp = *str;

JString Data = ""; // �߰�������

if (changes.contains(Temp))
{
	Data = ((ValueObject<JString>*)changes.getValue(Temp))->getDataCopy();
	FString cc = FString(UTF8_TO_TCHAR(Data.UTF8Representation().cstr()));
	DataList.Add(cc);
}
it->SetCostumeArray(CostumeList);
