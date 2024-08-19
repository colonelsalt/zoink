#pragma once

namespace bg {
namespace jack_home_floor1 {

constexpr u32 TILE_WIDTH = 32;
constexpr u32 TILE_HEIGHT = 32;
constexpr u32 TILE_COUNT = 1024;
constexpr u32 SIZE = 2048;

extern const u16 BG1[1024];
extern const u16 BG2[1024];
extern const u16 BG3[1024];
extern const u8 Collision[1024];
constexpr u32 WARPS_COUNT = 4;
extern const warp_tile Warps[4];
constexpr u32 CHARACTER_COUNT = 0;
extern const level_char Characters[0];
extern const u16* Layers[3];

}}
