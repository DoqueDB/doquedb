/*
 * Makefile.c --- Driver/Vector/c
 * 
 * Copyright (c) 1997, 2023 Ricoh Company, Ltd.
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

/****************************************/
/* following variables MUST be defined  */

MODULE = Vector
TARGET_BASE = SyDrvVct
SRCDIR = ..
HDRDIR = ../$(MODULE)
MESSAGE_DEFINITION = $(SRCDIR)/MessageDefinition.xml
MESSAGE_TARGET = $(MESSAGE_DEFINITION)_

LOCAL_EXPORT_HDRDIR = ../../include/$(MODULE)
TOP_EXPORT_HDRDIR = ../../../include/$(MODULE)
TOP_INSTALL_DIR = ../../../c.CONF
VERINFODIR=..$(S)..$(S)..$(S)version$(S)c.CONF
VERINFO_INCL=..$(S)..$(S)..$(S)version$(S)include

/* above variables MUST be defined      */
/****************************************/

/* headers created for message */
MESSAGE_HDRS = \
	$(HDRDIR)/MessageAll_Class.h \
	$(HDRDIR)/MessageAll_Number.h \
	$(HDRDIR)/MessageArgument.h \
	$(HDRDIR)/MessageFormat_English.h \
	$(HDRDIR)/MessageFormat_Japanese.h \
	$(HDRDIR)/MessageNumber_CaughtException.h \
	$(HDRDIR)/MessageNumber_CorruptFirstVectorKey.h \
	$(HDRDIR)/MessageNumber_CorruptLastVectorKey.h \
	$(HDRDIR)/MessageNumber_CorruptPageObjectCount.h \
	$(HDRDIR)/MessageNumber_CorruptTotalObjectCount.h \
	$(HDRDIR)/Message_CaughtException.h \
	$(HDRDIR)/Message_CorruptFirstVectorKey.h \
	$(HDRDIR)/Message_CorruptLastVectorKey.h \
	$(HDRDIR)/Message_CorruptPageObjectCount.h \
	$(HDRDIR)/Message_CorruptTotalObjectCount.h

/* headers installed in Driver */
LOCAL_EXPORT_HDRS =

/* headers installed in TOP */
TOP_EXPORT_HDRS = \
	$(HDRDIR)/FileDriver.h \
	$(HDRDIR)/Module.h \
	$(MESSAGE_HDRS)

HDRS = \
	$(TOP_EXPORT_HDRS) \
	$(LOCAL_EXPORT_HDRS) \
	$(HDRDIR)/FetchIterator.h \
	$(HDRDIR)/FieldIterator.h \
	$(HDRDIR)/File.h \
	$(HDRDIR)/FileDriver.h \
	$(HDRDIR)/FileInformation.h \
	$(HDRDIR)/FileOption.h \
	$(HDRDIR)/FileParameter.h \
	$(HDRDIR)/Module.h \
	$(HDRDIR)/Object.h \
	$(HDRDIR)/ObjectIterator.h \
	$(HDRDIR)/OpenOption.h \
	$(HDRDIR)/OpenParameter.h \
	$(HDRDIR)/PageManager.h \
	$(HDRDIR)/ReadIterator.h \
	$(HDRDIR)/ScanIterator.h \
	$(HDRDIR)/SearchIterator.h \
	$(HDRDIR)/UpdateIterator.h

MESSAGE_OBJS = \
	Message_CaughtException$O \
	Message_CorruptFirstVectorKey$O \
	Message_CorruptLastVectorKey$O \
	Message_CorruptPageObjectCount$O \
	Message_CorruptTotalObjectCount$O

OBJS = \
	$(MESSAGE_OBJS) \
	DBGetFileDriver$O \
	FetchIterator$O \
	FieldIterator$O \
	File$O \
	FileDriver$O \
	FileInformation$O \
	FileParameter$O \
	Object$O \
	ObjectIterator$O \
	OpenParameter$O \
	PageManager$O \
	ReadIterator$O \
	ScanIterator$O \
	SearchIterator$O \
	UpdateIterator$O \
	Vector$O

#ifdef SYD_DLL
EXPORTFLAGS = \
	-DSYD_VECTOR_EXPORT_FUNCTION \
	-DSYD_EXPORT_FUNCTION
#endif
EXTRACFLAGS = \
	$(EXPORTFLAGS)
EXTRALDFLAGS =
EXTRAPURIFYFLAGS =
EXTRAQUANTIFYFLAGS =
EXTRAPURECOVFLAGS =
EXTRALOCALCFLAGS =

KERNEL_LIBS = \
	SyKernel$L

LDLIBS = \
	$(KERNEL_LIBS) \
	SyDrvCom$L \
	$(MODLIBS)

/********************************************************/

#ifdef SYD_C_MS7_0
FORCEMULTIPLE = /force:multiple /* ModObjectがなぜか多重定義になってしまう */
#endif
EXTRALOCALDLLFLAGS =  \
	$(FORCEMULTIPLE)
RCLOCALFLAGS = /i $(VERINFO_INCL)

TARGET = $P$(TARGET_BASE)$L
DLLTARGET = $P$(TARGET_BASE)$D
OLTARGET = $(MODULE).ol

ALLTARGETS = \
	$(MESSAGE_TARGET) \
	$(TARGET)

/*
 * all
 */
AllTarget($(ALLTARGETS))

LibraryTarget($(TARGET), $(OBJS))

DLLTarget($(DLLTARGET), $(RESOURCE) $(OBJS) $(TARGET_EXPORT))

ObjectListTarget($(OLTARGET), $(OBJS), $(TOP_INSTALL_DIR))

/*
 * message
 */
MessageTarget($(MESSAGE_TARGET), $(MESSAGE_DEFINITION), $(BUILD_JAR))

/*
 * install library and header
 */
InstallHeaderTarget($(TOP_EXPORT_HDRS), $(TOP_EXPORT_HDRDIR))
/* InstallHeaderTarget($(LOCAL_EXPORT_HDRS), $(LOCAL_EXPORT_HDRDIR)) */
InstallLibraryTarget($(TARGET), $(TOP_INSTALL_DIR))
InstallDLLTarget($(DLLTARGET), $(TOP_INSTALL_DIR))

/*
 * clean
 */
CleanTarget($(ALLTARGETS))
CleanTarget($(DLLTARGET))
CleanTarget($(OBJS))
CleanTarget($(TARGET_BASE).exp)

#if defined(OS_WINDOWSNT4_0) || defined(OS_WINDOWS98)
ResourceTarget($(RESOURCE), $(VERINFODIR)\$(MODULE).rc)
#endif

#include "Makefile.h"

$(MESSAGE_HDRS): $(MESSAGE_TARGET)
$(MESSAGE_OBJS): $(MESSAGE_TARGET)

/*
  Copyright 1997, 2023 Ricoh Company, Ltd.
  All rights reserved.
*/
