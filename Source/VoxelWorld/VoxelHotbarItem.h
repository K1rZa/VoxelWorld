// VoxelHotbarItem.h

#pragma once

#include "CoreMinimal.h"
#include "VoxelHotbarItem.generated.h"

class UVoxelBlockData;
class UVoxelItemData;

USTRUCT(BlueprintType)
struct FVoxelHotbarSlot
{
    GENERATED_BODY()
    
    // ID блока из базы данных
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName BlockID;
    
    // Количество в слоте
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Count = 1;
    
    // Пустой ли слот
    bool IsEmpty() const { return BlockID.IsNone() || Count <= 0; }
    
    // Очистить слот
    void Clear() { BlockID = NAME_None; Count = 0; }
    
    FVoxelHotbarSlot() : BlockID(NAME_None), Count(0) {}
    FVoxelHotbarSlot(FName InBlockID, int32 InCount = 1) : BlockID(InBlockID), Count(InCount) {}
};
