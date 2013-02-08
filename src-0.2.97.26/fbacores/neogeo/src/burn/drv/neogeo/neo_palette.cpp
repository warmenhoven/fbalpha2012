#include "neogeo.h"
// Neo Geo -- palette functions

UINT8* NeoPalSrc[2];		// Pointer to input palettes
UINT32* NeoPalette;
INT32 nNeoPaletteBank;				// Selected palette bank

static UINT32* NeoPaletteData[2] = {NULL, NULL};
#ifndef __LIBRETRO_OPTIMIZATIONS__
static UINT16* NeoPaletteCopy[2] = {NULL, NULL};
#endif

UINT8 NeoRecalcPalette;

INT32 NeoInitPalette()
{
	for (INT32 i = 0; i < 2; i++) {
		if (NeoPaletteData[i]) {
			BurnFree(NeoPaletteData[i]);
		}
#ifndef __LIBRETRO_OPTIMIZATIONS__
		if (NeoPaletteCopy[i]) {
			BurnFree(NeoPaletteCopy[i]);
		}
#endif
		NeoPaletteData[i] = (UINT32*)BurnMalloc(4096 * sizeof(UINT32));
#ifndef __LIBRETRO_OPTIMIZATIONS__
		NeoPaletteCopy[i] = (UINT16*)BurnMalloc(4096 * sizeof(UINT16));
#endif
	}

	NeoRecalcPalette = 1;

	return 0;
}

void NeoExitPalette()
{
	for (INT32 i = 0; i < 2; i++) {
		BurnFree(NeoPaletteData[i]);
#ifndef __LIBRETRO_OPTIMIZATIONS__
		BurnFree(NeoPaletteCopy[i]);
#endif
	}
}

inline static UINT32 CalcCol(UINT16 nColour)
{
	INT32 r = (nColour & 0x0F00) >> 4;	// Red
	r |= ((nColour >> 11) & 8) >> 5;
	INT32 g = (nColour & 0x00F0);			// Green
	g |= ((nColour >> 10) & 8) >> 5;
	INT32 b = (nColour & 0x000F) << 4;	// Blue
	b |= ((nColour >> 9) & 8) >> 5;

	return BurnHighCol(r, g, b, 0);
}

#ifndef __LIBRETRO_OPTIMIZATIONS__
INT32 NeoUpdatePalette()
{
	if (NeoRecalcPalette) {
		INT32 i;
		UINT16* ps;
		UINT16* pc;
		UINT32* pd;

		// Update both palette banks
		for (INT32 j = 0; j < 2; j++) {
			for (i = 0, ps = (UINT16*)NeoPalSrc[j], pc = NeoPaletteCopy[j], pd = NeoPaletteData[j]; i < 4096; i++, ps++, pc++, pd++) {
				*pc = *ps;
				*pd = CalcCol(BURN_ENDIAN_SWAP_INT16(*ps));
			}
		}

		NeoRecalcPalette = 0;

	}

	return 0;
}
#endif

void NeoSetPalette()
{
	NeoPalette = NeoPaletteData[nNeoPaletteBank];
}

// Update the PC copy of the palette on writes to the palette memory
void __fastcall NeoPalWriteByte(UINT32 nAddress, UINT8 byteValue)
{
	nAddress &= 0x1FFF;
	nAddress ^= 1;

	NeoPalSrc[nNeoPaletteBank][nAddress] = byteValue;							// write byte

#ifndef __LIBRETRO_OPTIMIZATIONS__
	if (*((UINT8*)(NeoPaletteCopy[nNeoPaletteBank] + nAddress)) != byteValue) {
		*((UINT8*)(NeoPaletteCopy[nNeoPaletteBank] + nAddress)) = byteValue;
#endif
		NeoPaletteData[nNeoPaletteBank][nAddress >> 1] = CalcCol(*(UINT16*)(NeoPalSrc[nNeoPaletteBank] + (nAddress & ~0x01)));
#ifndef __LIBRETRO_OPTIMIZATIONS__
	}
#endif
}

void __fastcall NeoPalWriteWord(UINT32 nAddress, UINT16 wordValue)
{
	nAddress &= 0x1FFF;
	nAddress >>= 1;

	((UINT16*)NeoPalSrc[nNeoPaletteBank])[nAddress] = BURN_ENDIAN_SWAP_INT16(wordValue);		// write word

#ifndef __LIBRETRO_OPTIMIZATIONS__
	if (NeoPaletteCopy[nNeoPaletteBank][nAddress] != BURN_ENDIAN_SWAP_INT16(wordValue)) {
		NeoPaletteCopy[nNeoPaletteBank][nAddress] = BURN_ENDIAN_SWAP_INT16(wordValue);
#endif
		NeoPaletteData[nNeoPaletteBank][nAddress] = CalcCol(wordValue);
#ifndef __LIBRETRO_OPTIMIZATIONS__
	}
#endif
}
