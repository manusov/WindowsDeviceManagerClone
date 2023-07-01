/* ----------------------------------------------------------------------------------------

ENGINEERING SAMPLE.
Clone of Windows Device Manager.
Main source file (Head Block).

Project context and settings:

  1) Add resources for application icon and tree nodes icons,
     main menu, tool bar, dialogue(s).
  2) Required differentiation application icon for ia32 and x64,
     both for Debug and Release,
     Build events \ Event before build \ command line:
	 set icon for ia32 command line:
	   "copy Icons_App\app32.ico Icons_App\app.ico"
	 set icon for x64 command line:
       "copy Icons_App\app64.ico Icons_App\app.ico"

This settings both for x86 and x64, both for Debug and Release:
  3) Additional \ Char set \ Multi-byte encoding.
  4) Linker \ System \ Windows.
  5) Linker \ Input \ Additional dependences \ Add library: setupapi.lib.
  6) Linker \ Input \ Additional dependences \ Add library: Comctl32.lib.

This settings both for x86 and x64, for Release only:
  7) C++ \ Code build \ Runtime library \ Multi-thread (MT).

Special thanks to:

Microsoft enumerator sample:
https://github.com/microsoft/Windows-driver-samples/tree/main/setup/devcon

Windows GUI examples:
https://learn.microsoft.com/en-us/windows/win32/controls/scroll-a-bitmap-in-scroll-bars
https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-createfonta
https://www.piter.com/page/filesforbooks
https://storage.piter.com/support_insale/files_list.php?code=978546901361

Icons library and editor:
http://ru.softoware.org/apps/download-open-icon-library-for-windows.html
https://sourceforge.net/projects/openiconlibrary/
http://www.sibcode.com/junior-icon-editor/

TODO (DONE).
------------
1)+  Mouse move tracking when _NEW_GUI (at Main.h) enabled.
     Questions about offsetY.
     InvalidateRect must add toolY and subtract (toolY + statusY) instead rect = NULL?
	 InvalidateRect must be called conditionally for prevent flip.
	 YET FIXED FOR "SCROLL BAR BELOW STATUS BAR" VARIANT.
	 ADDED TOOLY, STATUSY, OFFSETY VARIABLES.

2)+  Screen flip when tree open-close, required correction of double bufferring logic.
     Caused by multi-invalidations instead final-only invalidation?
	 Occurred when scroll bar show status changed?
	 YET FIXED FOR "SCROLL BAR BELOW STATUS BAR" VARIANT.
	 ALSO FLIP WHEN WINDOW RESIZED UNDER ORACLE VIRTUALBOX GUEST = WINDOWS 7.

3)+  Large memory descriptors yet ignored.
	 Note low memory descriptors below and above 4GB supported.
	 FIXED.
	 See "case ResType_MemLarge:" at "DumpDeviceResourcesOfType", EnumRes.cpp.

4)+  Resource values (address, IRQ, DMA) must be sorted.
     See "RESOURCESORT rs = trControl[i];" at "EnumerateTransitListToGroupList",
	 EnumRes.cpp.
	 FIXED.
	 See "std::sort(rs.childRanges->begin(), rs.childRanges->end()," at
	 "EnumerateTransitListToGroupList", EnumRes.cpp.

5)+  Monospace font required, because address output better if formatted.
     FIXED.
	 See "case WM_CREATE:", TreeView.cpp.

6)-  Resources map is empty when 32-bit application runs under Win10 x64, WoW64 scenario.
	 THIS PROBLEM DOCUMENTED BY MICROSOFT:
	 https://learn.microsoft.com/en-us/windows/win32/api/cfgmgr32/nf-cfgmgr32-cm_get_first_log_conf_ex
	 https://learn.microsoft.com/en-us/windows/win32/api/cfgmgr32/nf-cfgmgr32-cm_get_first_log_conf
	 From MSDN:
	 "Note  Starting with Windows 8, CM_Get_First_Log_Conf_Ex returns CR_CALL_NOT_IMPLEMENTED
	 when used in a Wow64 scenario. To request information about the hardware resources on a
	 local machine it is necessary implement an architecture-native version of the application
	 using the hardware resource APIs. For example: An AMD64 application for AMD64 systems."

7)-  Address aliases and duplications, see descriptors flags for all descriptor types.
	 Example (Z590 platform): 
	 Why System Board Resources uses ports 70h-70h but port 71h is not declared?
	 Why extended CMOS/RTC ports 72h-73h also not declared?
	 THIS PROBLEM CAUSED BY NON-LEGACY CONFIGURATION?
	 IO DESCRIPTOR 70H-70H CONTAINS ATTRIBUTES (IOD_DesFlags = 11h):
	 #define fIOD_IO (0x1) // Port resource uses IO ports
	 #define fIOD_16_BIT_DECODE (0x0010)
	 By Windows 10 device manager results at Z590 platform:
	 CMOS/RTC device missing,
	 port 70h declared-used, range 71h-7Fh not declared,
	 but range 0000h-0CF7h used by "PCI Express Root Complex".
	 Physically, ports 70h and 71h accessible by RWEverything indexed I/O access.
	 RWEverything RTC/CMOS registers dump works.
	 See "DumpDeviceResourcesOfType", EnumRes.cpp.
	 See also:
	 https://learn.microsoft.com/en-us/windows-hardware/drivers/bringup/acpi-defined-devices
     RAM, ROM, PREFETCHABLE AND OTHER ATTRIBUTES CAN BE USED LATER.

8)+  Scroll bar below status bar, required swap, use SB_CTL instead SB_HORZ, SB_VERT?
     Fix this bug when refactoring by build class TreeViewDebug?
	 https://learn.microsoft.com/en-us/windows/win32/controls/create-scroll-bars
     https://learn.microsoft.com/ru-ru/windows/win32/controls/scroll-bars
     https://learn.microsoft.com/ru-ru/windows/win32/api/winuser/nf-winuser-createwindowexa
     https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowa
     https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexa
	 FIXED. 
	 Window callback procedure redesigned, old is TreeView1.cpp, new is TreeView2.cpp.

9)+  Bug if tree size reduced when node closed and this make scrolling not required,
     or change current scroll offset. Problem if this situation occurred when
	 scroll offset > 0.
	 FIXED.
	 See TreeView2.cpp. Tree redraw second time if scroll offset changed when tree
	 redraw first time.
	 
10)+ Refactoring required: Model, View, Controller paradigm now incorrectly used.
     Refactoring to files set:
       Main.cpp, MainGUI.cpp, 
	   Title.h, Global.h (or Global.h only include title data), 
	   ManageResources.cpp, ManageDLL.cpp, ManageKMD.cpp,
	   EnumConst.cpp, EnumWin.cpp, ... add new enumerators.
     Can copy work directory and change files step-by-step, without initializing
	 new project. Create new files and delete old unused files.
	 Old = source_v0_12_5_0.
	 New = source_v0_13_X_X.

11)+ Why replace:
     "MainGUI(hInstance, nCmdShow, pMr)"
	 to
	 "MainGUI(hInstance, nCmdShow, NULL)"
	 do not cause exception at "pMr->procedureName(...)" inside MainGUI class?
	 CAUSE:
	 Class pointer really used as table base pointer for virtual methods,
	 for non-virtual methods it only identify class for compiller.
	 Proof: make "pEnum = NULL" for faults demo.

TODO.
------
12) Vertical and horizontal scroll bars buttons push can be not visualized,
    scroll work, but button at scroll bar not pushed.

13) Bugs with items selection by keyboard, selected item out of visible area when
	move by cursor up, cursor down keys.

14) Optimization required: reduce size of TREENODE structure, for example:
    WORD iconId (16-bit) instead HICON iconHandle (32/64-bit).
	But ID to Handle translation required for this variant.
	Criterias [Compact structure] vs [Data ready for use].

15) Optimization required: calculate visible part of tree, by vertical.
    Skip draw operations for invisible parts.

16)	Profiling required.
	https://learn.microsoft.com/ru-ru/archive/msdn-magazine/2015/may/visual-studio-2015-analyze-performance-while-debugging-in-visual-studio-2015
	Debug \ Windows \ Show diagnostics tool.
	Measure time between breakpoints. Approx. 1 ms resolution. 
	Use RDTSC instruction or precision timers for more accurrate measurements.
	Note about compiler optimizations and "Debug" and "Release" mode differences.
---------------------------------------------------------------------------------------- */

#include <windows.h>
#include "resource.h"
#include "Global.h"
#include "MainGUI.h"
#include "ManageResources.h"
#include "EnumWin.h"
#include "EnumConst.h"

ManageResources*  pMr = NULL;
EnumConst*        pEnum = NULL;
MainGUI*          pMg = NULL;
PTREENODE         trees[2];

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	// Initialize common controls.
	int status = 0;
	LPCSTR szGetError = NULL;
	INITCOMMONCONTROLSEX icc;
	icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icc.dwICC = ICC_WIN95_CLASSES;
	if (InitCommonControlsEx(&icc))
	{
		// Initialize application and system context.
		pMr = new ManageResources();
#ifdef _EMULATED_MODE
		pEnum = new EnumConst();
#else
		pEnum = new EnumWin();
#endif
		if (pMr && pEnum)
		{
			if (pMr->getInitStatus())
			{
				pEnum->SetAndInitModel(pMr);
				// Set tree base coordinates.
				POINT treeBase = { X_BASE_TREE, Y_BASE_TREE };
				pMr->SetBase(treeBase);
				// Set tree data.
				trees[0] = pEnum->BuildTree(0);
				trees[1] = pEnum->BuildTree(1);
				pMr->SetTrees(trees);
				if (trees[0] && trees[1])
				{
					pMg = new MainGUI(hInstance, nCmdShow, pMr);
					if (pMg)
					{
						szGetError = pMg->GetErrorName();
						if (!szGetError)
						{
							pMg->MainWait(); // Handling user events
						}
						else
						{
							status = 6;
						}
					}
					else
					{
						status = 5;
					}
				}
				else
				{
					status = 4;
				}
			}
			else
			{
				status = 3;
			}
		}
		else
		{
			status = 2;
		}
	}
	else
	{
		status = 1;
	}
	// Messages if errors.
	if (status)
	{
		LPCSTR s = "?";
		switch (status)
		{
		case 1:
			s = "Common controls initialization failed.";
			break;
		case 2:
			s = "System or resource classes initialization failed.";
			break;
		case 3:
			s = "Resources initialization failed.";
			break;
		case 4:
			s = "Build system tree failed.";
			break;
		case 5:
			s = "GUI class initialization failed.";
			break;
		case 6:
			s = szGetError;
			break;
		default:
			s = "Unknown error.";
			break;
		}
		MessageBox(NULL, s, "Application starting error.", MB_OK | MB_ICONERROR);
	}
	// Restore system context and exit application.
	if (pMg) delete pMg;
	if (pEnum)
	{
		pEnum->ReleaseTree();
		delete pEnum;
	}
	if (pMr) delete pMr;
	return status;
}
