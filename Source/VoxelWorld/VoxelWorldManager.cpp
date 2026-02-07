// VoxelWorldManager.cpp

#include "VoxelWorldManager.h"
#include "VoxelChunk.h"
#include "VoxelDatabase.h"
#include "Kismet/GameplayStatics.h"

AVoxelWorldManager* AVoxelWorldManager::Instance = nullptr;

AVoxelWorldManager::AVoxelWorldManager()
{
    PrimaryActorTick.bCanEverTick = true;
    LastPlayerChunk = FIntVector2(-9999, -9999);
}

void AVoxelWorldManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Инициализируем базу данных
    UVoxelDatabase::Get();
    
    Instance = this;
    PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    
    if (PlayerPawn)
    {
        UpdateChunks();
    }
}

void AVoxelWorldManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    if (Instance == this) Instance = nullptr;
}

void AVoxelWorldManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!PlayerPawn)
    {
        PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (PlayerPawn) UpdateChunks();
        return;
    }
    
    FIntVector2 CurrentChunk = WorldToChunkCoords(PlayerPawn->GetActorLocation());
    if (CurrentChunk != LastPlayerChunk)
    {
        UpdateChunks();
        LastPlayerChunk = CurrentChunk;
    }
}

FIntVector2 AVoxelWorldManager::WorldToChunkCoords(const FVector& WorldPosition) const
{
    int32 ChunkX = FMath::FloorToInt(WorldPosition.X / (VoxelConstants::ChunkSizeX * VoxelConstants::BlockSize));
    int32 ChunkY = FMath::FloorToInt(WorldPosition.Y / (VoxelConstants::ChunkSizeY * VoxelConstants::BlockSize));
    return FIntVector2(ChunkX, ChunkY);
}

FIntVector AVoxelWorldManager::WorldPosToWorldBlock(const FVector& WorldPosition) const
{
    return FIntVector(
        FMath::FloorToInt(WorldPosition.X / VoxelConstants::BlockSize),
        FMath::FloorToInt(WorldPosition.Y / VoxelConstants::BlockSize),
        FMath::FloorToInt(WorldPosition.Z / VoxelConstants::BlockSize)
    );
}

FIntVector AVoxelWorldManager::WorldPosToSubBlock(const FVector& WorldPosition) const
{
    return FIntVector(
        FMath::FloorToInt(WorldPosition.X / VoxelConstants::PlayerBlockSize),
        FMath::FloorToInt(WorldPosition.Y / VoxelConstants::PlayerBlockSize),
        FMath::FloorToInt(WorldPosition.Z / VoxelConstants::PlayerBlockSize)
    );
}

AVoxelChunk* AVoxelWorldManager::GetChunkAt(int32 ChunkX, int32 ChunkY) const
{
    const FIntPoint Key(ChunkX, ChunkY);
    if (const AVoxelChunk* const* ChunkPtr = ActiveChunks.Find(Key))
    {
        return const_cast<AVoxelChunk*>(*ChunkPtr);
    }
    return nullptr;
}

bool AVoxelWorldManager::IsChunkInRange(int32 ChunkX, int32 ChunkY, const FIntVector2& PlayerChunk) const
{
    return FMath::Abs(ChunkX - PlayerChunk.X) <= VoxelConstants::RenderDistance &&
           FMath::Abs(ChunkY - PlayerChunk.Y) <= VoxelConstants::RenderDistance;
}

void AVoxelWorldManager::UpdateChunks()
{
    if (!PlayerPawn) return;
    
    FIntVector2 PlayerChunk = WorldToChunkCoords(PlayerPawn->GetActorLocation());
    
    TArray<FIntPoint> ChunksToUnload;
    for (auto& Pair : ActiveChunks)
    {
        if (!IsChunkInRange(Pair.Key.X, Pair.Key.Y, PlayerChunk))
            ChunksToUnload.Add(Pair.Key);
    }
    
    for (const FIntPoint& Coord : ChunksToUnload)
        UnloadChunk(Coord.X, Coord.Y);
    
    for (int32 X = PlayerChunk.X - VoxelConstants::RenderDistance; X <= PlayerChunk.X + VoxelConstants::RenderDistance; X++)
    {
        for (int32 Y = PlayerChunk.Y - VoxelConstants::RenderDistance; Y <= PlayerChunk.Y + VoxelConstants::RenderDistance; Y++)
        {
            if (!ActiveChunks.Contains(FIntPoint(X, Y)))
                LoadChunk(X, Y);
        }
    }
}

void AVoxelWorldManager::LoadChunk(int32 ChunkX, int32 ChunkY)
{
    FVector SpawnLocation(
        ChunkX * VoxelConstants::ChunkSizeX * VoxelConstants::BlockSize,
        ChunkY * VoxelConstants::ChunkSizeY * VoxelConstants::BlockSize,
        0.0f
    );
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    AVoxelChunk* NewChunk = GetWorld()->SpawnActor<AVoxelChunk>(AVoxelChunk::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams);
    
    if (NewChunk)
    {
        NewChunk->InitializeChunk(ChunkX, ChunkY);
        ActiveChunks.Add(FIntPoint(ChunkX, ChunkY), NewChunk);
    }
}

void AVoxelWorldManager::UnloadChunk(int32 ChunkX, int32 ChunkY)
{
    FIntPoint Key(ChunkX, ChunkY);
    if (AVoxelChunk** ChunkPtr = ActiveChunks.Find(Key))
    {
        if (*ChunkPtr) (*ChunkPtr)->Destroy();
        ActiveChunks.Remove(Key);
    }
}

bool AVoxelWorldManager::RemoveBlockAtWorldPosition(const FVector& WorldPosition)
{
    FIntVector SubBlockPos = WorldPosToSubBlock(WorldPosition);
    FIntVector2 ChunkCoords = WorldToChunkCoords(WorldPosition);
    
    AVoxelChunk* Chunk = GetChunkAt(ChunkCoords.X, ChunkCoords.Y);
    if (!Chunk) return false;
    
    // Сначала пробуем удалить маленький блок
    if (Chunk->RemoveSmallBlock(SubBlockPos))
    {
        Chunk->MarkDirty();
        return true;
    }
    
    // Иначе удаляем большой блок
    FIntVector WorldBlockPos = WorldPosToWorldBlock(WorldPosition);
    int32 LocalX = WorldBlockPos.X - ChunkCoords.X * VoxelConstants::ChunkSizeX;
    int32 LocalY = WorldBlockPos.Y - ChunkCoords.Y * VoxelConstants::ChunkSizeY;
    int32 LocalZ = WorldBlockPos.Z;
    
    FName BlockID = Chunk->GetBlock(LocalX, LocalY, LocalZ);
    if (!BlockID.IsNone())
    {
        Chunk->SetBlock(LocalX, LocalY, LocalZ, NAME_None);
        Chunk->MarkDirty();
        return true;
    }
    
    return false;
}

bool AVoxelWorldManager::PlaceSmallBlockAtWorldPosition(const FVector& WorldPosition, FName BlockID)
{
    FIntVector SubBlockPos = WorldPosToSubBlock(WorldPosition);
    FIntVector2 ChunkCoords = WorldToChunkCoords(WorldPosition);
    
    AVoxelChunk* Chunk = GetChunkAt(ChunkCoords.X, ChunkCoords.Y);
    if (!Chunk) return false;
    
    // Проверяем, не занята ли позиция мировым блоком
    FIntVector WorldBlockPos = WorldPosToWorldBlock(WorldPosition);
    int32 LocalX = WorldBlockPos.X - ChunkCoords.X * VoxelConstants::ChunkSizeX;
    int32 LocalY = WorldBlockPos.Y - ChunkCoords.Y * VoxelConstants::ChunkSizeY;
    int32 LocalZ = WorldBlockPos.Z;
    
    if (!Chunk->GetBlock(LocalX, LocalY, LocalZ).IsNone())
        return false;
    
    if (Chunk->HasSmallBlockAt(SubBlockPos))
        return false;
    
    Chunk->AddSmallBlock(SubBlockPos, BlockID);
    Chunk->MarkDirty();
    return true;
}

bool AVoxelWorldManager::PlaceLargeBlockAtWorldPosition(const FVector& WorldPosition, FName BlockID)
{
    FIntVector WorldBlockPos = WorldPosToWorldBlock(WorldPosition);
    FIntVector2 ChunkCoords = WorldToChunkCoords(WorldPosition);
    
    AVoxelChunk* Chunk = GetChunkAt(ChunkCoords.X, ChunkCoords.Y);
    if (!Chunk) return false;
    
    int32 LocalX = WorldBlockPos.X - ChunkCoords.X * VoxelConstants::ChunkSizeX;
    int32 LocalY = WorldBlockPos.Y - ChunkCoords.Y * VoxelConstants::ChunkSizeY;
    int32 LocalZ = WorldBlockPos.Z;
    
    if (LocalX < 0 || LocalX >= VoxelConstants::ChunkSizeX ||
        LocalY < 0 || LocalY >= VoxelConstants::ChunkSizeY ||
        LocalZ < 0 || LocalZ >= VoxelConstants::ChunkSizeZ)
    {
        return false;
    }
    
    if (!Chunk->GetBlock(LocalX, LocalY, LocalZ).IsNone())
        return false;
    
    Chunk->SetBlock(LocalX, LocalY, LocalZ, BlockID);
    Chunk->MarkDirty();
    return true;
}
