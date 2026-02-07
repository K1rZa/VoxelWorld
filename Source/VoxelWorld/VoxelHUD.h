// VoxelHUD.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "VoxelHotbarItem.h"
#include "VoxelInventoryComponent.h"
#include "VoxelHUD.generated.h"

UCLASS()
class VOXELWORLD_API AVoxelHUD : public AHUD
{
    GENERATED_BODY()

public:
    AVoxelHUD();
    
    virtual void DrawHUD() override;
    
    void SetInventoryComponent(UVoxelInventoryComponent* InComp) { InventoryComp = InComp; }

    // Вызывается из PlayerCharacter при ЛКМ/ПКМ когда инвентарь открыт
    void HandleInventoryClick(bool bLeftButton);

private:
    UPROPERTY()
    UVoxelInventoryComponent* InventoryComp = nullptr;

    // === Drawing ===
    void DrawCrosshair();
    void DrawHotbar();
    void DrawPlayerCoordinates();
    void DrawInventoryPanel();
    void DrawInventoryItem(float X, float Y, float Size, const FInventoryEntry& Entry, bool bHovered);

    // Заполненный прямоугольник (FCanvasTileItem + GWhiteTexture)
    void DrawRect(float X, float Y, float W, float H, const FLinearColor& Color);
    // Рамка из 4 прямоугольников
    void DrawBorder(float X, float Y, float W, float H, float Thickness, const FLinearColor& Color);

    // === Mouse ===
    FVector2D GetMousePosition() const;

    // === Layout constants ===
    static constexpr float HotbarSlotSize = 50.0f;
    static constexpr float HotbarSlotPadding = 4.0f;
    static constexpr float HotbarBottomMargin = 20.0f;

    static constexpr float InvPanelWidthRatio = 0.55f;
    static constexpr float InvPanelHeightRatio = 0.65f;
    static constexpr float InvItemSize = 56.0f;
    static constexpr float InvItemPadding = 6.0f;
    static constexpr float InvTabHeight = 36.0f;
    static constexpr float InvHeaderHeight = 48.0f;
    static constexpr float InvPanelPadding = 16.0f;

    // Цвета
    FLinearColor PanelBgColor      = FLinearColor(0.08f, 0.09f, 0.11f, 0.95f);
    FLinearColor PanelBorderColor  = FLinearColor(0.25f, 0.28f, 0.32f, 1.0f);
    FLinearColor TabActiveColor    = FLinearColor(0.18f, 0.45f, 0.72f, 1.0f);
    FLinearColor TabInactiveColor  = FLinearColor(0.14f, 0.15f, 0.18f, 0.9f);
    FLinearColor TabHoverColor     = FLinearColor(0.2f, 0.22f, 0.26f, 1.0f);
    FLinearColor ItemBgColor       = FLinearColor(0.12f, 0.13f, 0.16f, 1.0f);
    FLinearColor ItemHoverColor    = FLinearColor(0.22f, 0.35f, 0.55f, 1.0f);
    FLinearColor ItemBorderColor   = FLinearColor(0.2f, 0.22f, 0.25f, 1.0f);
    FLinearColor HotbarSelectedCol = FLinearColor(0.18f, 0.45f, 0.72f, 0.9f);
    FLinearColor HotbarBgCol       = FLinearColor(0.1f, 0.1f, 0.12f, 0.8f);
    FLinearColor AccentColor       = FLinearColor(0.3f, 0.6f, 0.95f, 1.0f);

    int32 HoveredInventoryIndex = INDEX_NONE;

    // Layout cache (вычисляется каждый кадр в DrawInventoryPanel)
    float CachedPanelX = 0.f, CachedPanelY = 0.f, CachedPanelW = 0.f, CachedPanelH = 0.f;
    float CachedGridStartX = 0.f, CachedGridStartY = 0.f;
    int32 CachedColumns = 1;
};
