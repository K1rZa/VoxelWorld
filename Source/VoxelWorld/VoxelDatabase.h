// VoxelDatabase.h
// Singleton менеджер для доступа к базе данных блоков и предметов

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "VoxelBlockData.h"
#include "VoxelItemData.h"
#include "VoxelDatabase.generated.h"

// Структура для DataTable блоков (альтернативный способ)
USTRUCT(BlueprintType)
struct FVoxelBlockTableRow : public FTableRowBase
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName BlockID;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DisplayName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FColor BlockColor = FColor(128, 128, 128);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsLargeBlock = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsDestructible = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Hardness = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsTransparent = false;
};

// Структура для DataTable предметов
USTRUCT(BlueprintType)
struct FVoxelItemTableRow : public FTableRowBase
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ItemID;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DisplayName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EItemType ItemType = EItemType::Block;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FColor IconColor = FColor::White;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxStackSize = 64;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName LinkedBlockID; // Ссылка на блок по ID
};

UCLASS(Blueprintable, BlueprintType)
class VOXELWORLD_API UVoxelDatabase : public UObject
{
    GENERATED_BODY()

public:
    // Получить singleton instance
    UFUNCTION(BlueprintCallable, Category = "Voxel Database")
    static UVoxelDatabase* Get();
    
    // Инициализация базы данных
    UFUNCTION(BlueprintCallable, Category = "Voxel Database")
    void Initialize();
    
    // === БЛОКИ ===
    
    // Получить данные блока по ID
    UFUNCTION(BlueprintCallable, Category = "Voxel Database|Blocks")
    UVoxelBlockData* GetBlockData(FName BlockID) const;
    
    // Получить данные блока по числовому индексу (для совместимости с EBlockType)
    UFUNCTION(BlueprintCallable, Category = "Voxel Database|Blocks")
    UVoxelBlockData* GetBlockDataByIndex(int32 Index) const;
    
    // Получить все блоки
    UFUNCTION(BlueprintCallable, Category = "Voxel Database|Blocks")
    TArray<UVoxelBlockData*> GetAllBlocks() const;
    
    // Получить блоки по категории
    UFUNCTION(BlueprintCallable, Category = "Voxel Database|Blocks")
    TArray<UVoxelBlockData*> GetBlocksByCategory(EBlockCategory Category) const;
    
    // Получить цвет блока по ID
    UFUNCTION(BlueprintCallable, Category = "Voxel Database|Blocks")
    FColor GetBlockColor(FName BlockID) const;
    
    // === МАТЕРИАЛЫ ===
    
    // Получить материал блока по ID
    UFUNCTION(BlueprintCallable, Category = "Voxel Database|Materials")
    UMaterialInterface* GetBlockMaterial(FName BlockID) const;
    
    // Получить индекс материала блока
    UFUNCTION(BlueprintCallable, Category = "Voxel Database|Materials")
    int32 GetBlockMaterialIndex(FName BlockID) const;
    
    // Зарегистрировать материал для индекса
    UFUNCTION(BlueprintCallable, Category = "Voxel Database|Materials")
    void RegisterMaterial(int32 MaterialIndex, UMaterialInterface* Material);
    
    // Получить материал по индексу
    UFUNCTION(BlueprintCallable, Category = "Voxel Database|Materials")
    UMaterialInterface* GetMaterialByIndex(int32 MaterialIndex) const;
    
    // Получить все уникальные индексы материалов
    UFUNCTION(BlueprintCallable, Category = "Voxel Database|Materials")
    TArray<int32> GetAllMaterialIndices() const;
    
    // Получить количество материалов
    UFUNCTION(BlueprintCallable, Category = "Voxel Database|Materials")
    int32 GetMaterialCount() const;
    
    // === ПРЕДМЕТЫ ===
    
    // Получить данные предмета по ID
    UFUNCTION(BlueprintCallable, Category = "Voxel Database|Items")
    UVoxelItemData* GetItemData(FName ItemID) const;
    
    // Получить все предметы
    UFUNCTION(BlueprintCallable, Category = "Voxel Database|Items")
    TArray<UVoxelItemData*> GetAllItems() const;
    
    // Получить предметы по типу
    UFUNCTION(BlueprintCallable, Category = "Voxel Database|Items")
    TArray<UVoxelItemData*> GetItemsByType(EItemType Type) const;
    
    // === РЕГИСТРАЦИЯ ===
    
    // Зарегистрировать блок
    UFUNCTION(BlueprintCallable, Category = "Voxel Database")
    void RegisterBlock(UVoxelBlockData* BlockData);
    
    // Зарегистрировать предмет
    UFUNCTION(BlueprintCallable, Category = "Voxel Database")
    void RegisterItem(UVoxelItemData* ItemData);
    
    // Загрузить все Data Assets из папки
    UFUNCTION(BlueprintCallable, Category = "Voxel Database")
    void LoadAllDataAssets();

protected:
    // Создать блоки по умолчанию (если Data Assets не найдены)
    void CreateDefaultBlocks();
    void CreateDefaultItems();

private:
    static UVoxelDatabase* Instance;
    
    // Хранилище блоков
    UPROPERTY()
    TMap<FName, UVoxelBlockData*> BlockRegistry;
    
    // Хранилище предметов
    UPROPERTY()
    TMap<FName, UVoxelItemData*> ItemRegistry;
    
    // Индексированный массив блоков (для быстрого доступа по числу)
    UPROPERTY()
    TArray<UVoxelBlockData*> BlocksByIndex;
    
    // Реестр материалов по индексу
    UPROPERTY()
    TMap<int32, UMaterialInterface*> MaterialRegistry;
    
    bool bIsInitialized = false;
};
