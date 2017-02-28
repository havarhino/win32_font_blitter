#pragma once
#include "stdafx.h"
#include <stdint.h>

#define WHITE_THRESHOLD (250)
#define BLACK_THRESHOLD (5)

#define USE_SIMPLE

class FontBlitter {

public:
	FontBlitter(HBITMAP inBitmap);
	void DrawLetter(HDC hdc, char c, int x, int y);
	void DrawNumber(HDC hdc, int N, int x, int y);
	void DrawString(HDC hdc, char * str, int x, int y);

private:
	BITMAPINFO fontBitmapInfo = { 0 };
	HBITMAP hBitmap = 0;
	int cellWidth = -1;
	int cellHeight = -1;
	BYTE * bitmapDataPtr = 0;
	int numGlyphs;

	BITMAPINFO glyphBmpInfo = {0};
	uint32_t ** glyphArray;
	uint32_t ** glyphArrayMask;

	void loadImages();
	void createGlyphs();

};