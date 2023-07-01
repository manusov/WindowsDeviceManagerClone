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

#pragma once
#ifndef MAINGUI_H
#define MAINGUI_H

#include <windows.h>
#include <windowsx.h>
#include <CommCtrl.h>
#include "resource.h"
#include "Global.h"
#include "ManageResources.h"

class MainGUI
{
public:
	MainGUI(HINSTANCE hInst, int cmdShow, ManageResources* pMr);
	~MainGUI();
	void SetAndInitModel(ManageResources* pMr);
	void MainWait();
	HWND GetHWnd();
	LPCSTR GetErrorName();
private:
	static HWND hWnd;
	static WNDCLASSEX wc;
	static ManageResources* pManageResources;
	static BOOL invalidationRequest;
	static LPCSTR errorName;
	static LPCSTR errorName1;
	static LPCSTR errorName2;
	// Dialogue procedure.
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	// Helpers.
	// Helpers for mouse click and position detection.
	static bool DetectMouseClick(int xPos, int yPos, PTREENODE p);
	static bool DetectMousePosition(int xPos, int yPos, PTREENODE p);
	// Helper for unmark items stay invisible after parent item close.
	static void HelperMarkedClosedChilds(PTREENODE pParent, PTREENODE& openNode, BOOL fTab);
	// Helper for node sequence of one layer.
	// Returns layer array (xleft, ytop, xright, ybottom),
	// This parameters better calculate during draw, because depend on font size,
	// current active font settings actual during draw.
	static RECT HelperDrawNodeLayerSized(PTREENODE pNodeList, int nodeCount, int treeBaseX, int treeBaseY,
		int iconStepX, int iconStepY, int iconSizeX, int iconSizeY, BOOL fTab, HDC hDC, HFONT hFont);
	// Helper for adjust horizontal scrolling parameters.
	// The horizontal scrolling range is defined by 
	// (bitmap_width) - (client_width). The current horizontal 
	// scroll value remains within the horizontal scrolling range.
	static void HelperAdjustScrollX(HWND hWnd, SCROLLINFO& scrollInfo, RECT& treeDimension,
		int xNewSize, int& xMaxScroll, int& xMinScroll, int& xCurrentScroll);
	// Helper for adjust vertical scrolling parameters.
	// The vertical scrolling range is defined by 
	// (bitmap_height) - (client_height). The current vertical 
	// scroll value remains within the vertical scrolling range. 
	static void HelperAdjustScrollY(HWND hWnd, SCROLLINFO& scrollInfo, RECT& treeDimension,
		int yNewSize, int& yMaxScroll, int& yMinScroll, int& yCurrentScroll);
	// Helper for make horizontal scrolling by given signed offset.
	static void HelperMakeScrollX(HWND hWnd, SCROLLINFO& scrollInfo,
		int xMaxScroll, int& xCurrentScroll, BOOL& fScroll, int addX);
	// Helper for make vertical scrolling by given signed offset.
	static void HelperMakeScrollY(HWND hWnd, SCROLLINFO& scrollInfo,
		int yMaxScroll, int& yCurrentScroll, BOOL& fScroll, int addY);
	// This part for support recursive tree levels and eliminate level count limits.
	// Helper for update open-close icon light depend on mouse cursor position near icon.
	static void HelperRecursiveMouseMove(PTREENODE p, HWND hWnd, HDC hdcScreenCompat, BOOL& fSize, BOOL forceUpdate,
		int mouseX, int mouseY, int xCurrentScroll, int yCurrentScroll, int offsetY);
	static RECT HelperRecursiveMouseClick(PTREENODE p, POINT b, HWND hWnd, HDC hdcScreenCompat, BOOL& fSize,
		PTREENODE& openNode, BOOL fTab, BITMAP bmp, HFONT hFont,
		int mouseX, int mouseY, int xCurrentScroll, int yCurrentScroll, int sel);
	// Helper for draw tree by nodes linked list and base coordinate point.
	// Returns tree array (xleft, ytop, xright, ybottom),
	// This parameters better calculate during draw, because depend on font size,
	// current active font settings actual during draw.
	static RECT HelperRecursiveDrawTree(PTREENODE p, POINT b, BOOL fTab, HDC hDC, HFONT hFont,
		int xCurrentScroll, int yCurrentScroll, int& dy);
	static RECT HelperRecursiveDT(PTREENODE p, POINT b, BOOL fTab, HDC hDC, HFONT hFont,
		int xCurrentScroll, int yCurrentScroll, int& dy);
	// Helper for mark nodes in the tree, direction flag means:
	// 0 = increment, mark next node or no changes if last node currently marked,
	// 1 = decrement, mark previous node or no changes if first (root) node currently marked.
	// Returns pointer to selected node.
	static PTREENODE HelperRecursiveMarkNode(BOOL direction, int sel);
	static void HelperRecursiveMN(PTREENODE& p1, PTREENODE& pFound, PTREENODE& pNext, PTREENODE& pBack, PTREENODE& pTemp);
	// Support deferred screen invalidation method for prevent blinking.
	// Note partial invalidation requests (if LPRECT not NULL) not deferred.
	static void ClearInvalidation();
	static void SetInvalidation();
	static void MakeInvalidation(HWND hWnd);
	// Support tool bar.
	static HWND InitToolBar(HWND hWnd);
	static HWND CreateAHorizontalScrollBar(HWND hwndParent, int sbHeight, int downY, int rightY);
	static HWND CreateAVerticalScrollBar(HWND hwndParent, int sbWidth, int upY, int downY);
	// Support "About" window.
	static INT_PTR AboutDlgProc(HWND, UINT, WPARAM, LPARAM);
	static void AboutHelperClose(HWND hWnd);
	static HCURSOR hCursorAbout;
	static HFONT hFontAbout;
	static HICON hIconAbout;
	static const LPCSTR szAbout;
	static const LPCSTR szClickable1;
	static const LPCSTR szClickable2;
	static const LPCSTR szClickable3;
	static const LPCSTR szClickable4;
	static const LPCSTR szLink1;
	static const LPCSTR szLink2;
	static const LPCSTR szLink3;
	static const LPCSTR szError;
	static CLICKSTRING cs[4];
	static const int N1;
	static const int N2;
	static const LPCSTR webLinks[];
};

#endif  // MAINGUI_H