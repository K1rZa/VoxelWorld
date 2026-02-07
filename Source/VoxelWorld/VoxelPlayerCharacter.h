// VoxelPlayerCharacter.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "VoxelHotbarItem.h"
#include "Camera/CameraShakeBase.h"
#include "VoxelPlayerCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class UCameraComponent;
class AVoxelHUD;
class UVoxelBlockData;
class UVoxelInventoryComponent;

UCLASS()
class VOXELWORLD_API AVoxelPlayerCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AVoxelPlayerCharacter();

    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    UFUNCTION(BlueprintCallable)
    FVoxelHotbarSlot GetSelectedHotbarSlot() const;

    UFUNCTION(BlueprintCallable)
    int32 GetSelectedSlotIndex() const;

    UFUNCTION(BlueprintCallable)
    UVoxelBlockData* GetSelectedBlockData() const;

    UFUNCTION(BlueprintCallable)
    UVoxelInventoryComponent* GetInventoryComponent() const { return InventoryComp; }

protected:
    virtual void BeginPlay() override;

    // === Input Actions (те же что были + InventoryToggleAction) ===
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* JumpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* CrouchAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* SprintAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* PlaceBlockAction;      // ЛКМ: размещение блока ИЛИ клик в инвентаре

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* RemoveBlockAction;     // ПКМ: удаление блока ИЛИ очистка слота хотбара

    // Единственный новый InputAction — клавиша E
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* InventoryToggleAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* HotbarSlot1Action;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* HotbarSlot2Action;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* HotbarSlot3Action;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* HotbarSlot4Action;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* HotbarSlot5Action;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* HotbarSlot6Action;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* HotbarSlot7Action;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* HotbarSlot8Action;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* HotbarSlot9Action;

    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    float WalkSpeed = 500.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    float SprintSpeed = 750.0f;

    bool bIsSprinting = false;
    bool bIsJumpHeld = false;
    float JumpRepeatInterval = 0.1f;
    float JumpTimer = 0.0f;

    // Camera Shake & FOV
    UPROPERTY(EditDefaultsOnly, Category = "Camera Shake")
    TSubclassOf<UCameraShakeBase> FootstepShake;
    UPROPERTY(EditDefaultsOnly, Category = "Camera Shake")
    TSubclassOf<UCameraShakeBase> JumpShake;
    UPROPERTY(EditDefaultsOnly, Category = "Camera Shake")
    TSubclassOf<UCameraShakeBase> CrouchShake;
    UPROPERTY(EditDefaultsOnly, Category = "Camera Shake")
    TSubclassOf<UCameraShakeBase> SprintShake;

    float DefaultFOV = 90.f;
    float TargetFOV = 90.f;
    float SprintFOV = 110.f;
    float FOVInterpSpeed = 10.f;
    float FootstepInterval = 0.45f;
    float FootstepTimer = 0.f;
    bool bWasCrouched = false;

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    UCameraComponent* FirstPersonCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
    UVoxelInventoryComponent* InventoryComp;

    UPROPERTY()
    AVoxelHUD* VoxelHUD;

    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void StartJump(const FInputActionValue& Value);
    void StopJump(const FInputActionValue& Value);
    void StartCrouch(const FInputActionValue& Value);
    void StopCrouch(const FInputActionValue& Value);
    void StartSprint(const FInputActionValue& Value);
    void StopSprint(const FInputActionValue& Value);

    // ЛКМ/ПКМ — переключаются между игрой и инвентарём
    void PlaceBlock(const FInputActionValue& Value);
    void RemoveBlock(const FInputActionValue& Value);

    void ToggleInventory(const FInputActionValue& Value);

    void SelectHotbarSlot(int32 Slot);
    void SelectSlot1(const FInputActionValue& Value) { SelectHotbarSlot(0); }
    void SelectSlot2(const FInputActionValue& Value) { SelectHotbarSlot(1); }
    void SelectSlot3(const FInputActionValue& Value) { SelectHotbarSlot(2); }
    void SelectSlot4(const FInputActionValue& Value) { SelectHotbarSlot(3); }
    void SelectSlot5(const FInputActionValue& Value) { SelectHotbarSlot(4); }
    void SelectSlot6(const FInputActionValue& Value) { SelectHotbarSlot(5); }
    void SelectSlot7(const FInputActionValue& Value) { SelectHotbarSlot(6); }
    void SelectSlot8(const FInputActionValue& Value) { SelectHotbarSlot(7); }
    void SelectSlot9(const FInputActionValue& Value) { SelectHotbarSlot(8); }

    void UpdateHUD();
    bool GetLookHit(FVector& OutHitLocation, FVector& OutHitNormal) const;
    void PlayCameraShake(TSubclassOf<UCameraShakeBase> ShakeClass, float Scale = 1.f);
};
