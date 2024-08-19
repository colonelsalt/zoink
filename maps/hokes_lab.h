#pragma once

namespace bg {
namespace hokes_lab {

constexpr u32 TILE_WIDTH = 32;
constexpr u32 TILE_HEIGHT = 32;
constexpr u32 TILE_COUNT = 1024;
constexpr u32 SIZE = 2048;

extern const u16 BG1[1024];
extern const u16 BG2[1024];
extern const u16 BG3[1024];
extern const u8 Collision[1024];
constexpr u32 CHARACTER_COUNT = 3;
extern const level_char Characters[3];
constexpr u32 WARPS_COUNT = 2;
extern const warp_tile Warps[2];
extern const u16* Layers[3];

}}
