// win32_font_blitter.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "win32_font_blitter.h"
#include "resource.h"
#include "FontBlitter.h"
#include "DrawOntoDC.h"
#include "FrameCounter.h"
#include <mutex>


#define MAX_LOADSTRING 100

#undef DRAW_FROM_WM_PAINT

FrameCounter * frameCounter = new FrameCounter();
wchar_t dbgStr[512] = L"";
RECT mainClientRect;
HDC h_dibDC = 0;
HBITMAP m_hDIBBitmap = 0;
HBITMAP m_hOldDIBBitmap = 0;
DrawMemory dm;

void log(LPCWSTR str) {
	OutputDebugStringW(str);
}
void log_1d(LPCWSTR str, int d1) {
	swprintf_s(dbgStr, str, d1);
	OutputDebugStringW(str);
}
void log_2d(LPCWSTR str, int d1, int d2) {
	swprintf_s(dbgStr, str, d1, d2);
	OutputDebugStringW(dbgStr);
}

FontBlitter ** fontBlitterArray = 0;
DrawOntoDC * drawOntoDC = NULL;
std::mutex drawOntoDC_Mutex;

// This function will allocate memory and fill that memory with the bitmap raw bitmap data.
// The caller of this method is responsible for free'ing the returned pointer, if it is non-null.
PixelMemory ToPixels(HBITMAP BitmapHandle) {
	BITMAP Bmp = { 0 };
	BYTE * Pixels = 0;
	BITMAPINFO info = { 0 };

	HDC DC = CreateCompatibleDC(NULL);
	//memset(&info, 0, sizeof(BITMAPINFO)); //not necessary really..
	HBITMAP OldBitmap = (HBITMAP)SelectObject(DC, BitmapHandle);
	GetObject(BitmapHandle, sizeof(Bmp), &Bmp);

	int H = Bmp.bmHeight;
	info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	info.bmiHeader.biWidth = Bmp.bmWidth;
	info.bmiHeader.biHeight = -H;
	info.bmiHeader.biPlanes = 1;
	info.bmiHeader.biBitCount = Bmp.bmBitsPixel;
	info.bmiHeader.biCompression = BI_RGB;
	info.bmiHeader.biSizeImage = ((Bmp.bmWidth * Bmp.bmBitsPixel + 31) / 32) * 4 * H;
	
	Pixels = (BYTE *)malloc(info.bmiHeader.biSizeImage * sizeof(BYTE));
	GetDIBits(DC, BitmapHandle, 0, H, Pixels, &info, DIB_RGB_COLORS);

	SelectObject(DC, OldBitmap);
	DeleteDC(DC);

	PixelMemory pm = { 0 };
	pm.bitsPerPixel = Bmp.bmBitsPixel;
	pm.dataPtr = (uint32_t *)Pixels;
	pm.pixelWidth = Bmp.bmWidth;
	pm.pixelHeight = H;
	pm.planes = 1;
	return pm;
}


FontBlitter ** createFontBlitters(HINSTANCE hInstance) {
   fontBlitterArray = new FontBlitter *[3];

   PixelMemory pm;

   HBITMAP myBmp = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BITMAP1));
   pm = ToPixels(myBmp);
   fontBlitterArray[0] = new FontBlitter(&pm, '!', true, 24, 24);

   myBmp = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BITMAP2));
   pm = ToPixels(myBmp);
   fontBlitterArray[1] = new FontBlitter(&pm, 0, false, 20, 20);

   myBmp = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BITMAP3));
   pm = ToPixels(myBmp);
   fontBlitterArray[2] = new FontBlitter(&pm, 0, true, 32, 32);

   return fontBlitterArray;
}

DrawMemory CreateDrawMemory(HWND hWnd) {
	DrawMemory dm = { 0 };
	HDC hdc = GetDC(hWnd);

	GetClientRect(hWnd, &mainClientRect);

	// Creating the DIB to draw in
	BITMAPINFO mybmi;

	int bitCount = 32;
	dm.pixelWidth  = mainClientRect.right;
	dm.pixelHeight = mainClientRect.bottom;
	dm.bitsPerPixel = bitCount;
	dm.planes = 1;
	int DIBrowByteWidth = ((dm.pixelWidth * (bitCount / 8) + 3) & -4);
	int totalBytes = DIBrowByteWidth * dm.pixelHeight;

	////// This is the BITMAPINFO structure values for the Green Ball
	mybmi.bmiHeader.biSize = sizeof(mybmi);
	mybmi.bmiHeader.biWidth = dm.pixelWidth;
	mybmi.bmiHeader.biHeight = -dm.pixelHeight;
	mybmi.bmiHeader.biPlanes = 1;
	mybmi.bmiHeader.biBitCount = bitCount;
	mybmi.bmiHeader.biCompression = BI_RGB;
	mybmi.bmiHeader.biSizeImage = totalBytes;
	mybmi.bmiHeader.biXPelsPerMeter = 0;
	mybmi.bmiHeader.biYPelsPerMeter = 0;

	h_dibDC = CreateCompatibleDC(hdc);
	m_hDIBBitmap = CreateDIBSection(hdc, &mybmi, DIB_RGB_COLORS, (VOID **)&(dm.dataPtr), NULL, 0);
	m_hOldDIBBitmap = (HBITMAP)SelectObject(h_dibDC, m_hDIBBitmap);

	ReleaseDC(hWnd, hdc);

	return dm;
}

void FreeDrawMemory() {
	SelectObject(h_dibDC, m_hOldDIBBitmap );
	DeleteDC(h_dibDC);
	DeleteObject(m_hDIBBitmap);
}

/**************** Standard Windows Function *************/
// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WIN32_FONT_BLITTER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WIN32_FONT_BLITTER));

	MSG msg = {0};

	while (msg.message != WM_QUIT)
	{
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			// Translate and dispatch message
			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		// Do update, rendering and all the real game loop stuff
		drawOntoDC_Mutex.lock();
		if (drawOntoDC != 0) {
			frameCounter->nextFrame();
			drawOntoDC->draw();
			HDC hdc = GetDC(msg.hwnd);
			BitBlt(hdc, 0, 0, dm.pixelWidth, dm.pixelHeight, h_dibDC, 0, 0, SRCCOPY);
			ReleaseDC(msg.hwnd, hdc);
		}
		drawOntoDC_Mutex.unlock();
	}

	delete frameCounter; frameCounter = NULL;

	drawOntoDC_Mutex.lock();
	delete drawOntoDC; drawOntoDC = NULL;
	FreeDrawMemory();
	drawOntoDC_Mutex.unlock();

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

	// The CS_OWNDC makes sure the same DC is returned each time GetDC is called.  This allows
	// The local memory frame buffer to always be attached to the same DC.
    wcex.style          = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    //wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WIN32_FONT_BLITTER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WIN32_FONT_BLITTER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   fontBlitterArray = createFontBlitters(hInstance);

   drawOntoDC_Mutex.lock();
   dm = CreateDrawMemory(hWnd);
   drawOntoDC = new DrawOntoDC(&dm, fontBlitterArray);
   drawOntoDC_Mutex.unlock();

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;

	case WM_ERASEBKGND:
		return 1;

    case WM_SIZE:
        {
			drawOntoDC_Mutex.lock();
			delete drawOntoDC; drawOntoDC = 0;
			FreeDrawMemory();

			dm = CreateDrawMemory(hWnd);
			drawOntoDC = new DrawOntoDC(&dm, fontBlitterArray);

			if (drawOntoDC != 0) {
				frameCounter->nextFrame();
				drawOntoDC->draw();
				HDC hdc = GetDC(hWnd);
				BitBlt(hdc, 0, 0, dm.pixelWidth, dm.pixelHeight, h_dibDC, 0, 0, SRCCOPY);
				ReleaseDC(hWnd, hdc);
			}
			drawOntoDC_Mutex.unlock();
		}
        break;

    case WM_MOVE:
        {
		}
        break;

    case WM_PAINT:
        {
			// You need this if you want the client area redrawn as the user resizes the window
			// However, this is not the main place where all the drawing occurs.  See above in the
			// message loop (PeekMessage followed by redraw).
			if (drawOntoDC != 0) {
				frameCounter->nextFrame();
				drawOntoDC->draw();
				HDC hdc = GetDC(hWnd);
				BitBlt(hdc, 0, 0, dm.pixelWidth, dm.pixelHeight, h_dibDC, 0, 0, SRCCOPY);
				ReleaseDC(hWnd, hdc);
			}

			// NEED THIS!!!!!!
			// See https://blogs.msdn.microsoft.com/oldnewthing/20141203-00/?p=43483
			//		"Suppose your window procedure doesn't paint when it gets a WM_PAINT message. What happens?
			//
			//		It depends on how you don't paint.
			//
			//		If you have an explicit handler for the WM_PAINT message that does nothing but return without 
			//		painting, then the window manager will turn around and put a new WM_PAINT message in your 
			//		queue. "And try harder this time." Remember that the rules for the WM_PAINT message are that 
			//		the window manager will generate a WM_PAINT message for any window that has a dirty region.
			//		If you fail to remove the dirty region in your WM_PAINT message handler, well, then the rules 
			//		state that you get another WM_PAINT message. (The most common way of clearing the dirty region 
			//		is to call Begin­Paint, but there are other less common ways, like Validate­Rect or 
			//		Redraw­Window with the RDW_VALIDATE flag.)"

			// If you don't have the BeginPaint/EndPaint (or other mechanism to clear dirty bit, like ValidateRect() ),
			// the WM_PAINT message won't be cleared.
#if 0
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			// TODO: Add any drawing code that uses hdc here...
			EndPaint(hWnd, &ps); 
#else
			ValidateRect(hWnd, 0);
#endif

			swprintf_s(dbgStr, L"WM_PAINT: frameRate=%d\n", frameCounter->getFrameRate());
			OutputDebugStringW(dbgStr);
		}
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
