#pragma once
#include <wchar.h>
#include <stdint.h>
#include "FontBlitter.h"

typedef struct {
	int pixelWidth;
	int pixelHeight;
	int bitsPerPixel;
	int planes;
	uint32_t * dataPtr;
} DrawMemory;

class DrawOntoDC {

public:
	DrawOntoDC(DrawMemory * inDrawMemory, FontBlitter ** inFontBlitterArray);
	~DrawOntoDC(void);

	void draw(void);
	void updateWindowDimensions(void);

private:
    wchar_t dbgStr[512] = L"";
	int xLoc = 0;
	uint32_t * ballArray;
	uint32_t * ballArrayMask;
	int ballDiameter;
	FontBlitter * fontBlitter;
	FontBlitter ** fontBlitterArray;


	// DIB Bitmap things
	int lineCounter = 0;
#if 0
	BYTE* m_pBitmapBits = NULL;
	HDC h_dibDC = NULL;
	HBITMAP m_hOldDIBBitmap = NULL;
	HBITMAP m_hDIBBitmap = NULL;
	int DIBwidth, DIBheight, DIBrowByteWidth;
	void createDIB(HDC hdc);
#endif
	DrawMemory dm = { 0 };

	uint32_t * getPixelAddress(int x, int y);
	void drawDIB();
};
