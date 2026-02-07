// VoxelChunk.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "VoxelChunk.generated.h"

namespace VoxelConstants
{
    constexpr float BlockSize = 80.0f;
    constexpr float PlayerBlockSize = 20.0f;
    constexpr float PlayerWidth = 78.0f;
    constexpr float PlayerHeight = 158.0f;
    constexpr int32 ChunkSizeX = 16;
    constexpr int32 ChunkSizeY = 16;
    constexpr int32 ChunkSizeZ = 32;
    constexpr int32 RenderDistance = 8;
    constexpr float InteractionDistance = 500.0f;

    // FBM Noise parameters
    constexpr float NoiseScale = 0.01f;
    constexpr float HeightAmplitude = 15.0f;
    constexpr float HeightBase = 7.5f;
    constexpr int32 NoiseOctaves = 5;
    constexpr float NoisePersistence = 0.05f;
    constexpr float NoiseLacunarity = 2.5f;
}

USTRUCT()
struct FSmallBlock
{
    GENERATED_BODY()

    FIntVector Position;
    FName BlockID;

    FSmallBlock() : BlockID(NAME_None) {}
    FSmallBlock(const FIntVector& Pos, FName InBlockID) : Position(Pos), BlockID(InBlockID) {}
};

USTRUCT()
struct FWorldBlock
{
    GENERATED_BODY()

    FName BlockID = NAME_None;

    bool IsAir() const { return BlockID.IsNone() || BlockID == "Air"; }
};

// Структура для хранения данных меша одной секции (материала)
struct FMeshSectionData
{
    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    TArray<FVector> Normals;
    TArray<FVector2D> UVs;
    TArray<FColor> Colors;
    
    void Reset()
    {
        Vertices.Empty();
        Triangles.Empty();
        Normals.Empty();
        UVs.Empty();
        Colors.Empty();
    }
    
    bool IsEmpty() const { return Vertices.Num() == 0; }
};

UCLASS()
class VOXELWORLD_API AVoxelChunk : public AActor
{
    GENERATED_BODY()

public:
    AVoxelChunk();

    void InitializeChunk(int32 ChunkX, int32 ChunkY);

    FName GetBlock(int32 X, int32 Y, int32 Z) const;
    void SetBlock(int32 X, int32 Y, int32 Z, FName BlockID);
    bool IsBlockSolid(int32 X, int32 Y, int32 Z) const;

    void AddSmallBlock(const FIntVector& WorldSubBlockPos, FName BlockID);
    bool RemoveSmallBlock(const FIntVector& WorldSubBlockPos);
    bool HasSmallBlockAt(const FIntVector& WorldSubBlockPos) const;
    FName GetSmallBlockID(const FIntVector& WorldSubBlockPos) const;

    void GenerateMesh();
    void MarkDirty() { bIsDirty = true; }
    FIntVector2 GetChunkCoords() const { return ChunkCoords; }

    // ======== Smooth terrain (Marching Cubes) ========
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel|Smooth")
    bool bUseSmoothTerrain = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel|Smooth", meta = (ClampMin = "0", ClampMax = "3"))
    int32 SmoothingPasses = 1;

    // Глубина сглаживания от поверхности (в блоках). 
    // Блоки ниже этой глубины рендерятся как кубы.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel|Smooth", meta = (ClampMin = "1", ClampMax = "8"))
    int32 SmoothSurfaceDepth = 3;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    UPROPERTY(VisibleAnywhere)
    UProceduralMeshComponent* MeshComponent;

    TArray<FName> Blocks;
    TArray<FSmallBlock> SmallBlocks;

    FIntVector2 ChunkCoords;
    bool bIsDirty = false;

    int32 GetBlockIndex(int32 X, int32 Y, int32 Z) const;
    void GenerateBlocksData();
    float GetFBMNoise(float X, float Y) const;
    
    // === Blocky mesh (оригинальная система) ===
    void GenerateBlockyMesh(TMap<int32, FMeshSectionData>& MeshSections);
    void AddFaceToSection(TMap<int32, FMeshSectionData>& Sections, int32 MaterialIndex,
                          const FVector& Position, const FVector& Normal, FName BlockID, float Size);
    
    // === Smooth mesh (Marching Cubes) ===
    
    TArray<float> DensityField;
    TArray<FName> DensityBlockIDs;
    
    static constexpr int32 DensityPadding = 1;
    int32 DensitySizeX() const { return VoxelConstants::ChunkSizeX + 1 + DensityPadding * 2; }
    int32 DensitySizeY() const { return VoxelConstants::ChunkSizeY + 1 + DensityPadding * 2; }
    int32 DensitySizeZ() const { return VoxelConstants::ChunkSizeZ + 1; }
    int32 DensityIndex(int32 X, int32 Y, int32 Z) const;
    float GetDensity(int32 X, int32 Y, int32 Z) const;
    
    void BuildDensityField();
    void SmoothDensityField();
    void GenerateSmoothMesh(TMap<int32, FMeshSectionData>& MeshSections);
    
    FName GetDominantBlockAt(int32 X, int32 Y, int32 Z) const;
    FVector InterpolateEdge(const FVector& P1, const FVector& P2, float V1, float V2) const;
    
    // Получить блок по мировым координатам (с доступом к соседним чанкам)
    bool IsWorldBlockSolid(int32 WorldBlockX, int32 WorldBlockY, int32 WorldBlockZ) const;
    FName GetWorldBlock(int32 WorldBlockX, int32 WorldBlockY, int32 WorldBlockZ) const;
    
    // FIX #2: Проверка, поставлен ли блок игроком (не из terrain generation)
    bool IsPlayerPlacedBlock(int32 LocalX, int32 LocalY, int32 LocalZ) const;
    
    // === Общие утилиты ===
    FColor GetBlockColor(FName BlockID) const;
    int32 GetBlockMaterialIndex(FName BlockID) const;
    void ApplyMaterialsToMesh();

    FIntVector WorldToLocalSubBlock(const FIntVector& WorldPos) const;
    bool IsSubBlockInChunk(const FIntVector& LocalPos) const;
    int32 FindSmallBlockIndex(const FIntVector& WorldPos) const;
    
    UPROPERTY()
    TMap<int32, UMaterialInterface*> SectionMaterials;

    // ======== Marching Cubes таблицы ========
    static const int32 EdgeTable[256];
    static const int32 TriTable[256][16];
};
