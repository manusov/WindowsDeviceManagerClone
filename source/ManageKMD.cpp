/* ----------------------------------------------------------------------------------------
Class for support low level kernel-mode driver (KMD written by FASM).
Load driver (after application start), unload driver (before application exit),
use driver functions (at application runtime).
Select ia32 or x64 DLL depends on operating system type.
Note at WoW64 mode (when ia32 application runs under x64 OS) used x64 KMD driver.

RESERVED FOR FUTURE USE.
---------------------------------------------------------------------------------------- */

#include "ManageKMD.h"
