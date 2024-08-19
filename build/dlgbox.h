
//{{BLOCK(dlgbox)

//======================================================================
//
//	dlgbox, 240x160@4, 
//	+ palette 16 entries, not compressed
//	+ 600 tiles not compressed
//	Total size: 32 + 19200 = 19232
//
//	Time-stamp: 2024-08-18, 14:48:12
//	Exported by Cearn's GBA Image Transmogrifier, v0.8.69
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#pragma once

namespace grit {

namespace dlgbox {
constexpr u32 TilesCount = 19200;
extern const unsigned int Tiles[4800];

constexpr u32 PalCount = 32;
extern const unsigned short Pal[16];

}}// GRIT_DLGBOX_H

//}}BLOCK(dlgbox)
