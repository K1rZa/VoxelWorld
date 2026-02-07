// VoxelInventoryComponent.h
// Компонент инвентаря — хранение данных, перетаскивание, категории

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VoxelHotbarItem.h"
#include "VoxelInventoryComponent.generated.h"

class UVoxelBlockData;

UENUM(BlueprintType)
enum class EInventoryCategory : uint8
{
    LargeBlocks   UMETA(DisplayName = "Large Blocks"),
    SmallBlocks   UMETA(DisplayName = "Small Blocks"),
    Items         UMETA(DisplayName = "Items"),
    Count         UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FInventoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName BlockID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FColor Color = FColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EInventoryCategory Category = EInventoryCategory::LargeBlocks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsLargeBlock = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxStackSize = 64;

    FInventoryEntry() {}
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VOXELWORLD_API UVoxelInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UVoxelInventoryComponent();

    virtual void BeginPlay() override;

    // === Hotbar ===

    static constexpr int32 HotbarSize = 9;

    UFUNCTION(BlueprintCallable, Category = "Inventory|Hotbar")
    TArray<FVoxelHotbarSlot> GetHotbarSlots() const { return HotbarSlots; }

    UFUNCTION(BlueprintCallable, Category = "Inventory|Hotbar")
    FVoxelHotbarSlot GetHotbarSlot(int32 Index) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory|Hotbar")
    void SetHotbarSlot(int32 Index, FName BlockID, int32 Count = 64);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Hotbar")
    void ClearHotbarSlot(int32 Index);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Hotbar")
    int32 GetSelectedSlot() const { return SelectedSlot; }

    UFUNCTION(BlueprintCallable, Category = "Inventory|Hotbar")
    void SetSelectedSlot(int32 Index);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Hotbar")
    UVoxelBlockData* GetSelectedBlockData() const;

    // === Inventory catalog (dynamically populated from VoxelDatabase) ===

    UFUNCTION(BlueprintCallable, Category = "Inventory|Catalog")
    void RefreshCatalog();

    UFUNCTION(BlueprintCallable, Category = "Inventory|Catalog")
    const TArray<FInventoryEntry>& GetCatalogForCategory(EInventoryCategory Category) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory|Catalog")
    int32 GetCategoryCount() const { return static_cast<int32>(EInventoryCategory::Count); }

    // === Inventory UI state ===

    UPROPERTY(BlueprintReadWrite, Category = "Inventory|UI")
    bool bIsInventoryOpen = false;

    UPROPERTY(BlueprintReadWrite, Category = "Inventory|UI")
    EInventoryCategory ActiveCategory = EInventoryCategory::LargeBlocks;

    UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
    void ToggleInventory();

    UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
    void SetCategory(EInventoryCategory Category);

    // Поместить предмет из каталога в хотбар (клик по предмету)
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool PlaceItemInHotbar(FName BlockID);

    // Переместить предмет в определённый слот хотбара
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void PlaceItemInHotbarSlot(FName BlockID, int32 SlotIndex);

    // Simple multicast delegates (no UPROPERTY — safe for hot reload)
    DECLARE_MULTICAST_DELEGATE(FOnInventoryChangedNative);
    FOnInventoryChangedNative OnInventoryChanged;

    DECLARE_MULTICAST_DELEGATE_OneParam(FOnInventoryToggledNative, bool);
    FOnInventoryToggledNative OnInventoryToggled;

private:
    UPROPERTY()
    TArray<FVoxelHotbarSlot> HotbarSlots;

    int32 SelectedSlot = 0;

    // Каталог — заполняется из VoxelDatabase
    TArray<FInventoryEntry> LargeBlocks;
    TArray<FInventoryEntry> SmallBlocks;
    TArray<FInventoryEntry> ItemEntries;

    // Пустой массив для безопасного возврата
    TArray<FInventoryEntry> EmptyEntries;

    void InitializeHotbar();
};
