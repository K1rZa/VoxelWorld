// VoxelHUD.cpp

#include "VoxelHUD.h"
#include "VoxelDatabase.h"
#include "VoxelBlockData.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Texture2D.h"
#include "CanvasItem.h"
#include "GameFramework/PlayerController.h"

AVoxelHUD::AVoxelHUD()
{
}

// ============================================================
// DrawRect / DrawBorder — ЗАПОЛНЕННЫЕ прямоугольники
// FCanvasBoxItem рисует только РАМКУ, поэтому используем FCanvasTileItem
// ============================================================

void AVoxelHUD::DrawRect(float X, float Y, float W, float H, const FLinearColor& Color)
{
    if (!Canvas || W <= 0.f || H <= 0.f) return;

    FCanvasTileItem TileItem(
        FVector2D(X, Y),
        GWhiteTexture,          // Встроенная белая 1x1 текстура движка
        FVector2D(W, H),
        Color
    );
    TileItem.BlendMode = SE_BLEND_Translucent;
    Canvas->DrawItem(TileItem);
}

void AVoxelHUD::DrawBorder(float X, float Y, float W, float H, float T, const FLinearColor& Color)
{
    DrawRect(X, Y, W, T, Color);           // Top
    DrawRect(X, Y + H - T, W, T, Color);   // Bottom
    DrawRect(X, Y, T, H, Color);           // Left
    DrawRect(X + W - T, Y, T, H, Color);   // Right
}

// ============================================================
// Mouse position helper
// ============================================================

FVector2D AVoxelHUD::GetMousePosition() const
{
    APlayerController* PC = GetOwningPlayerController();
    if (!PC) return FVector2D::ZeroVector;
    float MX, MY;
    PC->GetMousePosition(MX, MY);
    return FVector2D(MX, MY);
}

// ============================================================
// HandleInventoryClick — вызывается из PlayerCharacter
// при нажатии ЛКМ (PlaceBlockAction) / ПКМ (RemoveBlockAction)
// когда инвентарь открыт
// ============================================================

void AVoxelHUD::HandleInventoryClick(bool bLeftButton)
{
    if (!InventoryComp || !InventoryComp->bIsInventoryOpen) return;

    APlayerController* PC = GetOwningPlayerController();
    if (!PC) return;

    float MX, MY;
    PC->GetMousePosition(MX, MY);
    FVector2D MousePos(MX, MY);

    // --- Tab clicks (любая кнопка) ---
    {
        float TabTotalW = CachedPanelW - InvPanelPadding * 2.0f;
        int32 NumTabs = static_cast<int32>(EInventoryCategory::Count);
        float TabW = TabTotalW / NumTabs;
        float TabX = CachedPanelX + InvPanelPadding;
        float TabY = CachedPanelY + InvPanelPadding + InvHeaderHeight;

        for (int32 i = 0; i < NumTabs; i++)
        {
            float TX = TabX + i * TabW;
            if (MousePos.X >= TX && MousePos.X <= TX + TabW &&
                MousePos.Y >= TabY && MousePos.Y <= TabY + InvTabHeight)
            {
                InventoryComp->SetCategory(static_cast<EInventoryCategory>(i));
                return;
            }
        }
    }

    // --- ЛКМ по предмету в сетке → добавить в хотбар ---
    if (bLeftButton)
    {
        const TArray<FInventoryEntry>& Entries = InventoryComp->GetCatalogForCategory(InventoryComp->ActiveCategory);
        float CellSize = InvItemSize + InvItemPadding;

        for (int32 i = 0; i < Entries.Num(); i++)
        {
            int32 Col = i % CachedColumns;
            int32 Row = i / CachedColumns;

            float ItemX = CachedGridStartX + Col * CellSize;
            float ItemY = CachedGridStartY + Row * CellSize;

            if (ItemY + InvItemSize < CachedGridStartY) continue;
            if (ItemY > CachedPanelY + CachedPanelH) break;

            if (MousePos.X >= ItemX && MousePos.X <= ItemX + InvItemSize &&
                MousePos.Y >= ItemY && MousePos.Y <= ItemY + InvItemSize)
            {
                InventoryComp->PlaceItemInHotbar(Entries[i].BlockID);
                return;
            }
        }
    }

    // --- ПКМ по слоту хотбара → очистить ---
    if (!bLeftButton)
    {
        if (!Canvas) return;
        float HotbarWidth = (HotbarSlotSize + HotbarSlotPadding) * UVoxelInventoryComponent::HotbarSize - HotbarSlotPadding;
        float HotbarX = (Canvas->SizeX - HotbarWidth) / 2.0f;
        float HotbarY = Canvas->SizeY - HotbarSlotSize - HotbarBottomMargin;

        for (int32 i = 0; i < UVoxelInventoryComponent::HotbarSize; i++)
        {
            float SlotX = HotbarX + i * (HotbarSlotSize + HotbarSlotPadding);
            if (MousePos.X >= SlotX && MousePos.X <= SlotX + HotbarSlotSize &&
                MousePos.Y >= HotbarY && MousePos.Y <= HotbarY + HotbarSlotSize)
            {
                InventoryComp->ClearHotbarSlot(i);
                return;
            }
        }
    }
}

// ============================================================
// Main DrawHUD
// ============================================================

void AVoxelHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!InventoryComp) return;

    DrawHotbar();
    DrawPlayerCoordinates();

    if (InventoryComp->bIsInventoryOpen)
    {
        DrawInventoryPanel();
    }
    else
    {
        DrawCrosshair();
    }
}

// ============================================================
// Crosshair
// ============================================================

void AVoxelHUD::DrawCrosshair()
{
    if (!Canvas) return;
    float CX = Canvas->SizeX / 2.0f;
    float CY = Canvas->SizeY / 2.0f;
    DrawRect(CX - 2.0f, CY - 2.0f, 4.0f, 4.0f, FLinearColor::White);
}

// ============================================================
// Hotbar
// ============================================================

void AVoxelHUD::DrawHotbar()
{
    if (!Canvas || !InventoryComp) return;

    UVoxelDatabase* DB = UVoxelDatabase::Get();
    TArray<FVoxelHotbarSlot> Slots = InventoryComp->GetHotbarSlots();
    int32 SelectedSlot = InventoryComp->GetSelectedSlot();

    const float TotalW = (HotbarSlotSize + HotbarSlotPadding) * UVoxelInventoryComponent::HotbarSize - HotbarSlotPadding;
    const float HotbarX = (Canvas->SizeX - TotalW) / 2.0f;
    const float HotbarY = Canvas->SizeY - HotbarSlotSize - HotbarBottomMargin;

    FVector2D Mouse = GetMousePosition();

    for (int32 i = 0; i < UVoxelInventoryComponent::HotbarSize; i++)
    {
        float SlotX = HotbarX + i * (HotbarSlotSize + HotbarSlotPadding);
        float SlotY = HotbarY;

        bool bHovered = InventoryComp->bIsInventoryOpen &&
            Mouse.X >= SlotX && Mouse.X <= SlotX + HotbarSlotSize &&
            Mouse.Y >= SlotY && Mouse.Y <= SlotY + HotbarSlotSize;

        // Фон слота (ЗАПОЛНЕННЫЙ)
        FLinearColor BgColor;
        if (i == SelectedSlot)
            BgColor = HotbarSelectedCol;
        else if (bHovered)
            BgColor = FLinearColor(0.18f, 0.2f, 0.24f, 0.85f);
        else
            BgColor = HotbarBgCol;

        DrawRect(SlotX, SlotY, HotbarSlotSize, HotbarSlotSize, BgColor);

        // Рамка выбранного
        if (i == SelectedSlot)
        {
            DrawBorder(SlotX, SlotY, HotbarSlotSize, HotbarSlotSize, 2.0f, AccentColor);
        }

        // Содержимое
        if (Slots.IsValidIndex(i) && !Slots[i].IsEmpty() && DB)
        {
            UVoxelBlockData* BlockData = DB->GetBlockData(Slots[i].BlockID);
            if (BlockData)
            {
                float PreviewSize = HotbarSlotSize - 16.0f;
                float BX = SlotX + (HotbarSlotSize - PreviewSize) / 2.0f;
                float BY = SlotY + (HotbarSlotSize - PreviewSize) / 2.0f - 2.0f;

                DrawRect(BX, BY, PreviewSize, PreviewSize, FLinearColor(BlockData->BlockColor));

                // L/S
                FString SizeText = BlockData->bIsLargeBlock ? TEXT("L") : TEXT("S");
                Canvas->SetDrawColor(FColor(200, 200, 200, 255));
                FFontRenderInfo RI; RI.bClipText = true;
                Canvas->DrawText(GEngine->GetTinyFont(), SizeText, SlotX + 3.0f, SlotY + HotbarSlotSize - 12.0f, 1.0f, 1.0f, RI);

                // Count
                if (Slots[i].Count > 1)
                {
                    FString CountText = FString::Printf(TEXT("%d"), Slots[i].Count);
                    Canvas->DrawText(GEngine->GetTinyFont(), CountText, SlotX + HotbarSlotSize - 16.0f, SlotY + HotbarSlotSize - 12.0f, 1.0f, 1.0f, RI);
                }
            }
        }

        // Номер слота
        Canvas->SetDrawColor(FColor(150, 150, 155, 200));
        FFontRenderInfo NI; NI.bClipText = true;
        Canvas->DrawText(GEngine->GetTinyFont(), FString::Printf(TEXT("%d"), i + 1),
            SlotX + HotbarSlotSize - 10.0f, SlotY + 2.0f, 1.0f, 1.0f, NI);
    }
}

// ============================================================
// Inventory Panel
// ============================================================

void AVoxelHUD::DrawInventoryPanel()
{
    if (!Canvas || !InventoryComp) return;

    FVector2D Mouse = GetMousePosition();

    // === Размеры панели ===
    CachedPanelW = Canvas->SizeX * InvPanelWidthRatio;
    CachedPanelH = Canvas->SizeY * InvPanelHeightRatio;
    CachedPanelX = (Canvas->SizeX - CachedPanelW) / 2.0f;
    CachedPanelY = (Canvas->SizeY - CachedPanelH) / 2.0f - 40.0f;

    // === Тёмный оверлей на весь экран ===
    DrawRect(0, 0, Canvas->SizeX, Canvas->SizeY, FLinearColor(0.0f, 0.0f, 0.0f, 0.45f));

    // === Фон панели ===
    DrawRect(CachedPanelX, CachedPanelY, CachedPanelW, CachedPanelH, PanelBgColor);
    DrawBorder(CachedPanelX, CachedPanelY, CachedPanelW, CachedPanelH, 1.5f, PanelBorderColor);

    // === Заголовок ===
    Canvas->SetDrawColor(FColor(220, 225, 230, 255));
    FFontRenderInfo TRI; TRI.bClipText = true;
    Canvas->DrawText(GEngine->GetMediumFont(), TEXT("INVENTORY"),
        CachedPanelX + InvPanelPadding, CachedPanelY + InvPanelPadding - 2.0f,
        1.2f, 1.2f, TRI);

    // === Вкладки категорий ===
    float TabAreaW = CachedPanelW - InvPanelPadding * 2.0f;
    int32 NumTabs = static_cast<int32>(EInventoryCategory::Count);
    float TabW = TabAreaW / NumTabs;
    float TabX = CachedPanelX + InvPanelPadding;
    float TabY = CachedPanelY + InvPanelPadding + InvHeaderHeight;

    static const FString TabLabels[] = { TEXT("Large Blocks"), TEXT("Small Blocks"), TEXT("Items") };

    for (int32 i = 0; i < NumTabs; i++)
    {
        float TX = TabX + i * TabW;
        float TW = TabW - 2.0f;
        bool bActive = (static_cast<int32>(InventoryComp->ActiveCategory) == i);
        bool bTabHovered = Mouse.X >= TX && Mouse.X <= TX + TW &&
                           Mouse.Y >= TabY && Mouse.Y <= TabY + InvTabHeight;

        FLinearColor TabCol = bActive ? TabActiveColor : (bTabHovered ? TabHoverColor : TabInactiveColor);
        DrawRect(TX, TabY, TW, InvTabHeight, TabCol);

        // Подчёркивание активной
        if (bActive)
        {
            DrawRect(TX, TabY + InvTabHeight - 3.0f, TW, 3.0f, AccentColor);
        }

        // Текст
        FString Label = (i < 3) ? TabLabels[i] : TEXT("???");
        Canvas->SetDrawColor(bActive ? FColor(240, 245, 255, 255) : FColor(160, 165, 170, 255));
        FFontRenderInfo TabRI; TabRI.bClipText = true;
        float TextW, TextH;
        Canvas->StrLen(GEngine->GetSmallFont(), Label, TextW, TextH);
        Canvas->DrawText(GEngine->GetSmallFont(), Label,
            TX + (TW - TextW) / 2.0f,
            TabY + (InvTabHeight - TextH) / 2.0f,
            1.0f, 1.0f, TabRI);
    }

    // === Сетка предметов ===
    float GridY = TabY + InvTabHeight + InvItemPadding * 2;
    float GridX = CachedPanelX + InvPanelPadding;
    float GridW = CachedPanelW - InvPanelPadding * 2.0f;

    CachedGridStartX = GridX;
    CachedGridStartY = GridY;

    float CellSize = InvItemSize + InvItemPadding;
    CachedColumns = FMath::Max(1, FMath::FloorToInt(GridW / CellSize));

    // Разделитель
    DrawRect(GridX, GridY - InvItemPadding, GridW, 1.0f, PanelBorderColor);

    const TArray<FInventoryEntry>& Entries = InventoryComp->GetCatalogForCategory(InventoryComp->ActiveCategory);

    if (Entries.Num() == 0)
    {
        Canvas->SetDrawColor(FColor(120, 125, 130, 200));
        FFontRenderInfo ERI; ERI.bClipText = true;
        Canvas->DrawText(GEngine->GetSmallFont(), TEXT("No items in this category"),
            GridX + 10.0f, GridY + 20.0f, 1.0f, 1.0f, ERI);
    }
    else
    {
        HoveredInventoryIndex = INDEX_NONE;

        for (int32 i = 0; i < Entries.Num(); i++)
        {
            int32 Col = i % CachedColumns;
            int32 Row = i / CachedColumns;

            float ItemX = GridX + Col * CellSize;
            float ItemY = GridY + Row * CellSize;

            if (ItemY + InvItemSize < GridY) continue;
            if (ItemY > CachedPanelY + CachedPanelH - InvPanelPadding) break;

            bool bItemHovered = Mouse.X >= ItemX && Mouse.X <= ItemX + InvItemSize &&
                                Mouse.Y >= ItemY && Mouse.Y <= ItemY + InvItemSize;
            if (bItemHovered) HoveredInventoryIndex = i;

            DrawInventoryItem(ItemX, ItemY, InvItemSize, Entries[i], bItemHovered);
        }

        // === Tooltip ===
        if (HoveredInventoryIndex != INDEX_NONE && HoveredInventoryIndex < Entries.Num())
        {
            const FInventoryEntry& Entry = Entries[HoveredInventoryIndex];
            FString Tooltip = Entry.DisplayName.ToString();

            float TipW, TipH;
            Canvas->StrLen(GEngine->GetSmallFont(), Tooltip, TipW, TipH);

            float TipPad = 8.0f;
            float TipX = Mouse.X + 16.0f;
            float TipY = Mouse.Y - TipH - TipPad * 2;
            if (TipX + TipW + TipPad * 2 > Canvas->SizeX)
                TipX = Mouse.X - TipW - TipPad * 2 - 8.0f;
            if (TipY < 0) TipY = Mouse.Y + 20.0f;

            DrawRect(TipX, TipY, TipW + TipPad * 2, TipH + TipPad * 2, FLinearColor(0.06f, 0.07f, 0.09f, 0.95f));
            DrawBorder(TipX, TipY, TipW + TipPad * 2, TipH + TipPad * 2, 1.0f, FLinearColor(0.3f, 0.32f, 0.36f, 1.0f));

            Canvas->SetDrawColor(FColor(240, 242, 245, 255));
            FFontRenderInfo TipRI; TipRI.bClipText = true;
            Canvas->DrawText(GEngine->GetSmallFont(), Tooltip, TipX + TipPad, TipY + TipPad, 1.0f, 1.0f, TipRI);
        }
    }

    // === Подсказка внизу ===
    {
        FString Hint = TEXT("LMB - Add to hotbar  |  RMB on hotbar - Remove  |  E - Close");
        float HW, HH;
        Canvas->StrLen(GEngine->GetTinyFont(), Hint, HW, HH);
        Canvas->SetDrawColor(FColor(120, 125, 135, 180));
        FFontRenderInfo HRI; HRI.bClipText = true;
        Canvas->DrawText(GEngine->GetTinyFont(), Hint,
            CachedPanelX + (CachedPanelW - HW) / 2.0f,
            CachedPanelY + CachedPanelH - InvPanelPadding - HH,
            1.0f, 1.0f, HRI);
    }
}

// ============================================================
// Inventory item cell
// ============================================================

void AVoxelHUD::DrawInventoryItem(float X, float Y, float Size, const FInventoryEntry& Entry, bool bHovered)
{
    DrawRect(X, Y, Size, Size, bHovered ? ItemHoverColor : ItemBgColor);
    DrawBorder(X, Y, Size, Size, 1.0f, bHovered ? AccentColor : ItemBorderColor);

    // Block color preview
    float Pad = 10.0f;
    float PSize = Size - Pad * 2.0f;
    float PX = X + Pad;
    float PY = Y + Pad - 4.0f;

    DrawRect(PX, PY, PSize, PSize, FLinearColor(Entry.Color));

    // 3D shadow
    float SO = 3.0f;
    FLinearColor Shadow = FLinearColor(Entry.Color) * 0.55f;
    Shadow.A = 1.0f;
    DrawRect(PX + PSize, PY + SO, SO, PSize, Shadow);
    DrawRect(PX + SO, PY + PSize, PSize, SO, Shadow);

    // L/S label
    FString SizeLabel = Entry.bIsLargeBlock ? TEXT("L") : TEXT("S");
    Canvas->SetDrawColor(FColor(180, 185, 190, 220));
    FFontRenderInfo SRI; SRI.bClipText = true;
    Canvas->DrawText(GEngine->GetTinyFont(), SizeLabel, X + 3.0f, Y + Size - 12.0f, 1.0f, 1.0f, SRI);
}

// ============================================================
// Player coordinates
// ============================================================

void AVoxelHUD::DrawPlayerCoordinates()
{
    if (!Canvas) return;
    APawn* P = GetOwningPawn();
    if (!P) return;

    FVector Loc = P->GetActorLocation();
    int32 BX = FMath::FloorToInt(Loc.X / 80.0f);
    int32 BY = FMath::FloorToInt(Loc.Y / 80.0f);
    int32 BZ = FMath::FloorToInt(Loc.Z / 80.0f);

    FString Text = FString::Printf(TEXT("X: %d  Y: %d  Z: %d"), BX, BY, BZ);
    float TW, TH;
    Canvas->StrLen(GEngine->GetSmallFont(), Text, TW, TH);

    Canvas->SetDrawColor(FColor::White);
    FFontRenderInfo RI; RI.bClipText = true;
    Canvas->DrawText(GEngine->GetSmallFont(), Text, Canvas->SizeX - 16.0f - TW, 16.0f, 1.0f, 1.0f, RI);
}
