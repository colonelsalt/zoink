
//{{BLOCK(goblin)

//======================================================================
//
//	goblin, 800x32@4, 
//	+ palette 16 entries, not compressed
//	+ 400 tiles Metatiled by 4x4 not compressed
//	Total size: 32 + 12800 = 12832
//
//	Time-stamp: 2024-08-18, 14:48:14
//	Exported by Cearn's GBA Image Transmogrifier, v0.8.69
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#pragma once

namespace grit {

namespace goblin {
constexpr u32 TilesCount = 12800;
extern const unsigned char Tiles[12800];

constexpr u32 PalCount = 32;
extern const unsigned short Pal[16];

}}// GRIT_GOBLIN_H

//}}BLOCK(goblin)
