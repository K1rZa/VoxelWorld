// VoxelItemData.h
// Data Asset для хранения информации о предмете

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "VoxelItemData.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
    Block,      // Блок для размещения
    Tool,       // Инструмент
    Weapon,     // Оружие
    Consumable, // Расходуемый предмет
    Material,   // Материал для крафта
    Special     // Специальный предмет
};

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
    Common,
    Uncommon,
    Rare,
    Epic,
    Legendary
};

// Forward declaration
class UVoxelBlockData;

UCLASS(BlueprintType)
class VOXELWORLD_API UVoxelItemData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    // Уникальный ID предмета
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FName ItemID;
    
    // Отображаемое имя
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FText DisplayName;
    
    // Описание
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FText Description;
    
    // Тип предмета
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    EItemType ItemType = EItemType::Block;
    
    // Редкость
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    EItemRarity Rarity = EItemRarity::Common;
    
    // Иконка
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSoftObjectPtr<UTexture2D> Icon;
    
    // Цвет для UI (если нет иконки)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    FColor IconColor = FColor::White;
    
    // Максимальный размер стака
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = "1", ClampMax = "999"))
    int32 MaxStackSize = 64;
    
    // Ссылка на данные блока (если это блок)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Block", meta = (EditCondition = "ItemType == EItemType::Block"))
    TSoftObjectPtr<UVoxelBlockData> BlockData;
    
    // Можно ли выбросить
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties")
    bool bCanDrop = true;
    
    // Можно ли торговать
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties")
    bool bCanTrade = true;
    
    // Базовая цена
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Economy", meta = (ClampMin = "0"))
    int32 BaseValue = 1;

    virtual FPrimaryAssetId GetPrimaryAssetId() const override
    {
        return FPrimaryAssetId("VoxelItem", GetFName());
    }
    
    // Получить цвет редкости
    UFUNCTION(BlueprintCallable, Category = "Item")
    FColor GetRarityColor() const
    {
        switch (Rarity)
        {
            case EItemRarity::Common:    return FColor(200, 200, 200);
            case EItemRarity::Uncommon:  return FColor(30, 255, 30);
            case EItemRarity::Rare:      return FColor(30, 144, 255);
            case EItemRarity::Epic:      return FColor(163, 53, 238);
            case EItemRarity::Legendary: return FColor(255, 165, 0);
            default: return FColor::White;
        }
    }
};
