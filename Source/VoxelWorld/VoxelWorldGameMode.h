// VoxelWorldGameMode.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "VoxelWorldGameMode.generated.h"

UCLASS()
class VOXELWORLD_API AVoxelWorldGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AVoxelWorldGameMode();

protected:
    virtual void BeginPlay() override;
};
