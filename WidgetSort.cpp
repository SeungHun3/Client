

#include "Widget_ShopMain.h"
#include "Widget_ShopSlot.h"


void UWidget_ShopMain::Min_Price_Sort(TArray<class UWidget_ShopSlot*> SlotArray)
{
    // 위젯이 가지고 있는 가격순으로 내림차순 정렬
    for (int i = 0; i < SlotArray.Num(); i++) /*Num == size*/
    {
        int minIndex = i;
        for (int j = i + 1; j < SlotArray.Num(); ++j)
        {
            if (SlotArray[j]->Item_Infomation.ItemPrice < SlotArray[minIndex]->Item_Infomation.ItemPrice)
            {
                minIndex = j;
            }
        }
        if (minIndex != i)
        {
            UWidget_ShopSlot* tempSlot = SlotArray[i];
            SlotArray[i] = SlotArray[minIndex];
            SlotArray[minIndex] = tempSlot;
        }
    }

}
