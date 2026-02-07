// VoxelBlockData.h
// Data Asset для хранения информации о блоке

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "VoxelBlockData.generated.h"

UENUM(BlueprintType)
enum class EBlockCategory : uint8
{
    Natural,    // Природные (Stone, Dirt, Sand)
    Wood,       // Дерево
    Building,   // Строительные (Brick, Concrete)
    Decorative, // Декоративные
    Special     // Специальные (Glass, etc)
};

UCLASS(BlueprintType)
class VOXELWORLD_API UVoxelBlockData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    // Уникальный ID блока
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Block")
    FName BlockID;
    
    // Отображаемое имя
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Block")
    FText DisplayName;
    
    // Описание
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Block")
    FText Description;
    
    // Категория блока
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Block")
    EBlockCategory Category = EBlockCategory::Natural;
    
    // Цвет блока
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance")
    FColor BlockColor = FColor(128, 128, 128);
    
    // Материал блока (обязательно для текстур)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance")
    TSoftObjectPtr<UMaterialInterface> Material;
    
    // Индекс материала для группировки (блоки с одинаковым индексом рендерятся вместе)
    // -1 = использовать дефолтный материал с вертексными цветами
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance")
    int32 MaterialIndex = -1;
    
    // Иконка для UI
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSoftObjectPtr<UTexture2D> Icon;
    
    // Размер блока (true = большой 80cm, false = маленький 20cm)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties")
    bool bIsLargeBlock = true;
    
    // Можно ли разрушить
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties")
    bool bIsDestructible = true;
    
    // Прочность (время разрушения в секундах)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties", meta = (EditCondition = "bIsDestructible"))
    float Hardness = 1.0f;
    
    // Прозрачный ли блок (для Glass и подобных)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties")
    bool bIsTransparent = false;
    
    // Излучает ли свет
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties")
    bool bEmitsLight = false;
    
    // Интенсивность света
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties", meta = (EditCondition = "bEmitsLight", ClampMin = "0", ClampMax = "15"))
    int32 LightLevel = 0;
    
    // Звук при размещении
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
    TSoftObjectPtr<USoundBase> PlaceSound;
    
    // Звук при разрушении
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
    TSoftObjectPtr<USoundBase> BreakSound;
    
    // Звук шагов
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
    TSoftObjectPtr<USoundBase> FootstepSound;

    // Override для Asset Manager
    virtual FPrimaryAssetId GetPrimaryAssetId() const override
    {
        return FPrimaryAssetId("VoxelBlock", GetFName());
    }
};
