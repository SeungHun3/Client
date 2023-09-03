// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
//#include "../../Struct_Customizing.h"

#include "Widget_ShopMain.generated.h"

/**
 *
 */
UCLASS()
class TEST_LIDARSAMPLE_API UWidget_ShopMain : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<class UWidget_ShopSlot*> SlotWidgetArray;

	//낮은가격순 Array정렬
	UFUNCTION(BlueprintCallable)
	void Min_Price_Sort(TArray<class UWidget_ShopSlot*> SlotArray);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SelectClass(int TabCount);

};
