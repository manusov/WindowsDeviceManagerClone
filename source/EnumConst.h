/* ----------------------------------------------------------------------------------------
Class used for get emulated (constant) system information.
Class used for application debug purposes and as parent for system information classes.
BuildTree procedure builds constant tree as linked list of nodes descriptors.
At application debug, this class used as tree builder for data emulation.
At real system information show, this is parent class for system information builder.
---------------------------------------------------------------------------------------- */

#pragma once
#ifndef ENUMCONST_H
#define ENUMCONST_H

#include "Global.h"
#include "ManageResources.h"

class EnumConst
{
public:
	EnumConst();
	virtual ~EnumConst();
	// Don't use constructor and destructor for build and release tree, because
	// dynamical rebuild with model change and partial changes can be required.
	void SetAndInitModel(ManageResources* p);
	virtual PTREENODE BuildTree(UINT mode);
	virtual void ReleaseTree();
protected:
	static PTREENODE pTreeBase;
	static PTREENODE pTreeBaseBack;
	static ManageResources* pManageResources;
private:
	static LPCSTR EMULATED_NAME_1;
	static LPCSTR EMULATED_NAME_2;
};

#endif  // ENUMCONST_H