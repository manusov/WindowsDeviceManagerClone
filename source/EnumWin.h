/* ----------------------------------------------------------------------------------------
Class used for get system information: devices list and resource map at MS Windows
device manager style. WinAPI used (no direct hardware scan at this class).
This class is child of EnumConst class.
BuildTree procedure scans system information by WinAPI.
Builds system information tree as linked list of nodes descriptors.
At application debug, this class not used.
At real system information show, this is child class of EnumConst class.
Special thanks to Microsoft enumerator sample:
https://github.com/microsoft/Windows-driver-samples/tree/main/setup/devcon
---------------------------------------------------------------------------------------- */

#pragma once
#ifndef ENUMWIN_H
#define ENUMWIN_H

#include <windows.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SetupAPI.h>
#include <cfgmgr32.h>
#include <strsafe.h>
#include <vector>
#include <algorithm>
#include "Global.h"
#include "EnumConst.h"

class EnumWin :
    public EnumConst
{
public:
    EnumWin();
    virtual ~EnumWin() override;
    virtual PTREENODE BuildTree(UINT mode) override;
    virtual void ReleaseTree() override;
private:
    static LPCSTR MAIN_SYSTEM_NAME;
    static int MAIN_SYSTEM_ICON_INDEX;
    static GROUPSORT sortControl[];
    static const UINT SORT_CONTROL_LENGTH;
    static GROUPSORT resourceControl[];
    static const UINT RESOURCE_CONTROL_LENGTH;
    static RESOURCESORT transitControl[];
    static const UINT TRANSIT_CONTROL_LENGTH;
    static LPSTR pEnumBase;

    UINT EnumerateSystem(LPTSTR pBase, UINT64 pMax,
        PGROUPSORT sControl, UINT sControlLength,
        PGROUPSORT rsControl, UINT rsControlLength,
        PRESOURCESORT trControl, UINT trControlLength);
    LPTSTR GetDeviceStringProperty(_In_ HDEVINFO Devs, _In_ PSP_DEVINFO_DATA DevInfo, _In_ DWORD Prop);
    LPTSTR GetDeviceDescription(_In_ HDEVINFO Devs, _In_ PSP_DEVINFO_DATA DevInfo);

    void EnumerateDeviceResourcesToTransitList(HDEVINFO Devs, PSP_DEVINFO_DATA DevInfo, LPCSTR devName,
        PRESOURCESORT trControl, UINT trControlLength);
    void EnumerateTransitListToGroupList(LPTSTR& pBase, UINT64& pMax,
        PRESOURCESORT trControl, UINT trControlLength, PGROUPSORT rsControl, UINT rsControlLength);
    BOOL DumpDeviceResources(_In_ HDEVINFO Devs, _In_ PSP_DEVINFO_DATA DevInfo,
        LPCSTR devName, PRESOURCESORT trControl);
    BOOL DumpDeviceResourcesOfType(_In_ DEVINST DevInst, _In_ HMACHINE MachineHandle, _In_ LOG_CONF Config, _In_ RESOURCEID ReqResId,
        LPCSTR devName, PRESOURCESORT trControl);
};

#endif  // ENUMWIN_H
