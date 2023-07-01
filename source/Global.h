/* ----------------------------------------------------------------------------------------
Constants and data structures definitions,
include application title strings definitions.
---------------------------------------------------------------------------------------- */

#pragma once
#ifndef GLOBAL_H
#define GLOBAL_H

#include <windows.h>
#include <vector>

// Select emulated or system scan mode: uncomment this for debug.
// Emulated mode means use constant emulated system configuration info
// WITHOUT get system information.
// Two lists (device list view or resource map view) contains
// different tree levels count if emulated mode used.
// #define _EMULATED_MODE

#define BUILD_VERSION  "0.13.0.2"
#define BUILD_NAME_MAX 160

#if _WIN64
#ifndef _EMULATED_MODE
#define RESOURCE_DESCRIPTION  "System view. Win64 edition."
#define RESOURCE_VERSION      BUILD_VERSION
#define RESOURCE_COMPANY      "https://github.com/manusov"
#define RESOURCE_COPYRIGHT    "(C) 2023 Ilya Manusov."
#define PROGRAM_NAME_TEXT     "System view for Win64."
#define ABOUT_TEXT_1          "System view."
#define ABOUT_TEXT_2_1        "v"
#define ABOUT_TEXT_2_2        BUILD_VERSION
#define ABOUT_TEXT_2_3        "."
#define ABOUT_TEXT_2_4        "x64."
#define ABOUT_TEXT_3          RESOURCE_COPYRIGHT
#else
#define RESOURCE_DESCRIPTION  "System view. Win64 edition. Emulated mode for debug."
#define RESOURCE_VERSION      BUILD_VERSION
#define RESOURCE_COMPANY      "https://github.com/manusov"
#define RESOURCE_COPYRIGHT    "(C) 2023 Ilya Manusov."
#define PROGRAM_NAME_TEXT     "System view for Win64."
#define ABOUT_TEXT_1          "System view. Emulated mode."
#define ABOUT_TEXT_2_1        "v"
#define ABOUT_TEXT_2_2        BUILD_VERSION
#define ABOUT_TEXT_2_3        "."
#define ABOUT_TEXT_2_4        "x64."
#define ABOUT_TEXT_3          RESOURCE_COPYRIGHT
#endif
#elif _WIN32
#ifndef _EMULATED_MODE
#define RESOURCE_DESCRIPTION  "System view. Win32 edition."
#define RESOURCE_VERSION      BUILD_VERSION
#define RESOURCE_COMPANY      "https://github.com/manusov"
#define RESOURCE_COPYRIGHT    "(C) 2023 Ilya Manusov."
#define PROGRAM_NAME_TEXT     "System view for Win32."
#define ABOUT_TEXT_1          "System view."
#define ABOUT_TEXT_2_1        "v"
#define ABOUT_TEXT_2_2        BUILD_VERSION
#define ABOUT_TEXT_2_3        "."
#define ABOUT_TEXT_2_4        "ia32."
#define ABOUT_TEXT_3          RESOURCE_COPYRIGHT
#else
#define RESOURCE_DESCRIPTION  "System view. Win32 edition. Emulated mode for debug."
#define RESOURCE_VERSION      BUILD_VERSION
#define RESOURCE_COMPANY      "https://github.com/manusov"
#define RESOURCE_COPYRIGHT    "(C) 2023 Ilya Manusov."
#define PROGRAM_NAME_TEXT     "System view for Win32."
#define ABOUT_TEXT_1          "System view. Emulated mode."
#define ABOUT_TEXT_2_1        "v"
#define ABOUT_TEXT_2_2        BUILD_VERSION
#define ABOUT_TEXT_2_3        "."
#define ABOUT_TEXT_2_4        "ia32."
#define ABOUT_TEXT_3          RESOURCE_COPYRIGHT
#endif
#else
#define RESOURCE_DESCRIPTION  "UNKNOWN BUILD MODE."
#define RESOURCE_VERSION      BUILD_VERSION
#define RESOURCE_COMPANY      "https://github.com/manusov"
#define RESOURCE_COPYRIGHT    "(C) 2023 Ilya Manusov."
#define PROGRAM_NAME_TEXT     "UNKNOWN BUILD MODE."
#define ABOUT_TEXT_1          "System view."
#define ABOUT_TEXT_2_1        "UNKNOWN BUILD MODE. v"
#define ABOUT_TEXT_2_2        BUILD_VERSION
#define ABOUT_TEXT_2_3        "."
#define ABOUT_TEXT_2_4        "?"
#define ABOUT_TEXT_3          RESOURCE_COPYRIGHT
#endif

#define MAIN_WINDOW_BASE_X  640
#define MAIN_WINDOW_BASE_Y  240
#define MAIN_WINDOW_SIZE_X  600
#define MAIN_WINDOW_SIZE_Y  650

#define TREE_COUNT_ALLOCATED    1000
#define SYSTEM_TREE_MEMORY_MAX  1024*1024*2

#define X_BASE_TREE  10
#define Y_BASE_TREE  10
#define X_ICON_SIZE  16
#define Y_ICON_SIZE  16
#define X_ICON_STEP  18
#define Y_ICON_STEP  18

#define BACKGROUND_BRUSH  RGB(213, 240, 213)
#define SELECTED_BRUSH    RGB(245, 245, 120)

#define ID_TOOLBAR        501
#define ID_STATUSBAR      601
#define NUM_BUTTONS       5
#define SEPARATOR_WIDTH   6

#define SCROLLBAR_HEIGHT  16
#define SCROLLBAR_WIDTH   17

// "G_" means groups.
typedef enum {
	G_HTREE, G_ROOT, G_SWD,
	G_ACPI, G_ACPI_HAL, G_UEFI, G_SCSI, G_STORAGE, G_HID,
	G_PCI, G_USB, G_USBSTOR, G_BTH, G_DISPLAY, G_HDAUDIO,
	G_OTHER
} TREEGROUPS;

// Platform resource type classification.
typedef enum {
	RES_MEM, RES_LMEM, RES_IO, RES_IRQ, RES_DMA, 
	RES_MAX
} RESTYPES;

// Structure for classifying and sorting system enumeration results by groups.
// Note devices names get from WinAPI, groups names get from this structures.
typedef struct GROUPSORT {
	LPCSTR groupPattern;                // Pattern for detect device ID string type.
	LPCSTR groupName;                   // String for name of this group.
	int iconIndex;                      // Index of icon from icon pool for this group and childs.
	std::vector<LPCSTR>* childStrings;  // Pointer to vector with sequence of pointers to null terminated child strings (pairs).
} *PGROUPSORT;

// Structure for unified resource entry (Memory, Large memory, IO, IRQ, DMA).
typedef struct RESOURCEENTRY {
	LPCSTR deviceName;           // Pointer to enumerated device name, used this resource.
	DWORDLONG dataL;             // This field purpose is resource-specific
	DWORDLONG dataH;             // This field purpose is resource-specific
} *PRESOURCEENTRY;

// Structure for classifying and sorting system resources usage enumeration
// results by groups.
// Note devices names and resources ranges/values get from WinAPI, groups names
// (resource type name) get from this structures.
typedef struct RESOURCESORT {
	std::vector<RESOURCEENTRY>* childRanges;  // Pointer to resource entries.
} *PRESOURCESORT;

// Icons indexes must be sequental, even if icon resources identifiers not sequental.
typedef enum {
	ID_APP,
	ID_CLOSED, ID_CLOSED_LIGHT, ID_OPENED, ID_OPENED_LIGHT,
	ID_ACPI, ID_ACPI_HAL, ID_AUDIO, ID_AUDIO_IO, ID_BLUETOOTH, ID_DISPLAYS, ID_HID, ID_IDE,
	ID_IMAGE_PROCESSING, ID_KEYBOARDS, ID_MASS_STORAGE, ID_MOBILE_DEVICES, ID_MOUSES,
	ID_NETWORK, ID_OTHER, ID_PCI, ID_PCI_IDE, ID_PORTS, ID_PRINT, ID_PROCESSORS,
	ID_ROOT_ENUMERATOR, ID_SCSI, ID_SECURITY, ID_SOFT_COMPONENTS, ID_SOFT_DEVICES,
	ID_SYSTEM_TREE, ID_THIS_COMPUTER, ID_UEFI, ID_UM_BUS, ID_USB, ID_USB_STORAGE,
	ID_VIDEO_ADAPTERS,
	ID_RES_MEMORY, ID_RES_LARGE_MEMORY, ID_RES_IO, ID_RES_IRQ, ID_RES_DMA
} ICON_INDEXES;

// Structure for device node, used by TreeBuilder as DESTINATION data.
typedef struct TREENODE {
	HICON hNodeIcon;      // Icon handle for this device node, this and 2 next handles must be pre-loaded by LoadImage function, handle = F(resource id).
	HICON hClosedIcon;    // Icon handle for CLOSED state of device node, left to right arrow.
	HICON hOpenedIcon;    // Icon handle for CLOSED state of device node, up to down arrow.
	LPCSTR szNodeName;    // Pointer to node name null-terminated text string.
	TREENODE* childLink;  // Pointer to sequence of child nodes of this node, NULL if no child nodes.
	UINT childCount;      // Length of sequence of child nodes of this node.
	RECT clickArea;       // Area for mouse click detection: Xleft, Yup, Xright, Ydown, build by GUI routine, not by caller.
	UINT openable : 1;    // Binary flag: last level node, cannot be opened (0) or contain childs and can be opened (1)
	UINT opened : 1;      // Binary flag: node closed (0) or opened (1).
	UINT marked : 1;      // Binary flag: node not marked (0) or marked by keyboard selection (1).
	UINT prevMouse : 1;   // Binary flag: previous mouse cursor position status, outside open-close icon (0) or inside (1).
} *PTREENODE;

// Structure for list childs of device node, used by TreeBuilder as EMULATED SOURCE data.
typedef struct TREECHILDS {
	int childCount;             // Number of childs.
	int* childIconIndexes;      // Pointer to array of integers: indexes of child icons.
	LPCSTR* childNamesStrings;  // Pointer to array of pointers to strings: names of child nodes.
} *PTREECHILDS;

// Support "About" window.
// Note this parameters for elements inside "About" window.
// For change "About" window position relative main window use resource editor.

#define ABOUT_FONT_HEIGHT    16
#define ABOUT_YBASE1         38
#define ABOUT_YBASE2         100
#define ABOUT_YADD1          19
#define ABOUT_YADD2          19
#define ABOUT_YBOTTOM        47
#define ABOUT_ICONX          118
#define ABOUT_ICONY          2
#define ABOUT_ICONDX         32
#define ABOUT_ICONDY         32

#define ABOUT_FULL_SIZE_1    15
#define ABOUT_CLICK_START_1  8
#define ABOUT_CLICK_SIZE_1   7

#define ABOUT_FULL_SIZE_2    14
#define ABOUT_CLICK_START_2  -1
#define ABOUT_CLICK_SIZE_2   -1

#define ABOUT_FULL_SIZE_3    14
#define ABOUT_CLICK_START_3  0
#define ABOUT_CLICK_SIZE_3   14

#define ABOUT_FULL_SIZE_4    15
#define ABOUT_CLICK_START_4  0
#define ABOUT_CLICK_SIZE_4   15

#define ABOUT_CLICK_COLOR    0x00FF0000

typedef struct CLICKSTRING {
	LPCSTR stringPtr;
	int fullSize;
	int clickStart;
	int clickSize;
	int xmin;
	int xmax;
	int ymin;
	int ymax;
} *PCLICKSTRING;

#endif  // GLOBAL_H

