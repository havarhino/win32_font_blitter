#include "stdafx.h"
#include "FontBlitter.h"

FontBlitter::FontBlitter(HBITMAP inBitmap) {

	hBitmap = inBitmap;

	loadImages();
}

// This function will allocate memory and fill that memory with the bitmap raw bitmap data.
// The caller of this method is responsible for free'ing the returned pointer, if it is non-null.
BYTE * FontBlitter::ToPixels(HBITMAP BitmapHandle, BITMAPINFO & info ) {
	BITMAP Bmp = { 0 };
	BYTE * Pixels = 0;

	HDC DC = CreateCompatibleDC(NULL);
	memset(&info, 0, sizeof(BITMAPINFO)); //not necessary really..
	HBITMAP OldBitmap = (HBITMAP)SelectObject(DC, BitmapHandle);
	GetObject(BitmapHandle, sizeof(Bmp), &Bmp);

	info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	info.bmiHeader.biWidth = Bmp.bmWidth;
	info.bmiHeader.biHeight = Bmp.bmHeight;
	info.bmiHeader.biPlanes = 1;
	info.bmiHeader.biBitCount = Bmp.bmBitsPixel;
	info.bmiHeader.biCompression = BI_RGB;
	info.bmiHeader.biSizeImage = ((Bmp.bmWidth * Bmp.bmBitsPixel + 31) / 32) * 4 * Bmp.bmHeight;
	
	Pixels = (BYTE *)malloc(info.bmiHeader.biSizeImage * sizeof(BYTE));
	GetDIBits(DC, BitmapHandle, 0, Bmp.bmHeight, Pixels, &info, DIB_RGB_COLORS);
	SelectObject(DC, OldBitmap);

	DeleteDC(DC);
	return Pixels;
}

int * getFactors(int size, int & numberOfFactors) {
	numberOfFactors = 0;
	bool whoopsie = false;
	int maxFactor = size / 4;
	for(int i = 2; i <= maxFactor; i++) {
		if ( (size % i) == 0) {
			numberOfFactors++;
		}
	}
	if (numberOfFactors == 0) {
		throw "Yikes!!!   getFactors found no even multiples";
	}

	int * factors = (int *)malloc(numberOfFactors * sizeof(int));

	if (factors == 0) {
		throw "Yikes!!!   getFactors could not allocate array";
	}

	int index = 0;
	for(int i = 2; i <= maxFactor; i++) {
		if ( (size % i) == 0) {
			factors[index++] = i;
		}
	}

	return factors;

}

void computeRowColumnTotals(BYTE * pixels, int w, int h, int bytesPerPixel, int lineWidth, int ** pColTotals, int ** pRowTotals) {
	int * colTotals = (int *)malloc(w * sizeof(int));
	int * rowTotals = (int *)malloc(h * sizeof(int));

	*pColTotals = colTotals;
	*pRowTotals = rowTotals;

	for (int i = 0; i < w; i++) {
		colTotals[i] = 0;
	}
	for (int i = 0; i < h; i++) {
		rowTotals[i] = 0;
	}

	// The actual formula for converting R/G/B to intensity is
	//    I = 0.2989 * R + 0.5870 * G + 0.114 * B
	// But, floating point multiplication is pretty slow, and we don't need
	// super accurate intensity calculation to look for the most likely cell boundary spacing,
	// We could use 2 different formulae:
	//     A) Faster, but less accurate:
	//    I ~=  0.25 * R  +  0.625 * G     +  0.125*B
	//              - 2 parts red, 5 parts green, 1 part blue
	//          (  2*R    +  (4*G + G)     +  B) / 8;
	//          ( (R<<1)  +  ((G<<2) + G)  +  B) / >>3; 
	//     B)  This uses one more additions and one more shift, so not that much slower, but more accurate:
	//              - 5 parts red, 9 parts green, 2 part blue, which is basically
	//    I ~=  0.3125 * R  + 0.5625 * G  + 0.125 * B;
	//    I ~=  (     5*R       +      9*G     +   2*B ) / 16;
	//    i ~=  ( ((R<<2) + R)  + ((G<<3) + G) + (B<<1)) >> 4;

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			BYTE intensity;

			if (bytesPerPixel == 1) {
				BYTE * pixelPtr = &(pixels[lineWidth*y + x]);
				intensity = *pixelPtr;

			}
			else if (bytesPerPixel == 3) {
				BYTE * pixelPtr = &(pixels[lineWidth*y + x*bytesPerPixel]);
				int B = (int)(*pixelPtr++);
				int G = (int)(*pixelPtr++);
				int R = (int)(*pixelPtr++);

				intensity = (BYTE)((((R << 2) + R) + ((G << 3) + G) + (B << 1)) >> 4);
			}
			else {
				BYTE * pixelPtr = &(pixels[lineWidth*y + x*bytesPerPixel]);
				int B = (int)(*pixelPtr++);
				int G = (int)(*pixelPtr++);
				int R = (int)(*pixelPtr++);
				int a = (int)(*pixelPtr++);

				intensity = (BYTE)((((R << 2) + R) + ((G << 3) + G) + (B << 1)) >> 4);
			}

			colTotals[x] += (255 - intensity);
			rowTotals[y] += (255 - intensity);
		}
	}
}

int findMostLikelyPeriod(int maxPosition, int * totals) {
	long bestPeriodTotal = INT_MAX;
	int bestPeriod = -1;
	int factorSize;

	// Find all factors of the width to find possible cell width sizes
	int * factorArray = getFactors(maxPosition, factorSize);

	for (int i = 0; i < factorSize; i++) {
		int period = factorArray[i];
		long freqTotal = 0;
		int counter = 0;
		for (int p = 0; p < maxPosition; p += period) {
			freqTotal += (long)totals[p];
			counter++;
		}
		if (counter > 0) {
			freqTotal /= (long)counter;
		}
		else {
			freqTotal = INT_MAX;
			throw "Yikes!! We should never get counter == 0";
		}
		if (freqTotal < bestPeriodTotal) {
			bestPeriodTotal = freqTotal;
			bestPeriod = period;
		}
	}
	delete(factorArray);

	return bestPeriod;
}

void computeGridSize(BYTE * pixels, BITMAPINFO & info, int &gridWidth, int &gridHeight) {
	int w = info.bmiHeader.biWidth;
	int h = info.bmiHeader.biHeight;
	h = (h < 0) ? -h : h;
	int bytesPerPixel = info.bmiHeader.biBitCount / 8;
	int lineWidth = ((w * info.bmiHeader.biBitCount + 31) / 32) * 4;
	int * rowTotals = 0;
	int * colTotals = 0;

	if( (bytesPerPixel != 1) && (bytesPerPixel != 3) && (bytesPerPixel != 4) ) {
		throw "Yikes!!!   Unsuported bytes per pixel (not 1, 3 or 4)";
	}

	computeRowColumnTotals(pixels, w, h, bytesPerPixel, lineWidth, &colTotals, &rowTotals);

	gridWidth = findMostLikelyPeriod(w, colTotals);
	gridHeight = findMostLikelyPeriod(h, rowTotals);

	free( colTotals );
	free( rowTotals );
}

void FontBlitter::loadImages() {
	if (hBitmap != 0) {
		bitmapDataPtr = ToPixels(hBitmap, fontBitmapInfo);
		computeGridSize(bitmapDataPtr, fontBitmapInfo, cellWidth, cellHeight);
	}
}
