
//{{BLOCK(title_screen_tiles)

//======================================================================
//
//	title_screen_tiles, 632x64@8, 
//	+ palette 256 entries, not compressed
//	+ 632 tiles not compressed
//	Total size: 512 + 40448 = 40960
//
//	Time-stamp: 2024-08-18, 19:15:34
//	Exported by Cearn's GBA Image Transmogrifier, v0.8.69
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#pragma once

namespace grit {

namespace title_screen_tiles {
constexpr u32 TilesCount = 40448;
extern const unsigned char Tiles[40448];

constexpr u32 PalCount = 512;
extern const unsigned short Pal[256];

}}// GRIT_TITLE_SCREEN_TILES_H

//}}BLOCK(title_screen_tiles)
