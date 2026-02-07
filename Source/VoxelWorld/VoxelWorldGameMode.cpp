// VoxelWorldGameMode.cpp

#include "VoxelWorldGameMode.h"
#include "VoxelWorldManager.h"
#include "VoxelHUD.h"
#include "Engine/World.h"

AVoxelWorldGameMode::AVoxelWorldGameMode()
{
    DefaultPawnClass = nullptr;
    HUDClass = AVoxelHUD::StaticClass();
}

void AVoxelWorldGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    GetWorld()->SpawnActor<AVoxelWorldManager>(AVoxelWorldManager::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
}
