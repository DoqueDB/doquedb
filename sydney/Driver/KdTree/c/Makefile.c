/*
 * Makefile.c --- Driver/KdTree/c
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

MODULE = KdTree
TARGET_BASE = SyDrvKtr
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
	$(HDRDIR)/MessageNumber_VerifyFailed.h \
	$(HDRDIR)/Message_VerifyFailed.h

/* headers installed in Driver */
LOCAL_EXPORT_HDRS = 

/* headers installed in TOP */
TOP_EXPORT_HDRS = \
	$(HDRDIR)/Module.h \
	$(MESSAGE_HDRS)

HDRS = \
	$(TOP_EXPORT_HDRS) \
	$(HDRDIR)/Algorithm.h \
	$(HDRDIR)/Allocator.h \
	$(HDRDIR)/Archiver.h \
	$(HDRDIR)/AreaFile.h \
	$(HDRDIR)/AreaVectorFile.h \
	$(HDRDIR)/BtreeDataFile.h \
	$(HDRDIR)/BtreeFile.h \
	$(HDRDIR)/BtreePage.h \
	$(HDRDIR)/CalcVariance.h \
	$(HDRDIR)/DataFile.h \
	$(HDRDIR)/DoSearch.h \
	$(HDRDIR)/Entry.h \
	$(HDRDIR)/File.h \
	$(HDRDIR)/FileDriver.h \
	$(HDRDIR)/FileID.h \
	$(HDRDIR)/HeaderPage.h \
	$(HDRDIR)/IDVectorFile.h \
	$(HDRDIR)/IndexFile.h \
	$(HDRDIR)/InfoFile.h \
	$(HDRDIR)/KdTreeFile.h \
	$(HDRDIR)/KdTreeIndex.h \
	$(HDRDIR)/KdTreeIndexSet.h \
	$(HDRDIR)/LoadEntry.h \
	$(HDRDIR)/LogicalInterface.h \
	$(HDRDIR)/MakeTreeRecursive.h \
	$(HDRDIR)/MergeDaemon.h \
	$(HDRDIR)/MergeReserve.h \
	$(HDRDIR)/MultiFile.h \
	$(HDRDIR)/Node.h \
	$(HDRDIR)/OpenOption.h \
	$(HDRDIR)/Page.h \
	$(HDRDIR)/PagePointer.h \
	$(HDRDIR)/SortEntry.h \
	$(HDRDIR)/SubFile.h \
	$(HDRDIR)/SubFile2.h \
	$(HDRDIR)/VectorDataFile.h \
	$(HDRDIR)/VectorFile.h

MESSAGE_OBJS = \
	Message_VerifyFailed$O

OBJS = \
	$(MESSAGE_OBJS) \
	Allocator$O \
	Archiver$O \
	AreaFile$O \
	AreaVectorFile$O \
	BtreeDataFile$O \
	BtreeFile$O \
	BtreePage$O \
	CalcVariance$O \
	DBGetFileDriver$O \
	DataFile$O \
	DoSearch$O \
	Entry$O \
	File$O \
	FileDriver$O \
	FileID$O \
	HeaderPage$O \
	IDVectorFile$O \
	IndexFile$O \
	InfoFile$O \
	KdTreeFile$O \
	KdTreeIndex$O \
	KdTreeIndexSet$O \
	LoadEntry$O \
	LogicalInterface$O \
	MakeTreeRecursive$O \
	MergeDaemon$O \
	MergeReserve$O \
	MultiFile$O \
	Node$O \
	OpenOption$O \
	Page$O \
	SortEntry$O \
	SubFile$O \
	SubFile2$O \
	VectorDataFile$O \
	VectorFile$O

#ifdef SYD_DLL
EXPORTFLAGS = \
	-DSYD_KDTREE_EXPORT_FUNCTION \
	-DSYD_EXPORT_FUNCTION
#endif
EXTRACFLAGS = \
	$(EXPORTFLAGS) \
	-I..$(S)..$(S)..$(S)Kernel$(S)include
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
