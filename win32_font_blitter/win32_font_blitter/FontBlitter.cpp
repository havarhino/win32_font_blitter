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



	numGlyphs = rows * columns + firstGlyphOffset;

	glyphArray = new uint32_t*[numGlyphs];
	glyphArrayMask = new uint32_t*[numGlyphs];
	leftPadding = new unsigned char *[numGlyphs];
	rightPadding = new unsigned char *[numGlyphs];

	// First, create blank glyphs for data missing from first part of bitmap
	for (int i = 0; i < firstGlyphOffset; i++) {
		uint32_t * glyphDestPtr = new uint32_t[cellWidth * cellHeight];
		uint32_t * glyphMaskDestPtr = new uint32_t[cellWidth * cellHeight];
		glyphArray[i] = glyphDestPtr;
		glyphArrayMask[i] = glyphMaskDestPtr;
		leftPadding[i] = new unsigned char[cellHeight];
		rightPadding[i] = new unsigned char[cellHeight];

		for (int y = 0; y < cellHeight; y++) {
			uint32_t * destPtr = glyphDestPtr;
			uint32_t * destMaskPtr = glyphMaskDestPtr;
			for (int x = 0; x < cellWidth; x++) {
				uint32_t v = 0;
				*destPtr++ = v;
				*destMaskPtr++ = 0x00FFFFFF;
			}
			leftPadding[i][y] = 255;
			rightPadding[i][y] = 255;
			glyphDestPtr += cellWidth;
			glyphMaskDestPtr += cellWidth;
		}
	}

	uint32_t* wordBitmapDataPtr = pm.dataPtr;
	for (int r = 0; r < rows; r++) {
		for (int c = 0; c < columns; c++) {
			uint32_t * glyphDestPtr = new uint32_t[cellWidth * cellHeight];
			uint32_t * glyphMaskDestPtr = new uint32_t[cellWidth * cellHeight];
			glyphArray[r*columns + c + firstGlyphOffset] = glyphDestPtr;
			glyphArrayMask[r*columns + c + firstGlyphOffset] = glyphMaskDestPtr;
			leftPadding[r*columns + c + firstGlyphOffset] = new unsigned char[cellHeight];
			rightPadding[r*columns + c + firstGlyphOffset] = new unsigned char[cellHeight];

			uint32_t * glyphSrcPtr = (uint32_t*)(&wordBitmapDataPtr[(r * cellHeight) * pm.pixelWidth + (c*cellWidth)]);
			for (int y = 0; y < cellHeight; y++) {
				uint32_t * srcPtr = glyphSrcPtr;
				uint32_t * destPtr = glyphDestPtr;
				uint32_t * destMaskPtr = glyphMaskDestPtr;
				bool foundFirstPixel = false;
				int emptyLeft = 0;
				int emptyRight = 0;
				for (int x = 0; x < cellWidth; x++) {
					uint32_t v = (*srcPtr++);
					if( invertGlyphColor ) {
						v = ~v;
					}
					*destPtr++ = v;
					bool isEmpty = (v & 0x000F0F0F) == 0;
					*destMaskPtr++ =  isEmpty ? 0x00FFFFFF : 0x00000000;
					if (isEmpty) {
						if (foundFirstPixel) {
							emptyRight++;
						} else {
							emptyLeft++;
						}
					}
					else {
						foundFirstPixel = true;
						emptyRight = 0;
					}
				}
				// Since we are using unsigned char, we reserve 255 to mean entire
				// row is empty.  Therefore, if we have more left or right empty
				// padding than 254, we snapp it to 254 (Most font glyphs are not that big anyway... right??).
				// TODO:  Change from unsigned char to unsigned short if glyph widths of > 255 are expected.
				if (emptyLeft == cellWidth) {
					emptyLeft = 255;
					emptyRight = 255;
				}
				else {
					if (emptyLeft > 254) {
						emptyLeft = 254;
					}
					if (emptyRight > 254) {
						emptyRight = 254;
					}
				}
				leftPadding[r*columns + c + firstGlyphOffset][y] = emptyLeft;
				rightPadding[r*columns + c + firstGlyphOffset][y] = emptyRight;
				glyphSrcPtr += pm.pixelWidth;
				glyphDestPtr += cellWidth;
				glyphMaskDestPtr += cellWidth;
			}
		}
	}
}


void FontBlitter::DrawLetter(PixelMemory * destPM, char c, int x, int y) {
	int offset = c;

	if (offset < 0) {
return;
	}
	if (offset >= numGlyphs) {
		return;
	}

	if ((y >= destPM->pixelHeight) || (x >= destPM->pixelWidth) ||
		((y + cellHeight) <= 0) || ((x + cellWidth) <= 0)) {
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
	for (int i = 0; i < numDigits; i++) {
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
	bb->right = (int)(x + numDigits*cellWidth);
	bb->bottom = (int)(y + cellHeight);
}

int FontBlitter::DrawProportionalLetter(PixelMemory * destPM, char c, int x, int y, unsigned char ** hLineOffsets, int extraSpacing) {

	int newX = x;

	// For each line, compute how much white space padding between data
	unsigned char * previousPadding = 0;
	if (hLineOffsets != 0) {
		previousPadding = *hLineOffsets;
	}
	bool foundOverlappingData = false;
	bool foundThisData = false;
	bool foundPreviousData = false;
	int offset = c;

	int smallestSeparation = 255;
	for (int line = 0; line < cellHeight; line++) {
		int prevPad = 0;

		if (previousPadding != 0) {
			prevPad = previousPadding[line];
		}

		if (prevPad < 255) {
			foundPreviousData = true;
		}

		int thisPad = leftPadding[offset][line];
		if (thisPad < 255) {
			foundThisData = true;
		}

		if( ( prevPad != 255) && ( thisPad != 255) ) {
			foundOverlappingData = true;
			int lineSeparation = (int)(prevPad + thisPad);
			if (lineSeparation < smallestSeparation) {
				smallestSeparation = lineSeparation;
			}
		}
	}

    // Now, deal wit the space (or empty) glyph
	if (!foundOverlappingData) {
		smallestSeparation = 2 * cellWidth / 3;
	}

	newX = x - smallestSeparation + extraSpacing;
	if (newX > x) {
		newX = x;
	}
	
	DrawLetter(destPM, c, newX, y);

	*hLineOffsets = rightPadding[offset];
	return newX + cellWidth;
}

void FontBlitter::DrawProportionalString(PixelMemory * destPM, char * str, int x, int y, int extraSpacing) {
	size_t numDigits = strlen(str);
	unsigned char * pLineOffsets = 0;
	for( int i = 0; i < numDigits; i++) {
		x = DrawProportionalLetter(destPM, str[i], x, y, &pLineOffsets, extraSpacing);
	}
}

void FontBlitter::DrawProportionalNumber(PixelMemory * destPM, int N, int x, int y, int extraSpacing) {
	char buf[100];
	sprintf_s(buf, "%d", N);
	DrawProportionalString(destPM, buf, x, y, extraSpacing);
}

