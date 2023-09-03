// 메쉬 변경 시키는 함수 
void APawn_Player::ChangeMesh(const FString& ClassName, USkeletalMesh* Mesh)
{
	TArray<USkeletalMeshComponent*> MeshComponents;
	GetComponents<USkeletalMeshComponent>(MeshComponents);
	for (auto Parts : MeshComponents)
	{
		FString PartsName;
		PartsName = Parts->GetName();
		//UE_LOG(LogTemp, Log, TEXT("// Name :  %s "), *PartsName);
		if (PartsName == ClassName)
		{
			Parts->SetSkeletalMesh(Mesh);
		}
	}
}
