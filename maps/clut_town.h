#pragma once

namespace bg {
namespace clut_town {

constexpr u32 TILE_WIDTH = 96;
constexpr u32 TILE_HEIGHT = 96;
constexpr u32 TILE_COUNT = 9216;
constexpr u32 SIZE = 18432;

extern const u16 BG1[9216];
extern const u16 BG2[9216];
extern const u16 BG3[9216];
extern const u8 Collision[9216];
constexpr u32 WARPS_COUNT = 12;
extern const warp_tile Warps[12];
constexpr u32 SCRIPT_TILES_COUNT = 14;
extern const script_tile ScriptTiles[14];
constexpr u32 CHARACTER_COUNT = 12;
extern const level_char Characters[12];
extern const u16* Layers[3];

}}
