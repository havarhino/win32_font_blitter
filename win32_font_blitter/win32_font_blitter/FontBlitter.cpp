#include "FontBlitter.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
/*
void FontBlitter::commonHBITMAPConstructor(HBITMAP inBitmap, int inFirstGlyphOffset, bool inInvertGlyphColor, int inCellWidth, int inCellHeight) {
	hBitmap = inBitmap;
	numGlyphs = 0;

	firstGlyphOffset = inFirstGlyphOffset;
	invertGlyphColor = inInvertGlyphColor;
	cellWidth = inCellWidth;
	cellHeight = inCellHeight;

	loadImages();
}

FontBlitter::FontBlitter(HBITMAP inBitmap, int inFirstGlyphOffset, bool inInvertGlyphColor) {

	commonHBITMAPConstructor(inBitmap, inFirstGlyphOffset, inInvertGlyphColor, -1, -1);
}

FontBlitter::FontBlitter(HBITMAP inBitmap, int inFirstGlyphOffset, bool inInvertGlyphColor, int inCellWidth, int inCellHeight) {

	commonHBITMAPConstructor(inBitmap, inFirstGlyphOffset, inInvertGlyphColor, inCellWidth, inCellHeight);
}
*/

FontBlitter::FontBlitter(PixelMemory * inPixelMemory, int inFirstGlyphOffset, bool inInvertGlyphColor, int inCellWidth, int inCellHeight) {
	cellWidth = inCellWidth;
	cellHeight = inCellHeight;
	firstGlyphOffset = inFirstGlyphOffset;
	invertGlyphColor = inInvertGlyphColor;
	pm = *inPixelMemory;

	createGlyphs();
}

void FontBlitter::createGlyphs() {
	int columns = pm.pixelWidth / cellWidth;
	int rows = pm.pixelHeight / cellHeight;

	numGlyphs = rows * columns;

	glyphArray = new uint32_t*[numGlyphs];
	glyphArrayMask = new uint32_t*[numGlyphs];

	uint32_t* wordBitmapDataPtr = pm.dataPtr;
	for (int r = 0; r < rows; r++) {
		for (int c = 0; c < columns; c++) {
			uint32_t * glyphDestPtr = new uint32_t[cellWidth * cellHeight];
			uint32_t * glyphMaskDestPtr = new uint32_t[cellWidth * cellHeight];
			glyphArray[r*columns + c] = glyphDestPtr;
			glyphArrayMask[r*columns + c] = glyphMaskDestPtr;


			uint32_t * glyphSrcPtr = (uint32_t*)(&wordBitmapDataPtr[(r * cellHeight) * pm.pixelWidth + (c*cellWidth)]);
			for (int y = 0; y < cellHeight; y++) {
				uint32_t * srcPtr = glyphSrcPtr;
				uint32_t * destPtr = glyphDestPtr;
				uint32_t * destMaskPtr = glyphMaskDestPtr;
				for (int x = 0; x < cellWidth; x++) {
					uint32_t v = (*srcPtr++);
					if( invertGlyphColor ) {
						v = ~v;
					}
					*destPtr++ = v;
					*destMaskPtr++ = ((v & 0x000F0F0F)  == 0) ? 0x00FFFFFF : 0x00000000;
				}
				glyphSrcPtr += pm.pixelWidth;
				glyphDestPtr += cellWidth;
				glyphMaskDestPtr += cellWidth;
			}
		}
	}
}


void FontBlitter::DrawLetter(PixelMemory * destPM, char c, int x, int y) {
	int offset = c - firstGlyphOffset;

	if (offset < 0) {
		return;
	}
	if (offset >= numGlyphs) {
		return;
	}

	if ((y >= destPM->pixelHeight) || (x >= destPM->pixelWidth) ||
		((y + cellHeight)<=0) || ((x + cellWidth) <= 0)) {
		return;
	}

	int drawHeight = cellHeight;
	int drawWidth = cellWidth;
	int startX = x;
	int startY = y;

	if (startY + drawHeight > destPM->pixelHeight) {
		drawHeight = destPM->pixelHeight - startY;
	}
	if (startX + drawWidth > destPM->pixelWidth) {
		drawWidth = destPM->pixelWidth - startX;
	}
	uint32_t * srcPtr = glyphArray[offset];
	uint32_t * srcMaskPtr = glyphArrayMask[offset];
	if (startY < 0) {
		int beginningLinesToRemove = -startY;
		startY = 0;
		srcPtr += (beginningLinesToRemove)*cellWidth;
		srcMaskPtr += (beginningLinesToRemove)*cellWidth;
		drawHeight -= beginningLinesToRemove;
	}
	if (startX < 0) {
		int beginningPixelsToRemove = -startX;
		startX = 0;
		srcPtr += beginningPixelsToRemove;
		srcMaskPtr += beginningPixelsToRemove;
		drawWidth -= beginningPixelsToRemove;
	}

	for (int row = 0; row < drawHeight; row++) {
		uint32_t * destPtr = destPM->dataPtr;
		destPtr += (startY + row)*destPM->pixelWidth + startX;
		uint32_t * tmpDest = destPtr;
		uint32_t * tmpSrc = srcPtr;
		uint32_t * tmpSrcMask = srcMaskPtr;
		for (int col = 0; col < drawWidth; col++) {
			*tmpDest &= *tmpSrcMask++;
			*tmpDest++ |= *tmpSrc++;
		}
		destPtr += destPM->pixelWidth;
		srcPtr += cellWidth;
		srcMaskPtr += cellWidth;
	}
#if 0
#if 1
	StretchDIBits(hdc,
		x, y, cellWidth, cellHeight,
		0, 0, cellWidth, cellHeight,
		glyphArrayMask[offset], &glyphBmpInfo, DIB_RGB_COLORS, SRCAND);
	StretchDIBits(hdc,
		x, y, cellWidth, cellHeight,
		0, 0, cellWidth, cellHeight,
		glyphArray[offset], &glyphBmpInfo, DIB_RGB_COLORS, SRCPAINT);
#else
	StretchDIBits(hdc,
		x, y, cellWidth, cellHeight,
		0, 0, cellWidth, cellHeight,
		glyphArray[offset], &glyphBmpInfo, DIB_RGB_COLORS, SRCCOPY);
#endif
#endif

}

void FontBlitter::DrawString(PixelMemory * destPM, char * str, int x, int y) {
	size_t numDigits = strlen(str);
	for( int i = 0; i < numDigits; i++) {
		DrawLetter(destPM, str[i], x + i*cellWidth, y);
	}
}

void FontBlitter::DrawNumber(PixelMemory * destPM, int N, int x, int y) {
	char buf[100];
	sprintf_s(buf, "%d", N);
	DrawString(destPM, buf, x, y);
}

void FontBlitter::GetBoundingBox(PixelMemory * destPM, char * str, int x, int y, BoundingBox * bb) {
	size_t numDigits = strlen(str);
	bb->top = x;
	bb->left = y;
	bb->right = x + numDigits*cellWidth;
	bb->bottom = y + cellHeight;
}

