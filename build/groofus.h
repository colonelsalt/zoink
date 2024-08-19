
//{{BLOCK(groofus)

//======================================================================
//
//	groofus, 512x64@4, 
//	+ palette 16 entries, not compressed
//	+ 512 tiles Metatiled by 8x8 not compressed
//	Total size: 32 + 16384 = 16416
//
//	Time-stamp: 2024-08-18, 14:48:14
//	Exported by Cearn's GBA Image Transmogrifier, v0.8.69
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#pragma once

namespace grit {

namespace groofus {
constexpr u32 TilesCount = 16384;
extern const unsigned char Tiles[16384];

constexpr u32 PalCount = 32;
extern const unsigned short Pal[16];

}}// GRIT_GROOFUS_H

//}}BLOCK(groofus)
