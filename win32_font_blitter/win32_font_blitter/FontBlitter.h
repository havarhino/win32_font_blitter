#pragma once
#include <stdint.h>

typedef struct {
	int pixelWidth;
	int pixelHeight;
	int planes;
	int bitsPerPixel;
	uint32_t * dataPtr;
} PixelMemory;

typedef struct {
	int left;
	int top;
	int right;
	int bottom;
} BoundingBox;

class FontBlitter {

public:
	/*
	void commonHBITMAPConstructor(HBITMAP inBitmap, int inFirstGlyphOffset, bool inInvertGlyphColor, int inCellWidth, int inCellHeight);
	FontBlitter(HBITMAP inBitmap, int inFirstGlyphOffset, bool inInvertGlyphColor);
	FontBlitter(HBITMAP inBitmap, int inFirstGlyphOffset, bool inInvertGlyphColor, int inCellWidth, int inCellHeight);
	*/
	FontBlitter(PixelMemory * pixels, int inFirstGlyphOffset, bool inInvertGlyphColor, int inCellWidth, int inCellHeight);
	void DrawLetter(PixelMemory * destPM, char c, int x, int y);
	void DrawNumber(PixelMemory * destPM, int N, int x, int y);
	void DrawString(PixelMemory * destPM, char * str, int x, int y);
	void GetBoundingBox(PixelMemory * destPM, char * str, int x, int y, BoundingBox * bb);

private:
	int cellWidth = -1;
	int cellHeight = -1;
	int firstGlyphOffset = -1;
	bool invertGlyphColor = false;

	PixelMemory pm = { 0 };

	int numGlyphs;

	uint32_t ** glyphArray;
	uint32_t ** glyphArrayMask;

	void createGlyphs();

};