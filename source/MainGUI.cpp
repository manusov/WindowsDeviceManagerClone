/* ----------------------------------------------------------------------------------------
Class for application main window, include system tree view, main menu, tool bar (up),
status bar (down), vertical and horizontal scroll bars, "About" child dialogue window,
plug-ins child windows connections.
Special thanks to:
https://learn.microsoft.com/en-us/windows/win32/controls/create-scroll-bars
https://learn.microsoft.com/ru-ru/windows/win32/controls/scroll-bars
https://learn.microsoft.com/ru-ru/windows/win32/api/winuser/nf-winuser-createwindowexa
https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowa
https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexa

Creates GUI window with tree visualization.
Upgraded version with state variables and recursive levels.
S = Show region, can be resized by user actions (resize GUI window).
P = Physical region, depends on video hardware parameters (screen X,Y resolution).
V = Virtual region, depends on tree size.
Scroll means shift V relative S.
S maximum X,Y sizes defined by P.

	   |
   ----|----------
   |   | S |     |
   ----|----     |
   |   |         |
   |   |      P  |
   ----|---------|
	   |V

ROADMAP 1: SUPPORT BIG SCROLLABLE TREES WITHOUT MEMORY OVERFLOW:
-----------------------------------------------------------------
1.1) WM_CREATE handler: create GUI window content, show initial state.
1.2) WM_PAINT handler: visualize GUI window content, copy raster info with dual bufferring.
1.3) WM_SIZE handler: resize GUI window, update scroll control variables.
1.4) WM_LBUTTONUP handler: open and close nodes.
1.5) WM_MOUSEMOVE handler: mark nodes depend on mouse cursor near node.
1.6) WM_HSCROLL handler: horizontal scroll.
1.7) WM_VSCROLL handler: vertical scroll.
1.8) WM_MOUSEWHEEL handler: vertical scroll, alternative method.
1.9) WM_KEYDOWN handler: mark, unmark, move pointer, open and close by keys.

ROADMAP 2: RECURSIVE OR STACKED TREE LEVELS, REMOVE TREE LEVEL COUNT LIMITS:
-----------------------------------------------------------------------------
2.1) Method for verify results of next steps, data source = child class TreeControllerExt.
2.2) Recursive or stacked mouse move selection.
2.3) Recursive or stacked open-close nodes, means recursive or stacked draw layers.
2.4) Correct TAB selection for extra layers.
2.5) Verify all handlers in the Window Procedure.

OLD COMMENTS: Lines, selected by:
//
...
//
is first objects for refactoring and optimization.
Current used visualization method is slow, too many time required,
because full blanks and redraw.
Note TreeView.cpp use offset change for scroll, not redraw!
---------------------------------------------------------------------------------------- */

#include "MainGUI.h"

MainGUI::MainGUI(HINSTANCE hInst, int cmdShow, ManageResources* pMr)
{
	// some initialization not required because make static.
	invalidationRequest = false;
	pManageResources = pMr;
	hCursorAbout = NULL;
	hFontAbout = NULL;
	hIconAbout = NULL;

	CHAR buildName[BUILD_NAME_MAX];
	_snprintf_s(buildName, 
		BUILD_NAME_MAX, _TRUNCATE, "%s %s%s%s %s",
		ABOUT_TEXT_1, ABOUT_TEXT_2_1, ABOUT_TEXT_2_2, ABOUT_TEXT_2_3, ABOUT_TEXT_2_4);

	char szClassName[] = "KWndClass";
	wc.cbSize = sizeof(wc);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInst;
	wc.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APP));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MAIN_MENU);
	wc.lpszClassName = szClassName;
	wc.hIconSm = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APP));

	hWnd = NULL;
	if (RegisterClassEx(&wc))
	{
		hWnd = CreateWindow(szClassName, buildName, WS_OVERLAPPEDWINDOW,
			MAIN_WINDOW_BASE_X, MAIN_WINDOW_BASE_Y, MAIN_WINDOW_SIZE_X, MAIN_WINDOW_SIZE_Y,
			NULL, (HMENU)NULL, hInst, NULL);
		if (hWnd)
		{
			errorName = NULL;
			ShowWindow(hWnd, cmdShow);
		}
		else
		{
			errorName = errorName2;
		}
	}
	else
	{
		errorName = errorName1;
	}
}
MainGUI::~MainGUI()
{
	// Class destructor - reserved functionality.
}
void MainGUI::SetAndInitModel(ManageResources* pMr)
{
	pManageResources = pMr;
}
void MainGUI::MainWait()
{
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}
HWND MainGUI::GetHWnd()
{
	return hWnd;
}
LPCSTR MainGUI::GetErrorName()
{
	return errorName;
}
HWND MainGUI::hWnd;
WNDCLASSEX MainGUI::wc;
ManageResources* MainGUI::pManageResources;
BOOL MainGUI::invalidationRequest;
LPCSTR MainGUI::errorName;
LPCSTR MainGUI::errorName1 = "Cannot register application main window class.";
LPCSTR MainGUI::errorName2 = "Cannot create application main window.";
// Dialogue procedure.
LRESULT CALLBACK MainGUI::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// These variables are required by BeginPaint, EndPaint, BitBlt. 
	PAINTSTRUCT ps;              // Temporary storage for paint info  : ps.hdc = window, can be resized by user.
	static HDC hdcScreen;        // DC for entire screen              : hdcScreen = full screen.
	static HDC hdcScreenCompat;  // memory DC for screen              : hdcScreenCompat = bufferred copy of full screen.
	static HBITMAP hbmpCompat;   // bitmap handle to old DC 
	static BITMAP bmp;           // bitmap data structure 
	static BOOL fBlt;            // TRUE if BitBlt occurred 
	static BOOL fScroll;         // TRUE if scrolling occurred 
	static BOOL fSize;           // TRUE if fBlt & WM_SIZE 
	// This variable used for horizontal and vertical scrolling both.
	SCROLLINFO si;               // Temporary storage for scroll info
	// These variables are required for horizontal scrolling. 
	static int xMinScroll;       // minimum horizontal scroll value 
	static int xCurrentScroll;   // current horizontal scroll value 
	static int xMaxScroll;       // maximum horizontal scroll value 
	// These variables are required for vertical scrolling. 
	static int yMinScroll;       // minimum vertical scroll value 
	static int yCurrentScroll;   // current vertical scroll value 
	static int yMaxScroll;       // maximum vertical scroll value 
	// This variable are required for adjust scrolling by visualized tree size.
	static RECT treeDimension;
	// This variables are required for nodes selection by TAB key, use background color.
	static BOOL fTab;            // TRUE if selection mode activated
	static PTREENODE openNode;   // This node opened if SPACE, Gray+, Gray- pressed at selection mode
	// This variables are required for text font and background brush
	static HFONT hFont;          // Font handle
	static HBRUSH bgndBrush;     // Brush handle for background
	// This variables are required for restore context
	static HGDIOBJ backupBmp;
	static HBRUSH backupBrush;
	// Application tool bar and status bar handles.
	static HWND hWndToolBar;
	static HWND hWndStatusBar;
	// Application tool bar and status bar Y-sizes for viewer settings adjust.
	static int toolY;
	static int statusY;
	// Application main menu handle.
	static HMENU hMenu;
	// Information type selector
	static int selector;
	// Horizontal and vertical scroll bars handles and geometry, added at TreeView2
	// instead acroll bars as window properties.
	static HWND hScrollBarH;
	static HWND hScrollBarV;
	static int sbHeight;
	static int sbWidth;

	// Window callback procedure entry point.
	switch (uMsg)
	{
	case WM_CREATE:
	{
		// Pre-clear window area invalidation request.
		ClearInvalidation();
		// Fill window with background color, important for prevent blinking when window resize.
		bgndBrush = CreateSolidBrush(BACKGROUND_BRUSH);
		SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)bgndBrush);
		// Create font.
		hFont = CreateFont(16, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS,
			CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FIXED_PITCH, TEXT("System monospace"));
		// Create a normal DC and a memory DC for the entire 
		// screen. The normal DC provides a snapshot of the 
		// screen contents. The memory DC keeps a copy of this 
		// snapshot in the associated bitmap. 
		hdcScreen = CreateDC("DISPLAY", (PCTSTR)NULL, (PCTSTR)NULL, (CONST DEVMODE*) NULL);
		hdcScreenCompat = CreateCompatibleDC(hdcScreen);
		// Retrieve the metrics for the bitmap associated with the 
		// regular device context. 
		bmp.bmBitsPixel =
			(BYTE)GetDeviceCaps(hdcScreen, BITSPIXEL);
		bmp.bmPlanes = (BYTE)GetDeviceCaps(hdcScreen, PLANES);
		bmp.bmWidth = GetDeviceCaps(hdcScreen, HORZRES);
		bmp.bmHeight = GetDeviceCaps(hdcScreen, VERTRES);
		// The width must be byte-aligned. 
		bmp.bmWidthBytes = ((bmp.bmWidth + 15) & ~15) / 8;
		// Create a bitmap for the compatible DC. 
		hbmpCompat = CreateBitmap(bmp.bmWidth, bmp.bmHeight,
			bmp.bmPlanes, bmp.bmBitsPixel, (CONST VOID*) NULL);
		// Select the bitmap for the compatible DC.
		backupBmp = SelectObject(hdcScreenCompat, hbmpCompat);
		// Select the brush for the compatible DC.
		backupBrush = (HBRUSH)SelectObject(hdcScreenCompat, bgndBrush);
		// Create application tool bar and status bar.
		hWndToolBar = InitToolBar(hWnd);
		hWndStatusBar = CreateStatusWindow(WS_CHILD | WS_VISIBLE, " Ready...", hWnd, ID_STATUSBAR);
		// Adjust tool bar and status bar position and size.
		RECT r;
		if (hWndToolBar)
		{
			SendMessage(hWndToolBar, TB_AUTOSIZE, 0, 0);
			GetWindowRect(hWndToolBar, &r);
			toolY = r.bottom - r.top;        // Note static variable toolY = 0 if tool bar not initialized.
		}
		if (hWndStatusBar)
		{
			SendMessage(hWndStatusBar, WM_SIZE, wParam, lParam);
			GetWindowRect(hWndStatusBar, &r);
			statusY = r.bottom - r.top;      // Note static variable statusY = 0 if status bar not initialized.
		}
		// Initialize the flags. 
		fBlt = FALSE;
		fScroll = FALSE;
		fSize = FALSE;
		// Initialize the horizontal scrolling variables. 
		xMinScroll = 0;
		xCurrentScroll = 0;
		xMaxScroll = 0;
		// Initialize the vertical scrolling variables. 
		yMinScroll = 0;
		yCurrentScroll = 0;
		yMaxScroll = 0;
		fSize = TRUE;
		// Initialize pointer for open items by SPACE, Gray+, Gray- keys.
		openNode = pManageResources->GetTrees()[selector];
		// This for compatibility with MSDN example.
		fBlt = TRUE;
		// Get application main menu descriptor and default setup menu items.
		hMenu = GetMenu(hWnd);
		if (hMenu)
		{
			CheckMenuRadioItem(GetSubMenu(hMenu, 1), IDM_VIEW_DEVICES,
				IDM_VIEW_RESOURCES, IDM_VIEW_DEVICES, MF_BYCOMMAND);
		}
		// Default setup tool bar items.
		if (hWndToolBar)
		{
			SendMessage(hWndToolBar, TB_CHECKBUTTON, ID_TB_DEVICES, TRUE);
		}
		// Create horizontal and vertical scroll bars, added at TreeView2
		// instead acroll bars as window properties.
		sbHeight = SCROLLBAR_HEIGHT;
		sbWidth = SCROLLBAR_WIDTH;
		// Note scroll bars sizes corrected dynamically at WM_SIZE message handler.
		hScrollBarH = CreateAHorizontalScrollBar(hWnd, sbHeight, statusY, sbWidth);
		hScrollBarV = CreateAVerticalScrollBar(hWnd, sbWidth, toolY, statusY);
	}
	break;

	case WM_PAINT:
	{
		// Open paint context.
		BeginPaint(hWnd, &ps);
		// Paint bufferred copy.
		// Logic for prevent status bar blinking and correct revisual selections when mouse cursor moved.
		RECT r;
		GetClientRect(hWnd, &r);
		int dy1 = r.bottom - r.top;
		int dy2 = ps.rcPaint.bottom;
		if (dy1 == dy2)
		{   // This branch for repaint all window
			BitBlt(ps.hdc, 0, toolY, ps.rcPaint.right - sbWidth, ps.rcPaint.bottom - toolY - statusY - sbHeight,
				hdcScreenCompat, 0, 0, SRCCOPY);
		}
		else
		{   // This branch for repaint part of window, for example selections area near mouse cursor move.
			BitBlt(ps.hdc, 0, toolY, ps.rcPaint.right, ps.rcPaint.bottom,
				hdcScreenCompat, 0, 0, SRCCOPY);

		}
		// Close paint context.
		EndPaint(hWnd, &ps);
	}
	break;

	case WM_SIZE:
	{
		ClearInvalidation();
		int xNewSize = GET_X_LPARAM(lParam) - sbWidth;
		int yNewSize = GET_Y_LPARAM(lParam) - toolY - statusY - sbHeight;
		// Construction from original MSDN source, inspect it.
		if (fBlt)
			fSize = TRUE;
		// The horizontal scrolling range is defined by 
		// (tree_width) - (client_width). The current horizontal 
		// scroll value remains within the horizontal scrolling range.
		HelperAdjustScrollX(hScrollBarH, si, treeDimension, xNewSize, xMaxScroll, xMinScroll, xCurrentScroll);
		// The vertical scrolling range is defined by 
		// (tree_height) - (client_height). The current vertical 
		// scroll value remains within the vertical scrolling range. 
		HelperAdjustScrollY(hScrollBarV, si, treeDimension, yNewSize, yMaxScroll, yMinScroll, yCurrentScroll);
		BitBlt(hdcScreenCompat, 0, 0, bmp.bmWidth, bmp.bmHeight, NULL, 0, 0, PATCOPY);  // This for blank background
		int dy = 0;
		treeDimension = HelperRecursiveDrawTree(pManageResources->GetTrees()[selector], pManageResources->GetBase(), fTab, hdcScreenCompat, hFont,
			xCurrentScroll, yCurrentScroll, dy);
		// Adjust tool bar and status bar position and size when main window size changed.
		RECT r;
		if (hWndToolBar)
		{
			SendMessage(hWndToolBar, TB_AUTOSIZE, 0, 0);
			GetWindowRect(hWndToolBar, &r);
			toolY = r.bottom - r.top;
		}
		if (hWndStatusBar)
		{
			SendMessage(hWndStatusBar, WM_SIZE, wParam, lParam);
			GetWindowRect(hWndStatusBar, &r);
			statusY = r.bottom - r.top;
		}
		if (GetClientRect(hWnd, &r))
		{
			if (hScrollBarH)
			{
				SetWindowPos(hScrollBarH, NULL,
					r.left, r.bottom - sbHeight - statusY, r.right - sbWidth, sbHeight,
					SWP_SHOWWINDOW);
			}
			if (hScrollBarV)
			{
				SetWindowPos(hScrollBarV, NULL,
					r.right - sbWidth, r.top + toolY, sbWidth, r.bottom - toolY - statusY,
					SWP_SHOWWINDOW);
			}
		}
		MakeInvalidation(hWnd);
	}
	break;

	case WM_LBUTTONUP:
	{
		ClearInvalidation();
		int mouseX = GET_X_LPARAM(lParam);
		int mouseY = GET_Y_LPARAM(lParam);
		PTREENODE p = pManageResources->GetTrees()[selector];
		POINT b = pManageResources->GetBase();

		RECT t = HelperRecursiveMouseClick(p, b, hWnd, hdcScreenCompat, fSize,
			openNode, fTab, bmp, hFont,
			mouseX, mouseY - toolY, xCurrentScroll, yCurrentScroll, selector);
		if (t.right && t.bottom)
		{
			treeDimension = t;
		}

		RECT r = { 0,0,0,0 };
		if (GetClientRect(hWnd, &r))
		{
			int backXcurrentScroll = xCurrentScroll;
			int backYcurrentScroll = yCurrentScroll;

			int xNewSize = r.right - r.left - sbWidth;
			int yNewSize = r.bottom - r.top - toolY - statusY - sbHeight;
			// Construction from original MSDN source, inspect it.
			if (fBlt)
				fSize = TRUE;
			// The horizontal scrolling range is defined by 
			// (tree_width) - (client_width). The current horizontal 
			// scroll value remains within the horizontal scrolling range.
			HelperAdjustScrollX(hScrollBarH, si, treeDimension, xNewSize, xMaxScroll, xMinScroll, xCurrentScroll);
			// The vertical scrolling range is defined by 
			// (tree_height) - (client_height). The current vertical 
			// scroll value remains within the vertical scrolling range. 
			HelperAdjustScrollY(hScrollBarV, si, treeDimension, yNewSize, yMaxScroll, yMinScroll, yCurrentScroll);

			if ((backXcurrentScroll != xCurrentScroll) || (backYcurrentScroll != yCurrentScroll))
			{
				BitBlt(hdcScreenCompat, 0, 0, bmp.bmWidth, bmp.bmHeight, NULL, 0, 0, PATCOPY);  // This for blank background
				int dy = 0;
				treeDimension = HelperRecursiveDrawTree(pManageResources->GetTrees()[selector], pManageResources->GetBase(), fTab, hdcScreenCompat, hFont,
					xCurrentScroll, yCurrentScroll, dy);
			}
		}

		// Restore Open/Close icon lighting by mouse cursor after node clicked.
		HelperRecursiveMouseMove(p, hWnd, hdcScreenCompat, fSize, TRUE,
			mouseX, mouseY, xCurrentScroll, yCurrentScroll, toolY);
		MakeInvalidation(hWnd);
	}
	break;

	case WM_MOUSEMOVE:
	{
		ClearInvalidation();
		HelperRecursiveMouseMove(pManageResources->GetTrees()[selector], hWnd, hdcScreenCompat, fSize, FALSE,
			GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), xCurrentScroll, yCurrentScroll, toolY);
		MakeInvalidation(hWnd);
	}
	break;

	case WM_HSCROLL:
	{
		ClearInvalidation();
		int addX = 0;
		int scrollType = LOWORD(wParam);
		int value = HIWORD(wParam);
		switch (scrollType)
		{
		case SB_PAGEUP:
			addX = -50;  // User clicked the scroll bar shaft left the scroll box. 
			break;
		case SB_PAGEDOWN:
			addX = 50;  // User clicked the scroll bar shaft right the scroll box. 
			break;
		case SB_LINEUP:
			addX = -3;  // User clicked the left arrow. 
			break;
		case SB_LINEDOWN:
			addX = 3;   // User clicked the right arrow. 
			break;
		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
			addX = value - xCurrentScroll;  // User dragged the scroll box.
			break;
		default:
			addX = 0;
			break;
		}
		if (addX != 0)
		{
			HelperMakeScrollX(hScrollBarH, si, xMaxScroll, xCurrentScroll, fScroll, addX);
			BitBlt(hdcScreenCompat, 0, 0, bmp.bmWidth, bmp.bmHeight, NULL, 0, 0, PATCOPY);  // This for blank background
			int dy = 0;
			HelperRecursiveDrawTree(pManageResources->GetTrees()[selector], pManageResources->GetBase(), fTab, hdcScreenCompat, hFont,
				xCurrentScroll, yCurrentScroll, dy);
		}
		MakeInvalidation(hWnd);
	}
	break;

	case WM_VSCROLL:
	{
		ClearInvalidation();
		int addY = 0;
		int scrollType = LOWORD(wParam);
		int value = HIWORD(wParam);
		switch (scrollType)
		{
		case SB_PAGEUP:
			addY = -50;  // User clicked the scroll bar shaft above the scroll box. 
			break;
		case SB_PAGEDOWN:
			addY = 50;   // User clicked the scroll bar shaft below the scroll box. 
			break;
		case SB_LINEUP:
			addY = -3;   // User clicked the top arrow. 
			break;
		case SB_LINEDOWN:
			addY = 3;    // User clicked the bottom arrow. 
			break;
		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
			addY = value - yCurrentScroll;  // User dragged the scroll box.
			break;
		default:
			addY = 0;
			break;
		}
		if (addY != 0)
		{
			HelperMakeScrollY(hScrollBarV, si, yMaxScroll, yCurrentScroll, fScroll, addY);
			BitBlt(hdcScreenCompat, 0, 0, bmp.bmWidth, bmp.bmHeight, NULL, 0, 0, PATCOPY);  // This for blank background
			int dy = 0;
			HelperRecursiveDrawTree(pManageResources->GetTrees()[selector], pManageResources->GetBase(), fTab, hdcScreenCompat, hFont,
				xCurrentScroll, yCurrentScroll, dy);
		}
		MakeInvalidation(hWnd);
	}
	break;

	case WM_MOUSEWHEEL:
	{
		ClearInvalidation();
		int addY = -(short)HIWORD(wParam) / WHEEL_DELTA * 30;
		if (addY != 0)
		{
			HelperMakeScrollY(hScrollBarV, si, yMaxScroll, yCurrentScroll, fScroll, addY);
			BitBlt(hdcScreenCompat, 0, 0, bmp.bmWidth, bmp.bmHeight, NULL, 0, 0, PATCOPY);  // This for blank background
			int dy = 0;
			HelperRecursiveDrawTree(pManageResources->GetTrees()[selector], pManageResources->GetBase(), fTab, hdcScreenCompat, hFont,
				xCurrentScroll, yCurrentScroll, dy);
		}
		MakeInvalidation(hWnd);
	}
	break;

	case WM_KEYDOWN:
	{
		ClearInvalidation();
		int addX = 0;
		int addY = 0;
		int dy = 0;

		switch (wParam)
		{
		case VK_LEFT:
			addX = -3;
			break;
		case VK_RIGHT:
			addX = 3;
			break;
		case VK_UP:
			if (fTab)
			{
				openNode = HelperRecursiveMarkNode(FALSE, selector);
				BitBlt(hdcScreenCompat, 0, 0, bmp.bmWidth, bmp.bmHeight, NULL, 0, 0, PATCOPY);  // This for blank background
				dy = 0;
				HelperRecursiveDrawTree(pManageResources->GetTrees()[selector], pManageResources->GetBase(), fTab, hdcScreenCompat, hFont,
					xCurrentScroll, yCurrentScroll, dy);
				fSize = TRUE;
				SetInvalidation();  // InvalidateRect(hWnd, NULL, false);
			}
			else
			{
				addY = -3;
			}
			break;
		case VK_DOWN:
			if (fTab)
			{
				openNode = HelperRecursiveMarkNode(TRUE, selector);
				BitBlt(hdcScreenCompat, 0, 0, bmp.bmWidth, bmp.bmHeight, NULL, 0, 0, PATCOPY);  // This for blank background
				dy = 0;
				HelperRecursiveDrawTree(pManageResources->GetTrees()[selector], pManageResources->GetBase(), fTab, hdcScreenCompat, hFont,
					xCurrentScroll, yCurrentScroll, dy);
				fSize = TRUE;
				SetInvalidation();  // InvalidateRect(hWnd, NULL, false);
			}
			else
			{
				addY = 3;
			}
			break;
		case VK_PRIOR:
			addY = -50;
			break;
		case VK_NEXT:
			addY = 50;
			break;
		case VK_HOME:
			addY = -yCurrentScroll;
			break;
		case VK_END:
			addY = yMaxScroll;
			break;
		case VK_TAB:
			fTab = ~fTab;
			BitBlt(hdcScreenCompat, 0, 0, bmp.bmWidth, bmp.bmHeight, NULL, 0, 0, PATCOPY);  // This for blank background
			dy = 0;
			HelperRecursiveDrawTree(pManageResources->GetTrees()[selector], pManageResources->GetBase(), fTab, hdcScreenCompat, hFont,
				xCurrentScroll, yCurrentScroll, dy);
			fSize = TRUE;
			SetInvalidation();  // InvalidateRect(hWnd, NULL, false);
			break;

		case VK_ADD:
		case VK_SUBTRACT:
		case VK_SPACE:
			if (fTab && openNode && (openNode->openable))
			{
				openNode->opened = ~(openNode->opened);
				HelperMarkedClosedChilds(openNode, openNode, fTab);
				BitBlt(hdcScreenCompat, 0, 0, bmp.bmWidth, bmp.bmHeight, NULL, 0, 0, PATCOPY);  // This for blank background
				dy = 0;
				treeDimension = HelperRecursiveDrawTree(pManageResources->GetTrees()[selector], pManageResources->GetBase(), fTab, hdcScreenCompat, hFont,
					xCurrentScroll, yCurrentScroll, dy);
				fSize = TRUE;
				SetInvalidation();  // InvalidateRect(hWnd, NULL, false);

				RECT r = { 0,0,0,0 };
				if (GetClientRect(hWnd, &r))
				{
					int backXcurrentScroll = xCurrentScroll;
					int backYcurrentScroll = yCurrentScroll;

					int xNewSize = r.right - r.left;
					int yNewSize = r.bottom - r.top;
					if (fBlt)
						fSize = TRUE;
					HelperAdjustScrollX(hScrollBarH, si, treeDimension, xNewSize, xMaxScroll, xMinScroll, xCurrentScroll);
					HelperAdjustScrollY(hScrollBarV, si, treeDimension, yNewSize, yMaxScroll, yMinScroll, yCurrentScroll);

					if ((backXcurrentScroll != xCurrentScroll) || (backYcurrentScroll != yCurrentScroll))
					{
						BitBlt(hdcScreenCompat, 0, 0, bmp.bmWidth, bmp.bmHeight, NULL, 0, 0, PATCOPY);  // This for blank background
						int dy = 0;
						treeDimension = HelperRecursiveDrawTree(pManageResources->GetTrees()[selector], pManageResources->GetBase(), fTab, hdcScreenCompat, hFont,
							xCurrentScroll, yCurrentScroll, dy);
					}
				}
			}
			break;
		default:
			break;
		}
		if (addX != 0)
		{
			HelperMakeScrollX(hScrollBarH, si, xMaxScroll, xCurrentScroll, fScroll, addX);
			BitBlt(hdcScreenCompat, 0, 0, bmp.bmWidth, bmp.bmHeight, NULL, 0, 0, PATCOPY);  // This for blank background
			dy = 0;
			HelperRecursiveDrawTree(pManageResources->GetTrees()[selector], pManageResources->GetBase(), fTab, hdcScreenCompat, hFont,
				xCurrentScroll, yCurrentScroll, dy);
		}
		else if (addY != 0)
		{
			HelperMakeScrollY(hScrollBarV, si, yMaxScroll, yCurrentScroll, fScroll, addY);
			BitBlt(hdcScreenCompat, 0, 0, bmp.bmWidth, bmp.bmHeight, NULL, 0, 0, PATCOPY);  // This for blank background
			dy = 0;
			HelperRecursiveDrawTree(pManageResources->GetTrees()[selector], pManageResources->GetBase(), fTab, hdcScreenCompat, hFont,
				xCurrentScroll, yCurrentScroll, dy);
		}
		MakeInvalidation(hWnd);
	}
	break;

	case WM_COMMAND:
	{
		ClearInvalidation();
		RECT r;
		GetClientRect(hWnd, &r);
		int sx = r.right - r.left;
		int sy = r.bottom - r.top;
		DWORD sysx = (sy << 16) + sx;
		int dy = 0;
		switch (LOWORD(wParam))
		{
		case ID_TB_ABOUT:
		case IDM_HELP_ABOUT:
			// setTreeModel(pManageResources);
			DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ABOUT), hWnd, (DLGPROC)AboutDlgProc, 0);
			break;

		case ID_TB_EXIT:
		case IDM_FILE_EXIT:
			SendMessage(hWnd, WM_DESTROY, 0, 0);
			break;

		case ID_TB_DEVICES:
			selector = 0;
			treeDimension = HelperRecursiveDrawTree(pManageResources->GetTrees()[selector], pManageResources->GetBase(), fTab, hdcScreenCompat, hFont,
				xCurrentScroll, yCurrentScroll, dy);
			SendMessage(hWnd, WM_SIZE, SIZE_RESTORED, sysx);
			invalidationRequest = TRUE;
			if (hMenu)
			{
				CheckMenuRadioItem(GetSubMenu(hMenu, 1), IDM_VIEW_DEVICES,
					IDM_VIEW_RESOURCES, IDM_VIEW_DEVICES, MF_BYCOMMAND);
			}
			break;

		case ID_TB_RESOURCES:
			selector = 1;
			treeDimension = HelperRecursiveDrawTree(pManageResources->GetTrees()[selector], pManageResources->GetBase(), fTab, hdcScreenCompat, hFont,
				xCurrentScroll, yCurrentScroll, dy);
			SendMessage(hWnd, WM_SIZE, SIZE_RESTORED, sysx);
			invalidationRequest = TRUE;
			if (hMenu)
			{
				CheckMenuRadioItem(GetSubMenu(hMenu, 1), IDM_VIEW_DEVICES,
					IDM_VIEW_RESOURCES, IDM_VIEW_RESOURCES, MF_BYCOMMAND);
			}
			break;

		case IDM_VIEW_DEVICES:
			selector = 0;
			treeDimension = HelperRecursiveDrawTree(pManageResources->GetTrees()[selector], pManageResources->GetBase(), fTab, hdcScreenCompat, hFont,
				xCurrentScroll, yCurrentScroll, dy);
			SendMessage(hWnd, WM_SIZE, SIZE_RESTORED, sysx);
			invalidationRequest = TRUE;
			CheckMenuRadioItem(GetSubMenu(hMenu, 1), IDM_VIEW_DEVICES,
				IDM_VIEW_RESOURCES, LOWORD(wParam), MF_BYCOMMAND);
			if (hWndToolBar)
			{
				SendMessage(hWndToolBar, TB_CHECKBUTTON, ID_TB_DEVICES, TRUE);
				SendMessage(hWndToolBar, TB_CHECKBUTTON, ID_TB_RESOURCES, FALSE);
			}
			break;

		case IDM_VIEW_RESOURCES:
			selector = 1;
			treeDimension = HelperRecursiveDrawTree(pManageResources->GetTrees()[selector], pManageResources->GetBase(), fTab, hdcScreenCompat, hFont,
				xCurrentScroll, yCurrentScroll, dy);
			SendMessage(hWnd, WM_SIZE, SIZE_RESTORED, sysx);
			invalidationRequest = TRUE;
			CheckMenuRadioItem(GetSubMenu(hMenu, 1), IDM_VIEW_DEVICES,
				IDM_VIEW_RESOURCES, LOWORD(wParam), MF_BYCOMMAND);
			if (hWndToolBar)
			{
				SendMessage(hWndToolBar, TB_CHECKBUTTON, ID_TB_DEVICES, FALSE);
				SendMessage(hWndToolBar, TB_CHECKBUTTON, ID_TB_RESOURCES, TRUE);
			}
			break;

		default:
			break;
		}
		MakeInvalidation(hWnd);
	}
	break;

	case WM_NOTIFY:
	{
		LPTOOLTIPTEXT lpTTT = (LPTOOLTIPTEXT)lParam;
		if (lpTTT->hdr.code == TTN_NEEDTEXT)
		{
			lpTTT->hinst = GetModuleHandle(NULL);
			lpTTT->lpszText = MAKEINTRESOURCE(lpTTT->hdr.idFrom);
		}
	}
	break;

	case WM_DESTROY:
	{
		if (backupBrush) SelectObject(hdcScreenCompat, backupBrush);
		if (backupBmp) SelectObject(hdcScreenCompat, backupBmp);
		if (hbmpCompat) DeleteObject(hbmpCompat);
		if (hdcScreenCompat) DeleteDC(hdcScreenCompat);
		if (hdcScreen) DeleteDC(hdcScreen);
		if (hFont) DeleteObject(hFont);
		if (bgndBrush) DeleteObject(bgndBrush);
		PostQuitMessage(0);
	}
	break;

	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}
// Helpers.
// Helpers for mouse click and position detection
bool MainGUI::DetectMouseClick(int xPos, int yPos, PTREENODE p)
{
	return (xPos > p->clickArea.left) && (xPos < p->clickArea.right) && (yPos > p->clickArea.top) && (yPos < p->clickArea.bottom);
}
bool MainGUI::DetectMousePosition(int xPos, int yPos, PTREENODE p)
{
	return (xPos > (p->clickArea.left + 1)) && (xPos < (p->clickArea.right - 1)) && (yPos > (p->clickArea.top + 1)) && (yPos < (p->clickArea.bottom - 1));
}
// Helper for unmark items stay invisible after parent item close.
void MainGUI::HelperMarkedClosedChilds(PTREENODE pParent, PTREENODE& openNode, BOOL fTab)
{
	if (fTab && (pParent->openable) && (!pParent->opened))
	{
		PTREENODE p1 = pParent->childLink;
		PTREENODE pMarked = NULL;
		UINT n1 = pParent->childCount;
		if (p1)
		{
			for (UINT i = 0; i < n1; i++)
			{
				if (p1->marked)
				{
					pMarked = p1;
					break;
				}
				if ((p1->openable) && (!p1->opened))
				{
					PTREENODE p2 = p1->childLink;
					UINT n2 = p1->childCount;
					if (p2)
					{
						for (UINT j = 0; j < n2; j++)
						{
							if (p2->marked)
							{
								pMarked = p2;
								break;
							}
							p2++;
						}
					}
				}
				if (pMarked) break;
				p1++;
			}

			if (pMarked)
			{
				pMarked->marked = 0;  // Unmark child node because it now hide.
				pParent->marked = 1;  // Mark parent node instead hide child node.
				openNode = pParent;   // Change current selected node for keyboard operations.
			}
		}
	}
}
// This for early start X scroll bar show.
#define X_ADDEND 16
// Helper for node sequence of one layer.
// Returns layer array (xleft, ytop, xright, ybottom),
// This parameters better calculate during draw, because depend on font size,
// current active font settings actual during draw.
RECT MainGUI::HelperDrawNodeLayerSized(PTREENODE pNodeList, int nodeCount, int nodeBaseX, int nodeBaseY,
	int iconStepX, int iconStepY, int iconSizeX, int iconSizeY, BOOL fTab, HDC hDC, HFONT hFont)
{
	RECT layerDimension = { 0,0,0,0 };
	HFONT hOldFont = NULL;
	if (hFont) hOldFont = (HFONT)SelectObject(hDC, hFont);
	int oldBkMode = SetBkMode(hDC, TRANSPARENT);
	int tempX = 0;
	int skipY = 0;
	for (int i = 0; i < nodeCount; i++)
	{
		// Draw open/close icon, node icon and text string.
		HICON hIcon;
		if (pNodeList->openable)
		{
			pNodeList->opened ? hIcon = pNodeList->hOpenedIcon : hIcon = pNodeList->hClosedIcon;
			// Draw open-close icon
			DrawIconEx(hDC, nodeBaseX, nodeBaseY + iconStepY * i + iconStepY * skipY, hIcon, iconSizeX, iconSizeY, 0, NULL, DI_NORMAL | DI_COMPAT);
			tempX = nodeBaseX + iconStepX;
		}
		else
		{
			tempX = nodeBaseX;
		}
		hIcon = pNodeList->hNodeIcon;
		// Draw node icon
		DrawIconEx(hDC, tempX, nodeBaseY + iconStepY * i + iconStepY * skipY, hIcon, iconSizeX, iconSizeY, 0, NULL, DI_NORMAL | DI_COMPAT);
		int length = (int)strlen(pNodeList->szNodeName);

		if ((fTab) && (pNodeList->marked))
		{
			// Draw node text string, TAB selection ACTIVE and this node marked
			int oldBkMode = SetBkMode(hDC, OPAQUE);
			COLORREF oldBkColor = SetBkColor(hDC, SELECTED_BRUSH);
			TextOut(hDC, tempX + iconStepX, nodeBaseY + iconStepY * i + iconStepY * skipY, pNodeList->szNodeName, length);
			if (oldBkMode) SetBkMode(hDC, oldBkMode);
			if (oldBkColor != CLR_INVALID) SetBkColor(hDC, oldBkColor);
		}
		else
		{
			// Draw node text string, TAB selection mode NOT ACTIVE or this node not marked
			TextOut(hDC, tempX + iconStepX, nodeBaseY + iconStepY * i + iconStepY * skipY, pNodeList->szNodeName, length);
		}

		// Set coordinates for mouse clicks detections
		pNodeList->clickArea.left = nodeBaseX;
		pNodeList->clickArea.right = nodeBaseX + iconSizeX;
		pNodeList->clickArea.top = nodeBaseY + iconStepY * i + iconStepY * skipY;
		pNodeList->clickArea.bottom = nodeBaseY + iconStepY * i + iconStepY * skipY + iconSizeY;
		// Set coordinates for this node, later used by scroll parameters detection
		POINT base = pManageResources->GetBase();
		int tempSX = tempX + iconStepX + base.x + X_ADDEND;
		SIZE textSize = { 0,0 };
		if (GetTextExtentPoint32(hDC, pNodeList->szNodeName, length, &textSize))
		{
			tempSX += textSize.cx;
		}
		if (tempSX > layerDimension.right) { layerDimension.right = tempSX; }
		// Advance screen coordinates and list pointer.
		if (pNodeList->opened) skipY += pNodeList->childCount;
		pNodeList++;
	}
	if (oldBkMode) SetBkMode(hDC, oldBkMode);
	if (hOldFont) SelectObject(hDC, hOldFont);
	layerDimension.left = nodeBaseX;
	layerDimension.top = nodeBaseY;
	layerDimension.bottom = nodeBaseY + iconStepY * nodeCount + iconStepY * skipY;
	return layerDimension;
}
// Helper for adjust horizontal scrolling parameters.
// The horizontal scrolling range is defined by 
// (bitmap_width) - (client_width). The current horizontal 
// scroll value remains within the horizontal scrolling range.
void MainGUI::HelperAdjustScrollX(HWND hWnd, SCROLLINFO& scrollInfo, RECT& treeDimension,
	int xNewSize, int& xMaxScroll, int& xMinScroll, int& xCurrentScroll)
{
	int tempSize = treeDimension.right - treeDimension.left;     // added
	xMaxScroll = max(tempSize - xNewSize, 0);                    // max(bmp.bmWidth - xNewSize, 0);
	xCurrentScroll = min(xCurrentScroll, xMaxScroll);
	scrollInfo.cbSize = sizeof(SCROLLINFO);
	scrollInfo.fMask = SIF_RANGE | SIF_PAGE | SIF_POS | SIF_DISABLENOSCROLL;
	scrollInfo.nMin = xMinScroll;
	scrollInfo.nMax = tempSize;                                  // bmp.bmWidth;
	scrollInfo.nPage = xNewSize;
	scrollInfo.nPos = xCurrentScroll;
	SetScrollInfo(hWnd, SB_CTL, &scrollInfo, TRUE);
}
// Helper for adjust vertical scrolling parameters.
// The vertical scrolling range is defined by 
// (bitmap_height) - (client_height). The current vertical 
// scroll value remains within the vertical scrolling range. 
void MainGUI::HelperAdjustScrollY(HWND hWnd, SCROLLINFO& scrollInfo, RECT& treeDimension,
	int yNewSize, int& yMaxScroll, int& yMinScroll, int& yCurrentScroll)
{
	int tempSize = treeDimension.bottom - treeDimension.top;     // added
	yMaxScroll = max(tempSize - yNewSize, 0);                    // max(bmp.bmHeight - yNewSize, 0);
	yCurrentScroll = min(yCurrentScroll, yMaxScroll);
	scrollInfo.cbSize = sizeof(SCROLLINFO);
	scrollInfo.fMask = SIF_RANGE | SIF_PAGE | SIF_POS | SIF_DISABLENOSCROLL;
	scrollInfo.nMin = yMinScroll;
	scrollInfo.nMax = tempSize;                                  // bmp.bmHeight;
	scrollInfo.nPage = yNewSize;
	scrollInfo.nPos = yCurrentScroll;
	SetScrollInfo(hWnd, SB_CTL, &scrollInfo, TRUE);
}
// Helper for make horizontal scrolling by given signed offset.
void MainGUI::HelperMakeScrollX(HWND hWnd, SCROLLINFO& scrollInfo,
	int xMaxScroll, int& xCurrentScroll, BOOL& fScroll, int addX)
{
	int xNewPos = xCurrentScroll + addX;
	// New position must be between 0 and the screen width. 
	xNewPos = max(0, xNewPos);
	xNewPos = min(xMaxScroll, xNewPos);
	// If the current position does not change, do not scroll.
	if (xNewPos != xCurrentScroll)
	{
		// Set the scroll flag to TRUE. 
		fScroll = TRUE;
		// Update the current scroll position. 
		xCurrentScroll = xNewPos;
		// Update the scroll bar position.
		scrollInfo.cbSize = sizeof(SCROLLINFO);
		scrollInfo.fMask = SIF_POS | SIF_DISABLENOSCROLL;
		scrollInfo.nPos = xCurrentScroll;
		SetScrollInfo(hWnd, SB_CTL, &scrollInfo, TRUE);
		// Request for all window repaint
		SetInvalidation();  // InvalidateRect(hWnd, NULL, false);
	}
}
// Helper for make vertical scrolling by given signed offset.
void MainGUI::HelperMakeScrollY(HWND hWnd, SCROLLINFO& scrollInfo,
	int yMaxScroll, int& yCurrentScroll, BOOL& fScroll, int addY)
{
	int yNewPos = yCurrentScroll + addY;
	// New position must be between 0 and the screen height. 
	yNewPos = max(0, yNewPos);
	yNewPos = min(yMaxScroll, yNewPos);
	// If the current position does not change, do not scroll.
	if (yNewPos != yCurrentScroll)
	{
		// Set the scroll flag to TRUE. 
		fScroll = TRUE;
		// Update the current scroll position. 
		yCurrentScroll = yNewPos;
		// Update the scroll bar position.
		scrollInfo.cbSize = sizeof(SCROLLINFO);
		scrollInfo.fMask = SIF_POS | SIF_DISABLENOSCROLL;
		scrollInfo.nPos = yCurrentScroll;
		SetScrollInfo(hWnd, SB_CTL, &scrollInfo, TRUE);
		// Request for all window repaint
		SetInvalidation();  // InvalidateRect(hWnd, NULL, false);
	}
}
// This part for support recursive tree levels and eliminate level count limits.
void MainGUI::HelperRecursiveMouseMove(PTREENODE p, HWND hWnd, HDC hdcScreenCompat, BOOL& fSize, BOOL forceUpdate,
	int mouseX, int mouseY, int xCurrentScroll, int yCurrentScroll, int offsetY)
{

	// mouseY = screen coordinate, contain tool bar size.
	// mouseY - offsetY = coordinate in the compatible context.
	bool currentMouse = (DetectMousePosition(mouseX, mouseY - offsetY, p) && (p->openable));
	if (((currentMouse != p->prevMouse) || forceUpdate) && p->openable)
	{   // Operation for node, addressed by caller.
		int index;
		if (p->opened)
		{
			currentMouse ? index = ID_OPENED_LIGHT : index = ID_OPENED;
		}
		else
		{
			currentMouse ? index = ID_CLOSED_LIGHT : index = ID_CLOSED;
		}
		HICON hIcon = pManageResources->GetIconHandleByIndex(index);

		RECT a;
		DrawIconEx(hdcScreenCompat, p->clickArea.left, p->clickArea.top, hIcon,
			X_ICON_SIZE, Y_ICON_SIZE, 0, NULL, DI_NORMAL | DI_COMPAT);
		fSize = TRUE;
		a.left = p->clickArea.left;
		a.top = p->clickArea.top + offsetY;
		a.right = p->clickArea.right;
		a.bottom = p->clickArea.bottom + offsetY;
		InvalidateRect(hWnd, &a, false);
		p->prevMouse = currentMouse;
	}
	if (p->opened)
	{
		int n = p->childCount;
		p = p->childLink;
		if (p && (n > 0))
		{   // Recursive operation for childs nodes of node, addressed by caller.
			for (int i = 0; i < n; i++)
			{
				HelperRecursiveMouseMove(p, hWnd, hdcScreenCompat, fSize, forceUpdate,
					mouseX, mouseY, xCurrentScroll, yCurrentScroll, offsetY);
				p++;
			}
		}
	}
}
RECT MainGUI::HelperRecursiveMouseClick(PTREENODE p, POINT b, HWND hWnd, HDC hdcScreenCompat, BOOL& fSize,
	PTREENODE& openNode, BOOL fTab, BITMAP bmp, HFONT hFont,
	int mouseX, int mouseY, int xCurrentScroll, int yCurrentScroll, int sel)
{
	RECT rDimension = { 0,0,0,0 };
	RECT rTemp = { 0,0,0,0 };
	if (DetectMouseClick(mouseX, mouseY, p) && (p->openable))
	{
		p->opened = ~(p->opened);
		HelperMarkedClosedChilds(p, openNode, fTab);
		BitBlt(hdcScreenCompat, 0, 0, bmp.bmWidth, bmp.bmHeight, NULL, 0, 0, PATCOPY);  // This for blank background
		int dy = 0;
		rDimension = HelperRecursiveDrawTree(pManageResources->GetTrees()[sel], pManageResources->GetBase(),
			fTab, hdcScreenCompat, hFont, xCurrentScroll, yCurrentScroll, dy);
		fSize = TRUE;
		SetInvalidation();  // InvalidateRect(hWnd, NULL, false);
	}
	else if (p->openable)
	{
		int n = p->childCount;
		p = p->childLink;
		if (p)
		{
			b.x += X_ICON_STEP;
			for (int i = 0; i < n; i++)
			{
				if (p->openable)
				{
					b.y += Y_ICON_STEP;
					rTemp = HelperRecursiveMouseClick(p, b, hWnd, hdcScreenCompat, fSize,
						openNode, fTab, bmp, hFont,
						mouseX, mouseY, xCurrentScroll, yCurrentScroll, sel);
					// Update maximal dimensions.
					if (rTemp.left < rDimension.left) { rDimension.left = rTemp.left; }
					if (rTemp.right > rDimension.right) { rDimension.right = rTemp.right; }
					if (rTemp.top < rDimension.top) { rDimension.top = rTemp.top; }
					if (rTemp.bottom > rDimension.bottom) { rDimension.bottom = rTemp.bottom; }
				}
				p++;
			}
		}
	}
	return rDimension;
}
// Helper for draw tree by nodes linked list and base coordinate point.
// Returns tree array (xleft, ytop, xright, ybottom),
// This parameters better calculate during draw, because depend on font size,
// current active font settings actual during draw.
RECT MainGUI::HelperRecursiveDrawTree(PTREENODE p, POINT b, BOOL fTab, HDC hDC, HFONT hFont,
	int xCurrentScroll, int yCurrentScroll, int& dy)
{
	b.x -= xCurrentScroll;
	b.y -= yCurrentScroll;
	return HelperRecursiveDT(p, b, fTab, hDC, hFont, xCurrentScroll, yCurrentScroll, dy);
}
RECT MainGUI::HelperRecursiveDT(PTREENODE p, POINT b, BOOL fTab, HDC hDC, HFONT hFont,
	int xCurrentScroll, int yCurrentScroll, int& dy)
{
	RECT rDimension = { 0,0,0,0 };
	RECT rTemp;
	int skipY = 0;
	if (p)
	{   // Draw node, addressed by caller.
		rDimension = HelperDrawNodeLayerSized(p, 1, b.x, b.y,
			X_ICON_STEP, Y_ICON_STEP, X_ICON_SIZE, Y_ICON_SIZE, fTab, hDC, hFont);

		PTREENODE childLink = p->childLink;
		int childCount = p->childCount;
		if ((childLink) && (childCount) && (p->openable) && (p->opened))
		{
			for (int i = 0; i < childCount; i++)
			{   // Recursive operation for childs nodes of node, addressed by caller.
				PTREENODE childChildLink = childLink->childLink;
				int childChildCount = childLink->childCount;
				POINT b1 = { b.x + X_ICON_STEP * 2, b.y + Y_ICON_STEP + Y_ICON_STEP * skipY };
				if (childLink->openable) { b1.x = b.x + X_ICON_STEP; }
				int dy = 0;
				rTemp = HelperRecursiveDT(childLink, b1, fTab, hDC, hFont, xCurrentScroll, yCurrentScroll, dy);
				// Update maximal dimensions.
				if (rTemp.left < rDimension.left) { rDimension.left = rTemp.left; }
				if (rTemp.right > rDimension.right) { rDimension.right = rTemp.right; }
				if (rTemp.top < rDimension.top) { rDimension.top = rTemp.top; }
				if (rTemp.bottom > rDimension.bottom) { rDimension.bottom = rTemp.bottom; }
				// Update Y size and pointer.
				skipY = skipY + 1 + dy;
				childLink++;
			}
		}
		POINT base = pManageResources->GetBase();
		rDimension.left = b.x;
		rDimension.top = b.y;
		rDimension.bottom = b.y + Y_ICON_STEP * 2 + Y_ICON_STEP * skipY + base.y;
	}
	dy = skipY;
	return rDimension;
}
// Helper for mark nodes in the tree, direction flag means:
// 0 = increment, mark next node or no changes if last node currently marked,
// 1 = decrement, mark previous node or no changes if first (root) node currently marked.
// Returns pointer to selected node.
PTREENODE MainGUI::HelperRecursiveMarkNode(BOOL direction, int sel)
{
	PTREENODE p1 = pManageResources->GetTrees()[sel];
	PTREENODE pFound = NULL;
	PTREENODE pNext = NULL;
	PTREENODE pBack = NULL;
	PTREENODE pTemp = NULL;

	HelperRecursiveMN(p1, pFound, pNext, pBack, pTemp);

	PTREENODE retPointer = NULL;
	if (direction && pFound && pNext)
	{
		pFound->marked = 0;
		pNext->marked = 1;
		retPointer = pNext;
	}
	else if ((!direction) && pFound && pBack)
	{
		pFound->marked = 0;
		pBack->marked = 1;
		retPointer = pBack;
	}
	return retPointer;
}
void MainGUI::HelperRecursiveMN(PTREENODE& p1, PTREENODE& pFound, PTREENODE& pNext, PTREENODE& pBack, PTREENODE& pTemp)
{
	if (p1)
	{
		if (p1->marked)
		{
			pFound = p1;
		}

		if ((p1->openable) && (p1->opened) && (p1->childLink))
		{
			PTREENODE p2 = p1->childLink;
			if (p2)
			{
				pTemp = p1;
				for (UINT i = 0; i < (p1->childCount); i++)
				{
					if (pFound && (!pNext)) pNext = p2;
					if ((p2) && (p2->marked)) pFound = p2;
					if (pFound && pTemp && (!pBack)) pBack = pTemp;

					if ((p2) && (p2->openable) && (p2->opened) && (p2->childLink))
					{
						HelperRecursiveMN(p2, pFound, pNext, pBack, pTemp);
					}
					else
					{
						pTemp = p2;
					}
					p2++;
				}
			}
		}
	}
}
void MainGUI::ClearInvalidation()
{
	invalidationRequest = false;
}
void MainGUI::SetInvalidation()
{
	invalidationRequest = true;
}
void MainGUI::MakeInvalidation(HWND hWnd)
{
	if (invalidationRequest)
	{
		InvalidateRect(hWnd, NULL, false);
		invalidationRequest = false;
	}
}
// Support tool bar.
HWND MainGUI::InitToolBar(HWND hWnd)
{
	HWND hToolBar;
	int btnID[NUM_BUTTONS] = { ID_TB_DEVICES, ID_TB_RESOURCES, ID_SEP, ID_TB_ABOUT, ID_TB_EXIT };
	int btnStyle[NUM_BUTTONS] = { TBSTYLE_CHECKGROUP, TBSTYLE_CHECKGROUP, TBSTYLE_SEP, TBSTYLE_BUTTON, TBSTYLE_BUTTON };
	TBBUTTON tbb[NUM_BUTTONS];
	memset(tbb, 0, sizeof(tbb));

	for (int i = 0; i < NUM_BUTTONS; ++i)
	{
		if (btnID[i] == ID_SEP)
			tbb[i].iBitmap = SEPARATOR_WIDTH;
		else  tbb[i].iBitmap = i;

		tbb[i].idCommand = btnID[i];
		tbb[i].fsState = TBSTATE_ENABLED;
		tbb[i].fsStyle = btnStyle[i];
	}

	hToolBar = CreateToolbarEx(hWnd,
		WS_CHILD | WS_VISIBLE | WS_BORDER | TBSTYLE_TOOLTIPS,
		ID_TOOLBAR, NUM_BUTTONS, GetModuleHandle(NULL), IDR_TOOLBAR,
		tbb, NUM_BUTTONS, 0, 0, 0, 0, sizeof(TBBUTTON));
	return hToolBar;
}
// Description:
//   Creates a horizontal scroll bar along the bottom of the parent 
//   window's area.
// Parameters:
//   hwndParent = handle to the parent window.
//   sbHeight   = height, in pixels, of the scroll bar.
//   downY      = size reserved below scroll bar for status bar.
//   rightY     = size reserved right scroll bar for vertical scroll bar
// Returns:
//   The handle to the scroll bar.
HWND MainGUI::CreateAHorizontalScrollBar(HWND hwndParent, int sbHeight, int downY, int rightY)
{
	RECT rect;
	// Get the dimensions of the parent window's client area;
	if (!GetClientRect(hwndParent, &rect))
		return NULL;
	// Create the scroll bar.
	return (CreateWindowEx(
		0,                               // no extended styles 
		"SCROLLBAR",                     // scroll bar control class 
		(PTSTR)NULL,                     // no window text 
		WS_CHILD | WS_VISIBLE            // window styles  
		| SBS_HORZ,                      // horizontal scroll bar style 
		rect.left,                       // horizontal position 
		rect.bottom - sbHeight - downY,  // vertical position 
		rect.right - rightY,             // width of the scroll bar 
		sbHeight,                        // height of the scroll bar
		hwndParent,                      // handle to main window 
		(HMENU)NULL,                     // no menu 
		GetModuleHandle(NULL),           // instance owning this window 
		(PVOID)NULL                      // pointer not needed 
	));
}
// Description:
//   Creates a vertical scroll bar along the right of the parent 
//   window's area.
// Parameters:
//   hwndParent = handle to the parent window.
//   sbWidth    = width, in pixels, of the scroll bar.
//   upY        = size reserved above scroll bar for tool bar.
//   downY      = size reserved below scroll bar for horizontal scroll bar and status bar.
// Returns:
//   The handle to the scroll bar.
HWND MainGUI::CreateAVerticalScrollBar(HWND hwndParent, int sbWidth, int upY, int downY)
{
	RECT rect;
	// Get the dimensions of the parent window's client area;
	if (!GetClientRect(hwndParent, &rect))
		return NULL;
	// Create the scroll bar.
	return (CreateWindowEx(
		0,                          // no extended styles 
		"SCROLLBAR",                // scroll bar control class 
		(PTSTR)NULL,                // no window text 
		WS_CHILD | WS_VISIBLE       // window styles  
		| SBS_VERT,                 // vertical scroll bar style 
		rect.right - sbWidth,       // horizontal position 
		rect.top + upY,             // vertical position 
		sbWidth,                    // width of the scroll bar 
		rect.bottom - upY - downY,  // height of the scroll bar
		hwndParent,                 // handle to main window 
		(HMENU)NULL,                // no menu 
		GetModuleHandle(NULL),      // instance owning this window 
		(PVOID)NULL                 // pointer not needed 
	));
}
// Data for support "About" window.
HCURSOR MainGUI::hCursorAbout;
HFONT MainGUI::hFontAbout;
HICON MainGUI::hIconAbout;
const LPCSTR MainGUI::szAbout = "Program info";
const LPCSTR MainGUI::szClickable1 = "More at GitHub.";
const LPCSTR MainGUI::szClickable2 = "Developed with";
const LPCSTR MainGUI::szClickable3 = "Visual Studio.";
const LPCSTR MainGUI::szClickable4 = "Flat Assembler.";
const LPCSTR MainGUI::szLink1 = "https://github.com/manusov?tab=repositories";
const LPCSTR MainGUI::szLink2 = "https://visualstudio.microsoft.com/ru/downloads/";
const LPCSTR MainGUI::szLink3 = "https://flatassembler.net/";
const LPCSTR MainGUI::szError = "Shell error.";
CLICKSTRING MainGUI::cs[4] = {
	{ szClickable1, ABOUT_FULL_SIZE_1, ABOUT_CLICK_START_1, ABOUT_CLICK_SIZE_1, -1, -1, -1, -1 },
	{ szClickable2, ABOUT_FULL_SIZE_2, ABOUT_CLICK_START_2, ABOUT_CLICK_SIZE_2, -1, -1, -1, -1 },
	{ szClickable3, ABOUT_FULL_SIZE_3, ABOUT_CLICK_START_3, ABOUT_CLICK_SIZE_3, -1, -1, -1, -1 },
	{ szClickable4, ABOUT_FULL_SIZE_4, ABOUT_CLICK_START_4, ABOUT_CLICK_SIZE_4, -1, -1, -1, -1 }
};
const int MainGUI::N1 = 3;
const int MainGUI::N2 = sizeof(cs) / sizeof(CLICKSTRING);
const LPCSTR MainGUI::webLinks[] = { szLink1, NULL, szLink2, szLink3 };

// Support "About" window.
void MainGUI::AboutHelperClose(HWND hWnd)
{
	if (hFontAbout) DeleteObject(hFontAbout);
	EndDialog(hWnd, 0);
}
INT_PTR MainGUI::AboutDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	INT_PTR status = FALSE;
	switch (uMsg)
	{

	case WM_INITDIALOG:
	{
		SetWindowText(hWnd, szAbout);
		SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)(pManageResources->GetIconHandleByIndex(ID_APP)));
		hCursorAbout = LoadCursor(NULL, IDC_HAND);
		hFontAbout = CreateFont(ABOUT_FONT_HEIGHT, 7, 0, 0, FW_DONTCARE, TRUE, 0, 0,
			DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Verdana"));
		hIconAbout = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_BOOKS),
			IMAGE_ICON, ABOUT_ICONDX, ABOUT_ICONDX, LR_DEFAULTCOLOR);
		status = TRUE;
	}
	break;

	case WM_CLOSE:
	{
		AboutHelperClose(hWnd);
		status = FALSE;
	}
	break;

	case WM_COMMAND:
	{
		if (wParam == ID_ABOUT_OK)
		{
			AboutHelperClose(hWnd);
		}
		status = FALSE;
	}
	break;

	case WM_LBUTTONDOWN:
	{
		int x = GET_X_LPARAM(lParam);
		int y = GET_Y_LPARAM(lParam);
		for (int i = 0; i < N2; i++)
		{
			int xmin = cs[i].xmin;
			int xmax = cs[i].xmax;
			int ymin = cs[i].ymin;
			int ymax = cs[i].ymax;
			if ((xmin >= 0) && (xmax >= 0) && (ymin >= 0) && (ymax >= 0) &&
				(x > xmin) && (x < xmax) && (y > ymin) && (y < ymax))
			{
				if (hCursorAbout) SetCursor(hCursorAbout);
				LPCSTR s = webLinks[i];
				if (s)
				{
					HINSTANCE h = ShellExecute(NULL, 0, s, NULL, NULL, SW_NORMAL);
					if ((INT_PTR)h <= 32)
					{
						MessageBox(NULL, szError, NULL, MB_ICONERROR);
					}
				}
			}
		}
		status = FALSE;
	}
	break;

	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
	{
		int x = GET_X_LPARAM(lParam);
		int y = GET_Y_LPARAM(lParam);
		for (int i = 0; i < N2; i++)
		{
			int xmin = cs[i].xmin;
			int xmax = cs[i].xmax;
			int ymin = cs[i].ymin;
			int ymax = cs[i].ymax;
			if ((xmin >= 0) && (xmax >= 0) && (ymin >= 0) && (ymax >= 0) &&
				(x > xmin) && (x < xmax) && (y > ymin) && (y < ymax))
			{
				if (hCursorAbout) SetCursor(hCursorAbout);
			}
		}
		status = FALSE;
	}
	break;

	case WM_PAINT:
	{
		status = TRUE;
		PAINTSTRUCT ps;
		RECT r1, r2;
		HFONT hFontBack;
		TEXTMETRIC tm;
		SIZE s1; // , s2;
		HDC hDC = BeginPaint(hWnd, &ps);
		if (hDC && hCursorAbout && hFontAbout && hIconAbout)
		{
			if (GetClientRect(hWnd, &r1))
			{
				r2.left = r1.left;
				r2.right = r1.right;
				r2.top = r1.top;
				r2.bottom = r1.bottom - ABOUT_YBOTTOM;
				int bkMode = SetBkMode(hDC, TRANSPARENT);
				if (bkMode)
				{
					if (FillRect(hDC, &r2, (HBRUSH)(COLOR_WINDOW + 1)))
					{
						if (DrawIcon(hDC, ABOUT_ICONX, ABOUT_ICONY, hIconAbout))
						{
							hFontBack = (HFONT)SelectObject(hDC, hFontAbout);
							if (hFontBack)
							{
								if (GetTextMetrics(hDC, &tm))
								{
									r2.top = ABOUT_YBASE1;
									r2.bottom = ABOUT_YBASE1 + ABOUT_YADD1;
									CHAR buffer[BUILD_NAME_MAX];
									for (int i = 0; i < N1; i++)
									{
										switch (i)
										{
										case 0:
											_snprintf_s(buffer, BUILD_NAME_MAX, _TRUNCATE, "%s", ABOUT_TEXT_1);
											break;
										case 1:
											_snprintf_s(buffer, BUILD_NAME_MAX, _TRUNCATE, "%s%s%s %s",
												ABOUT_TEXT_2_1, ABOUT_TEXT_2_2, ABOUT_TEXT_2_3, ABOUT_TEXT_2_4);
											break;
										case 2:
											_snprintf_s(buffer, BUILD_NAME_MAX, _TRUNCATE, "%s", ABOUT_TEXT_3);
											break;
										}
										if (!DrawText(hDC, buffer, -1, &r2, DT_CENTER + DT_NOPREFIX)) break;
										r2.top += ABOUT_YADD1;
										r2.bottom += ABOUT_YADD1;
									}

									int y = ABOUT_YBASE2;
									r2.top = r1.top;
									r2.bottom = r1.bottom;
									for (int i = 0; i < N2; i++)
									{
										cs[i].ymin = y;
										cs[i].ymax = y + tm.tmHeight;
										if (!GetTextExtentPoint32(hDC, cs[i].stringPtr, cs[i].fullSize, &s1)) break;  // full string width, prefix part + clickable part
										r2.left = (r1.right - r1.left - s1.cx) / 2;
										r2.right = r1.right;
										r2.top = y;
										r2.bottom = y + ABOUT_YADD2;
										if (cs[i].fullSize != cs[i].clickSize)
										{
											if (!DrawText(hDC, cs[i].stringPtr, cs[i].clickStart, &r2, DT_LEFT + DT_NOPREFIX)) break;
										}
										if (cs[i].clickStart >= 0)
										{
											cs[i].xmax = r2.left + s1.cx;
											if (!GetTextExtentPoint32(hDC, cs[i].stringPtr, cs[i].clickStart, &s1)) break;  // prefix part width, left from clickable
											r2.left += s1.cx;
											cs[i].xmin = r2.left;
											COLORREF backColor = SetTextColor(hDC, ABOUT_CLICK_COLOR);
											if (!DrawText(hDC, cs[i].stringPtr + cs[i].clickStart, cs[i].clickSize, &r2, DT_LEFT + DT_NOPREFIX)) break;
											SetTextColor(hDC, backColor);
										}
										y += ABOUT_YADD2;
									}
								}
								else
								{
									status = FALSE;
								}
								SelectObject(hDC, hFontBack);
							}
							else
							{
								status = FALSE;
							}
						}
						else
						{
							status = FALSE;
						}
					}
					else
					{
						status = FALSE;
					}
					SetBkMode(hDC, bkMode);
				}
				else
				{
					status = FALSE;
				}
			}
			else
			{
				status = FALSE;
			}
			EndPaint(hWnd, &ps);
		}
		else
		{
			status = FALSE;
		}
	}
	break;

	default:
	{
		status = FALSE;
	}
	break;

	}
	return status;
}

