#pragma once

namespace bg {
namespace forbidden_forest {

constexpr u32 TILE_WIDTH = 128;
constexpr u32 TILE_HEIGHT = 128;
constexpr u32 TILE_COUNT = 16384;
constexpr u32 SIZE = 32768;

extern const u16 BG1[16384];
extern const u16 BG2[16384];
extern const u16 BG3[16384];
extern const u8 Collision[16384];
constexpr u32 WARPS_COUNT = 4;
extern const warp_tile Warps[4];
constexpr u32 CHARACTER_COUNT = 6;
extern const level_char Characters[6];
extern const u16* Layers[3];

}}
