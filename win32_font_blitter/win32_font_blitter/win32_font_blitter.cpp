// win32_font_blitter.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "win32_font_blitter.h"
#include "resource.h"
#include "FontBlitter.h"
#include "DrawOntoDC.h"
#include "FrameCounter.h"


#define MAX_LOADSTRING 100

#undef DRAW_FROM_WM_PAINT

FrameCounter * frameCounter = new FrameCounter();
wchar_t dbgStr[512] = L"";

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

FontBlitter ** fontBlitterArray = 0;
DrawOntoDC * drawOntoDC = NULL;

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
		frameCounter->nextFrame();
		drawOntoDC->draw();

	}

	delete drawOntoDC; drawOntoDC = NULL;
	delete frameCounter; frameCounter = NULL;

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

   fontBlitterArray = new FontBlitter *[3];

   HBITMAP myBmp = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BITMAP1));
   fontBlitterArray[0] = new FontBlitter(myBmp, '!', true, 24, 24);

   myBmp = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BITMAP2));
   fontBlitterArray[1] = new FontBlitter(myBmp, 0, false, 20, 20);

   myBmp = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BITMAP3));
   fontBlitterArray[2] = new FontBlitter(myBmp, 0, true, 32, 32);

   drawOntoDC = new DrawOntoDC(hWnd, fontBlitterArray);

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
			drawOntoDC->updateWindowDimensions();
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
			frameCounter->nextFrame();
			drawOntoDC->draw();

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
