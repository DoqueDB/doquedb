/*
 * Makefile.c --- Kernel/c
 * 
 * Copyright (c) 1999, 2023, 2024 Ricoh Company, Ltd.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

COMMONDIRS = \
	Common \
	Communication \
	Exception \
	Os

KERNELDIRS = \
	Admin \
	Analysis \
	Buffer \
	Checkpoint \
	Execution \
	Lock \
	LogicalFile \
	LogicalLog \
	Opt \
	PhysicalFile \
	Plan \
	Schema \
	Server \
	Statement \
	Trans \
	Utility \
	Version \
	DServer \
	DSchema \
	DExecution \
	DPlan

CLIENTDIRS = \
	Client \
	Client2

MESSAGEDIRS = \
	Message

SUBDIRS = \
	$(COMMONDIRS) \
	$(KERNELDIRS) \
	$(CLIENTDIRS) \
	$(MESSAGEDIRS)

CLIENTSUBDIRS = \
	$(COMMONDIRS) \
	$(CLIENTDIRS) \
	$(MESSAGEDIRS)

#ifdef SYD_COVERAGE
kernel: objlist-r all-r install-r dll-r installdll-r
#else
kernel: installh-r objlist-r all-r install-r dll-r installdll-r
#endif

/********************************************************/

#ifdef SYD_DLL

MODULE = Kernel
KERNEL_BASE = SyKernel
#if defined(OS_WINDOWSNT4_0) || defined(OS_WINDOWS98)
CLIENT_BASE = TRMeister
#else
CLIENT_BASE = DoqueDB
#endif

TOP_INSTALL_DIR = ../../c.CONF
VERINFODIR=..$(S)..$(S)version$(S)c.CONF
VERINFO_INCL=..$(S)..$(S)version$(S)include

/* CAUTION: The order of following ol files is very important */
KERNEL_OL = \
	$(COMMON_OL) \
	$(TOP_INSTALL_DIR)/Lock.ol \
	$(TOP_INSTALL_DIR)/Trans.ol \
	$(TOP_INSTALL_DIR)/Buffer.ol \
	$(TOP_INSTALL_DIR)/Version.ol \
	$(TOP_INSTALL_DIR)/Checkpoint.ol \
	$(TOP_INSTALL_DIR)/PhysicalFile.ol \
	$(TOP_INSTALL_DIR)/LogicalLog.ol \
	$(TOP_INSTALL_DIR)/LogicalFile.ol \
	$(TOP_INSTALL_DIR)/Statement.ol \
	$(TOP_INSTALL_DIR)/Schema.ol \
	$(TOP_INSTALL_DIR)/Utility.ol \
	$(TOP_INSTALL_DIR)/Plan.ol \
	$(TOP_INSTALL_DIR)/Analysis.ol \
	$(TOP_INSTALL_DIR)/Execution.ol \
	$(TOP_INSTALL_DIR)/Opt.ol \
	$(TOP_INSTALL_DIR)/Admin.ol \
	$(TOP_INSTALL_DIR)/Server.ol \
	$(TOP_INSTALL_DIR)/Communication.ol \
	$(TOP_INSTALL_DIR)/Client.ol /* for sqli */ \
	$(TOP_INSTALL_DIR)/Client2.ol \
	$(TOP_INSTALL_DIR)/DServer.ol \
	$(TOP_INSTALL_DIR)/DSchema.ol \
	$(TOP_INSTALL_DIR)/DExecution.ol \
	$(TOP_INSTALL_DIR)/DPlan.ol

KERNEL_RESOURCE = $(RESOURCE)

CLIENT_OL = \
	$(COMMON_OL) \
	$(TOP_INSTALL_DIR)/Client.ol \
	$(TOP_INSTALL_DIR)/Client2.ol \
	$(TOP_INSTALL_DIR)/Communication_C.ol

COMMON_OL = \
	$(TOP_INSTALL_DIR)/Exception.ol \
	$(TOP_INSTALL_DIR)/Os.ol \
	$(TOP_INSTALL_DIR)/Common.ol

CLIENT_RESOURCE = ResourceName(Client)
RCLOCALFLAGS = /i $(VERINFO_INCL)

KERNEL_OBJECTLIST = kernelobjs.ol
CLIENT_OBJECTLIST = clientobjs.ol

#ifdef SYD_C_MS
KERNEL_OBJS = $(KERNEL_RESOURCE) $(KERNEL_BASE)$O
CLIENT_OBJS = $(CLIENT_RESOURCE) Sydney$O
#else
KERNEL_OBJS = $(KERNEL_BASE)$O
CLIENT_OBJS = Sydney$O
#endif

EXTRALOCALDLLFLAGS = 

LDLIBS = \
	$(MODLIBS) \
	$(ZLIBDLL) \
	$(OPENSSLDLL)

#ifdef SYD_OS_LINUX
LDLIBS += $(BOOSTDLL)
#endif  
  
KERNEL_TARGET = $P$(KERNEL_BASE)$L
KERNEL_DLLTARGET = $P$(KERNEL_BASE)$D
CLIENT_TARGET = $P$(CLIENT_BASE)$(CLIENTSUFFIX)$L
CLIENT_DLLTARGET = $P$(CLIENT_BASE)$(CLIENTSUFFIX)$D

ALLTARGETS = \
	$(CLIENT_TARGET) \
	$(KERNEL_TARGET)

#ifdef OS_RHLINUX6_0
.NOTPARALLEL:
#endif

AllTarget($(ALLTARGETS))
AllTargetT(clientall, $(CLIENT_TARGET))

LibraryTarget4($(CLIENT_TARGET), $(CLIENT_OBJECTLIST), $(CLIENT_OBJS))
#ifdef SYD_C_MS
DLLTarget4($(CLIENT_DLLTARGET), $(CLIENT_OBJECTLIST), $(CLIENT_BASE)$(CLIENTSUFFIX).exp $(CLIENT_OBJS))
#endif
#if defined(SYD_C_SC) || defined(SYD_C_GCC)
DLLTarget4($(CLIENT_DLLTARGET), $(CLIENT_OBJECTLIST), $(CLIENT_OBJS))
#endif

InstallLibraryTarget($(CLIENT_TARGET), $(TOP_INSTALL_DIR))
InstallLibraryTargetT(clientinstall, $(CLIENT_TARGET), $(TOP_INSTALL_DIR))
InstallDLLTarget($(CLIENT_DLLTARGET), $(TOP_INSTALL_DIR))
InstallDLLTargetT(clientinstalldll, $(CLIENT_DLLTARGET), $(TOP_INSTALL_DIR))

LibraryTarget4($(KERNEL_TARGET), $(KERNEL_OBJECTLIST), $(KERNEL_OBJS))
#ifdef SYD_C_MS
DLLTarget4($(KERNEL_DLLTARGET), $(KERNEL_OBJECTLIST), $(KERNEL_BASE).exp $(KERNEL_OBJS))
#endif
#if defined(SYD_C_SC) || defined(SYD_C_GCC)
DLLTarget4($(KERNEL_DLLTARGET), $(KERNEL_OBJECTLIST), $(KERNEL_OBJS))
#endif

InstallLibraryTarget($(KERNEL_TARGET), $(TOP_INSTALL_DIR))
InstallDLLTarget($(KERNEL_DLLTARGET), $(TOP_INSTALL_DIR))

CleanTarget($(CLIENT_TARGET) $(CLIENT_DLLTARGET))
CleanTarget($(KERNEL_TARGET) $(KERNEL_DLLTARGET))

#if defined(OS_WINDOWSNT4_0) || defined(OS_WINDOWS98)
ResourceTarget($(CLIENT_RESOURCE), $(VERINFODIR)\Client.rc)
ResourceTarget($(KERNEL_RESOURCE), $(VERINFODIR)\Kernel.rc)
#endif

ObjectListTargetMergeT($(CLIENT_OBJECTLIST), clientobjlist, $(CLIENT_OL), .)
ObjectListTargetMerge($(KERNEL_OBJECTLIST), $(KERNEL_OL), .)

#endif

#include "Makefile.h"

/*
  Copyright (c) 1999, 2023 ,2024 Ricoh Company, Ltd.
  All rights reserved.
*/
