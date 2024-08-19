#pragma once

namespace bg {
namespace city {

constexpr u32 TILE_WIDTH = 96;
constexpr u32 TILE_HEIGHT = 96;
constexpr u32 TILE_COUNT = 9216;
constexpr u32 SIZE = 18432;

extern const u16 Tile_Layer_1[9216];
extern const u16 Tile_Layer_2[9216];
extern const u16 Tile_Layer_3[9216];
extern const u8 Collision[9216];
constexpr u32 WARPS_COUNT = 6;
extern const warp_tile Warps[6];
constexpr u32 CHARACTER_COUNT = 5;
extern const level_char Characters[5];
constexpr u32 SCRIPT_TILES_COUNT = 4;
extern const script_tile ScriptTiles[4];
extern const u16* Layers[3];

}}
