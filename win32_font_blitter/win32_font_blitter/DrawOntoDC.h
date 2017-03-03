#pragma once
#include <Windows.h>
#include <wchar.h>
#include <stdint.h>
#include "FontBlitter.h"


class DrawOntoDC {

public:
	DrawOntoDC(HWND hWindow, FontBlitter ** inFontBlitterArray);
	~DrawOntoDC(void);

	void draw(void);
	void updateWindowDimensions(void);
	void invalidate(void);

private:
	HWND hWnd = NULL;
	RECT mainWindowRect;
	RECT mainClientRect;
    wchar_t dbgStr[512] = L"";
    HBRUSH regionBrush = NULL;
	int xLoc = 0;
	BITMAPINFO ballBmpInfo = {0};
	uint32_t * ballArray;
	uint32_t * ballArrayMask;
	int ballDiameter;
	FontBlitter * fontBlitter;
	FontBlitter ** fontBlitterArray;


	// DIB Bitmap things
	int lineCounter = 0;
	BYTE* m_pBitmapBits = NULL;
	HDC h_dibDC = NULL;
	HBITMAP m_hOldDIBBitmap = NULL;
	HBITMAP m_hDIBBitmap = NULL;
	int DIBwidth, DIBheight, DIBrowByteWidth;

	HPEN greenPen = NULL;

	uint32_t * getPixelAddress(int x, int y);
	void createDIB(HDC hdc);
	void drawDIB(HDC hdc);
	void log(LPCWSTR str);
	void log_1d(LPCWSTR str, int d1);
	void log_2d(LPCWSTR str, int d1, int d2);
};
