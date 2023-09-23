TArray<FString> DataList;

FString str = "myData";

const TCHAR* TempTCHAR = *str;
const char* Temp = new char[FCString::Strlen(TempTCHAR) + 1];
FCStringAnsi::Strcpy(const_cast<char*>(Temp), FCString::Strlen(TempTCHAR) + 1, TCHAR_TO_UTF8(TempTCHAR));

JString Data = ""; //추가데이터

if (changes.contains(Temp))
{
	Data = ((ValueObject<JString>*)changes.getValue(Temp))->getDataCopy();
	FString cc = FString(UTF8_TO_TCHAR(Data.UTF8Representation().cstr()));
	DataList.Add(cc);
}

FMemory::Free((void*)Temp);
it->SetCostumeArray(CostumeList);