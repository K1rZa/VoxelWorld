// VoxelDatabase.cpp

#include "VoxelDatabase.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

UVoxelDatabase* UVoxelDatabase::Instance = nullptr;

UVoxelDatabase* UVoxelDatabase::Get()
{
    if (!Instance)
    {
        Instance = NewObject<UVoxelDatabase>();
        Instance->AddToRoot(); // Prevent garbage collection
        Instance->Initialize();
    }
    return Instance;
}

void UVoxelDatabase::Initialize()
{
    if (bIsInitialized) return;
    
    // Пробуем загрузить Data Assets
    LoadAllDataAssets();
    
    // Если блоков нет, создаём дефолтные
    if (BlockRegistry.Num() == 0)
    {
        CreateDefaultBlocks();
    }
    
    // Если предметов нет, создаём дефолтные
    if (ItemRegistry.Num() == 0)
    {
        CreateDefaultItems();
    }
    
    bIsInitialized = true;
    
    UE_LOG(LogTemp, Log, TEXT("VoxelDatabase initialized with %d blocks and %d items"), 
           BlockRegistry.Num(), ItemRegistry.Num());
}

void UVoxelDatabase::LoadAllDataAssets()
{
    // Сначала пробуем через Asset Manager
    UAssetManager& AssetManager = UAssetManager::Get();
    
    TArray<FPrimaryAssetId> BlockAssetIds;
    AssetManager.GetPrimaryAssetIdList(FPrimaryAssetType("VoxelBlock"), BlockAssetIds);
    
    UE_LOG(LogTemp, Log, TEXT("Asset Manager found %d VoxelBlock assets"), BlockAssetIds.Num());
    
    for (const FPrimaryAssetId& AssetId : BlockAssetIds)
    {
        FSoftObjectPath AssetPath = AssetManager.GetPrimaryAssetPath(AssetId);
        if (UVoxelBlockData* BlockData = Cast<UVoxelBlockData>(AssetPath.TryLoad()))
        {
            RegisterBlock(BlockData);
            UE_LOG(LogTemp, Log, TEXT("Loaded block from Asset Manager: %s"), *BlockData->BlockID.ToString());
        }
    }
    
    // Если Asset Manager ничего не нашёл, загружаем напрямую из папки
    if (BlockRegistry.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Asset Manager found no blocks, trying direct load from /Game/Data/Blocks/"));
        
        // Прямая загрузка по известным путям
        TArray<FString> BlockPaths = {
            TEXT("/Game/Data/Blocks/DA_StoneL.DA_StoneL"),
            TEXT("/Game/Data/Blocks/DA_StoneS.DA_StoneS"),
            TEXT("/Game/Data/Blocks/DA_Grass.DA_Grass"),
            TEXT("/Game/Data/Blocks/DA_Sand.DA_Sand"),
            TEXT("/Game/Data/Blocks/DA_GrassL.DA_GrassL"),
            TEXT("/Game/Data/Blocks/DA_SandL.DA_SandL")
        };
        
        for (const FString& Path : BlockPaths)
        {
            if (UVoxelBlockData* BlockData = LoadObject<UVoxelBlockData>(nullptr, *Path))
            {
                RegisterBlock(BlockData);
                UE_LOG(LogTemp, Log, TEXT("Direct loaded block: %s (ID: %s)"), *Path, *BlockData->BlockID.ToString());
            }
        }
    }
    
    // Загружаем предметы
    TArray<FPrimaryAssetId> ItemAssetIds;
    AssetManager.GetPrimaryAssetIdList(FPrimaryAssetType("VoxelItem"), ItemAssetIds);
    
    for (const FPrimaryAssetId& AssetId : ItemAssetIds)
    {
        FSoftObjectPath AssetPath = AssetManager.GetPrimaryAssetPath(AssetId);
        if (UVoxelItemData* ItemData = Cast<UVoxelItemData>(AssetPath.TryLoad()))
        {
            RegisterItem(ItemData);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("LoadAllDataAssets complete: %d blocks loaded"), BlockRegistry.Num());
}

void UVoxelDatabase::CreateDefaultBlocks()
{
    // Создаём блоки программно (для тестирования без Data Assets)
    // MaterialIndex определяет группировку для рендеринга:
    // 0 = Natural (Stone, Dirt, Sand, Gravel)
    // 1 = Grass (отдельно для текстуры травы)
    // 2 = Wood
    // 3 = Building (Brick, Concrete, Cobblestone)
    // 4 = Decorative (Wool)
    // 5 = Special (Glass, Glowstone)
    
    auto CreateBlock = [this](FName ID, const FString& Name, FColor Color, EBlockCategory Cat, int32 MatIndex, bool bLarge = true, bool bTransparent = false)
    {
        UVoxelBlockData* Block = NewObject<UVoxelBlockData>(this);
        Block->BlockID = ID;
        Block->DisplayName = FText::FromString(Name);
        Block->BlockColor = Color;
        Block->Category = Cat;
        Block->MaterialIndex = MatIndex;
        Block->bIsLargeBlock = bLarge;
        Block->bIsTransparent = bTransparent;
        Block->Hardness = 1.0f;
        RegisterBlock(Block);
        return Block;
    };
    
    // Природные блоки (большие) - MaterialIndex 0
    CreateBlock("Stone", "Stone", FColor(128, 128, 128), EBlockCategory::Natural, 0);
    CreateBlock("Dirt", "Dirt", FColor(139, 90, 43), EBlockCategory::Natural, 0);
    CreateBlock("Sand", "Sand", FColor(238, 214, 175), EBlockCategory::Natural, 0);
    CreateBlock("Gravel", "Gravel", FColor(136, 140, 141), EBlockCategory::Natural, 0);
    
    // Трава - MaterialIndex 1 (отдельно для особой текстуры)
    CreateBlock("Grass", "Grass", FColor(34, 139, 34), EBlockCategory::Natural, 1);
    
    // Деревянные блоки - MaterialIndex 2
    CreateBlock("Wood", "Wood Log", FColor(160, 82, 45), EBlockCategory::Wood, 2);
    CreateBlock("Planks", "Wood Planks", FColor(222, 184, 135), EBlockCategory::Wood, 2);
    
    // Строительные блоки - MaterialIndex 3
    CreateBlock("Cobblestone", "Cobblestone", FColor(100, 100, 100), EBlockCategory::Building, 3);
    CreateBlock("Brick", "Brick", FColor(178, 34, 34), EBlockCategory::Building, 3);
    CreateBlock("StoneBrick", "Stone Brick", FColor(120, 120, 120), EBlockCategory::Building, 3);
    CreateBlock("Concrete", "Concrete", FColor(180, 180, 180), EBlockCategory::Building, 3);
    
    // Декоративные блоки - MaterialIndex 4
    CreateBlock("Wool_White", "White Wool", FColor(255, 255, 255), EBlockCategory::Decorative, 4);
    CreateBlock("Wool_Red", "Red Wool", FColor(200, 50, 50), EBlockCategory::Decorative, 4);
    CreateBlock("Wool_Blue", "Blue Wool", FColor(50, 50, 200), EBlockCategory::Decorative, 4);
    CreateBlock("Wool_Green", "Green Wool", FColor(50, 200, 50), EBlockCategory::Decorative, 4);
    
    // Специальные блоки - MaterialIndex 5
    auto Glass = CreateBlock("Glass", "Glass", FColor(200, 220, 255), EBlockCategory::Special, 5, true, true);
    Glass->bIsTransparent = true;
    
    auto Glowstone = CreateBlock("Glowstone", "Glowstone", FColor(255, 230, 150), EBlockCategory::Special, 5);
    Glowstone->bEmitsLight = true;
    Glowstone->LightLevel = 15;
    
    // Маленькие версии блоков (используют те же индексы материалов)
    CreateBlock("Stone_Small", "Stone (Small)", FColor(128, 128, 128), EBlockCategory::Natural, 0, false);
    CreateBlock("Brick_Small", "Brick (Small)", FColor(178, 34, 34), EBlockCategory::Building, 3, false);
    CreateBlock("Planks_Small", "Planks (Small)", FColor(222, 184, 135), EBlockCategory::Wood, 2, false);
    CreateBlock("Cobblestone_Small", "Cobblestone (Small)", FColor(100, 100, 100), EBlockCategory::Building, 3, false);
    CreateBlock("Concrete_Small", "Concrete (Small)", FColor(180, 180, 180), EBlockCategory::Building, 3, false);
}

void UVoxelDatabase::CreateDefaultItems()
{
    auto CreateBlockItem = [this](FName ItemID, FName BlockID, const FString& Name, FColor Color)
    {
        UVoxelItemData* Item = NewObject<UVoxelItemData>(this);
        Item->ItemID = ItemID;
        Item->DisplayName = FText::FromString(Name);
        Item->ItemType = EItemType::Block;
        Item->IconColor = Color;
        Item->MaxStackSize = 64;
        
        // Связываем с блоком
        if (UVoxelBlockData* Block = GetBlockData(BlockID))
        {
            Item->BlockData = Block;
        }
        
        RegisterItem(Item);
        return Item;
    };
    
    // Создаём предметы для всех блоков
    for (const auto& Pair : BlockRegistry)
    {
        UVoxelBlockData* Block = Pair.Value;
        FName ItemID = FName(*FString::Printf(TEXT("Item_%s"), *Block->BlockID.ToString()));
        CreateBlockItem(ItemID, Block->BlockID, Block->DisplayName.ToString(), Block->BlockColor);
    }
}

void UVoxelDatabase::RegisterBlock(UVoxelBlockData* BlockData)
{
    if (!BlockData) return;
    
    BlockRegistry.Add(BlockData->BlockID, BlockData);
    BlocksByIndex.Add(BlockData);
    
    UE_LOG(LogTemp, Verbose, TEXT("Registered block: %s"), *BlockData->BlockID.ToString());
}

void UVoxelDatabase::RegisterItem(UVoxelItemData* ItemData)
{
    if (!ItemData) return;
    
    ItemRegistry.Add(ItemData->ItemID, ItemData);
    
    UE_LOG(LogTemp, Verbose, TEXT("Registered item: %s"), *ItemData->ItemID.ToString());
}

UVoxelBlockData* UVoxelDatabase::GetBlockData(FName BlockID) const
{
    if (const UVoxelBlockData* const* Found = BlockRegistry.Find(BlockID))
    {
        return const_cast<UVoxelBlockData*>(*Found);
    }
    return nullptr;
}

UVoxelBlockData* UVoxelDatabase::GetBlockDataByIndex(int32 Index) const
{
    if (BlocksByIndex.IsValidIndex(Index))
    {
        return BlocksByIndex[Index];
    }
    return nullptr;
}

TArray<UVoxelBlockData*> UVoxelDatabase::GetAllBlocks() const
{
    TArray<UVoxelBlockData*> Result;
    BlockRegistry.GenerateValueArray(Result);
    return Result;
}

TArray<UVoxelBlockData*> UVoxelDatabase::GetBlocksByCategory(EBlockCategory Category) const
{
    TArray<UVoxelBlockData*> Result;
    for (const auto& Pair : BlockRegistry)
    {
        if (Pair.Value->Category == Category)
        {
            Result.Add(Pair.Value);
        }
    }
    return Result;
}

FColor UVoxelDatabase::GetBlockColor(FName BlockID) const
{
    if (UVoxelBlockData* Block = GetBlockData(BlockID))
    {
        return Block->BlockColor;
    }
    return FColor::White;
}

UVoxelItemData* UVoxelDatabase::GetItemData(FName ItemID) const
{
    if (const UVoxelItemData* const* Found = ItemRegistry.Find(ItemID))
    {
        return const_cast<UVoxelItemData*>(*Found);
    }
    return nullptr;
}

TArray<UVoxelItemData*> UVoxelDatabase::GetAllItems() const
{
    TArray<UVoxelItemData*> Result;
    ItemRegistry.GenerateValueArray(Result);
    return Result;
}

TArray<UVoxelItemData*> UVoxelDatabase::GetItemsByType(EItemType Type) const
{
    TArray<UVoxelItemData*> Result;
    for (const auto& Pair : ItemRegistry)
    {
        if (Pair.Value->ItemType == Type)
        {
            Result.Add(Pair.Value);
        }
    }
    return Result;
}

// === МАТЕРИАЛЫ ===

UMaterialInterface* UVoxelDatabase::GetBlockMaterial(FName BlockID) const
{
    if (UVoxelBlockData* Block = GetBlockData(BlockID))
    {
        if (Block->Material.IsValid() || !Block->Material.IsNull())
        {
            return Block->Material.LoadSynchronous();
        }
    }
    return nullptr;
}

int32 UVoxelDatabase::GetBlockMaterialIndex(FName BlockID) const
{
    if (UVoxelBlockData* Block = GetBlockData(BlockID))
    {
        return FMath::Max(0, Block->MaterialIndex);
    }
    return 0; // Дефолтная секция для неизвестных блоков
}

void UVoxelDatabase::RegisterMaterial(int32 MaterialIndex, UMaterialInterface* Material)
{
    if (Material && MaterialIndex >= 0)
    {
        MaterialRegistry.Add(MaterialIndex, Material);
        UE_LOG(LogTemp, Log, TEXT("Registered material at index %d: %s"), MaterialIndex, *Material->GetName());
    }
}

UMaterialInterface* UVoxelDatabase::GetMaterialByIndex(int32 MaterialIndex) const
{
    if (const UMaterialInterface* const* Found = MaterialRegistry.Find(MaterialIndex))
    {
        return const_cast<UMaterialInterface*>(*Found);
    }
    return nullptr;
}

TArray<int32> UVoxelDatabase::GetAllMaterialIndices() const
{
    TArray<int32> Indices;
    
    // Собираем все уникальные индексы из блоков
    TSet<int32> UniqueIndices;
    for (const auto& Pair : BlockRegistry)
    {
        UniqueIndices.Add(Pair.Value->MaterialIndex);
    }
    
    Indices = UniqueIndices.Array();
    Indices.Sort();
    
    return Indices;
}

int32 UVoxelDatabase::GetMaterialCount() const
{
    return GetAllMaterialIndices().Num();
}
