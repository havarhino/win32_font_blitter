#include "stdafx.h"
#include "DrawOntoDC.h"
#include <math.h>

int locX = 0;

DrawOntoDC::DrawOntoDC(HWND hWindow) {
	hWnd = hWindow;
	updateWindowDimensions();

	//ballSet = new BallSet(41, mainClientRect.right, mainClientRect.bottom);

	greenPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
    regionBrush = CreateSolidBrush(RGB(100, 200, 255));

	HDC hdc = GetDC(hWnd);

	createDIB(hdc);

	ReleaseDC(hWnd, hdc);

}

DrawOntoDC::~DrawOntoDC(void) {
	// Now, destroy the memory DC
	SelectObject(h_dibDC, m_hOldDIBBitmap );
	DeleteDC(h_dibDC);
	DeleteObject(m_hDIBBitmap);

	//delete ballSet;
	DeleteObject(regionBrush);
	DeleteObject(greenPen);
}

void DrawOntoDC::createDIB(HDC hdc) {
	// Creating the DIB to draw in
	BITMAPINFO mybmi;

	DIBwidth = mainClientRect.right;
	DIBheight = mainClientRect.bottom;
	int bitCount = 32;
	DIBrowByteWidth = ((DIBwidth * (bitCount / 8) + 3) & -4);
	int totalBytes = DIBrowByteWidth * DIBheight;

	////// This is the BITMAPINFO structure values for the Green Ball
	mybmi.bmiHeader.biSize = sizeof(mybmi);
	mybmi.bmiHeader.biWidth = DIBwidth;
	mybmi.bmiHeader.biHeight = -DIBheight;
	mybmi.bmiHeader.biPlanes = 1;
	mybmi.bmiHeader.biBitCount = bitCount;
	mybmi.bmiHeader.biCompression = BI_RGB;
	mybmi.bmiHeader.biSizeImage = totalBytes;
	mybmi.bmiHeader.biXPelsPerMeter = 0;
	mybmi.bmiHeader.biYPelsPerMeter = 0;

	h_dibDC = CreateCompatibleDC(hdc);
	m_hDIBBitmap = CreateDIBSection(hdc, &mybmi, DIB_RGB_COLORS, (VOID **)&m_pBitmapBits, NULL, 0);
	m_hOldDIBBitmap = (HBITMAP)SelectObject(h_dibDC, m_hDIBBitmap);

}

void DrawOntoDC::drawDIB(HDC hdc) {

	int W = mainClientRect.right;
	int H = mainClientRect.bottom;
	// Clear the background of the DIB
	memset(m_pBitmapBits, 0, DIBrowByteWidth * DIBheight);

#if 0
	for (int x = 0; x < DIBwidth; x++) {
		uint32_t * ptr = (uint32_t *)m_pBitmapBits + (lineCounter % DIBheight) * W + x;
		*ptr = GREEN;
	}
#endif


	// Draw things directly into bitmap using pointer to memory

#if 0
	///////   DRAW THE HOLLOW RED CIRCLE
	lineCounter++;

	double R = 50.0;
	double deltaAngle = 1.0 / R;
	for (double i = 0.0; i < 2 * PI; i += deltaAngle) {
		int x = (int)(lineCounter + cos(i)*R) % W;
		int y = (int)(lineCounter + sin(i)*R) % H;
		if ((x >= 0) && (x < W) && (y >= 0) && (y < H)) {
			*getPixelAddress(x, y) = RED;
		}
	}
#endif

	//ballSet->draw(h_dibDC, W, H);

	///////////////   DRAW REGULAR WINDOWS DRAWING THINGS

	// Now, draw any Windows drawing objects
	//    Saving the original object
	HGDIOBJ original = SelectObject(hdc, greenPen);

#if 0
	MoveToEx(h_dibDC, 0, 0, NULL);
	LineTo(h_dibDC, W-1,0);
	LineTo(h_dibDC, W-1,H-1);
	LineTo(h_dibDC, 0,H-1);
	LineTo(h_dibDC, 0,0);
#endif

	wchar_t szBuff[128];
    swprintf(szBuff, L"Here is a window Pointer: %p", hWnd);

	RECT r;
	r.left = locX;
	r.top - 150;
	r.right = r.left + 100;
	r.bottom = 250;

	FillRect(h_dibDC, &r, regionBrush);
	locX++;
	if (locX > 500) {
		locX = 0;
	}

	TextOutW(h_dibDC, 20, 50, szBuff, wcslen(szBuff) );

#if 0
	MoveToEx(h_dibDC, xLoc % DIBwidth, 20, NULL);
	LineTo(h_dibDC, xLoc % DIBwidth + 200, 122);
	xLoc++;
#endif

	// Now, copy the mem DC into the screen DC
#if 0
	int x = (int)(sin((double)lineCounter/20.0)*103.0 + 103.0);
	int y = (int)(cos((double)lineCounter/20.0)*103.0 + 103.0);
	BitBlt(hdc, x, y, DIBwidth, DIBheight, h_dibDC, 0, 0, SRCCOPY );
#else
	BitBlt(hdc, 0, 0, DIBwidth, DIBheight, h_dibDC, 0, 0, SRCCOPY );
#endif

	SelectObject(hdc, original);
}

uint32_t * DrawOntoDC::getPixelAddress(int x, int y) {
	if (x < 0) {
		x = 0;
	}
	if (y < 0) {
		y = 0;
	}
	if (x >= mainClientRect.right) {
		x = mainClientRect.right - 1;
	}
	if (y >= mainClientRect.bottom) {
		y = mainClientRect.bottom - 1;
	}
	uint32_t *pixel = (uint32_t *)m_pBitmapBits + mainClientRect.right * y + x;
	return pixel;
}

void DrawOntoDC::draw(void) {

	HDC hdc = GetDC(hWnd);

	try {

		drawDIB(hdc);

		ReleaseDC(hWnd, hdc);
	}
	catch(...) {
		// The try/catch ensures that we will always call EndPaint() even if an exception occurs.
		ReleaseDC(hWnd, hdc);
		throw;
	}

}

void DrawOntoDC::updateWindowDimensions(void) {
	GetWindowRect(hWnd, &mainWindowRect);
	GetClientRect(hWnd, &mainClientRect);
}

void DrawOntoDC::invalidate(void) {
	InvalidateRect(hWnd, &mainClientRect, FALSE);
}

void DrawOntoDC::log(LPCWSTR str) {
	OutputDebugStringW(str);
}
void DrawOntoDC::log_1d(LPCWSTR str, int d1) {
	swprintf_s(dbgStr, str, d1);
	OutputDebugStringW(str);
}
void DrawOntoDC::log_2d(LPCWSTR str, int d1, int d2) {
	swprintf_s(dbgStr, str, d1, d2);
	OutputDebugStringW(dbgStr);
}