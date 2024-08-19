
//{{BLOCK(item_box)

//======================================================================
//
//	item_box, 16x16@4, 
//	+ palette 16 entries, not compressed
//	+ 4 tiles Metatiled by 2x2 not compressed
//	Total size: 32 + 128 = 160
//
//	Time-stamp: 2024-08-18, 14:48:15
//	Exported by Cearn's GBA Image Transmogrifier, v0.8.69
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#pragma once

namespace grit {

namespace item_box {
constexpr u32 TilesCount = 128;
extern const unsigned char Tiles[128];

constexpr u32 PalCount = 32;
extern const unsigned short Pal[16];

}}// GRIT_ITEM_BOX_H

//}}BLOCK(item_box)
