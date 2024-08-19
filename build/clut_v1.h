
//{{BLOCK(clut_v1)

//======================================================================
//
//	clut_v1, 192x168@8, 
//	+ palette 256 entries, not compressed
//	+ 504 tiles not compressed
//	Total size: 512 + 32256 = 32768
//
//	Time-stamp: 2024-08-18, 14:48:12
//	Exported by Cearn's GBA Image Transmogrifier, v0.8.69
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#pragma once

namespace grit {

namespace clut_v1 {
constexpr u32 TilesCount = 32256;
extern const unsigned char Tiles[32256];

constexpr u32 PalCount = 512;
extern const unsigned short Pal[256];

}}// GRIT_CLUT_V1_H

//}}BLOCK(clut_v1)
