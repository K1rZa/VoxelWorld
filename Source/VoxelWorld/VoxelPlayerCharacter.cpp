// VoxelPlayerCharacter.cpp

#include "VoxelPlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "VoxelWorldManager.h"
#include "VoxelDatabase.h"
#include "VoxelBlockData.h"
#include "VoxelHUD.h"
#include "VoxelChunk.h"
#include "VoxelInventoryComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"

AVoxelPlayerCharacter::AVoxelPlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    GetCapsuleComponent()->InitCapsuleSize(VoxelConstants::PlayerWidth / 2.0f, VoxelConstants::PlayerHeight / 2.0f);

    FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
    FirstPersonCamera->SetRelativeLocation(FVector(0.0f, 0.0f, VoxelConstants::PlayerHeight / 2.0f - 10.0f));
    FirstPersonCamera->bUsePawnControlRotation = true;

    InventoryComp = CreateDefaultSubobject<UVoxelInventoryComponent>(TEXT("InventoryComponent"));

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = true;
    bUseControllerRotationRoll = false;

    DefaultFOV = 90.f;
    TargetFOV = 90.f;
    SprintFOV = 110.f;
    FOVInterpSpeed = 10.f;
    FootstepInterval = 0.45f;
    FootstepTimer = 0.f;
    bWasCrouched = false;
    bIsSprinting = false;
    bIsJumpHeld = false;
    JumpTimer = 0.0f;
    JumpRepeatInterval = 0.1f;
}

void AVoxelPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    UVoxelDatabase::Get();

    UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (Movement)
    {
        Movement->MaxWalkSpeed = WalkSpeed;
        Movement->MaxWalkSpeedCrouched = 250.0f;
        Movement->MaxAcceleration = 2500.0f;
        Movement->BrakingDecelerationWalking = 2500.0f;
        Movement->BrakingDecelerationFalling = 1250.0f;
        Movement->BrakingFrictionFactor = 4.0f;
        Movement->GroundFriction = 8.0f;
        Movement->GravityScale = 2.0f;
        Movement->JumpZVelocity = 600.0f;
        Movement->FallingLateralFriction = 0.6f;
        Movement->AirControl = 0.35f;
        Movement->AirControlBoostMultiplier = 2.0f;
        Movement->AirControlBoostVelocityThreshold = 25.0f;
        Movement->bCanWalkOffLedges = true;
        Movement->bCanWalkOffLedgesWhenCrouching = true;
        Movement->NavAgentProps.bCanCrouch = true;
        Movement->SetCrouchedHalfHeight(VoxelConstants::PlayerHeight / 4.0f);
        Movement->bOrientRotationToMovement = false;
    }

    if (APlayerController* PC = Cast<APlayerController>(Controller))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            if (DefaultMappingContext)
            {
                Subsystem->AddMappingContext(DefaultMappingContext, 0);
            }
        }

        PC->bShowMouseCursor = false;
        PC->SetInputMode(FInputModeGameOnly());

        VoxelHUD = Cast<AVoxelHUD>(PC->GetHUD());
        if (VoxelHUD)
        {
            VoxelHUD->SetInventoryComponent(InventoryComp);
        }
    }

    if (FirstPersonCamera)
    {
        DefaultFOV = FirstPersonCamera->FieldOfView;
        TargetFOV = DefaultFOV;
    }
}

void AVoxelPlayerCharacter::UpdateHUD()
{
    // HUD читает данные напрямую из InventoryComp
}

void AVoxelPlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsJumpHeld)
    {
        JumpTimer -= DeltaTime;
        if (JumpTimer <= 0.0f)
        {
            Jump();
            JumpTimer = JumpRepeatInterval;
        }
    }

    if (FirstPersonCamera)
    {
        float CurrentFOV = FirstPersonCamera->FieldOfView;
        float NewFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, FOVInterpSpeed);
        FirstPersonCamera->SetFieldOfView(NewFOV);
    }

    UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (Movement)
    {
        FVector Velocity = GetVelocity();

        if (Movement->IsMovingOnGround() && Velocity.Size() > 10.f && !Movement->IsCrouching())
        {
            FootstepTimer -= DeltaTime;
            if (FootstepTimer <= 0.f)
            {
                PlayCameraShake(FootstepShake, 1.f);
                FootstepTimer = FootstepInterval;
            }
        }
        else
        {
            FootstepTimer = 0.f;
        }

        bool bCurrentlyCrouched = Movement->IsCrouching();
        if (bCurrentlyCrouched && Velocity.Size() > 10.f && !bWasCrouched)
        {
            PlayCameraShake(CrouchShake, 1.f);
        }
        bWasCrouched = bCurrentlyCrouched;
    }
}

void AVoxelPlayerCharacter::PlayCameraShake(TSubclassOf<UCameraShakeBase> ShakeClass, float Scale)
{
    if (!ShakeClass) return;
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (PC->PlayerCameraManager)
        {
            PC->PlayerCameraManager->StartCameraShake(ShakeClass, Scale);
        }
    }
}

void AVoxelPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    UEnhancedInputComponent* EIC = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

    if (MoveAction)
        EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AVoxelPlayerCharacter::Move);
    if (LookAction)
        EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AVoxelPlayerCharacter::Look);
    if (JumpAction)
    {
        EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &AVoxelPlayerCharacter::StartJump);
        EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &AVoxelPlayerCharacter::StopJump);
    }
    if (CrouchAction)
    {
        EIC->BindAction(CrouchAction, ETriggerEvent::Started, this, &AVoxelPlayerCharacter::StartCrouch);
        EIC->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AVoxelPlayerCharacter::StopCrouch);
    }
    if (SprintAction)
    {
        EIC->BindAction(SprintAction, ETriggerEvent::Started, this, &AVoxelPlayerCharacter::StartSprint);
        EIC->BindAction(SprintAction, ETriggerEvent::Completed, this, &AVoxelPlayerCharacter::StopSprint);
    }

    // ЛКМ и ПКМ — одни и те же InputAction для игры и инвентаря
    if (PlaceBlockAction)
        EIC->BindAction(PlaceBlockAction, ETriggerEvent::Started, this, &AVoxelPlayerCharacter::PlaceBlock);
    if (RemoveBlockAction)
        EIC->BindAction(RemoveBlockAction, ETriggerEvent::Started, this, &AVoxelPlayerCharacter::RemoveBlock);

    // Инвентарь (E)
    if (InventoryToggleAction)
        EIC->BindAction(InventoryToggleAction, ETriggerEvent::Started, this, &AVoxelPlayerCharacter::ToggleInventory);

    if (HotbarSlot1Action) EIC->BindAction(HotbarSlot1Action, ETriggerEvent::Started, this, &AVoxelPlayerCharacter::SelectSlot1);
    if (HotbarSlot2Action) EIC->BindAction(HotbarSlot2Action, ETriggerEvent::Started, this, &AVoxelPlayerCharacter::SelectSlot2);
    if (HotbarSlot3Action) EIC->BindAction(HotbarSlot3Action, ETriggerEvent::Started, this, &AVoxelPlayerCharacter::SelectSlot3);
    if (HotbarSlot4Action) EIC->BindAction(HotbarSlot4Action, ETriggerEvent::Started, this, &AVoxelPlayerCharacter::SelectSlot4);
    if (HotbarSlot5Action) EIC->BindAction(HotbarSlot5Action, ETriggerEvent::Started, this, &AVoxelPlayerCharacter::SelectSlot5);
    if (HotbarSlot6Action) EIC->BindAction(HotbarSlot6Action, ETriggerEvent::Started, this, &AVoxelPlayerCharacter::SelectSlot6);
    if (HotbarSlot7Action) EIC->BindAction(HotbarSlot7Action, ETriggerEvent::Started, this, &AVoxelPlayerCharacter::SelectSlot7);
    if (HotbarSlot8Action) EIC->BindAction(HotbarSlot8Action, ETriggerEvent::Started, this, &AVoxelPlayerCharacter::SelectSlot8);
    if (HotbarSlot9Action) EIC->BindAction(HotbarSlot9Action, ETriggerEvent::Started, this, &AVoxelPlayerCharacter::SelectSlot9);
}

// ============================================================
// Inventory toggle
// ============================================================

void AVoxelPlayerCharacter::ToggleInventory(const FInputActionValue& Value)
{
    if (InventoryComp)
    {
        InventoryComp->ToggleInventory();
    }
}

// ============================================================
// ЛКМ: инвентарь открыт → клик в инвентаре; закрыт → размещение блока
// ============================================================

void AVoxelPlayerCharacter::PlaceBlock(const FInputActionValue& Value)
{
    // Если инвентарь открыт — перенаправляем клик в HUD
    if (InventoryComp && InventoryComp->bIsInventoryOpen)
    {
        if (VoxelHUD)
        {
            VoxelHUD->HandleInventoryClick(true); // true = left button
        }
        return;
    }

    // Обычное размещение блока
    AVoxelWorldManager* WorldManager = AVoxelWorldManager::GetInstance();
    if (!WorldManager) return;

    UVoxelBlockData* BlockData = GetSelectedBlockData();
    if (!BlockData) return;

    FVector HitLocation, HitNormal;
    if (!GetLookHit(HitLocation, HitNormal)) return;

    float BlockSize = BlockData->bIsLargeBlock ? VoxelConstants::BlockSize : VoxelConstants::PlayerBlockSize;
    FVector NewBlockPos = HitLocation + HitNormal * (BlockSize * 0.5f);

    FVector PlayerPos = GetActorLocation();
    FBox PlayerBox(
        PlayerPos - FVector(VoxelConstants::PlayerWidth / 2, VoxelConstants::PlayerWidth / 2, VoxelConstants::PlayerHeight / 2),
        PlayerPos + FVector(VoxelConstants::PlayerWidth / 2, VoxelConstants::PlayerWidth / 2, VoxelConstants::PlayerHeight / 2));

    FBox BlockBox(
        NewBlockPos - FVector(BlockSize / 2),
        NewBlockPos + FVector(BlockSize / 2));

    if (!PlayerBox.Intersect(BlockBox))
    {
        if (BlockData->bIsLargeBlock)
        {
            WorldManager->PlaceLargeBlockAtWorldPosition(NewBlockPos, BlockData->BlockID);
        }
        else
        {
            WorldManager->PlaceSmallBlockAtWorldPosition(NewBlockPos, BlockData->BlockID);
        }
    }
}

// ============================================================
// ПКМ: инвентарь открыт → очистка слота; закрыт → удаление блока
// ============================================================

void AVoxelPlayerCharacter::RemoveBlock(const FInputActionValue& Value)
{
    // Если инвентарь открыт — перенаправляем клик в HUD
    if (InventoryComp && InventoryComp->bIsInventoryOpen)
    {
        if (VoxelHUD)
        {
            VoxelHUD->HandleInventoryClick(false); // false = right button
        }
        return;
    }

    // Обычное удаление блока
    AVoxelWorldManager* WorldManager = AVoxelWorldManager::GetInstance();
    if (!WorldManager) return;

    FVector HitLocation, HitNormal;
    if (!GetLookHit(HitLocation, HitNormal)) return;

    FVector BlockPos = HitLocation - HitNormal * 1.0f;
    WorldManager->RemoveBlockAtWorldPosition(BlockPos);
}

// ============================================================
// Hotbar
// ============================================================

void AVoxelPlayerCharacter::SelectHotbarSlot(int32 Slot)
{
    if (!InventoryComp) return;
    InventoryComp->SetSelectedSlot(Slot);

    if (UVoxelBlockData* BlockData = InventoryComp->GetSelectedBlockData())
    {
        FString SizeStr = BlockData->bIsLargeBlock ? TEXT("80cm") : TEXT("20cm");
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::White,
                FString::Printf(TEXT("%s (%s)"), *BlockData->DisplayName.ToString(), *SizeStr));
        }
    }
}

FVoxelHotbarSlot AVoxelPlayerCharacter::GetSelectedHotbarSlot() const
{
    return InventoryComp ? InventoryComp->GetHotbarSlot(InventoryComp->GetSelectedSlot()) : FVoxelHotbarSlot();
}

int32 AVoxelPlayerCharacter::GetSelectedSlotIndex() const
{
    return InventoryComp ? InventoryComp->GetSelectedSlot() : 0;
}

UVoxelBlockData* AVoxelPlayerCharacter::GetSelectedBlockData() const
{
    return InventoryComp ? InventoryComp->GetSelectedBlockData() : nullptr;
}

bool AVoxelPlayerCharacter::GetLookHit(FVector& OutHitLocation, FVector& OutHitNormal) const
{
    if (!FirstPersonCamera) return false;

    FVector Start = FirstPersonCamera->GetComponentLocation();
    FVector End = Start + FirstPersonCamera->GetForwardVector() * VoxelConstants::InteractionDistance;

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params))
    {
        OutHitLocation = HitResult.ImpactPoint;
        OutHitNormal = HitResult.ImpactNormal;
        return true;
    }
    return false;
}

// ============================================================
// Movement
// ============================================================

void AVoxelPlayerCharacter::Move(const FInputActionValue& Value)
{
    FVector2D MV = Value.Get<FVector2D>();
    if (Controller)
    {
        const FRotator YawRot(0, Controller->GetControlRotation().Yaw, 0);
        AddMovementInput(FRotationMatrix(YawRot).GetUnitAxis(EAxis::X), MV.Y);
        AddMovementInput(FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y), MV.X);
    }
}

void AVoxelPlayerCharacter::Look(const FInputActionValue& Value)
{
    // Не вращаем камеру когда инвентарь открыт
    if (InventoryComp && InventoryComp->bIsInventoryOpen) return;

    FVector2D LV = Value.Get<FVector2D>();
    if (Controller)
    {
        AddControllerYawInput(LV.X);
        AddControllerPitchInput(LV.Y);
    }
}

void AVoxelPlayerCharacter::StartJump(const FInputActionValue& Value)
{
    bIsJumpHeld = true;
    JumpTimer = 0.0f;
    Jump();
    PlayCameraShake(JumpShake, 1.f);
}

void AVoxelPlayerCharacter::StopJump(const FInputActionValue& Value)
{
    bIsJumpHeld = false;
    StopJumping();
}

void AVoxelPlayerCharacter::StartCrouch(const FInputActionValue& Value)
{
    if (!bIsSprinting) Crouch();
}

void AVoxelPlayerCharacter::StopCrouch(const FInputActionValue& Value)
{
    UnCrouch();
}

void AVoxelPlayerCharacter::StartSprint(const FInputActionValue& Value)
{
    bIsSprinting = true;
    if (UCharacterMovementComponent* M = GetCharacterMovement())
    {
        M->MaxWalkSpeed = SprintSpeed;
        M->NavAgentProps.bCanCrouch = false;
        if (M->IsCrouching()) UnCrouch();
    }
    TargetFOV = SprintFOV;
    PlayCameraShake(SprintShake, 1.f);
}

void AVoxelPlayerCharacter::StopSprint(const FInputActionValue& Value)
{
    bIsSprinting = false;
    if (UCharacterMovementComponent* M = GetCharacterMovement())
    {
        M->MaxWalkSpeed = WalkSpeed;
        M->NavAgentProps.bCanCrouch = true;
    }
    TargetFOV = DefaultFOV;
}
