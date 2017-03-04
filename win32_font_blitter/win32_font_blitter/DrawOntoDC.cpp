#include "DrawOntoDC.h"
#include <math.h>
#include <stdlib.h>

int locX = 0;

DrawOntoDC::DrawOntoDC(DrawMemory * inDrawMemory, FontBlitter ** inFontBlitterArray) {
	fontBlitterArray = inFontBlitterArray;
	fontBlitter = inFontBlitterArray[1];

	dm = *inDrawMemory;

	updateWindowDimensions();

	ballDiameter = 258;

	ballArray = new uint32_t[ballDiameter*ballDiameter];
	ballArrayMask = new uint32_t[ballDiameter*ballDiameter];
	
	double ballRadius = ballDiameter / 2.0;
	for (int y = 0; y < ballDiameter; y++) {
		for (int x = 0; x < ballDiameter; x++) {
			double cx = (double)x - ballRadius;
			double cy = (double)y - ballRadius;
			if (cx*cx + cy*cy > ballRadius * ballRadius) {
				ballArray[y*ballDiameter + x] = 0x00000000;
				ballArrayMask[y*ballDiameter + x] = 0x00FFFFFF;
			}
			else {
				ballArray[y*ballDiameter + x] = 0x0000FF00;
				ballArrayMask[y*ballDiameter + x] = 0x00000000;
			}
		}
	}


}

DrawOntoDC::~DrawOntoDC(void) {
	delete ballArray;
	delete ballArrayMask;
}

void drawBitmap(DrawMemory * dm, int destX, int destY, uint32_t * srcPtr, int srcW, int srcH) {
	if ((destY >= dm->pixelHeight) || (destX >= dm->pixelWidth) ||
		(destY + srcH <= 0) || (destX + srcW <= 0) ) {
		return;
	}

	if (destY + srcH > dm->pixelHeight) {
		srcH = dm->pixelHeight - destY;
	}
	if (destX + srcW > dm->pixelWidth) {
		srcW = dm->pixelWidth - destX;
	}

	int startY = destY;
	int startX = destX;
	int drawWidth = srcW;
	int drawHeight = srcH;
	uint32_t * startSrc = srcPtr;
	if (startY < 0) {
		int beginningLinesToRemove = -startY;
		startY = 0;
		startSrc += (beginningLinesToRemove)*srcW;
		drawHeight -= beginningLinesToRemove;
	}
	if (startX < 0) {
		int beginningPixelsToRemove = -startX;
		startX = 0;
		startSrc += beginningPixelsToRemove;
		drawWidth -= beginningPixelsToRemove;
	}

	uint32_t * destPtr = dm->dataPtr + (startY * dm->pixelWidth) + startX;
	for (int y = 0; y < drawHeight; y++) {
		uint32_t * tmpSrc = startSrc;
		uint32_t * tmpDest = destPtr;
		for (int x = 0; x < drawWidth; x++) {
			*tmpDest++ = *tmpSrc++;
		}
		startSrc += srcW;
		destPtr += dm->pixelWidth;
	}
}

#define PI (3.1415926535)

void DrawOntoDC::drawDIB() {

	// Clear the background of the DIB
	memset(dm.dataPtr, 0, dm.pixelWidth * dm.pixelHeight * 4);

	// Draw things directly into bitmap using pointer to memory

	///////   DRAW THE HOLLOW RED CIRCLE
	lineCounter++;

	double R = 50.0;
	double deltaAngle = 1.0 / R;
	for (double i = 0.0; i < 2 * PI; i += deltaAngle) {
		int x = (int)(lineCounter + cos(i)*R) % dm.pixelWidth;
		int y = (int)(lineCounter + sin(i)*R) % dm.pixelHeight;
		if ((x >= 0) && (x < dm.pixelWidth) && (y >= 0) && (y < dm.pixelHeight)) {
			*getPixelAddress(x, y) = 0x00FF0000;
		}
	}

	//ballSet->draw(h_dibDC, W, H);
	int x = (int)(sin((double)lineCounter/80.0)*270.0 + 203.0);
	int y = (int)(cos((double)lineCounter/80.0)*240.0 + 203.0);
	drawBitmap(&dm, x, y, ballArray, ballDiameter, ballDiameter);

#if 0
	x = (int)(sin((double)(lineCounter+40)/80.0)*550.0 + 203.0);
	y = (int)(cos((double)(lineCounter+40)/80.0)*550.0 + 203.0);
	drawBitmap(&dm, x, y, ballArray, ballDiameter, ballDiameter);
#endif


	PixelMemory pm;
	pm.bitsPerPixel = dm.bitsPerPixel;
	pm.dataPtr = dm.dataPtr;
	pm.pixelWidth = dm.pixelWidth;
	pm.pixelHeight = dm.pixelHeight;
	pm.planes = dm.planes;

	fontBlitter->DrawNumber(&pm, lineCounter, 10, 35);
	fontBlitter->DrawString(&pm, "ABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*", 10, 60);
	fontBlitter->DrawString(&pm, "0123456789 !@#$%^&*(){}[];:'\",.<>/?`~", 10, 85);

	BoundingBox bb;
	fontBlitter->GetBoundingBox(&pm, "O", 0, 0, &bb);
	fontBlitter->DrawLetter(&pm, 'O', -5, 10);
	fontBlitter->DrawLetter(&pm, 'O', 10, -bb.bottom/2);
	fontBlitter->DrawLetter(&pm, 'O', -200, 10);
	fontBlitter->DrawLetter(&pm, 'O', 10, -100);
	fontBlitter->DrawLetter(&pm, 'O', -200, -100);
	fontBlitter->DrawLetter(&pm, 'O', pm.pixelWidth - bb.right, pm.pixelHeight-bb.bottom/2);
	fontBlitter->DrawLetter(&pm, 'O', pm.pixelWidth - bb.right/4, pm.pixelHeight-bb.bottom);
	fontBlitter->DrawLetter(&pm, 'O', pm.pixelWidth - bb.right, pm.pixelHeight+bb.bottom/2);
	fontBlitter->DrawLetter(&pm, 'O', pm.pixelWidth + bb.right/4, pm.pixelHeight-bb.bottom);
	fontBlitter->DrawLetter(&pm, 'O', pm.pixelWidth + bb.right/4, pm.pixelHeight+bb.bottom);

	int wiggle = 14;
	int halfwiggle = wiggle / 2;
	for (y = 0; y < 18; y++) {
		for (x = 0; x < pm.pixelWidth/24 ; x++) {
			fontBlitterArray[rand()%3]->DrawLetter(&pm, '!' + rand() % ('Z' - '!' + 1),
				    10 + x * 24 + rand() % wiggle - halfwiggle,
				    120 + y * 24 + rand() % wiggle - halfwiggle);
		}
	}
	
	///////////////   DRAW REGULAR WINDOWS DRAWING THINGS

	// Now, draw any Windows drawing objects
	//    Saving the original object

	//wchar_t szBuff[128];
    //swprintf(szBuff, L"Here is a window Pointer: %p", hWnd);

	//TextOutW(h_dibDC, 20, 50, szBuff, wcslen(szBuff) );

	// Now, copy the mem DC into the screen DC

}

uint32_t * DrawOntoDC::getPixelAddress(int x, int y) {
	if (x < 0) {
		x = 0;
	}
	if (y < 0) {
		y = 0;
	}
	if (x >= dm.pixelWidth) {
		x = dm.pixelWidth - 1;
	}
	if (y >= dm.pixelHeight) {
		y = dm.pixelHeight - 1;
	}
	uint32_t *pixel = (uint32_t *)dm.dataPtr + dm.pixelWidth * y + x;
	return pixel;
}

void DrawOntoDC::draw(void) {
	drawDIB();
}

void DrawOntoDC::updateWindowDimensions(void) {
}

