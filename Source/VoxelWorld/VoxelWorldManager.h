// VoxelWorldManager.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelWorldManager.generated.h"

class AVoxelChunk;

UCLASS()
class VOXELWORLD_API AVoxelWorldManager : public AActor
{
    GENERATED_BODY()

public:
    AVoxelWorldManager();

    virtual void Tick(float DeltaTime) override;

    bool RemoveBlockAtWorldPosition(const FVector& WorldPosition);
    bool PlaceSmallBlockAtWorldPosition(const FVector& WorldPosition, FName BlockID);
    bool PlaceLargeBlockAtWorldPosition(const FVector& WorldPosition, FName BlockID);
    
    static AVoxelWorldManager* GetInstance() { return Instance; }

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    static AVoxelWorldManager* Instance;
    
    UPROPERTY()
    TMap<FIntPoint, AVoxelChunk*> ActiveChunks;
    
    UPROPERTY()
    APawn* PlayerPawn;
    
    FIntVector2 LastPlayerChunk;
    
    void UpdateChunks();
    FIntVector2 WorldToChunkCoords(const FVector& WorldPosition) const;
    AVoxelChunk* GetChunkAt(int32 ChunkX, int32 ChunkY) const;
    void LoadChunk(int32 ChunkX, int32 ChunkY);
    void UnloadChunk(int32 ChunkX, int32 ChunkY);
    bool IsChunkInRange(int32 ChunkX, int32 ChunkY, const FIntVector2& PlayerChunk) const;
    
    FIntVector WorldPosToWorldBlock(const FVector& WorldPosition) const;
    FIntVector WorldPosToSubBlock(const FVector& WorldPosition) const;
};
