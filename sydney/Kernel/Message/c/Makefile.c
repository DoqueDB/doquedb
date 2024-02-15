/*
 * Makefile.c --- Kernel/Message/c
 * 
 * Copyright (c) 1999, 2023 Ricoh Company, Ltd.
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

SUBDIRS =
CLIENTSUBDIRS =

/****************************************/
/* following variables MUST be defined  */

MODULE = Message
TARGET_BASE_JPN = SyMesJpn$(CLIENTSUFFIX)
TARGET_BASE_ENG = SyMesEng$(CLIENTSUFFIX)
SRCDIR = ..
HDRDIR = ../$(MODULE)
MESSAGE_DEFINITION =
MESSAGE_TARGET =

LOCAL_EXPORT_HDRDIR = ../../include/$(MODULE)
TOP_EXPORT_HDRDIR = ../../../include/$(MODULE)
TOP_INSTALL_DIR = ../../../c.CONF
VERINFODIR=..$(S)..$(S)..$(S)version$(S)c.CONF
VERINFO_INCL=..$(S)..$(S)..$(S)version$(S)include

/* above variables MUST be defined      */
/****************************************/

/* headers created for message */
MESSAGE_HDRS =

/* headers installed in Kernel */
LOCAL_EXPORT_HDRS =

/* headers installed in TOP */
TOP_EXPORT_HDRS = \
	$(HDRDIR)/Message.h \
	$(MESSAGE_HDRS)

HDRS = \
	AutoCriticalSection.h \
	$(TOP_EXPORT_HDRS) \
	$(LOCAL_EXPORT_HDRS)

MESSAGE_OBJS =

OBJS_COMMON = \
	$(MESSAGE_OBJS) \
	Message$O

OBJS_JPN = \
	$(OBJS_COMMON) \
	Japanese$O

OBJS_ENG = \
	$(OBJS_COMMON) \
	English$O

#ifdef SYD_DLL
EXPORTFLAGS = \
	-DSYD_MESSAGE_EXPORT_FUNCTION
#endif
EXTRACFLAGS = \
	$(EXPORTFLAGS)
EXTRALDFLAGS =
EXTRAPURIFYFLAGS =
EXTRAQUANTIFYFLAGS =
EXTRAPURECOVFLAGS =
EXTRALOCALCFLAGS =
LDLIBS = \
	$(MODLIBS)

/********************************************************/

EXTRALOCALDLLFLAGS = 
RCLOCALFLAGS = /i $(VERINFO_INCL)

TARGET_JPN = \
	$P$(TARGET_BASE_JPN)$L
TARGET_ENG = \
	$P$(TARGET_BASE_ENG)$L

DLLTARGET_JPN = \
	$P$(TARGET_BASE_JPN)$D
DLLTARGET_ENG = \
	$P$(TARGET_BASE_ENG)$D
OLTARGET = $(MODULE).ol

ALLTARGETS = \
	$(MESSAGE_TARGET) \
	$(TARGET_JPN) \
	$(TARGET_ENG)

/*
 * all
 */
AllTarget($(ALLTARGETS))
clientall:: all

LibraryTarget($(TARGET_JPN), $(OBJS_JPN))
LibraryTarget($(TARGET_ENG), $(OBJS_ENG))
DLLTarget($(DLLTARGET_JPN), $(RESOURCE) $(OBJS_JPN) ExportFileName($(TARGET_BASE_JPN)))
DLLTarget($(DLLTARGET_ENG), $(RESOURCE) $(OBJS_ENG) ExportFileName($(TARGET_BASE_ENG)))

#ifdef SYD_DLL
ObjectListTarget($(OLTARGET), $(OBJS_COMMON), $(TOP_INSTALL_DIR))
#else
ObjectListTarget($(OLTARGET), $(OBJS_ENG), $(TOP_INSTALL_DIR))
#endif
clientobjlist:: objlist

/*
 * message
 */
/* MessageTarget($(MESSAGE_TARGET), $(MESSAGE_DEFINITION), $(BUILD_JAR)) */

/*
 * install library and header
 */
InstallHeaderTarget($(TOP_EXPORT_HDRS), $(TOP_EXPORT_HDRDIR))
/* InstallHeaderTarget($(LOCAL_EXPORT_HDRS), $(LOCAL_EXPORT_HDRDIR)) */
InstallLibraryTarget($(TARGET_JPN), $(TOP_INSTALL_DIR))
InstallLibraryTarget($(TARGET_ENG), $(TOP_INSTALL_DIR))
clientinstall:: install
InstallDLLTarget($(DLLTARGET_JPN), $(TOP_INSTALL_DIR))
InstallDLLTarget($(DLLTARGET_ENG), $(TOP_INSTALL_DIR))
clientinstalldll:: installdll

/*
 * clean
 */
CleanTarget($(ALLTARGETS))
CleanTarget($(DLLTARGET_JPN))
CleanTarget($(DLLTARGET_ENG))
CleanTarget($(OBJS_JPN))
CleanTarget($(OBJS_ENG))
CleanTarget($(TARGET_BASE_JPN).exp)
CleanTarget($(TARGET_BASE_ENG).exp)

#if defined(OS_WINDOWSNT4_0) || defined(OS_WINDOWS98)
ResourceTarget($(RESOURCE), $(VERINFODIR)\$(MODULE).rc)
#endif

#include "Makefile.h"

/*
  Copyright (c) 1999, 2023 Ricoh Company, Ltd.
  All rights reserved.
*/
