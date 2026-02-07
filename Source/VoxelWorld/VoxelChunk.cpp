// VoxelChunk.cpp

#include "VoxelChunk.h"
#include "VoxelDatabase.h"
#include "VoxelWorldManager.h"
#include "Engine/CollisionProfile.h"
#include "Math/UnrealMathUtility.h"

// ============================================================
// Marching Cubes lookup tables
// ============================================================

const int32 AVoxelChunk::EdgeTable[256] = {
    0x0  , 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c,
    0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09, 0xf00,
    0x190, 0x99 , 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c,
    0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90,
    0x230, 0x339, 0x33 , 0x13a, 0x636, 0x73f, 0x435, 0x53c,
    0xa3c, 0xb35, 0x83f, 0x936, 0xe3a, 0xf33, 0xc39, 0xd30,
    0x3a0, 0x2a9, 0x1a3, 0xaa , 0x7a6, 0x6af, 0x5a5, 0x4ac,
    0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0,
    0x460, 0x569, 0x663, 0x76a, 0x66 , 0x16f, 0x265, 0x36c,
    0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60,
    0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0xff , 0x3f5, 0x2fc,
    0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0,
    0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x55 , 0x15c,
    0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950,
    0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0xcc ,
    0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0,
    0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5, 0xfcc,
    0xcc , 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0,
    0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c,
    0x15c, 0x55 , 0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650,
    0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6, 0xfff, 0xcf5, 0xdfc,
    0x2fc, 0x3f5, 0xff , 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0,
    0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c,
    0x36c, 0x265, 0x16f, 0x66 , 0x76a, 0x663, 0x569, 0x460,
    0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac,
    0x4ac, 0x5a5, 0x6af, 0x7a6, 0xaa , 0x1a3, 0x2a9, 0x3a0,
    0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c,
    0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x33 , 0x339, 0x230,
    0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c,
    0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x99 , 0x190,
    0xf00, 0xe09, 0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c,
    0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203, 0x109, 0x0
};

const int32 AVoxelChunk::TriTable[256][16] = {
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 0, 8, 3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 0, 1, 9,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 1, 8, 3, 9, 8, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 1, 2,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 0, 8, 3, 1, 2,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 9, 2,10, 0, 2, 9,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 2, 8, 3, 2,10, 8,10, 9, 8,-1,-1,-1,-1,-1,-1,-1},
    { 3,11, 2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 0,11, 2, 8,11, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 1, 9, 0, 2, 3,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 1,11, 2, 1, 9,11, 9, 8,11,-1,-1,-1,-1,-1,-1,-1},
    { 3,10, 1,11,10, 3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 0,10, 1, 0, 8,10, 8,11,10,-1,-1,-1,-1,-1,-1,-1},
    { 3, 9, 0, 3,11, 9,11,10, 9,-1,-1,-1,-1,-1,-1,-1},
    { 9, 8,10,10, 8,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 4, 7, 8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 4, 3, 0, 7, 3, 4,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 0, 1, 9, 8, 4, 7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 4, 1, 9, 4, 7, 1, 7, 3, 1,-1,-1,-1,-1,-1,-1,-1},
    { 1, 2,10, 8, 4, 7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 3, 4, 7, 3, 0, 4, 1, 2,10,-1,-1,-1,-1,-1,-1,-1},
    { 9, 2,10, 9, 0, 2, 8, 4, 7,-1,-1,-1,-1,-1,-1,-1},
    { 2,10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4,-1,-1,-1,-1},
    { 8, 4, 7, 3,11, 2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {11, 4, 7,11, 2, 4, 2, 0, 4,-1,-1,-1,-1,-1,-1,-1},
    { 9, 0, 1, 8, 4, 7, 2, 3,11,-1,-1,-1,-1,-1,-1,-1},
    { 4, 7,11, 9, 4,11, 9,11, 2, 9, 2, 1,-1,-1,-1,-1},
    { 3,10, 1, 3,11,10, 7, 8, 4,-1,-1,-1,-1,-1,-1,-1},
    { 1,11,10, 1, 4,11, 1, 0, 4, 7,11, 4,-1,-1,-1,-1},
    { 4, 7, 8, 9, 0,11, 9,11,10,11, 0, 3,-1,-1,-1,-1},
    { 4, 7,11, 4,11, 9, 9,11,10,-1,-1,-1,-1,-1,-1,-1},
    { 9, 5, 4,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 9, 5, 4, 0, 8, 3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 0, 5, 4, 1, 5, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 8, 5, 4, 8, 3, 5, 3, 1, 5,-1,-1,-1,-1,-1,-1,-1},
    { 1, 2,10, 9, 5, 4,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 3, 0, 8, 1, 2,10, 4, 9, 5,-1,-1,-1,-1,-1,-1,-1},
    { 5, 2,10, 5, 4, 2, 4, 0, 2,-1,-1,-1,-1,-1,-1,-1},
    { 2,10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8,-1,-1,-1,-1},
    { 9, 5, 4, 2, 3,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 0,11, 2, 0, 8,11, 4, 9, 5,-1,-1,-1,-1,-1,-1,-1},
    { 0, 5, 4, 0, 1, 5, 2, 3,11,-1,-1,-1,-1,-1,-1,-1},
    { 2, 1, 5, 2, 5, 8, 2, 8,11, 4, 8, 5,-1,-1,-1,-1},
    {10, 3,11,10, 1, 3, 9, 5, 4,-1,-1,-1,-1,-1,-1,-1},
    { 4, 9, 5, 0, 8, 1, 8,10, 1, 8,11,10,-1,-1,-1,-1},
    { 5, 4, 0, 5, 0,11, 5,11,10,11, 0, 3,-1,-1,-1,-1},
    { 5, 4, 8, 5, 8,10,10, 8,11,-1,-1,-1,-1,-1,-1,-1},
    { 9, 7, 8, 5, 7, 9,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 9, 3, 0, 9, 5, 3, 5, 7, 3,-1,-1,-1,-1,-1,-1,-1},
    { 0, 7, 8, 0, 1, 7, 1, 5, 7,-1,-1,-1,-1,-1,-1,-1},
    { 1, 5, 3, 3, 5, 7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 9, 7, 8, 9, 5, 7,10, 1, 2,-1,-1,-1,-1,-1,-1,-1},
    {10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3,-1,-1,-1,-1},
    { 8, 0, 2, 8, 2, 5, 8, 5, 7,10, 5, 2,-1,-1,-1,-1},
    { 2,10, 5, 2, 5, 3, 3, 5, 7,-1,-1,-1,-1,-1,-1,-1},
    { 7, 9, 5, 7, 8, 9, 3,11, 2,-1,-1,-1,-1,-1,-1,-1},
    { 9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7,11,-1,-1,-1,-1},
    { 2, 3,11, 0, 1, 8, 1, 7, 8, 1, 5, 7,-1,-1,-1,-1},
    {11, 2, 1,11, 1, 7, 7, 1, 5,-1,-1,-1,-1,-1,-1,-1},
    { 9, 5, 8, 8, 5, 7,10, 1, 3,10, 3,11,-1,-1,-1,-1},
    { 5, 7, 0, 5, 0, 9, 7,11, 0, 1, 0,10,11,10, 0,-1},
    {11,10, 0,11, 0, 3,10, 5, 0, 8, 0, 7, 5, 7, 0,-1},
    {11,10, 5, 7,11, 5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {10, 6, 5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 0, 8, 3, 5,10, 6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 9, 0, 1, 5,10, 6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 1, 8, 3, 1, 9, 8, 5,10, 6,-1,-1,-1,-1,-1,-1,-1},
    { 1, 6, 5, 2, 6, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 1, 6, 5, 1, 2, 6, 3, 0, 8,-1,-1,-1,-1,-1,-1,-1},
    { 9, 6, 5, 9, 0, 6, 0, 2, 6,-1,-1,-1,-1,-1,-1,-1},
    { 5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8,-1,-1,-1,-1},
    { 2, 3,11,10, 6, 5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {11, 0, 8,11, 2, 0,10, 6, 5,-1,-1,-1,-1,-1,-1,-1},
    { 0, 1, 9, 2, 3,11, 5,10, 6,-1,-1,-1,-1,-1,-1,-1},
    { 5,10, 6, 1, 9, 2, 9,11, 2, 9, 8,11,-1,-1,-1,-1},
    { 6, 3,11, 6, 5, 3, 5, 1, 3,-1,-1,-1,-1,-1,-1,-1},
    { 0, 8,11, 0,11, 5, 0, 5, 1, 5,11, 6,-1,-1,-1,-1},
    { 3,11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9,-1,-1,-1,-1},
    { 6, 5, 9, 6, 9,11,11, 9, 8,-1,-1,-1,-1,-1,-1,-1},
    { 5,10, 6, 4, 7, 8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 4, 3, 0, 4, 7, 3, 6, 5,10,-1,-1,-1,-1,-1,-1,-1},
    { 1, 9, 0, 5,10, 6, 8, 4, 7,-1,-1,-1,-1,-1,-1,-1},
    {10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4,-1,-1,-1,-1},
    { 6, 1, 2, 6, 5, 1, 4, 7, 8,-1,-1,-1,-1,-1,-1,-1},
    { 1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7,-1,-1,-1,-1},
    { 8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6,-1,-1,-1,-1},
    { 7, 3, 9, 7, 9, 4, 3, 2, 9, 5, 9, 6, 2, 6, 9,-1},
    { 3,11, 2, 7, 8, 4,10, 6, 5,-1,-1,-1,-1,-1,-1,-1},
    { 5,10, 6, 4, 7, 2, 4, 2, 0, 2, 7,11,-1,-1,-1,-1},
    { 0, 1, 9, 4, 7, 8, 2, 3,11, 5,10, 6,-1,-1,-1,-1},
    { 9, 2, 1, 9,11, 2, 9, 4,11, 7,11, 4, 5,10, 6,-1},
    { 8, 4, 7, 3,11, 5, 3, 5, 1, 5,11, 6,-1,-1,-1,-1},
    { 5, 1,11, 5,11, 6, 1, 0,11, 7,11, 4, 0, 4,11,-1},
    { 0, 5, 9, 0, 6, 5, 0, 3, 6,11, 6, 3, 8, 4, 7,-1},
    { 6, 5, 9, 6, 9,11, 4, 7, 9, 7,11, 9,-1,-1,-1,-1},
    {10, 4, 9, 6, 4,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 4,10, 6, 4, 9,10, 0, 8, 3,-1,-1,-1,-1,-1,-1,-1},
    {10, 0, 1,10, 6, 0, 6, 4, 0,-1,-1,-1,-1,-1,-1,-1},
    { 8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1,10,-1,-1,-1,-1},
    { 1, 4, 9, 1, 2, 4, 2, 6, 4,-1,-1,-1,-1,-1,-1,-1},
    { 3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4,-1,-1,-1,-1},
    { 0, 2, 4, 4, 2, 6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 8, 3, 2, 8, 2, 4, 4, 2, 6,-1,-1,-1,-1,-1,-1,-1},
    {10, 4, 9,10, 6, 4,11, 2, 3,-1,-1,-1,-1,-1,-1,-1},
    { 0, 8, 2, 2, 8,11, 4, 9,10, 4,10, 6,-1,-1,-1,-1},
    { 3,11, 2, 0, 1, 6, 0, 6, 4, 6, 1,10,-1,-1,-1,-1},
    { 6, 4, 1, 6, 1,10, 4, 8, 1, 2, 1,11, 8,11, 1,-1},
    { 9, 6, 4, 9, 3, 6, 9, 1, 3,11, 6, 3,-1,-1,-1,-1},
    { 8,11, 1, 8, 1, 0,11, 6, 1, 9, 1, 4, 6, 4, 1,-1},
    { 3,11, 6, 3, 6, 0, 0, 6, 4,-1,-1,-1,-1,-1,-1,-1},
    { 6, 4, 8,11, 6, 8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 7,10, 6, 7, 8,10, 8, 9,10,-1,-1,-1,-1,-1,-1,-1},
    { 0, 7, 3, 0,10, 7, 0, 9,10, 6, 7,10,-1,-1,-1,-1},
    {10, 6, 7, 1,10, 7, 1, 7, 8, 1, 8, 0,-1,-1,-1,-1},
    {10, 6, 7,10, 7, 1, 1, 7, 3,-1,-1,-1,-1,-1,-1,-1},
    { 1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7,-1,-1,-1,-1},
    { 2, 6, 9, 2, 9, 1, 6, 7, 9, 0, 9, 3, 7, 3, 9,-1},
    { 7, 8, 0, 7, 0, 6, 6, 0, 2,-1,-1,-1,-1,-1,-1,-1},
    { 7, 3, 2, 6, 7, 2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 2, 3,11,10, 6, 8,10, 8, 9, 8, 6, 7,-1,-1,-1,-1},
    { 2, 0, 7, 2, 7,11, 0, 9, 7, 6, 7,10, 9,10, 7,-1},
    { 1, 8, 0, 1, 7, 8, 1,10, 7, 6, 7,10, 2, 3,11,-1},
    {11, 2, 1,11, 1, 7,10, 6, 1, 6, 7, 1,-1,-1,-1,-1},
    { 8, 9, 6, 8, 6, 7, 9, 1, 6,11, 6, 3, 1, 3, 6,-1},
    { 0, 9, 1,11, 6, 7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 7, 8, 0, 7, 0, 6, 3,11, 0,11, 6, 0,-1,-1,-1,-1},
    { 7,11, 6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 7, 6,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 3, 0, 8,11, 7, 6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 0, 1, 9,11, 7, 6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 8, 1, 9, 8, 3, 1,11, 7, 6,-1,-1,-1,-1,-1,-1,-1},
    {10, 1, 2, 6,11, 7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 1, 2,10, 3, 0, 8, 6,11, 7,-1,-1,-1,-1,-1,-1,-1},
    { 2, 9, 0, 2,10, 9, 6,11, 7,-1,-1,-1,-1,-1,-1,-1},
    { 6,11, 7, 2,10, 3,10, 8, 3,10, 9, 8,-1,-1,-1,-1},
    { 7, 2, 3, 6, 2, 7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 7, 0, 8, 7, 6, 0, 6, 2, 0,-1,-1,-1,-1,-1,-1,-1},
    { 2, 7, 6, 2, 3, 7, 0, 1, 9,-1,-1,-1,-1,-1,-1,-1},
    { 1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6,-1,-1,-1,-1},
    {10, 7, 6,10, 1, 7, 1, 3, 7,-1,-1,-1,-1,-1,-1,-1},
    {10, 7, 6, 1, 7,10, 1, 8, 7, 1, 0, 8,-1,-1,-1,-1},
    { 0, 3, 7, 0, 7,10, 0,10, 9, 6,10, 7,-1,-1,-1,-1},
    { 7, 6,10, 7,10, 8, 8,10, 9,-1,-1,-1,-1,-1,-1,-1},
    { 6, 8, 4,11, 8, 6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 3, 6,11, 3, 0, 6, 0, 4, 6,-1,-1,-1,-1,-1,-1,-1},
    { 8, 6,11, 8, 4, 6, 9, 0, 1,-1,-1,-1,-1,-1,-1,-1},
    { 9, 4, 6, 9, 6, 3, 9, 3, 1,11, 3, 6,-1,-1,-1,-1},
    { 6, 8, 4, 6,11, 8, 2,10, 1,-1,-1,-1,-1,-1,-1,-1},
    { 1, 2,10, 3, 0,11, 0, 6,11, 0, 4, 6,-1,-1,-1,-1},
    { 4,11, 8, 4, 6,11, 0, 2, 9, 2,10, 9,-1,-1,-1,-1},
    {10, 9, 3,10, 3, 2, 9, 4, 3,11, 3, 6, 4, 6, 3,-1},
    { 8, 2, 3, 8, 4, 2, 4, 6, 2,-1,-1,-1,-1,-1,-1,-1},
    { 0, 4, 2, 4, 6, 2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 1, 9, 0, 2, 3, 4, 2, 4, 6, 4, 3, 8,-1,-1,-1,-1},
    { 1, 9, 4, 1, 4, 2, 2, 4, 6,-1,-1,-1,-1,-1,-1,-1},
    { 8, 1, 3, 8, 6, 1, 8, 4, 6, 6,10, 1,-1,-1,-1,-1},
    {10, 1, 0,10, 0, 6, 6, 0, 4,-1,-1,-1,-1,-1,-1,-1},
    { 4, 6, 3, 4, 3, 8, 6,10, 3, 0, 3, 9,10, 9, 3,-1},
    {10, 9, 4, 6,10, 4,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 4, 9, 5, 7, 6,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 0, 8, 3, 4, 9, 5,11, 7, 6,-1,-1,-1,-1,-1,-1,-1},
    { 5, 0, 1, 5, 4, 0, 7, 6,11,-1,-1,-1,-1,-1,-1,-1},
    {11, 7, 6, 8, 3, 4, 3, 5, 4, 3, 1, 5,-1,-1,-1,-1},
    { 9, 5, 4,10, 1, 2, 7, 6,11,-1,-1,-1,-1,-1,-1,-1},
    { 6,11, 7, 1, 2,10, 0, 8, 3, 4, 9, 5,-1,-1,-1,-1},
    { 7, 6,11, 5, 4,10, 4, 2,10, 4, 0, 2,-1,-1,-1,-1},
    { 3, 4, 8, 3, 5, 4, 3, 2, 5,10, 5, 2,11, 7, 6,-1},
    { 7, 2, 3, 7, 6, 2, 5, 4, 9,-1,-1,-1,-1,-1,-1,-1},
    { 9, 5, 4, 0, 8, 6, 0, 6, 2, 6, 8, 7,-1,-1,-1,-1},
    { 3, 6, 2, 3, 7, 6, 1, 5, 0, 5, 4, 0,-1,-1,-1,-1},
    { 6, 2, 8, 6, 8, 7, 2, 1, 8, 4, 8, 5, 1, 5, 8,-1},
    { 9, 5, 4,10, 1, 6, 1, 7, 6, 1, 3, 7,-1,-1,-1,-1},
    { 1, 6,10, 1, 7, 6, 1, 0, 7, 8, 7, 0, 9, 5, 4,-1},
    { 4, 0,10, 4,10, 5, 0, 3,10, 6,10, 7, 3, 7,10,-1},
    { 7, 6,10, 7,10, 8, 5, 4,10, 4, 8,10,-1,-1,-1,-1},
    { 6, 9, 5, 6,11, 9,11, 8, 9,-1,-1,-1,-1,-1,-1,-1},
    { 3, 6,11, 0, 6, 3, 0, 5, 6, 0, 9, 5,-1,-1,-1,-1},
    { 0,11, 8, 0, 5,11, 0, 1, 5, 5, 6,11,-1,-1,-1,-1},
    { 6,11, 3, 6, 3, 5, 5, 3, 1,-1,-1,-1,-1,-1,-1,-1},
    { 1, 2,10, 9, 5,11, 9,11, 8,11, 5, 6,-1,-1,-1,-1},
    { 0,11, 3, 0, 6,11, 0, 9, 6, 5, 6, 9, 1, 2,10,-1},
    {11, 8, 5,11, 5, 6, 8, 0, 5,10, 5, 2, 0, 2, 5,-1},
    { 6,11, 3, 6, 3, 5, 2,10, 3,10, 5, 3,-1,-1,-1,-1},
    { 5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2,-1,-1,-1,-1},
    { 9, 5, 6, 9, 6, 0, 0, 6, 2,-1,-1,-1,-1,-1,-1,-1},
    { 1, 5, 8, 1, 8, 0, 5, 6, 8, 3, 8, 2, 6, 2, 8,-1},
    { 1, 5, 6, 2, 1, 6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 1, 3, 6, 1, 6,10, 3, 8, 6, 5, 6, 9, 8, 9, 6,-1},
    {10, 1, 0,10, 0, 6, 9, 5, 0, 5, 6, 0,-1,-1,-1,-1},
    { 0, 3, 8, 5, 6,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {10, 5, 6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {11, 5,10, 7, 5,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {11, 5,10,11, 7, 5, 8, 3, 0,-1,-1,-1,-1,-1,-1,-1},
    { 5,11, 7, 5,10,11, 1, 9, 0,-1,-1,-1,-1,-1,-1,-1},
    {10, 7, 5,10,11, 7, 9, 8, 1, 8, 3, 1,-1,-1,-1,-1},
    {11, 1, 2,11, 7, 1, 7, 5, 1,-1,-1,-1,-1,-1,-1,-1},
    { 0, 8, 3, 1, 2, 7, 1, 7, 5, 7, 2,11,-1,-1,-1,-1},
    { 9, 7, 5, 9, 2, 7, 9, 0, 2, 2,11, 7,-1,-1,-1,-1},
    { 7, 5, 2, 7, 2,11, 5, 9, 2, 3, 2, 8, 9, 8, 2,-1},
    { 2, 5,10, 2, 3, 5, 3, 7, 5,-1,-1,-1,-1,-1,-1,-1},
    { 8, 2, 0, 8, 5, 2, 8, 7, 5,10, 2, 5,-1,-1,-1,-1},
    { 9, 0, 1, 5,10, 3, 5, 3, 7, 3,10, 2,-1,-1,-1,-1},
    { 9, 8, 2, 9, 2, 1, 8, 7, 2,10, 2, 5, 7, 5, 2,-1},
    { 1, 3, 5, 3, 7, 5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 0, 8, 7, 0, 7, 1, 1, 7, 5,-1,-1,-1,-1,-1,-1,-1},
    { 9, 0, 3, 9, 3, 5, 5, 3, 7,-1,-1,-1,-1,-1,-1,-1},
    { 9, 8, 7, 5, 9, 7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 5, 8, 4, 5,10, 8,10,11, 8,-1,-1,-1,-1,-1,-1,-1},
    { 5, 0, 4, 5,11, 0, 5,10,11,11, 3, 0,-1,-1,-1,-1},
    { 0, 1, 9, 8, 4,10, 8,10,11,10, 4, 5,-1,-1,-1,-1},
    {10,11, 4,10, 4, 5,11, 3, 4, 9, 4, 1, 3, 1, 4,-1},
    { 2, 5, 1, 2, 8, 5, 2,11, 8, 4, 5, 8,-1,-1,-1,-1},
    { 0, 4,11, 0,11, 3, 4, 5,11, 2,11, 1, 5, 1,11,-1},
    { 0, 2, 5, 0, 5, 9, 2,11, 5, 4, 5, 8,11, 8, 5,-1},
    { 9, 4, 5, 2,11, 3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 2, 5,10, 3, 5, 2, 3, 4, 5, 3, 8, 4,-1,-1,-1,-1},
    { 5,10, 2, 5, 2, 4, 4, 2, 0,-1,-1,-1,-1,-1,-1,-1},
    { 3,10, 2, 3, 5,10, 3, 8, 5, 4, 5, 8, 0, 1, 9,-1},
    { 5,10, 2, 5, 2, 4, 1, 9, 2, 9, 4, 2,-1,-1,-1,-1},
    { 8, 4, 5, 8, 5, 3, 3, 5, 1,-1,-1,-1,-1,-1,-1,-1},
    { 0, 4, 5, 1, 0, 5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 8, 4, 5, 8, 5, 3, 9, 0, 5, 0, 3, 5,-1,-1,-1,-1},
    { 9, 4, 5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 4,11, 7, 4, 9,11, 9,10,11,-1,-1,-1,-1,-1,-1,-1},
    { 0, 8, 3, 4, 9, 7, 9,11, 7, 9,10,11,-1,-1,-1,-1},
    { 1,10,11, 1,11, 4, 1, 4, 0, 7, 4,11,-1,-1,-1,-1},
    { 3, 1, 4, 3, 4, 8, 1,10, 4, 7, 4,11,10,11, 4,-1},
    { 4,11, 7, 9,11, 4, 9, 2,11, 9, 1, 2,-1,-1,-1,-1},
    { 9, 7, 4, 9,11, 7, 9, 1,11, 2,11, 1, 0, 8, 3,-1},
    {11, 7, 4,11, 4, 2, 2, 4, 0,-1,-1,-1,-1,-1,-1,-1},
    {11, 7, 4,11, 4, 2, 8, 3, 4, 3, 2, 4,-1,-1,-1,-1},
    { 2, 9,10, 2, 7, 9, 2, 3, 7, 7, 4, 9,-1,-1,-1,-1},
    { 9,10, 7, 9, 7, 4,10, 2, 7, 8, 7, 0, 2, 0, 7,-1},
    { 3, 7,10, 3,10, 2, 7, 4,10, 1,10, 0, 4, 0,10,-1},
    { 1,10, 2, 8, 7, 4,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 4, 9, 1, 4, 1, 7, 7, 1, 3,-1,-1,-1,-1,-1,-1,-1},
    { 4, 9, 1, 4, 1, 7, 0, 8, 1, 8, 7, 1,-1,-1,-1,-1},
    { 4, 0, 3, 7, 4, 3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 4, 8, 7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 9,10, 8,10,11, 8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 3, 0, 9, 3, 9,11,11, 9,10,-1,-1,-1,-1,-1,-1,-1},
    { 0, 1,10, 0,10, 8, 8,10,11,-1,-1,-1,-1,-1,-1,-1},
    { 3, 1,10,11, 3,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 1, 2,11, 1,11, 9, 9,11, 8,-1,-1,-1,-1,-1,-1,-1},
    { 3, 0, 9, 3, 9,11, 1, 2, 9, 2,11, 9,-1,-1,-1,-1},
    { 0, 2,11, 8, 0,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 3, 2,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 2, 3, 8, 2, 8,10,10, 8, 9,-1,-1,-1,-1,-1,-1,-1},
    { 9,10, 2, 0, 9, 2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 2, 3, 8, 2, 8,10, 0, 1, 8, 1,10, 8,-1,-1,-1,-1},
    { 1,10, 2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 1, 3, 8, 9, 1, 8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 0, 9, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    { 0, 3, 8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}
};

// ============================================================
// Constructor & lifecycle
// ============================================================

AVoxelChunk::AVoxelChunk()
{
    PrimaryActorTick.bCanEverTick = true;

    MeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;

    MeshComponent->bUseComplexAsSimpleCollision = true;
    MeshComponent->bUseAsyncCooking = false;
    MeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    MeshComponent->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);

    Blocks.SetNum(VoxelConstants::ChunkSizeX * VoxelConstants::ChunkSizeY * VoxelConstants::ChunkSizeZ);
}

void AVoxelChunk::BeginPlay()
{
    Super::BeginPlay();
}

void AVoxelChunk::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsDirty)
    {
        GenerateMesh();
        bIsDirty = false;
    }
}

// ============================================================
// Chunk initialization & noise
// ============================================================

void AVoxelChunk::InitializeChunk(int32 ChunkX, int32 ChunkY)
{
    ChunkCoords = FIntVector2(ChunkX, ChunkY);

    float WorldX = ChunkX * VoxelConstants::ChunkSizeX * VoxelConstants::BlockSize;
    float WorldY = ChunkY * VoxelConstants::ChunkSizeY * VoxelConstants::BlockSize;
    SetActorLocation(FVector(WorldX, WorldY, 0.0f));

    GenerateBlocksData();
    GenerateMesh();
}

float AVoxelChunk::GetFBMNoise(float X, float Y) const
{
    float Total = 0.0f;
    float Frequency = 1.0f;
    float Amplitude = 1.0f;
    float MaxValue = 0.0f;

    for (int32 Octave = 0; Octave < VoxelConstants::NoiseOctaves; ++Octave)
    {
        float Noise = FMath::PerlinNoise2D(FVector2D(X * Frequency, Y * Frequency));
        Total += Noise * Amplitude;
        MaxValue += Amplitude;

        Amplitude *= VoxelConstants::NoisePersistence;
        Frequency *= VoxelConstants::NoiseLacunarity;
    }

    return Total / MaxValue;
}

void AVoxelChunk::GenerateBlocksData()
{
    UVoxelDatabase* DB = UVoxelDatabase::Get();
    
    FName StoneID = "StoneL";
    FName GrassID = "GrassL";  
    FName SandID = "SandL";
    
    if (DB)
    {
        if (!DB->GetBlockData(StoneID)) StoneID = "Stone";
        if (!DB->GetBlockData(GrassID)) GrassID = "Grass";
        if (!DB->GetBlockData(SandID)) SandID = "Sand";
        
        UE_LOG(LogTemp, Log, TEXT("GenerateBlocksData using: Stone=%s, Grass=%s, Sand=%s"), 
               *StoneID.ToString(), *GrassID.ToString(), *SandID.ToString());
    }
    
    for (int32 X = 0; X < VoxelConstants::ChunkSizeX; X++)
    {
        for (int32 Y = 0; Y < VoxelConstants::ChunkSizeY; Y++)
        {
            float WorldX = (ChunkCoords.X * VoxelConstants::ChunkSizeX + X) * VoxelConstants::NoiseScale;
            float WorldY = (ChunkCoords.Y * VoxelConstants::ChunkSizeY + Y) * VoxelConstants::NoiseScale;

            float NoiseValue = GetFBMNoise(WorldX, WorldY);
            float Height = VoxelConstants::HeightBase + NoiseValue * VoxelConstants::HeightAmplitude;
            int32 MaxHeight = FMath::Clamp(FMath::RoundToInt(Height), 1, VoxelConstants::ChunkSizeZ - 1);

            for (int32 Z = 0; Z < VoxelConstants::ChunkSizeZ; Z++)
            {
                FName BlockID = NAME_None;

                if (Z < MaxHeight - 3)
                {
                    BlockID = StoneID;
                }
                else if (Z < MaxHeight)
                {
                    if (MaxHeight < 6)
                    {
                        BlockID = SandID;
                    }
                    else
                    {
                        BlockID = GrassID;
                    }
                }

                SetBlock(X, Y, Z, BlockID);
            }
        }
    }
}

// ============================================================
// Block access
// ============================================================

int32 AVoxelChunk::GetBlockIndex(int32 X, int32 Y, int32 Z) const
{
    return X + Y * VoxelConstants::ChunkSizeX + Z * VoxelConstants::ChunkSizeX * VoxelConstants::ChunkSizeY;
}

FName AVoxelChunk::GetBlock(int32 X, int32 Y, int32 Z) const
{
    if (X < 0 || X >= VoxelConstants::ChunkSizeX ||
        Y < 0 || Y >= VoxelConstants::ChunkSizeY ||
        Z < 0 || Z >= VoxelConstants::ChunkSizeZ)
    {
        return NAME_None;
    }

    return Blocks[GetBlockIndex(X, Y, Z)];
}

void AVoxelChunk::SetBlock(int32 X, int32 Y, int32 Z, FName BlockID)
{
    if (X < 0 || X >= VoxelConstants::ChunkSizeX ||
        Y < 0 || Y >= VoxelConstants::ChunkSizeY ||
        Z < 0 || Z >= VoxelConstants::ChunkSizeZ)
    {
        return;
    }

    Blocks[GetBlockIndex(X, Y, Z)] = BlockID;
}

bool AVoxelChunk::IsBlockSolid(int32 X, int32 Y, int32 Z) const
{
    FName BlockID = GetBlock(X, Y, Z);
    return !BlockID.IsNone();
}

// ============================================================
// World block access (cross-chunk, для Marching Cubes)
// ============================================================

FName AVoxelChunk::GetWorldBlock(int32 WorldBlockX, int32 WorldBlockY, int32 WorldBlockZ) const
{
    if (WorldBlockZ < 0 || WorldBlockZ >= VoxelConstants::ChunkSizeZ)
        return NAME_None;
    
    // Определяем, в каком чанке находится этот блок
    int32 TargetChunkX = FMath::FloorToInt32((float)WorldBlockX / VoxelConstants::ChunkSizeX);
    int32 TargetChunkY = FMath::FloorToInt32((float)WorldBlockY / VoxelConstants::ChunkSizeY);
    
    int32 LocalX = WorldBlockX - TargetChunkX * VoxelConstants::ChunkSizeX;
    int32 LocalY = WorldBlockY - TargetChunkY * VoxelConstants::ChunkSizeY;
    
    // Если это наш чанк — быстрый путь
    if (TargetChunkX == ChunkCoords.X && TargetChunkY == ChunkCoords.Y)
    {
        return GetBlock(LocalX, LocalY, WorldBlockZ);
    }
    
    // Иначе — запрашиваем у WorldManager
    AVoxelWorldManager* WM = AVoxelWorldManager::GetInstance();
    if (!WM) return NAME_None;
    
    // Используем публичный метод GetChunkAt (нужно добавить)
    // Пока используем хак: генерируем noise для этого блока напрямую
    // чтобы узнать высоту в этой точке (это детерминистично!)
    float NoiseX = WorldBlockX * VoxelConstants::NoiseScale;
    float NoiseY = WorldBlockY * VoxelConstants::NoiseScale;
    float NoiseValue = GetFBMNoise(NoiseX, NoiseY);
    float Height = VoxelConstants::HeightBase + NoiseValue * VoxelConstants::HeightAmplitude;
    int32 MaxHeight = FMath::Clamp(FMath::RoundToInt(Height), 1, VoxelConstants::ChunkSizeZ - 1);
    
    if (WorldBlockZ < MaxHeight)
    {
        // Возвращаем примерный ID (для цвета/материала)
        if (WorldBlockZ < MaxHeight - 3)
            return FName("Stone");
        else if (MaxHeight < 6)
            return FName("Sand");
        else
            return FName("Grass");
    }
    
    return NAME_None;
}

bool AVoxelChunk::IsWorldBlockSolid(int32 WorldBlockX, int32 WorldBlockY, int32 WorldBlockZ) const
{
    FName Block = GetWorldBlock(WorldBlockX, WorldBlockY, WorldBlockZ);
    return !Block.IsNone();
}

// ============================================================
// FIX #2 helper: check if a block was placed by the player
// ============================================================

bool AVoxelChunk::IsPlayerPlacedBlock(int32 LocalX, int32 LocalY, int32 LocalZ) const
{
    FName BlockID = GetBlock(LocalX, LocalY, LocalZ);
    if (BlockID.IsNone()) return false;
    
    // Определяем по noise: должен ли здесь быть блок от генерации террейна?
    // Если noise говорит "тут воздух", но блок есть — значит игрок его поставил.
    int32 WorldBlockX = ChunkCoords.X * VoxelConstants::ChunkSizeX + LocalX;
    int32 WorldBlockY = ChunkCoords.Y * VoxelConstants::ChunkSizeY + LocalY;
    
    float NoiseX = (float)WorldBlockX * VoxelConstants::NoiseScale;
    float NoiseY = (float)WorldBlockY * VoxelConstants::NoiseScale;
    float NoiseValue = GetFBMNoise(NoiseX, NoiseY);
    float Height = VoxelConstants::HeightBase + NoiseValue * VoxelConstants::HeightAmplitude;
    int32 MaxHeight = FMath::Clamp(FMath::RoundToInt(Height), 1, VoxelConstants::ChunkSizeZ - 1);
    
    // Noise говорит что тут должен быть воздух (Z >= MaxHeight),
    // но блок есть — значит игрок его поставил
    return (LocalZ >= MaxHeight);
}

// ============================================================
// Small blocks
// ============================================================

FIntVector AVoxelChunk::WorldToLocalSubBlock(const FIntVector& WorldPos) const
{
    int32 ChunkStartX = ChunkCoords.X * VoxelConstants::ChunkSizeX * 4;
    int32 ChunkStartY = ChunkCoords.Y * VoxelConstants::ChunkSizeY * 4;

    return FIntVector(
        WorldPos.X - ChunkStartX,
        WorldPos.Y - ChunkStartY,
        WorldPos.Z
    );
}

bool AVoxelChunk::IsSubBlockInChunk(const FIntVector& LocalPos) const
{
    return LocalPos.X >= 0 && LocalPos.X < VoxelConstants::ChunkSizeX * 4 &&
           LocalPos.Y >= 0 && LocalPos.Y < VoxelConstants::ChunkSizeY * 4 &&
           LocalPos.Z >= 0 && LocalPos.Z < VoxelConstants::ChunkSizeZ * 4;
}

int32 AVoxelChunk::FindSmallBlockIndex(const FIntVector& WorldPos) const
{
    for (int32 i = 0; i < SmallBlocks.Num(); i++)
    {
        if (SmallBlocks[i].Position == WorldPos)
            return i;
    }
    return INDEX_NONE;
}

void AVoxelChunk::AddSmallBlock(const FIntVector& WorldSubBlockPos, FName BlockID)
{
    if (FindSmallBlockIndex(WorldSubBlockPos) == INDEX_NONE)
    {
        SmallBlocks.Add(FSmallBlock(WorldSubBlockPos, BlockID));
    }
}

bool AVoxelChunk::RemoveSmallBlock(const FIntVector& WorldSubBlockPos)
{
    int32 Index = FindSmallBlockIndex(WorldSubBlockPos);
    if (Index != INDEX_NONE)
    {
        SmallBlocks.RemoveAt(Index);
        return true;
    }
    return false;
}

bool AVoxelChunk::HasSmallBlockAt(const FIntVector& WorldSubBlockPos) const
{
    return FindSmallBlockIndex(WorldSubBlockPos) != INDEX_NONE;
}

FName AVoxelChunk::GetSmallBlockID(const FIntVector& WorldSubBlockPos) const
{
    int32 Index = FindSmallBlockIndex(WorldSubBlockPos);
    if (Index != INDEX_NONE)
    {
        return SmallBlocks[Index].BlockID;
    }
    return NAME_None;
}

// ============================================================
// Block colors & materials
// ============================================================

FColor AVoxelChunk::GetBlockColor(FName BlockID) const
{
    UVoxelDatabase* DB = UVoxelDatabase::Get();
    if (DB)
    {
        if (UVoxelBlockData* BlockData = DB->GetBlockData(BlockID))
        {
            return BlockData->BlockColor;
        }
    }

    if (BlockID == "Stone") return FColor(128, 128, 128);
    if (BlockID == "Grass") return FColor(34, 139, 34);
    if (BlockID == "Sand") return FColor(238, 214, 175);
    if (BlockID == "Dirt") return FColor(139, 90, 43);

    return FColor::White;
}

int32 AVoxelChunk::GetBlockMaterialIndex(FName BlockID) const
{
    UVoxelDatabase* DB = UVoxelDatabase::Get();
    if (DB)
    {
        int32 Index = DB->GetBlockMaterialIndex(BlockID);
        return (Index >= 0) ? Index : 0;
    }
    return 0;
}

// ============================================================
// Blocky mesh generation (original)
// ============================================================

void AVoxelChunk::AddFaceToSection(TMap<int32, FMeshSectionData>& Sections, int32 MaterialIndex,
                                    const FVector& Position, const FVector& Normal, FName BlockID, float Size)
{
    FMeshSectionData& Section = Sections.FindOrAdd(MaterialIndex);
    
    int32 VertexStart = Section.Vertices.Num();
    FColor Color = GetBlockColor(BlockID);

    if (Normal.Z > 0)
    {
        Section.Vertices.Add(Position + FVector(0, 0, Size));
        Section.Vertices.Add(Position + FVector(0, Size, Size));
        Section.Vertices.Add(Position + FVector(Size, Size, Size));
        Section.Vertices.Add(Position + FVector(Size, 0, Size));
    }
    else if (Normal.Z < 0)
    {
        Section.Vertices.Add(Position + FVector(0, 0, 0));
        Section.Vertices.Add(Position + FVector(Size, 0, 0));
        Section.Vertices.Add(Position + FVector(Size, Size, 0));
        Section.Vertices.Add(Position + FVector(0, Size, 0));
    }
    else if (Normal.X > 0)
    {
        Section.Vertices.Add(Position + FVector(Size, 0, 0));
        Section.Vertices.Add(Position + FVector(Size, 0, Size));
        Section.Vertices.Add(Position + FVector(Size, Size, Size));
        Section.Vertices.Add(Position + FVector(Size, Size, 0));
    }
    else if (Normal.X < 0)
    {
        Section.Vertices.Add(Position + FVector(0, 0, 0));
        Section.Vertices.Add(Position + FVector(0, Size, 0));
        Section.Vertices.Add(Position + FVector(0, Size, Size));
        Section.Vertices.Add(Position + FVector(0, 0, Size));
    }
    else if (Normal.Y > 0)
    {
        Section.Vertices.Add(Position + FVector(0, Size, 0));
        Section.Vertices.Add(Position + FVector(Size, Size, 0));
        Section.Vertices.Add(Position + FVector(Size, Size, Size));
        Section.Vertices.Add(Position + FVector(0, Size, Size));
    }
    else
    {
        Section.Vertices.Add(Position + FVector(0, 0, 0));
        Section.Vertices.Add(Position + FVector(0, 0, Size));
        Section.Vertices.Add(Position + FVector(Size, 0, Size));
        Section.Vertices.Add(Position + FVector(Size, 0, 0));
    }

    Section.Triangles.Add(VertexStart + 0);
    Section.Triangles.Add(VertexStart + 1);
    Section.Triangles.Add(VertexStart + 2);
    Section.Triangles.Add(VertexStart + 0);
    Section.Triangles.Add(VertexStart + 2);
    Section.Triangles.Add(VertexStart + 3);

    for (int32 i = 0; i < 4; i++)
    {
        Section.Normals.Add(Normal);
        Section.Colors.Add(Color);
    }

    Section.UVs.Add(FVector2D(0, 0));
    Section.UVs.Add(FVector2D(0, 1));
    Section.UVs.Add(FVector2D(1, 1));
    Section.UVs.Add(FVector2D(1, 0));
}

void AVoxelChunk::GenerateBlockyMesh(TMap<int32, FMeshSectionData>& MeshSections)
{
    for (int32 X = 0; X < VoxelConstants::ChunkSizeX; X++)
    {
        for (int32 Y = 0; Y < VoxelConstants::ChunkSizeY; Y++)
        {
            for (int32 Z = 0; Z < VoxelConstants::ChunkSizeZ; Z++)
            {
                FName BlockID = GetBlock(X, Y, Z);
                if (BlockID.IsNone()) continue;

                int32 MaterialIndex = GetBlockMaterialIndex(BlockID);
                FVector Position(X * VoxelConstants::BlockSize, Y * VoxelConstants::BlockSize, Z * VoxelConstants::BlockSize);

                if (!IsBlockSolid(X, Y, Z + 1))
                    AddFaceToSection(MeshSections, MaterialIndex, Position, FVector(0, 0, 1), BlockID, VoxelConstants::BlockSize);
                if (!IsBlockSolid(X, Y, Z - 1))
                    AddFaceToSection(MeshSections, MaterialIndex, Position, FVector(0, 0, -1), BlockID, VoxelConstants::BlockSize);
                if (!IsBlockSolid(X + 1, Y, Z))
                    AddFaceToSection(MeshSections, MaterialIndex, Position, FVector(1, 0, 0), BlockID, VoxelConstants::BlockSize);
                if (!IsBlockSolid(X - 1, Y, Z))
                    AddFaceToSection(MeshSections, MaterialIndex, Position, FVector(-1, 0, 0), BlockID, VoxelConstants::BlockSize);
                if (!IsBlockSolid(X, Y + 1, Z))
                    AddFaceToSection(MeshSections, MaterialIndex, Position, FVector(0, 1, 0), BlockID, VoxelConstants::BlockSize);
                if (!IsBlockSolid(X, Y - 1, Z))
                    AddFaceToSection(MeshSections, MaterialIndex, Position, FVector(0, -1, 0), BlockID, VoxelConstants::BlockSize);
            }
        }
    }

    // Маленькие блоки
    for (const FSmallBlock& Block : SmallBlocks)
    {
        FIntVector LocalPos = WorldToLocalSubBlock(Block.Position);
        if (!IsSubBlockInChunk(LocalPos)) continue;

        int32 MaterialIndex = GetBlockMaterialIndex(Block.BlockID);
        FVector Position(
            LocalPos.X * VoxelConstants::PlayerBlockSize,
            LocalPos.Y * VoxelConstants::PlayerBlockSize,
            LocalPos.Z * VoxelConstants::PlayerBlockSize);

        auto HasNeighbor = [&](int32 DX, int32 DY, int32 DZ) -> bool
        {
            FIntVector NeighborWorld = Block.Position + FIntVector(DX, DY, DZ);
            if (HasSmallBlockAt(NeighborWorld)) return true;

            int32 WorldBlockX = FMath::FloorToInt((float)NeighborWorld.X / 4.0f);
            int32 WorldBlockY = FMath::FloorToInt((float)NeighborWorld.Y / 4.0f);
            int32 WorldBlockZ = FMath::FloorToInt((float)NeighborWorld.Z / 4.0f);
            int32 LocalBlockX = WorldBlockX - ChunkCoords.X * VoxelConstants::ChunkSizeX;
            int32 LocalBlockY = WorldBlockY - ChunkCoords.Y * VoxelConstants::ChunkSizeY;
            return IsBlockSolid(LocalBlockX, LocalBlockY, WorldBlockZ);
        };

        if (!HasNeighbor(0, 0, 1))  AddFaceToSection(MeshSections, MaterialIndex, Position, FVector(0, 0, 1), Block.BlockID, VoxelConstants::PlayerBlockSize);
        if (!HasNeighbor(0, 0, -1)) AddFaceToSection(MeshSections, MaterialIndex, Position, FVector(0, 0, -1), Block.BlockID, VoxelConstants::PlayerBlockSize);
        if (!HasNeighbor(1, 0, 0))  AddFaceToSection(MeshSections, MaterialIndex, Position, FVector(1, 0, 0), Block.BlockID, VoxelConstants::PlayerBlockSize);
        if (!HasNeighbor(-1, 0, 0)) AddFaceToSection(MeshSections, MaterialIndex, Position, FVector(-1, 0, 0), Block.BlockID, VoxelConstants::PlayerBlockSize);
        if (!HasNeighbor(0, 1, 0))  AddFaceToSection(MeshSections, MaterialIndex, Position, FVector(0, 1, 0), Block.BlockID, VoxelConstants::PlayerBlockSize);
        if (!HasNeighbor(0, -1, 0)) AddFaceToSection(MeshSections, MaterialIndex, Position, FVector(0, -1, 0), Block.BlockID, VoxelConstants::PlayerBlockSize);
    }
}

// ============================================================
// Density field & Marching Cubes (smooth terrain)
// ============================================================

int32 AVoxelChunk::DensityIndex(int32 X, int32 Y, int32 Z) const
{
    return X + Y * DensitySizeX() + Z * DensitySizeX() * DensitySizeY();
}

float AVoxelChunk::GetDensity(int32 X, int32 Y, int32 Z) const
{
    if (X < 0 || X >= DensitySizeX() ||
        Y < 0 || Y >= DensitySizeY() ||
        Z < 0 || Z >= DensitySizeZ())
    {
        return -1.0f;
    }
    return DensityField[DensityIndex(X, Y, Z)];
}

void AVoxelChunk::BuildDensityField()
{
    const int32 SX = DensitySizeX();
    const int32 SY = DensitySizeY();
    const int32 SZ = DensitySizeZ();
    const int32 P = DensityPadding;
    
    DensityField.SetNumZeroed(SX * SY * SZ);
    DensityBlockIDs.SetNum(SX * SY * SZ);
    
    // Определяем правильные ID блоков из базы данных (как в GenerateBlocksData)
    UVoxelDatabase* DB = UVoxelDatabase::Get();
    FName StoneID = "StoneL";
    FName GrassID = "GrassL";
    FName SandID = "SandL";
    if (DB)
    {
        if (!DB->GetBlockData(StoneID)) StoneID = "Stone";
        if (!DB->GetBlockData(GrassID)) GrassID = "Grass";
        if (!DB->GetBlockData(SandID)) SandID = "Sand";
    }
    
    // Мировые координаты начала чанка в блоках
    const int32 ChunkWorldStartX = ChunkCoords.X * VoxelConstants::ChunkSizeX;
    const int32 ChunkWorldStartY = ChunkCoords.Y * VoxelConstants::ChunkSizeY;
    
    for (int32 DX = 0; DX < SX; DX++)
    {
        for (int32 DY = 0; DY < SY; DY++)
        {
            // Мировая координата этой вершины density field в блоках
            int32 WorldVertX = ChunkWorldStartX - P + DX;
            int32 WorldVertY = ChunkWorldStartY - P + DY;
            
            // Вычисляем непрерывную высоту через noise (та же формула, что и в GenerateBlocksData)
            float NoiseX = (float)WorldVertX * VoxelConstants::NoiseScale;
            float NoiseY = (float)WorldVertY * VoxelConstants::NoiseScale;
            float NoiseValue = GetFBMNoise(NoiseX, NoiseY);
            float ContinuousHeight = VoxelConstants::HeightBase + NoiseValue * VoxelConstants::HeightAmplitude;
            
            // Определяем тип блока на этой высоте
            int32 IntHeight = FMath::Clamp(FMath::RoundToInt(ContinuousHeight), 1, VoxelConstants::ChunkSizeZ - 1);
            FName SurfaceBlock;
            if (IntHeight < 6)
                SurfaceBlock = SandID;
            else
                SurfaceBlock = GrassID;
            
            for (int32 DZ = 0; DZ < SZ; DZ++)
            {
                int32 Idx = DensityIndex(DX, DY, DZ);
                
                // Density = ContinuousHeight - Z
                // Положительное = под поверхностью (твёрдое)
                // Отрицательное = над поверхностью (воздух)
                float Density = ContinuousHeight - (float)DZ;
                
                // Клампим чтобы не было слишком больших значений
                Density = FMath::Clamp(Density, -2.0f, 2.0f);
                
                // Дно: Z=0 всегда твёрдое
                if (DZ == 0)
                {
                    Density = FMath::Max(Density, 1.0f);
                }
                
                // Учитываем ручные изменения блоков (SetBlock/RemoveBlock)
                if (DZ > 0)
                {
                    int32 BlockZ = DZ - 1;
                    FName ActualBlock = GetWorldBlock(WorldVertX, WorldVertY, BlockZ);
                    bool bShouldBeSolid = (BlockZ < IntHeight);
                    bool bActuallySolid = !ActualBlock.IsNone();
                    
                    // Если игрок удалил блок — принудительно делаем воздух
                    if (bShouldBeSolid && !bActuallySolid)
                    {
                        Density = -2.0f;  // Агрессивное значение чтобы smoothing не заполнил дырку
                    }
                    // Если игрок поставил блок — принудительно делаем твёрдым
                    // НО: только если это НЕ блок, поставленный игроком (large block)
                    // Блоки игрока будут отрисованы как blocky отдельно
                    else if (!bShouldBeSolid && bActuallySolid)
                    {
                        // FIX #2: Проверяем, является ли это блоком игрока
                        // Для блока внутри нашего чанка проверяем через IsPlayerPlacedBlock
                        int32 LocalX = WorldVertX - ChunkCoords.X * VoxelConstants::ChunkSizeX;
                        int32 LocalY = WorldVertY - ChunkCoords.Y * VoxelConstants::ChunkSizeY;
                        
                        bool bIsPlayerBlock = false;
                        if (LocalX >= 0 && LocalX < VoxelConstants::ChunkSizeX &&
                            LocalY >= 0 && LocalY < VoxelConstants::ChunkSizeY)
                        {
                            bIsPlayerBlock = IsPlayerPlacedBlock(LocalX, LocalY, BlockZ);
                        }
                        
                        if (!bIsPlayerBlock)
                        {
                            // Обычный блок (не от игрока) — включаем в density field
                            Density = FMath::Max(Density, 0.5f);
                        }
                        // else: блок игрока — НЕ включаем в density field,
                        // он будет отрисован как blocky ниже
                    }
                }
                
                DensityField[Idx] = Density;
                
                // Определяем BlockID для цвета/материала
                if (Density > 0.0f)
                {
                    if (DZ < IntHeight - 3)
                        DensityBlockIDs[Idx] = StoneID;
                    else
                        DensityBlockIDs[Idx] = SurfaceBlock;
                }
                else
                {
                    DensityBlockIDs[Idx] = SurfaceBlock;
                }
            }
        }
    }
}

void AVoxelChunk::SmoothDensityField()
{
    if (SmoothingPasses <= 0) return;
    
    const int32 SX = DensitySizeX();
    const int32 SY = DensitySizeY();
    const int32 SZ = DensitySizeZ();
    
    TArray<float> TempField;
    TempField.SetNumZeroed(SX * SY * SZ);
    
    for (int32 Pass = 0; Pass < SmoothingPasses; Pass++)
    {
        for (int32 X = 0; X < SX; X++)
        {
            for (int32 Y = 0; Y < SY; Y++)
            {
                for (int32 Z = 0; Z < SZ; Z++)
                {
                    int32 Idx = DensityIndex(X, Y, Z);
                    
                    // Не сглаживаем дно
                    if (Z <= 1)
                    {
                        TempField[Idx] = DensityField[Idx];
                        continue;
                    }
                    
                    // Не сглаживаем точки далеко от поверхности (оптимизация)
                    float CurDensity = DensityField[Idx];
                    if (FMath::Abs(CurDensity) > 1.5f)
                    {
                        TempField[Idx] = CurDensity;
                        continue;
                    }
                    
                    float Sum = 0.0f;
                    float Weight = 0.0f;
                    
                    for (int32 NDX = -1; NDX <= 1; NDX++)
                    {
                        for (int32 NDY = -1; NDY <= 1; NDY++)
                        {
                            for (int32 NDZ = -1; NDZ <= 1; NDZ++)
                            {
                                int32 NX = X + NDX;
                                int32 NY = Y + NDY;
                                int32 NZ = Z + NDZ;
                                
                                // Центральная точка имеет больший вес
                                float W = (NDX == 0 && NDY == 0 && NDZ == 0) ? 4.0f : 1.0f;
                                
                                if (NX >= 0 && NX < SX &&
                                    NY >= 0 && NY < SY &&
                                    NZ >= 0 && NZ < SZ)
                                {
                                    Sum += DensityField[DensityIndex(NX, NY, NZ)] * W;
                                    Weight += W;
                                }
                                else
                                {
                                    Sum += -1.0f * W;
                                    Weight += W;
                                }
                            }
                        }
                    }
                    
                    TempField[Idx] = Sum / Weight;
                }
            }
        }
        
        FMemory::Memcpy(DensityField.GetData(), TempField.GetData(), SX * SY * SZ * sizeof(float));
    }
}

FName AVoxelChunk::GetDominantBlockAt(int32 DensityX, int32 DensityY, int32 DensityZ) const
{
    int32 Idx = DensityIndex(
        FMath::Clamp(DensityX, 0, DensitySizeX() - 1),
        FMath::Clamp(DensityY, 0, DensitySizeY() - 1),
        FMath::Clamp(DensityZ, 0, DensitySizeZ() - 1)
    );
    
    if (Idx >= 0 && Idx < DensityBlockIDs.Num() && !DensityBlockIDs[Idx].IsNone())
    {
        return DensityBlockIDs[Idx];
    }
    
    return FName("Stone");
}

FVector AVoxelChunk::InterpolateEdge(const FVector& P1, const FVector& P2, float V1, float V2) const
{
    if (FMath::Abs(V1) < KINDA_SMALL_NUMBER) return P1;
    if (FMath::Abs(V2) < KINDA_SMALL_NUMBER) return P2;
    if (FMath::Abs(V1 - V2) < KINDA_SMALL_NUMBER) return P1;
    
    float T = -V1 / (V2 - V1);
    T = FMath::Clamp(T, 0.0f, 1.0f);
    
    return P1 + T * (P2 - P1);
}

void AVoxelChunk::GenerateSmoothMesh(TMap<int32, FMeshSectionData>& MeshSections)
{
    // ============================================================
    // ГИБРИДНЫЙ РЕНДЕР:
    // - Верхний слой (SmoothSurfaceDepth блоков от поверхности) → Marching Cubes
    // - Всё ниже → обычные кубы (blocky), чтобы игрок мог копать
    // - Блоки поставленные игроком → всегда blocky
    // ============================================================
    
    // Определяем правильные ID блоков из базы данных
    UVoxelDatabase* DB = UVoxelDatabase::Get();
    FName StoneID = "StoneL";
    FName GrassID = "GrassL";
    FName SandID = "SandL";
    if (DB)
    {
        if (!DB->GetBlockData(StoneID)) StoneID = "Stone";
        if (!DB->GetBlockData(GrassID)) GrassID = "Grass";
        if (!DB->GetBlockData(SandID)) SandID = "Sand";
    }
    
    const float BS = VoxelConstants::BlockSize;
    
    // ============================================================
    // Шаг 1: Для каждого столбца (X,Y) определяем высоту поверхности
    // ============================================================
    TArray<int32> SurfaceHeight;
    SurfaceHeight.SetNum(VoxelConstants::ChunkSizeX * VoxelConstants::ChunkSizeY);
    
    for (int32 X = 0; X < VoxelConstants::ChunkSizeX; X++)
    {
        for (int32 Y = 0; Y < VoxelConstants::ChunkSizeY; Y++)
        {
            int32 WorldBlockX = ChunkCoords.X * VoxelConstants::ChunkSizeX + X;
            int32 WorldBlockY = ChunkCoords.Y * VoxelConstants::ChunkSizeY + Y;
            float NoiseX = (float)WorldBlockX * VoxelConstants::NoiseScale;
            float NoiseY = (float)WorldBlockY * VoxelConstants::NoiseScale;
            float NoiseValue = GetFBMNoise(NoiseX, NoiseY);
            float Height = VoxelConstants::HeightBase + NoiseValue * VoxelConstants::HeightAmplitude;
            int32 MaxH = FMath::Clamp(FMath::RoundToInt(Height), 1, VoxelConstants::ChunkSizeZ - 1);
            SurfaceHeight[X + Y * VoxelConstants::ChunkSizeX] = MaxH;
        }
    }
    
    // Хелпер: является ли блок частью сглаживаемой поверхности
    auto IsSurfaceLayer = [&](int32 X, int32 Y, int32 Z) -> bool
    {
        if (X < 0 || X >= VoxelConstants::ChunkSizeX ||
            Y < 0 || Y >= VoxelConstants::ChunkSizeY)
            return false;
        int32 SH = SurfaceHeight[X + Y * VoxelConstants::ChunkSizeX];
        // Сглаживаем блоки в диапазоне [SH - SmoothSurfaceDepth, SH + 1]
        return (Z >= SH - SmoothSurfaceDepth && Z <= SH + 1);
    };
    
    // ============================================================
    // Шаг 2: Рендерим НИЖНИЕ блоки как кубы (blocky)
    // ============================================================
    for (int32 X = 0; X < VoxelConstants::ChunkSizeX; X++)
    {
        for (int32 Y = 0; Y < VoxelConstants::ChunkSizeY; Y++)
        {
            for (int32 Z = 0; Z < VoxelConstants::ChunkSizeZ; Z++)
            {
                FName BlockID = GetBlock(X, Y, Z);
                if (BlockID.IsNone()) continue;
                
                // Пропускаем блоки в зоне сглаживания — они будут через MC
                // Но блоки игрока всегда blocky
                bool bPlayerBlock = IsPlayerPlacedBlock(X, Y, Z);
                if (!bPlayerBlock && IsSurfaceLayer(X, Y, Z)) continue;
                
                int32 MaterialIndex = GetBlockMaterialIndex(BlockID);
                FVector Position(X * BS, Y * BS, Z * BS);
                
                // Проверяем видимость каждой грани
                auto IsFaceVisible = [&](int32 NX, int32 NY, int32 NZ) -> bool
                {
                    if (NZ < 0 || NZ >= VoxelConstants::ChunkSizeZ) return true;
                    if (NX < 0 || NX >= VoxelConstants::ChunkSizeX ||
                        NY < 0 || NY >= VoxelConstants::ChunkSizeY)
                        return true; // Граница чанка — видно
                    
                    FName NeighborID = GetBlock(NX, NY, NZ);
                    if (NeighborID.IsNone()) return true; // Воздух — видно
                    
                    // Если сосед в surface layer — грань скрыта (MC покроет),
                    // НО: если сосед находится в surface layer, он может быть
                    // удалённым или нет. Если сосед solid и в surface layer — MC покроет.
                    // Грань скрыта.
                    return false; // Сосед — solid блок, грань скрыта
                };
                
                if (IsFaceVisible(X, Y, Z + 1))
                    AddFaceToSection(MeshSections, MaterialIndex, Position, FVector(0, 0, 1), BlockID, BS);
                if (IsFaceVisible(X, Y, Z - 1))
                    AddFaceToSection(MeshSections, MaterialIndex, Position, FVector(0, 0, -1), BlockID, BS);
                if (IsFaceVisible(X + 1, Y, Z))
                    AddFaceToSection(MeshSections, MaterialIndex, Position, FVector(1, 0, 0), BlockID, BS);
                if (IsFaceVisible(X - 1, Y, Z))
                    AddFaceToSection(MeshSections, MaterialIndex, Position, FVector(-1, 0, 0), BlockID, BS);
                if (IsFaceVisible(X, Y + 1, Z))
                    AddFaceToSection(MeshSections, MaterialIndex, Position, FVector(0, 1, 0), BlockID, BS);
                if (IsFaceVisible(X, Y - 1, Z))
                    AddFaceToSection(MeshSections, MaterialIndex, Position, FVector(0, -1, 0), BlockID, BS);
            }
        }
    }
    
    // ============================================================
    // Шаг 3: Marching Cubes для поверхностного слоя
    // ============================================================
    BuildDensityField();
    SmoothDensityField();
    
    const int32 P = DensityPadding;
    
    static const int32 CubeVerts[8][3] = {
        {0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0},
        {0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}
    };
    
    static const int32 EdgeVertices[12][2] = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0},
        {4, 5}, {5, 6}, {6, 7}, {7, 4},
        {0, 4}, {1, 5}, {2, 6}, {3, 7}
    };
    
    for (int32 X = 0; X < VoxelConstants::ChunkSizeX; X++)
    {
        for (int32 Y = 0; Y < VoxelConstants::ChunkSizeY; Y++)
        {
            int32 SH = SurfaceHeight[X + Y * VoxelConstants::ChunkSizeX];
            
            // MC только в зоне поверхности
            int32 ZMin = FMath::Max(0, SH - SmoothSurfaceDepth - 1);
            int32 ZMax = FMath::Min(VoxelConstants::ChunkSizeZ - 1, SH + 2);
            
            for (int32 Z = ZMin; Z <= ZMax; Z++)
            {
                int32 DFX = X + P;
                int32 DFY = Y + P;
                int32 DFZ = Z;
                
                float CubeDensity[8];
                for (int32 i = 0; i < 8; i++)
                {
                    CubeDensity[i] = GetDensity(
                        DFX + CubeVerts[i][0],
                        DFY + CubeVerts[i][1],
                        DFZ + CubeVerts[i][2]
                    );
                }
                
                int32 CubeIndex = 0;
                for (int32 i = 0; i < 8; i++)
                {
                    if (CubeDensity[i] > 0.0f)
                    {
                        CubeIndex |= (1 << i);
                    }
                }
                
                if (EdgeTable[CubeIndex] == 0)
                    continue;
                
                // Определяем блок для цвета/материала
                FName BlockID;
                if (SH < 6)
                    BlockID = SandID;
                else
                    BlockID = GrassID;
                
                int32 MaterialIndex = GetBlockMaterialIndex(BlockID);
                FColor Color = GetBlockColor(BlockID);
                
                FMeshSectionData& Section = MeshSections.FindOrAdd(MaterialIndex);
                
                FVector CubePositions[8];
                for (int32 i = 0; i < 8; i++)
                {
                    CubePositions[i] = FVector(
                        (X + CubeVerts[i][0]) * BS,
                        (Y + CubeVerts[i][1]) * BS,
                        (Z + CubeVerts[i][2]) * BS
                    );
                }
                
                FVector EdgePoints[12];
                int32 EdgeBits = EdgeTable[CubeIndex];
                for (int32 i = 0; i < 12; i++)
                {
                    if (EdgeBits & (1 << i))
                    {
                        int32 V0Idx = EdgeVertices[i][0];
                        int32 V1Idx = EdgeVertices[i][1];
                        EdgePoints[i] = InterpolateEdge(
                            CubePositions[V0Idx], CubePositions[V1Idx],
                            CubeDensity[V0Idx], CubeDensity[V1Idx]
                        );
                    }
                }
                
                for (int32 i = 0; TriTable[CubeIndex][i] != -1; i += 3)
                {
                    FVector TV0 = EdgePoints[TriTable[CubeIndex][i]];
                    FVector TV1 = EdgePoints[TriTable[CubeIndex][i + 1]];
                    FVector TV2 = EdgePoints[TriTable[CubeIndex][i + 2]];
                    
                    FVector Normal = FVector::CrossProduct(TV1 - TV0, TV2 - TV0).GetSafeNormal();
                    
                    if (Normal.IsNearlyZero())
                        continue;
                    
                    int32 VertStart = Section.Vertices.Num();
                    
                    // Прямой порядок вершин
                    Section.Vertices.Add(TV0);
                    Section.Vertices.Add(TV1);
                    Section.Vertices.Add(TV2);
                    
                    Section.Triangles.Add(VertStart);
                    Section.Triangles.Add(VertStart + 1);
                    Section.Triangles.Add(VertStart + 2);
                    
                    // Инвертируем нормаль для правильного освещения
                    FVector FlippedNormal = -Normal;
                    for (int32 V = 0; V < 3; V++)
                    {
                        Section.Normals.Add(FlippedNormal);
                        Section.Colors.Add(Color);
                    }
                    
                    FVector AbsNormal = FlippedNormal.GetAbs();
                    for (const FVector& Vert : {TV0, TV1, TV2})
                    {
                        FVector2D UV;
                        if (AbsNormal.Z >= AbsNormal.X && AbsNormal.Z >= AbsNormal.Y)
                            UV = FVector2D(Vert.X / BS, Vert.Y / BS);
                        else if (AbsNormal.X >= AbsNormal.Y)
                            UV = FVector2D(Vert.Y / BS, Vert.Z / BS);
                        else
                            UV = FVector2D(Vert.X / BS, Vert.Z / BS);
                        Section.UVs.Add(UV);
                    }
                }
            }
        }
    }
    
    // ============================================================
    // Шаг 4: Маленькие блоки — всегда blocky
    // ============================================================
    for (const FSmallBlock& Block : SmallBlocks)
    {
        FIntVector LocalPos = WorldToLocalSubBlock(Block.Position);
        if (!IsSubBlockInChunk(LocalPos)) continue;

        int32 MaterialIndex = GetBlockMaterialIndex(Block.BlockID);
        FVector Position(
            LocalPos.X * VoxelConstants::PlayerBlockSize,
            LocalPos.Y * VoxelConstants::PlayerBlockSize,
            LocalPos.Z * VoxelConstants::PlayerBlockSize);

        auto HasNeighbor = [&](int32 DX, int32 DY, int32 DZ) -> bool
        {
            FIntVector NeighborWorld = Block.Position + FIntVector(DX, DY, DZ);
            if (HasSmallBlockAt(NeighborWorld)) return true;
            int32 WorldBlockX = FMath::FloorToInt((float)NeighborWorld.X / 4.0f);
            int32 WorldBlockY = FMath::FloorToInt((float)NeighborWorld.Y / 4.0f);
            int32 WorldBlockZ = FMath::FloorToInt((float)NeighborWorld.Z / 4.0f);
            int32 LocalBlockX = WorldBlockX - ChunkCoords.X * VoxelConstants::ChunkSizeX;
            int32 LocalBlockY = WorldBlockY - ChunkCoords.Y * VoxelConstants::ChunkSizeY;
            return IsBlockSolid(LocalBlockX, LocalBlockY, WorldBlockZ);
        };

        if (!HasNeighbor(0, 0, 1))  AddFaceToSection(MeshSections, MaterialIndex, Position, FVector(0, 0, 1), Block.BlockID, VoxelConstants::PlayerBlockSize);
        if (!HasNeighbor(0, 0, -1)) AddFaceToSection(MeshSections, MaterialIndex, Position, FVector(0, 0, -1), Block.BlockID, VoxelConstants::PlayerBlockSize);
        if (!HasNeighbor(1, 0, 0))  AddFaceToSection(MeshSections, MaterialIndex, Position, FVector(1, 0, 0), Block.BlockID, VoxelConstants::PlayerBlockSize);
        if (!HasNeighbor(-1, 0, 0)) AddFaceToSection(MeshSections, MaterialIndex, Position, FVector(-1, 0, 0), Block.BlockID, VoxelConstants::PlayerBlockSize);
        if (!HasNeighbor(0, 1, 0))  AddFaceToSection(MeshSections, MaterialIndex, Position, FVector(0, 1, 0), Block.BlockID, VoxelConstants::PlayerBlockSize);
        if (!HasNeighbor(0, -1, 0)) AddFaceToSection(MeshSections, MaterialIndex, Position, FVector(0, -1, 0), Block.BlockID, VoxelConstants::PlayerBlockSize);
    }
}

// ============================================================
// Material application
// ============================================================

void AVoxelChunk::ApplyMaterialsToMesh()
{
    UVoxelDatabase* DB = UVoxelDatabase::Get();
    if (!DB) return;
    
    TMap<int32, UMaterialInterface*> MaterialsToApply;
    
    TArray<UVoxelBlockData*> AllBlocks = DB->GetAllBlocks();
    for (UVoxelBlockData* Block : AllBlocks)
    {
        if (!Block) continue;
        int32 MatIndex = FMath::Max(0, Block->MaterialIndex);
        if (!MaterialsToApply.Contains(MatIndex))
        {
            UMaterialInterface* Mat = nullptr;
            if (!Block->Material.IsNull())
            {
                Mat = Block->Material.LoadSynchronous();
            }
            if (Mat)
            {
                MaterialsToApply.Add(MatIndex, Mat);
            }
        }
    }
    
    for (const auto& Pair : SectionMaterials)
    {
        int32 SectionIndex = Pair.Key;
        if (UMaterialInterface** MatPtr = MaterialsToApply.Find(SectionIndex))
        {
            MeshComponent->SetMaterial(SectionIndex, *MatPtr);
        }
    }
}

// ============================================================
// Main mesh generation
// ============================================================

void AVoxelChunk::GenerateMesh()
{
    TMap<int32, FMeshSectionData> MeshSections;

    if (bUseSmoothTerrain)
    {
        GenerateSmoothMesh(MeshSections);
    }
    else
    {
        GenerateBlockyMesh(MeshSections);
    }

    MeshComponent->ClearAllMeshSections();
    SectionMaterials.Empty();

    for (const auto& Pair : MeshSections)
    {
        int32 SectionIndex = Pair.Key;
        const FMeshSectionData& Section = Pair.Value;
        
        if (Section.IsEmpty()) continue;
        
        int32 MeshSectionIndex = FMath::Max(0, SectionIndex);
        
        MeshComponent->CreateMeshSection(
            MeshSectionIndex,
            Section.Vertices,
            Section.Triangles,
            Section.Normals,
            Section.UVs,
            Section.Colors,
            TArray<FProcMeshTangent>(),
            true
        );
        
        SectionMaterials.Add(MeshSectionIndex, nullptr);
    }
    
    ApplyMaterialsToMesh();

    if (MeshSections.Num() > 0)
    {
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
        MeshComponent->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
        MeshComponent->RecreatePhysicsState();
    }
}
