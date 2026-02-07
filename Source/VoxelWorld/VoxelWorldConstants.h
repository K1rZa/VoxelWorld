// VoxelWorldConstants.h

#pragma once

#include "CoreMinimal.h"

namespace VoxelConstants
{
    // World block size (80 cm)
    constexpr float BlockSize = 80.0f;
    
    // Player block size (20 cm - в 4 раза меньше)
    constexpr float PlayerBlockSize = 20.0f;
    
    // Player size
    constexpr float PlayerWidth = 78.0f;
    constexpr float PlayerHeight = 158.0f;
    
    // World size in blocks
    constexpr int32 WorldSizeX = 512;
    constexpr int32 WorldSizeY = 512;
    constexpr int32 WorldSizeZ = 32;
    
    // Chunk size in world blocks
    constexpr int32 ChunkSizeX = 16;
    constexpr int32 ChunkSizeY = 16;
    constexpr int32 ChunkSizeZ = 32;
    
    // Render distance
    constexpr int32 RenderDistance = 8;
    
    // Number of chunks
    constexpr int32 WorldChunksX = WorldSizeX / ChunkSizeX;
    constexpr int32 WorldChunksY = WorldSizeY / ChunkSizeY;
    
    // Interaction distance
    constexpr float InteractionDistance = 500.0f;
}

UENUM(BlueprintType)
enum class EBlockType : uint8
{
    Air = 0,
    Stone = 1,
    Grass = 2,
    Sand = 3,
    PlayerBlock = 4
};
