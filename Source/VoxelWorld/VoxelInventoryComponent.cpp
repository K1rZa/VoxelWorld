// VoxelInventoryComponent.cpp

#include "VoxelInventoryComponent.h"
#include "VoxelDatabase.h"
#include "VoxelBlockData.h"
#include "GameFramework/PlayerController.h"

UVoxelInventoryComponent::UVoxelInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UVoxelInventoryComponent::BeginPlay()
{
    Super::BeginPlay();
    InitializeHotbar();
    RefreshCatalog();
}

// === Hotbar ===

void UVoxelInventoryComponent::InitializeHotbar()
{
    HotbarSlots.SetNum(HotbarSize);
    // Хотбар полностью пустой при старте
    for (int32 i = 0; i < HotbarSize; i++)
    {
        HotbarSlots[i].Clear();
    }
}

FVoxelHotbarSlot UVoxelInventoryComponent::GetHotbarSlot(int32 Index) const
{
    if (HotbarSlots.IsValidIndex(Index))
    {
        return HotbarSlots[Index];
    }
    return FVoxelHotbarSlot();
}

void UVoxelInventoryComponent::SetHotbarSlot(int32 Index, FName BlockID, int32 Count)
{
    if (HotbarSlots.IsValidIndex(Index))
    {
        HotbarSlots[Index] = FVoxelHotbarSlot(BlockID, Count);
        OnInventoryChanged.Broadcast();
    }
}

void UVoxelInventoryComponent::ClearHotbarSlot(int32 Index)
{
    if (HotbarSlots.IsValidIndex(Index))
    {
        HotbarSlots[Index].Clear();
        OnInventoryChanged.Broadcast();
    }
}

void UVoxelInventoryComponent::SetSelectedSlot(int32 Index)
{
    SelectedSlot = FMath::Clamp(Index, 0, HotbarSize - 1);
    OnInventoryChanged.Broadcast();
}

UVoxelBlockData* UVoxelInventoryComponent::GetSelectedBlockData() const
{
    FVoxelHotbarSlot Slot = GetHotbarSlot(SelectedSlot);
    if (Slot.IsEmpty()) return nullptr;

    UVoxelDatabase* DB = UVoxelDatabase::Get();
    if (!DB) return nullptr;

    return DB->GetBlockData(Slot.BlockID);
}

// === Catalog ===

void UVoxelInventoryComponent::RefreshCatalog()
{
    LargeBlocks.Empty();
    SmallBlocks.Empty();
    ItemEntries.Empty();

    UVoxelDatabase* DB = UVoxelDatabase::Get();
    if (!DB) return;

    TArray<UVoxelBlockData*> AllBlocks = DB->GetAllBlocks();

    for (UVoxelBlockData* Block : AllBlocks)
    {
        if (!Block) continue;

        FInventoryEntry Entry;
        Entry.BlockID = Block->BlockID;
        Entry.DisplayName = Block->DisplayName;
        Entry.Color = Block->BlockColor;
        Entry.bIsLargeBlock = Block->bIsLargeBlock;
        Entry.MaxStackSize = 64;

        if (Block->bIsLargeBlock)
        {
            Entry.Category = EInventoryCategory::LargeBlocks;
            LargeBlocks.Add(Entry);
        }
        else
        {
            Entry.Category = EInventoryCategory::SmallBlocks;
            SmallBlocks.Add(Entry);
        }
    }

    // Раздел Items пока пустой — сюда пойдут инструменты, ресурсы и т.д.
    // Можно заполнить из ItemRegistry:
    TArray<UVoxelItemData*> AllItems = DB->GetAllItems();
    for (UVoxelItemData* Item : AllItems)
    {
        if (!Item) continue;
        // Пропускаем блочные предметы — они уже в LargeBlocks/SmallBlocks
        if (Item->ItemType == EItemType::Block) continue;

        FInventoryEntry Entry;
        Entry.BlockID = Item->ItemID;
        Entry.DisplayName = Item->DisplayName;
        Entry.Color = Item->IconColor;
        Entry.Category = EInventoryCategory::Items;
        Entry.MaxStackSize = Item->MaxStackSize;
        ItemEntries.Add(Entry);
    }

    UE_LOG(LogTemp, Log, TEXT("Inventory catalog: %d large, %d small, %d items"),
        LargeBlocks.Num(), SmallBlocks.Num(), ItemEntries.Num());
}

const TArray<FInventoryEntry>& UVoxelInventoryComponent::GetCatalogForCategory(EInventoryCategory Category) const
{
    switch (Category)
    {
        case EInventoryCategory::LargeBlocks: return LargeBlocks;
        case EInventoryCategory::SmallBlocks: return SmallBlocks;
        case EInventoryCategory::Items:       return ItemEntries;
        default: return EmptyEntries;
    }
}

// === UI State ===

void UVoxelInventoryComponent::ToggleInventory()
{
    bIsInventoryOpen = !bIsInventoryOpen;
    OnInventoryToggled.Broadcast(bIsInventoryOpen);

    // Показываем/скрываем курсор
    APawn* Owner = Cast<APawn>(GetOwner());
    if (!Owner) return;
    APlayerController* PC = Cast<APlayerController>(Owner->GetController());
    if (!PC) return;

    if (bIsInventoryOpen)
    {
        PC->bShowMouseCursor = true;
        PC->SetInputMode(FInputModeGameAndUI().SetHideCursorDuringCapture(false));
    }
    else
    {
        PC->bShowMouseCursor = false;
        PC->SetInputMode(FInputModeGameOnly());
    }
}

void UVoxelInventoryComponent::SetCategory(EInventoryCategory Category)
{
    if (Category < EInventoryCategory::Count)
    {
        ActiveCategory = Category;
    }
}

bool UVoxelInventoryComponent::PlaceItemInHotbar(FName BlockID)
{
    // Ищем первый пустой слот
    for (int32 i = 0; i < HotbarSize; i++)
    {
        if (HotbarSlots[i].IsEmpty())
        {
            SetHotbarSlot(i, BlockID, 64);
            return true;
        }
    }
    // Если нет пустых — заменяем выбранный
    SetHotbarSlot(SelectedSlot, BlockID, 64);
    return true;
}

void UVoxelInventoryComponent::PlaceItemInHotbarSlot(FName BlockID, int32 SlotIndex)
{
    SetHotbarSlot(SlotIndex, BlockID, 64);
}
