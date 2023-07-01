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

#include "EnumWin.h"

EnumWin::EnumWin()
{
    // Reserved functionality.
}
EnumWin::~EnumWin()
{
    // Reserved functionality.
}
PTREENODE EnumWin::BuildTree(UINT mode)
{
    // Select mode: devices tree or resources tree.
    PGROUPSORT tempControl = sortControl;
    UINT tempLength = SORT_CONTROL_LENGTH;
    if (mode)
    {
        tempControl = resourceControl;
        tempLength = RESOURCE_CONTROL_LENGTH;
        pTreeBaseBack = pTreeBase;
        pTreeBase = NULL;
    }

    // Build sequence of strings.
    pEnumBase = (LPSTR)malloc(SYSTEM_TREE_MEMORY_MAX);
    UINT countEnum = 0;
    if (mode)
    {
        countEnum = EnumerateSystem(pEnumBase, SYSTEM_TREE_MEMORY_MAX,
            sortControl, SORT_CONTROL_LENGTH,
            resourceControl, RESOURCE_CONTROL_LENGTH, transitControl, TRANSIT_CONTROL_LENGTH);
    }
    else
    {
        countEnum = EnumerateSystem(pEnumBase, SYSTEM_TREE_MEMORY_MAX,
            sortControl, SORT_CONTROL_LENGTH,
            NULL, 0, NULL, 0);
    }

    if (countEnum)
    {
        // Build root node - This computer.
        // pTreeBase = (PTREENODE)malloc((countEnum + 1) * sizeof(TREENODE));  // BUG: +1 because root, but required +X because classes.
        pTreeBase = (PTREENODE)malloc(SYSTEM_TREE_MEMORY_MAX);
        //
        PTREENODE p = pTreeBase;
        if (p)
        {
            // Build root node "This computer".
            p->hNodeIcon = pManageResources->GetIconHandleByIndex(MAIN_SYSTEM_ICON_INDEX);
            p->hClosedIcon = pManageResources->GetIconHandleByIndex(ID_CLOSED);
            p->hOpenedIcon = pManageResources->GetIconHandleByIndex(ID_OPENED);
            p->szNodeName = MAIN_SYSTEM_NAME;
            p->childLink = NULL;
            p->childCount = 0;
            p->clickArea.left = 0;
            p->clickArea.top = 0;
            p->clickArea.right = 0;
            p->clickArea.bottom = 0;
            p->openable = 0;
            p->opened = 0;
            p->marked = 1;
            p->prevMouse = 0;

            // Build tree level 1 - classes nodes, childs of tree nodes.
            PGROUPSORT pSortCtrl = tempControl;
            PTREENODE pClassBase = p + 1;
            UINT rootChilds = 0;
            for (UINT i = 0; i < tempLength; i++)
            {
                p++;
                p->hNodeIcon = pManageResources->GetIconHandleByIndex(pSortCtrl->iconIndex);
                p->hClosedIcon = pManageResources->GetIconHandleByIndex(ID_CLOSED);
                p->hOpenedIcon = pManageResources->GetIconHandleByIndex(ID_OPENED);
                p->szNodeName = pSortCtrl->groupName;
                p->childLink = NULL;
                p->childCount = 0;
                p->clickArea.left = 0;
                p->clickArea.top = 0;
                p->clickArea.right = 0;
                p->clickArea.bottom = 0;
                p->openable = 0;
                p->opened = 0;
                p->marked = 0;
                p->prevMouse = 0;
                pSortCtrl++;
                rootChilds++;
            }

            // Build tree level 2 - devices nodes, childs of classes nodes.
            pSortCtrl = tempControl;
            for (UINT i = 0; i < rootChilds; i++)
            {
                UINT classChilds = (UINT)(pSortCtrl->childStrings->size());
                if (classChilds)
                {
                    PTREENODE pDeviceBase = p + 1;
                    std::vector<LPCSTR>::iterator vit = pSortCtrl->childStrings->begin();
                    for (UINT j = 0; j < classChilds; j++)
                    {
                        p++;
                        p->hNodeIcon = pManageResources->GetIconHandleByIndex(pSortCtrl->iconIndex);
                        p->hClosedIcon = pManageResources->GetIconHandleByIndex(ID_CLOSED);
                        p->hOpenedIcon = pManageResources->GetIconHandleByIndex(ID_OPENED);

                        LPCSTR s = *vit++;
                        if (s)
                        {
                            p->szNodeName = s;
                        }
                        else
                        {
                            p->szNodeName = "?";
                        }

                        p->childLink = NULL;
                        p->childCount = 0;
                        p->clickArea.left = 0;
                        p->clickArea.top = 0;
                        p->clickArea.right = 0;
                        p->clickArea.bottom = 0;
                        p->openable = 0;
                        p->opened = 0;
                        p->marked = 0;
                        p->prevMouse = 0;
                    }

                    pClassBase->childLink = pDeviceBase;
                    pClassBase->childCount = classChilds;
                    pClassBase->openable = 1;
                }
                pClassBase++;
                pSortCtrl++;
            }

            // Update root node "This computer" after childs enumerated.
            if (rootChilds)
            {
                pTreeBase->childLink = pTreeBase + 1;
                pTreeBase->childCount = rootChilds;
                pTreeBase->openable = 1;
                pTreeBase->opened = 1;
            }
        }
    }
    return pTreeBase;
}
void EnumWin::ReleaseTree()
{
    if (pTreeBase)
    {
        free(pTreeBase);
        pTreeBase = NULL;
    }
    if (pTreeBaseBack)
    {
        free(pTreeBaseBack);
        pTreeBaseBack = NULL;
    }
    if (pEnumBase)
    {
        free(pEnumBase);
        pEnumBase = NULL;
    }
    PGROUPSORT p1 = sortControl;
    for (UINT i = 0; i < SORT_CONTROL_LENGTH; i++)
    {
        if (p1->childStrings)
        {
            delete(p1->childStrings);
            p1->childStrings = NULL;
            p1++;
        }
    }
    p1 = resourceControl;
    for (UINT i = 0; i < RESOURCE_CONTROL_LENGTH; i++)
    {
        if (p1->childStrings)
        {
            delete(p1->childStrings);
            p1->childStrings = NULL;
            p1++;
        }
    }
    PRESOURCESORT p2 = transitControl;
    for (UINT i = 0; i < TRANSIT_CONTROL_LENGTH; i++)
    {
        if (p2->childRanges)
        {
            delete(p2->childRanges);
            p2->childRanges = NULL;
            p2++;
        }
    }
}
LPCSTR EnumWin::MAIN_SYSTEM_NAME = "This computer";
int EnumWin::MAIN_SYSTEM_ICON_INDEX = ID_THIS_COMPUTER;

GROUPSORT EnumWin::sortControl[] = {
    { "HTREE"    , "System tree"               , ID_SYSTEM_TREE      , new std::vector<LPCSTR> },
    { "ROOT"     , "Root enumerator"           , ID_ROOT_ENUMERATOR  , new std::vector<LPCSTR> },
    { "SWD"      , "Software defined devices"  , ID_SOFT_DEVICES     , new std::vector<LPCSTR> },
    { "ACPI"     , "ACPI"                      , ID_ACPI             , new std::vector<LPCSTR> },
    { "ACPI_HAL" , "ACPI HAL"                  , ID_ACPI_HAL         , new std::vector<LPCSTR> },
    { "UEFI"     , "UEFI"                      , ID_UEFI             , new std::vector<LPCSTR> },
    { "SCSI"     , "SCSI"                      , ID_SCSI             , new std::vector<LPCSTR> },
    { "STORAGE"  , "Mass storage"              , ID_MASS_STORAGE     , new std::vector<LPCSTR> },
    { "HID"      , "Human interface devices"   , ID_HID              , new std::vector<LPCSTR> },
    { "PCI"      , "PCI"                       , ID_PCI              , new std::vector<LPCSTR> },
    { "USB"      , "USB"                       , ID_USB              , new std::vector<LPCSTR> },
    { "USBSTOR"  , "USB mass storage"          , ID_USB_STORAGE      , new std::vector<LPCSTR> },
    { "BTH"      , "Bluetooth"                 , ID_BLUETOOTH        , new std::vector<LPCSTR> },
    { "DISPLAY"  , "Video displays"            , ID_DISPLAYS         , new std::vector<LPCSTR> },
    { "HDAUDIO"  , "High definition audio"     , ID_AUDIO            , new std::vector<LPCSTR> },
    { "UMB"      , "User mode bus"             , ID_UM_BUS           , new std::vector<LPCSTR> },
    { "IDE"      , "IDE/ATAPI controllers"     , ID_IDE              , new std::vector<LPCSTR> },
    { "PCIIDE"   , "PCI IDE/ATAPI controllers" , ID_PCI_IDE          , new std::vector<LPCSTR> },
    { "OTHER"    , "Other devices types"       , ID_OTHER            , new std::vector<LPCSTR> }
};
const UINT EnumWin::SORT_CONTROL_LENGTH = sizeof(sortControl) / sizeof(GROUPSORT);

GROUPSORT EnumWin::resourceControl[] = {
    { NULL       , "Memory"                    , ID_RES_MEMORY       , new std::vector<LPCSTR> },
    { NULL       , "Large memory"              , ID_RES_LARGE_MEMORY , new std::vector<LPCSTR> },
    { NULL       , "IO"                        , ID_RES_IO           , new std::vector<LPCSTR> },
    { NULL       , "IRQ"                       , ID_RES_IRQ          , new std::vector<LPCSTR> },
    { NULL       , "DMA"                       , ID_RES_DMA          , new std::vector<LPCSTR> }
};
const UINT EnumWin::RESOURCE_CONTROL_LENGTH = sizeof(resourceControl) / sizeof(GROUPSORT);

RESOURCESORT EnumWin::transitControl[] = {
    { new std::vector<RESOURCEENTRY> },
    { new std::vector<RESOURCEENTRY> },
    { new std::vector<RESOURCEENTRY> },
    { new std::vector<RESOURCEENTRY> },
    { new std::vector<RESOURCEENTRY> }
};
const UINT EnumWin::TRANSIT_CONTROL_LENGTH = sizeof(transitControl) / sizeof(RESOURCESORT);

LPSTR EnumWin::pEnumBase = NULL;

// Enumerator.

UINT EnumWin::EnumerateSystem(LPTSTR pBase, UINT64 pMax,
    PGROUPSORT sControl, UINT sControlLength,
    PGROUPSORT rsControl, UINT rsControlLength,
    PRESOURCESORT trControl, UINT trControlLength)
{
    int enumCount = 0;
    HDEVINFO hDevinfo = SetupDiGetClassDevsEx(NULL, NULL, NULL, DIGCF_PRESENT | DIGCF_ALLCLASSES, NULL, NULL, NULL);
    if ((hDevinfo) && (hDevinfo != INVALID_HANDLE_VALUE))
    {
        SP_DEVINFO_LIST_DETAIL_DATA devInfoListDetail;
        devInfoListDetail.cbSize = sizeof(devInfoListDetail);
        if (SetupDiGetDeviceInfoListDetail(hDevinfo, &devInfoListDetail))
        {
            SP_DEVINFO_DATA devInfo;
            devInfo.cbSize = sizeof(devInfo);
            for (DWORD devIndex = 0; SetupDiEnumDeviceInfo(hDevinfo, devIndex, &devInfo); devIndex++)
            {
                TCHAR devID[MAX_DEVICE_ID_LEN];
                BOOL b = TRUE;
                SP_DEVINFO_LIST_DETAIL_DATA devInfoListDetail;

                // Get device enumeration path string.
                devInfoListDetail.cbSize = sizeof(devInfoListDetail);
                if ((!SetupDiGetDeviceInfoListDetail(hDevinfo, &devInfoListDetail)) ||
                    (CM_Get_Device_ID_Ex(devInfo.DevInst, devID, MAX_DEVICE_ID_LEN, 0, devInfoListDetail.RemoteMachineHandle) != CR_SUCCESS))
                {
                    StringCchCopy(devID, ARRAYSIZE(devID), TEXT("?"));
                    b = FALSE;  // this flag yet redundant
                }

                LPCSTR pathString = NULL;
                LPCSTR nameString = NULL;
                LPCSTR summaryString = NULL;

                // Copy device enumeration path string.
                size_t length = strlen(devID);
                strncpy_s(pBase, (size_t)pMax, devID, length);
                pathString = pBase;
                pBase += (length + 1);
                pMax -= (length + 1);
                if (pMax <= 0) break;

                // Get device friendly description string (name).
                LPTSTR descr = GetDeviceDescription(hDevinfo, &devInfo);
                if (descr)
                {
                    // Copy device friendly description string.
                    size_t length = strlen(descr);
                    strncpy_s(pBase, (size_t)pMax, descr, length);
                    nameString = pBase;
                    pBase += (length + 1);
                    pMax -= (length + 1);
                    delete[] descr;
                }
                if (pMax <= 0) break;

                // Build and copy summary (name and path) string.
                UINT adv = 0;
                if (nameString && pathString)
                {
                    adv = snprintf(pBase, (size_t)pMax, "%s ( %s )", nameString, pathString);
                }
                else if (nameString)
                {
                    adv = snprintf(pBase, (size_t)pMax, "%s", nameString);
                }
                else if (pathString)
                {
                    adv = snprintf(pBase, (size_t)pMax, "%s", pathString);
                }
                else
                {
                    adv = snprintf(pBase, (size_t)pMax, "< No description strings >");
                }
                // Save pointer to description string
                summaryString = pBase;

                // Conditionally run resource usage enumeration agent
                if (trControl && trControlLength)
                {
                    EnumerateDeviceResourcesToTransitList(hDevinfo, &devInfo, summaryString, trControl, trControlLength);
                }

                // Modify pointer and size limit, add 0 termination char.
                pBase += adv;
                pMax -= adv;
                if (pMax <= 0) break;
                *pBase = 0;
                pBase++;
                pMax--;
                if (pMax <= 0) break;

                // Classify string pair entry and add to sort control list.
                if (pathString)
                {
                    PGROUPSORT p = sControl;
                    BOOL recognized = FALSE;
                    for (UINT i = 0; i < (sControlLength - 1); i++)  // -1 because last entry is "OTHER"
                    {
                        // Build pattern for comparision.
                        TCHAR pattern[MAX_DEVICE_ID_LEN + 2];
                        LPTSTR patternTemp = pattern;
                        size_t patternLength = strlen(p->groupPattern);
                        strncpy_s(patternTemp, MAX_DEVICE_ID_LEN, p->groupPattern, patternLength);
                        patternTemp += patternLength;
                        *(patternTemp++) = '\\';
                        *(patternTemp++) = 0;
                        // Compare pathString and pattern for detect device class.
                        patternLength++;
                        if ((strlen(pathString) > patternLength) && (!strncmp(pathString, pattern, patternLength)))
                        {
                            // This fragment executed if strings match. Add device name to recognized group.
                            p->childStrings->push_back(summaryString);
                            recognized = TRUE;
                            break;
                        }
                        p++;
                    }
                    if (!recognized)

                    {   // This fragment executed if all strings mismatch, classify as "OTHER" device.
                        // Add device name to "OTHER" group.
                        p->childStrings->push_back(summaryString);
                    }
                    enumCount++;
                }
            }

            // Terminate sequence of strings.
            if (pMax < 0)
            {
                enumCount = 0;  // Invalidate results if buffer overflows.
            }
        }
        if (!SetupDiDestroyDeviceInfoList(hDevinfo)) enumCount = 0;
    }
    // Conditionally run translation resource usage enumeration agent results
    // to resource tree map, sequence of GROUPSORT structures with
    // Memory, Large memory, IO, IRQ, DMA.

    if (trControl && trControlLength && rsControl && rsControlLength)
    {
        EnumerateTransitListToGroupList(pBase, pMax, trControl, trControlLength, rsControl, rsControlLength);
        // Terminate sequence of strings.
        if (pMax < 0)
        {
            enumCount = 0;  // Invalidate results if buffer overflows.
        }
    }

    return enumCount;
}
LPTSTR EnumWin::GetDeviceDescription(_In_ HDEVINFO Devs, _In_ PSP_DEVINFO_DATA DevInfo)
/*++

Routine Description:

    Return a string containing a description of the device, otherwise NULL
    Always try friendly name first

Arguments:

    Devs    )_ uniquely identify device
    DevInfo )

Return Value:

    string containing description

--*/

{
    LPTSTR desc;
    desc = GetDeviceStringProperty(Devs, DevInfo, SPDRP_FRIENDLYNAME);
    if (!desc) {
        desc = GetDeviceStringProperty(Devs, DevInfo, SPDRP_DEVICEDESC);
    }
    return desc;
}
LPTSTR EnumWin::GetDeviceStringProperty(_In_ HDEVINFO Devs, _In_ PSP_DEVINFO_DATA DevInfo, _In_ DWORD Prop)
/*++

Routine Description:

    Return a string property for a device, otherwise NULL

Arguments:

    Devs    )_ uniquely identify device
    DevInfo )
    Prop     - string property to obtain

Return Value:

    string containing description

--*/

{
    LPTSTR buffer;
    DWORD size;
    DWORD reqSize;
    DWORD dataType;
    DWORD szChars;

    size = 1024; // initial guess
    buffer = new TCHAR[(size / sizeof(TCHAR)) + 1];
    if (!buffer) {
        return NULL;
    }
    while (!SetupDiGetDeviceRegistryProperty(Devs, DevInfo, Prop, &dataType, (LPBYTE)buffer, size, &reqSize)) {
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            goto failed;
        }
        if (dataType != REG_SZ) {
            goto failed;
        }
        size = reqSize;
        delete[] buffer;
        buffer = new TCHAR[(size / sizeof(TCHAR)) + 1];
        if (!buffer) {
            goto failed;
        }
    }
    szChars = reqSize / sizeof(TCHAR);
    buffer[szChars] = TEXT('\0');
    return buffer;

failed:
    if (buffer) {
        delete[] buffer;
    }
    return NULL;
}
void EnumWin::EnumerateDeviceResourcesToTransitList(HDEVINFO Devs, PSP_DEVINFO_DATA DevInfo, LPCSTR devName, PRESOURCESORT trControl, UINT trControlLength)
{
    if (trControl && (trControlLength >= RES_MAX))
    {
        // Return BOOL value yet not used, resource not logged if failure.
        DumpDeviceResources(Devs, DevInfo, devName, trControl);
    }
}
void EnumWin::EnumerateTransitListToGroupList(LPTSTR& pBase, UINT64& pMax,
    PRESOURCESORT trControl, UINT trControlLength, PGROUPSORT rsControl, UINT rsControlLength)
{
    if (trControl && (trControlLength >= RES_MAX) && rsControl && (rsControlLength >= RES_MAX))
    {
        // Cycle for resource types, i = resource type, select one af resource lists (MEM, LARGE MEM, IO, IRQ, DMA).
        for (int i = 0; i < RES_MAX; i++)
        {
            if (pMax <= 0) break;
            // Source data: get resource list structure from array of lists (MEM, LARGE MEM, IO, IRQ, DMA).
            RESOURCESORT rs = trControl[i];
            // Sorting resource list with comparator as lambda expression.
            std::sort(rs.childRanges->begin(), rs.childRanges->end(),
                [](RESOURCEENTRY x, RESOURCEENTRY y)
                {
                    bool flag;
            if (x.dataL == y.dataL)  // compare start address
            {   // variant with end address if start address match
                flag = x.dataH < y.dataH;
            }
            else
            {   // variant with start address if it differrent
                flag = x.dataL < y.dataL;
            }
            return flag;
                });
            // Get size of resource list and iterator.
            size_t n = rs.childRanges->size();
            std::vector<RESOURCEENTRY>::iterator vit = rs.childRanges->begin();
            // Destination data: get node structure from tree control structures array.
            GROUPSORT gs = rsControl[i];
            // Cycle for resources ranges per type, j = index in the resources lists per each type.
            for (size_t j = 0; j < n; j++)
            {
                RESOURCEENTRY re = *vit++;
                UINT adv = 0;
                int a;
                switch (i)
                {
                case RES_MEM:
                    adv = snprintf(pBase, (size_t)pMax, "%08I64Xh-%08I64Xh : %s", re.dataL, re.dataH, re.deviceName);
                    gs.childStrings->push_back(pBase);
                    break;
                case RES_LMEM:
                    adv = snprintf(pBase, (size_t)pMax, "%016I64Xh-%016I64Xh : %s", re.dataL, re.dataH, re.deviceName);
                    gs.childStrings->push_back(pBase);
                    break;
                case RES_IO:
                    adv = snprintf(pBase, (size_t)pMax, "%04I64Xh-%04I64Xh : %s", re.dataL, re.dataH, re.deviceName);
                    gs.childStrings->push_back(pBase);
                    break;
                case RES_IRQ:
                    a = (int)re.dataL;
                    adv = snprintf(pBase, (size_t)pMax, "%d : %s", a, re.deviceName);
                    gs.childStrings->push_back(pBase);
                    break;
                case RES_DMA:
                    a = (int)re.dataL;
                    adv = snprintf(pBase, (size_t)pMax, "%d : %s", a, re.deviceName);
                    gs.childStrings->push_back(pBase);
                    break;
                default:
                    break;
                }
                // Modify pointer and size limit, add 0 termination char.
                pBase += adv;
                pMax -= adv;
                if (pMax <= 0) break;
                *pBase = 0;
                pBase++;
                pMax--;
                if (pMax <= 0) break;
            }
        }
    }
}
BOOL EnumWin::DumpDeviceResources(_In_ HDEVINFO Devs, _In_ PSP_DEVINFO_DATA DevInfo,
    LPCSTR devName, PRESOURCESORT trControl)
    /*++

    Routine Description:

        Dump Resources to stdout

    Arguments:

        Devs    )_ uniquely identify device
        DevInfo )

    Return Value:

        none

    --*/
{
    SP_DEVINFO_LIST_DETAIL_DATA devInfoListDetail;
    ULONG status = 0;
    ULONG problem = 0;
    LOG_CONF config = 0;
    BOOL haveConfig = FALSE;

    //
    // see what state the device is in
    //
    devInfoListDetail.cbSize = sizeof(devInfoListDetail);
    if ((!SetupDiGetDeviceInfoListDetail(Devs, &devInfoListDetail)) ||
        (CM_Get_DevNode_Status_Ex(&status, &problem, DevInfo->DevInst, 0, devInfoListDetail.RemoteMachineHandle) != CR_SUCCESS)) {
        return FALSE;
    }

    //
    // see if the device is running and what resources it might be using
    //
    if (!(status & DN_HAS_PROBLEM)) {
        //
        // If this device is running, does this devinst have a ALLOC log config?
        //
        if (CM_Get_First_Log_Conf_Ex(&config,
            DevInfo->DevInst,
            ALLOC_LOG_CONF,
            devInfoListDetail.RemoteMachineHandle) == CR_SUCCESS) {
            haveConfig = TRUE;
        }
    }
    if (!haveConfig) {
        //
        // If no config so far, does it have a FORCED log config?
        // (note that technically these resources might be used by another device
        // but is useful info to show)
        //
        if (CM_Get_First_Log_Conf_Ex(&config,
            DevInfo->DevInst,
            FORCED_LOG_CONF,
            devInfoListDetail.RemoteMachineHandle) == CR_SUCCESS) {
            haveConfig = TRUE;
        }
    }

    if (!haveConfig) {
        //
        // if there's a hardware-disabled problem, boot-config isn't valid
        // otherwise use this if we don't have anything else
        //
        if (!(status & DN_HAS_PROBLEM) || (problem != CM_PROB_HARDWARE_DISABLED)) {
            //
            // Does it have a BOOT log config?
            //
            if (CM_Get_First_Log_Conf_Ex(&config,
                DevInfo->DevInst,
                BOOT_LOG_CONF,
                devInfoListDetail.RemoteMachineHandle) == CR_SUCCESS) {
                haveConfig = TRUE;
            }
        }
    }

    if (!haveConfig) {
        //
        // if we don't have any configuration, display an apropriate message
        //
/*
        Padding(1);
        FormatToStream(stdout, (status & DN_STARTED) ? MSG_DUMP_NO_RESOURCES : MSG_DUMP_NO_RESERVED_RESOURCES);
*/
        return TRUE;
    }
    /*
        Padding(1);
        FormatToStream(stdout, (status & DN_STARTED) ? MSG_DUMP_RESOURCES : MSG_DUMP_RESERVED_RESOURCES);
    */
    //
    // dump resources
    //
    DumpDeviceResourcesOfType(DevInfo->DevInst, devInfoListDetail.RemoteMachineHandle, config, ResType_All,
        devName, trControl);

    //
    // release handle
    //
    CM_Free_Log_Conf_Handle(config);

    return TRUE;
}
BOOL EnumWin::DumpDeviceResourcesOfType(_In_ DEVINST DevInst, _In_ HMACHINE MachineHandle, _In_ LOG_CONF Config, _In_ RESOURCEID ReqResId,
    LPCSTR devName, PRESOURCESORT trControl)
{
    RES_DES prevResDes = (RES_DES)Config;
    RES_DES resDes = 0;
    RESOURCEID resId = ReqResId;
    ULONG dataSize;
    PBYTE resDesData;
    BOOL  retval = FALSE;

    UNREFERENCED_PARAMETER(DevInst);

    while (CM_Get_Next_Res_Des_Ex(&resDes, prevResDes, ReqResId, &resId, 0, MachineHandle) == CR_SUCCESS) {
        if (prevResDes != Config) {
            CM_Free_Res_Des_Handle(prevResDes);
        }
        prevResDes = resDes;
        if (CM_Get_Res_Des_Data_Size_Ex(&dataSize, resDes, 0, MachineHandle) != CR_SUCCESS) {
            continue;
        }
        resDesData = new BYTE[dataSize];
        if (!resDesData) {
            continue;
        }
        if (CM_Get_Res_Des_Data_Ex(resDes, resDesData, dataSize, 0, MachineHandle) != CR_SUCCESS) {
            delete[] resDesData;
            continue;
        }
        switch (resId)
        {
        case ResType_Mem:
        {
            PMEM_RESOURCE  pMemData = (PMEM_RESOURCE)resDesData;
            if (pMemData->MEM_Header.MD_Alloc_End - pMemData->MEM_Header.MD_Alloc_Base + 1)
            {
                // Padding(2);
                // _tprintf(TEXT("MEM : %08I64x-%08I64x\n"), pMemData->MEM_Header.MD_Alloc_Base, pMemData->MEM_Header.MD_Alloc_End);
                //
                DWORDLONG base = pMemData->MEM_Header.MD_Alloc_Base;
                DWORDLONG end = pMemData->MEM_Header.MD_Alloc_End;
                DWORDLONG limit = 0xFFFFFFFFL;
                int index = RES_MEM;
                if ((base > limit) || (end > limit)) index = RES_LMEM;
                RESOURCEENTRY e;
                e.deviceName = devName;
                e.dataL = base;
                e.dataH = end;
                trControl[index].childRanges->push_back(e);
                //
                retval = TRUE;
            }
            break;
        }
        case ResType_MemLarge:
        {
            PMEM_LARGE_RESOURCE  pMemData = (PMEM_LARGE_RESOURCE)resDesData;
            DWORDLONG base = pMemData->MEM_LARGE_Header.MLD_Alloc_Base;
            DWORDLONG end = pMemData->MEM_LARGE_Header.MLD_Alloc_End;
            DWORDLONG limit = 0xFFFFFFFFL;
            int index = RES_MEM;
            if ((base > limit) || (end > limit)) index = RES_LMEM;
            RESOURCEENTRY e;
            e.deviceName = devName;
            e.dataL = base;
            e.dataH = end;
            trControl[index].childRanges->push_back(e);
            break;
        }

        case ResType_IO:
        {
            PIO_RESOURCE   pIoData = (PIO_RESOURCE)resDesData;
            if (pIoData->IO_Header.IOD_Alloc_End - pIoData->IO_Header.IOD_Alloc_Base + 1)
            {
                // Padding(2);
                // _tprintf(TEXT("IO  : %04I64x-%04I64x\n"), pIoData->IO_Header.IOD_Alloc_Base, pIoData->IO_Header.IOD_Alloc_End);
                //
                DWORDLONG base = pIoData->IO_Header.IOD_Alloc_Base;
                DWORDLONG end = pIoData->IO_Header.IOD_Alloc_End;
                DWORDLONG limit = 0xFFFFFFFFL;
                int index = RES_IO;
                if (pIoData->IO_Header.IOD_DesFlags & fIOD_Memory)
                {
                    index = RES_MEM;
                    if ((base > limit) || (end > limit)) index = RES_LMEM;
                }
                RESOURCEENTRY e;
                e.deviceName = devName;
                e.dataL = base;
                e.dataH = end;
                trControl[index].childRanges->push_back(e);
                //
                retval = TRUE;
            }
            break;
        }

        case ResType_DMA:
        {
            PDMA_RESOURCE  pDmaData = (PDMA_RESOURCE)resDesData;
            // Padding(2);
            // _tprintf(TEXT("DMA : %u\n"), pDmaData->DMA_Header.DD_Alloc_Chan);
            //
            DWORDLONG dma = pDmaData->DMA_Header.DD_Alloc_Chan;
            RESOURCEENTRY e;
            e.deviceName = devName;
            e.dataL = dma;
            e.dataH = 0;
            trControl[RES_DMA].childRanges->push_back(e);
            //
            retval = TRUE;
            break;
        }

        case ResType_IRQ:
        {
            PIRQ_RESOURCE  pIrqData = (PIRQ_RESOURCE)resDesData;
            // Padding(2);
            // _tprintf(TEXT("IRQ : %u\n"), pIrqData->IRQ_Header.IRQD_Alloc_Num);
            //
            DWORDLONG irq = pIrqData->IRQ_Header.IRQD_Alloc_Num;
            RESOURCEENTRY e;
            e.deviceName = devName;
            e.dataL = irq;
            e.dataH = 0;
            trControl[RES_IRQ].childRanges->push_back(e);
            //
            retval = TRUE;
            break;
        }
        }
        delete[] resDesData;
    }
    if (prevResDes != Config)
    {
        CM_Free_Res_Des_Handle(prevResDes);
    }
    return retval;
}
