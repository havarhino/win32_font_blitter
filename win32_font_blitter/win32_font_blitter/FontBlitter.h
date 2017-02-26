#pragma once
#include "stdafx.h"

#define WHITE_THRESHOLD (250)
#define BLACK_THRESHOLD (5)

class FontBlitter {

public:
	FontBlitter(HBITMAP inBitmap);

private:
	BITMAPINFO fontBitmapInfo = { 0 };
	HBITMAP hBitmap = 0;
	int cellWidth = -1;
	int cellHeight = -1;
	BYTE * bitmapDataPtr = 0;

	BYTE * ToPixels(HBITMAP BitmapHandle, BITMAPINFO & info);
	void computeGridSize(BYTE * pixels, BITMAPINFO & info, int &gridWidth, int &gridHeight);
	void loadImages();

};