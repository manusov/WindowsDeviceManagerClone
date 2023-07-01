/* ----------------------------------------------------------------------------------------
Class for load application resources (icons, strings tables, dialogs, menus, toolbars,
dialogs and other resource types). Support get resource handles by resource IDs and
indexes at application runtime.
Contains tree data model storage:
icons, text strings, structures with tree nodes descriptors. Include storage for 
visualized model.
---------------------------------------------------------------------------------------- */

#include "ManageResources.h"

ManageResources::ManageResources()
{
	HICON hIcon = NULL;
	HMODULE hModule = GetModuleHandle(NULL);
	for (int i = 0; i < ICON_COUNT; i++)
	{
		hIcon = (HICON)LoadImage(hModule, MAKEINTRESOURCE(ICON_IDS[i]),
			IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
		iconHandles[i] = hIcon;
		if (!hIcon) break;
	}
	initStatus = (hIcon != NULL);
}
ManageResources::~ManageResources()
{
	for (int i = 0; i < ICON_COUNT; i++) { if (iconHandles[i]) DestroyIcon(iconHandles[i]); }
}
UINT ManageResources::GetIconIdByIndex(int index)
{
	UINT id = 0;
	if (index < ICON_COUNT)
	{
		id = ICON_IDS[index];
	}
	return id;
}
HICON ManageResources::GetIconHandleByIndex(int index)
{
	HICON hIcon = NULL;
	if (index < ICON_COUNT)
	{
		hIcon = iconHandles[index];
	}
	return hIcon;
}
LPCSTR ManageResources::GetIconNameByIndex(int index)
{
	LPCSTR name = NULL;
	if (index < ICON_COUNT)
	{
		name = ICON_NAMES[index];
	}
	return name;
}
PTREENODE* ManageResources::GetTrees()
{
	return trees;
}
void ManageResources::SetTrees(PTREENODE* pt)
{
	trees = pt;
}
POINT ManageResources::GetBase()
{
	return base;
}
void ManageResources::SetBase(POINT b)
{
	base = b;
}
PTREECHILDS ManageResources::getNodeChilds()
{
	return nodeChilds;
}
int ManageResources::getNodeChildsCount()
{
	return NODE_CHILDS_COUNT;
}
BOOL ManageResources::getInitStatus()
{
	return initStatus;
}

const char* ManageResources::ICON_NAMES[] =
{
	"Application icon",
	"Item closed", "Item closed light", "Item opened", "Item opened light",
	"ACPI", "ACPI HAL", "Audio controllers", "Audio inputs and outputs", "Bluetooth", "Video displays", "Human interface devices", "IDE controllers",
	"Image processing devices", "Keyboards", "Mass storage devices", "Mobile devices", "Mouses",
	"Network controllers", "Other devices", "PCI", "PCI IDE controllers", "COM and LPT ports", "Print queues", "Processors",
	"Root enumerator", "SCSI controllers", "Security devices", "Software components", "Software defined devices",
	"System tree", "This computer", "UEFI", "User mode bus enumerator", "USB", "USB mass storage devices",
	"Video adapters",
	"Memory", "Large memory", "IO", "IRQ", "DMA"
};
const int ManageResources::ICON_IDS[] =
{
	IDI_APP,
	IDI_CLOSED, IDI_CLOSED_LIGHT, IDI_OPENED, IDI_OPENED_LIGHT,
	IDI_ACPI, IDI_ACPI_HAL, IDI_AUDIO, IDI_AUDIO_IO, IDI_BLUETOOTH, IDI_DISPLAYS, IDI_HID, IDI_IDE,
	IDI_IMAGE_PROCESSING, IDI_KEYBOARDS, IDI_MASS_STORAGE, IDI_MOBILE_DEVICES, IDI_MOUSES,
	IDI_NETWORK, IDI_OTHER, IDI_PCI, IDI_PCI_IDE, IDI_PORTS, IDI_PRINT, IDI_PROCESSORS,
	IDI_ROOT_ENUMERATOR, IDI_SCSI, IDI_SECURITY, IDI_SOFT_COMPONENTS, IDI_SOFT_DEVICES,
	IDI_SYSTEM_TREE, IDI_THIS_COMPUTER, IDI_UEFI, IDI_UM_BUS, IDI_USB, IDI_USB_STORAGE,
	IDI_VIDEO_ADAPTERS,
	IDI_RES_MEMORY, IDI_RES_LARGE_MEMORY, IDI_RES_IO, IDI_RES_IRQ, IDI_RES_DMA
};
const int ManageResources::ICON_COUNT = sizeof(ICON_IDS) / sizeof(int);
HICON ManageResources::iconHandles[ICON_COUNT];
PTREENODE* ManageResources::trees = NULL;
POINT ManageResources::base = { 0, 0 };
// Child nodes descriptions per each category, for emulated constant tree parameters.
LPCSTR STRINGS_BLUETOOTH[] = { "Intel(R) Wireless Bluetooth", "Microsoft Bluetooth enumerator", "Some Bluetooth adapter" };
int ICONS_BLUETOOTH[] = { ID_BLUETOOTH, ID_BLUETOOTH, ID_BLUETOOTH };
int COUNT_BLUETOOTH = sizeof(ICONS_BLUETOOTH) / sizeof(int);
LPCSTR STRINGS_AUDIOINOUT[] = { "Speakers", "Microphone" };
int ICONS_AUDIOINOUT[] = { ID_AUDIO_IO, ID_AUDIO_IO };
int COUNT_AUDIOINOUT = sizeof(ICONS_AUDIOINOUT) / sizeof(int);
LPCSTR STRINGS_VIDEOADAPTER[] = { "GTX850", "RTX3060Ti" };
int ICONS_VIDEOADAPTER[] = { ID_VIDEO_ADAPTERS, ID_VIDEO_ADAPTERS };
int COUNT_VIDEOADAPTER = sizeof(ICONS_VIDEOADAPTER) / sizeof(int);
LPCSTR STRINGS_EMBEDDEDSOFT[] = { "AMI UEFI firmware", "UEFI Video Driver" };
int ICONS_EMBEDDEDSOFT[] = { ID_UEFI, ID_UEFI };
int COUNT_EMBEDDEDSOFT = sizeof(ICONS_EMBEDDEDSOFT) / sizeof(int);
LPCSTR STRINGS_DISKDRIVE[] = { "Samsung", "Toshiba" };
int ICONS_DISKDRIVE[] = { ID_SCSI, ID_SCSI };
int COUNT_DISKDRIVE = sizeof(ICONS_DISKDRIVE) / sizeof(int);
LPCSTR STRINGS_AUDIO[] = { "HD Webcam", "High Definition Audio", "NVIDIA WDM" };
int ICONS_AUDIO[] = { ID_AUDIO, ID_AUDIO, ID_AUDIO };
int COUNT_AUDIO = sizeof(ICONS_AUDIO) / sizeof(int);
LPCSTR STRINGS_KEYBOARD[] = { "Keyboard" };
int ICONS_KEYBOARD[] = { ID_KEYBOARDS };
int COUNT_KEYBOARD = sizeof(ICONS_KEYBOARD) / sizeof(int);
LPCSTR STRINGS_SOFTCOMPONENTS[] = { "Intel WiFi" };
int ICONS_SOFTCOMPONENTS[] = { ID_SOFT_COMPONENTS };
int COUNT_SOFTCOMPONENTS = sizeof(ICONS_SOFTCOMPONENTS) / sizeof(int);
LPCSTR STRINGS_COMPUTER[] = { "x64-based computer with ACPI" };
int ICONS_COMPUTER[] = { ID_THIS_COMPUTER };
int COUNT_COMPUTER = sizeof(ICONS_COMPUTER) / sizeof(int);
LPCSTR STRINGS_IDECTRL[] = { "Standard SATA AHCI" };
int ICONS_IDECTRL[] = { ID_IDE };
int COUNT_IDECTRL = sizeof(ICONS_IDECTRL) / sizeof(int);
LPCSTR STRINGS_USBCTRL[] = { "USB3 xHCI", "USB Root Hub" };
int ICONS_USBCTRL[] = { ID_USB, ID_USB };
int COUNT_USBCTRL = sizeof(ICONS_USBCTRL) / sizeof(int);
LPCSTR STRINGS_STORAGECTRL[] = { "Microsoft storage" };
int ICONS_STORAGECTRL[] = { ID_MASS_STORAGE };
int COUNT_STORAGECTRL = sizeof(ICONS_STORAGECTRL) / sizeof(int);
LPCSTR STRINGS_MONITOR[] = { "ASUS" };
int ICONS_MONITOR[] = { ID_DISPLAYS };
int COUNT_MONITOR = sizeof(ICONS_MONITOR) / sizeof(int);
LPCSTR STRINGS_MOUSE[] = { "Mouse" };
int ICONS_MOUSE[] = { ID_MOUSES };
int COUNT_MOUSE = sizeof(ICONS_MOUSE) / sizeof(int);
LPCSTR STRINGS_PRINTER[] = { "Canon Pixma IP2700" };
int ICONS_PRINTER[] = { ID_PRINT };
int COUNT_PRINTER = sizeof(ICONS_PRINTER) / sizeof(int);
LPCSTR STRINGS_MOBILEDEVICES[] = { "USB Flash Drive" };
int ICONS_MOBILEDEVICES[] = { ID_MOBILE_DEVICES };
int COUNT_MOBILEDEVICES = sizeof(ICONS_MOBILEDEVICES) / sizeof(int);
LPCSTR STRINGS_PORTS[] = { "COM1", "COM2", "LPT" };
int ICONS_PORTS[] = { ID_PORTS, ID_PORTS, ID_PORTS };
int COUNT_PORTS = sizeof(ICONS_PORTS) / sizeof(int);
LPCSTR STRINGS_SOFTDEVICES[] = { "Microsoft Root Enumerator" };
int ICONS_SOFTDEVICES[] = { ID_SOFT_DEVICES };
int COUNT_SOFTDEVICES = sizeof(ICONS_SOFTDEVICES) / sizeof(int);
LPCSTR STRINGS_PROCESSOR[] = { "Intel Core i7", "Intel Core i7", "Intel Core i7", "Intel Core i7" };
int ICONS_PROCESSOR[] = { ID_PROCESSORS, ID_PROCESSORS, ID_PROCESSORS, ID_PROCESSORS };
int COUNT_PROCESSOR = sizeof(ICONS_PROCESSOR) / sizeof(int);
LPCSTR STRINGS_NETWORK[] = { "Realtek LAN" };
int ICONS_NETWORK[] = { ID_NETWORK };
int COUNT_NETWORK = sizeof(ICONS_NETWORK) / sizeof(int);
LPCSTR STRINGS_SYSTEM[] = { "DMA", "IRQ", "Timer", "RTC" };
int ICONS_SYSTEM[] = { ID_SYSTEM_TREE, ID_SYSTEM_TREE, ID_SYSTEM_TREE, ID_SYSTEM_TREE };
int COUNT_SYSTEM = sizeof(ICONS_SYSTEM) / sizeof(int);
LPCSTR STRINGS_HID[] = { "Input device" };
int ICONS_HID[] = { ID_HID };
int COUNT_HID = sizeof(ICONS_HID) / sizeof(int);
LPCSTR STRINGS_USBDEVICES[] = { "Webcam", "Mouse", "Printer" };
int ICONS_USBDEVICES[] = { ID_USB_STORAGE, ID_USB_STORAGE, ID_USB_STORAGE };
int COUNT_USBDEVICES = sizeof(ICONS_USBDEVICES) / sizeof(int);
LPCSTR STRINGS_SECURITY[] = { "Trust Platform Module TPM 2.0" };
int ICONS_SECURITY[] = { ID_SECURITY };
int COUNT_SECURITY = sizeof(ICONS_SECURITY) / sizeof(int);
LPCSTR STRINGS_IMAGEDEVICES[] = { "Logitech HD Webcam C310" };
int ICONS_IMAGEDEVICES[] = { ID_IMAGE_PROCESSING };
int COUNT_IMAGEDEVICES = sizeof(ICONS_IMAGEDEVICES) / sizeof(int);
// Directory of child nodes per each category, for emulated constant tree parameters.
TREECHILDS ManageResources::nodeChilds[] = {
	{ COUNT_BLUETOOTH      , ICONS_BLUETOOTH     , STRINGS_BLUETOOTH      } ,
	{ COUNT_AUDIOINOUT     , ICONS_AUDIOINOUT    , STRINGS_AUDIOINOUT     } ,
	{ COUNT_VIDEOADAPTER   , ICONS_VIDEOADAPTER  , STRINGS_VIDEOADAPTER   } ,
	{ COUNT_EMBEDDEDSOFT   , ICONS_EMBEDDEDSOFT  , STRINGS_EMBEDDEDSOFT   } ,
	{ COUNT_DISKDRIVE      , ICONS_DISKDRIVE     , STRINGS_DISKDRIVE      } ,
	{ COUNT_AUDIO          , ICONS_AUDIO         , STRINGS_AUDIO          } ,
	{ COUNT_KEYBOARD       , ICONS_KEYBOARD      , STRINGS_KEYBOARD       } ,
	{ COUNT_SOFTCOMPONENTS , ICONS_SOFTCOMPONENTS, STRINGS_SOFTCOMPONENTS } ,
	{ COUNT_COMPUTER       , ICONS_COMPUTER      , STRINGS_COMPUTER       } ,
	{ COUNT_IDECTRL        , ICONS_IDECTRL       , STRINGS_IDECTRL        } ,
	{ COUNT_USBCTRL        , ICONS_USBCTRL       , STRINGS_USBCTRL        } ,
	{ COUNT_STORAGECTRL    , ICONS_STORAGECTRL   , STRINGS_STORAGECTRL    } ,
	{ COUNT_MONITOR        , ICONS_MONITOR       , STRINGS_MONITOR        } ,
	{ COUNT_MOUSE          , ICONS_MOUSE         , STRINGS_MOUSE          } ,
	{ COUNT_PRINTER        , ICONS_PRINTER       , STRINGS_PRINTER        } ,
	{ COUNT_MOBILEDEVICES  , ICONS_MOBILEDEVICES , STRINGS_MOBILEDEVICES  } ,
	{ COUNT_PORTS          , ICONS_PORTS         , STRINGS_PORTS          } ,
	{ COUNT_SOFTDEVICES    , ICONS_SOFTDEVICES   , STRINGS_SOFTDEVICES    } ,
	{ COUNT_PROCESSOR      , ICONS_PROCESSOR     , STRINGS_PROCESSOR      } ,
	{ COUNT_NETWORK        , ICONS_NETWORK       , STRINGS_NETWORK        } ,
	{ COUNT_SYSTEM         , ICONS_SYSTEM        , STRINGS_SYSTEM         } ,
	{ COUNT_HID            , ICONS_HID           , STRINGS_HID            } ,
	{ COUNT_USBDEVICES     , ICONS_USBDEVICES    , STRINGS_USBDEVICES     } ,
	{ COUNT_SECURITY       , ICONS_SECURITY      , STRINGS_SECURITY       } ,
	{ COUNT_IMAGEDEVICES   , ICONS_IMAGEDEVICES  , STRINGS_IMAGEDEVICES   }
};
int ManageResources::NODE_CHILDS_COUNT = sizeof(nodeChilds) / sizeof(TREECHILDS);
// Resources initialization status
BOOL ManageResources::initStatus = FALSE;
