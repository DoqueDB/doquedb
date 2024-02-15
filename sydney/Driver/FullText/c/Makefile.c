/*
 * Makefile.c --- Driver/FullText/c
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

MODULE = FullText
TARGET_BASE = SyDrvFts
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
	$(HDRDIR)/MessageNumber_InaccurateRowid.h \
	$(HDRDIR)/MessageNumber_NotEqualEntryCount.h \
	$(HDRDIR)/MessageNumber_VerifyAbort.h \
	$(HDRDIR)/Message_InaccurateRowid.h \
	$(HDRDIR)/Message_NotEqualEntryCount.h \
	$(HDRDIR)/Message_VerifyAbort.h

/* headers installed in Driver */
LOCAL_EXPORT_HDRS =

/* headers installed in TOP */
TOP_EXPORT_HDRS = \
	$(HDRDIR)/Module.h \
	$(HDRDIR)/OpenOption.h \
	$(HDRDIR)/FileDriver.h \
	$(HDRDIR)/FileOption.h \
	$(MESSAGE_HDRS)

#ifdef SYD_CPU_SPARC
OTHERINFO_HDRS = \
	$(HDRDIR)/OtherInformationFile0.h \
	$(HDRDIR)/OtherInformationFile2.h \
	$(HDRDIR)/VariableFile.h \
	$(HDRDIR)/VectorFile.h
#else
OTHERINFO_HDRS = \
	$(HDRDIR)/OtherInformationFile0.h \
	$(HDRDIR)/OtherInformationFile1.h \
	$(HDRDIR)/OtherInformationFile2.h \
	$(HDRDIR)/SectionFile.h \
	$(HDRDIR)/VariableFile.h \
	$(HDRDIR)/VectorFile.h
#endif

HDRS = \
	$(TOP_EXPORT_HDRS) \
	$(LOCAL_EXPORT_HDRS) \
	$(OTHERINFO_HDRS) \
	$(HDRDIR)/DelayIndexFile.h \
	$(HDRDIR)/FieldMask.h \
	$(HDRDIR)/File.h \
	$(HDRDIR)/FileDriver.h \
	$(HDRDIR)/FileID.h \
	$(HDRDIR)/IndexFile.h \
	$(HDRDIR)/InfoFile.h \
	$(HDRDIR)/LogicalInterface.h \
	$(HDRDIR)/MergeDaemon.h \
	$(HDRDIR)/MergeReserve.h \
	$(HDRDIR)/OtherInformationFile.h \
	$(HDRDIR)/Parameter.h \
	$(HDRDIR)/SimpleIndexFile.h

MESSAGE_OBJS = \
	Message_InaccurateRowid$O \
	Message_NotEqualEntryCount$O \
	Message_VerifyAbort$O

#ifdef SYD_CPU_SPARC
OTHERINFO_OBJS = \
	OtherInformationFile2$O \
	VariableFile$O \
	VectorFile$O
#else
OTHERINFO_OBJS = \
	OtherInformationFile2$O \
	SectionFile$O \
	VariableFile$O \
	VectorFile$O
#endif

OBJS = \
	$(MESSAGE_OBJS) \
	$(OTHERINFO_OBJS) \
	DelayIndexFile$O \
	FieldMask$O \
	FileDriver$O \
	FileID$O \
	FullText$O \
	IndexFile$O \
	InfoFile$O \
	LogicalInterface$O \
	MergeDaemon$O \
	MergeReserve$O \
	SimpleIndexFile$O \
	SyDrvFts$O

#ifdef SYD_DLL
EXPORTFLAGS = \
	-DSYD_FULLTEXT_EXPORT_FUNCTION \
	-DSYD_EXPORT_FUNCTION
#endif
#ifdef SYD_C_GCC
USE_LARGE_VECTOR =
#else
USE_LARGE_VECTOR = \
	-DSYD_USE_LARGE_VECTOR
#endif
EXTRACFLAGS = \
	$(EXPORTFLAGS) \
	-DSYD_INVERTED \
	-DSYD_CLUSTERING \
	$(USE_LARGE_VECTOR) \
	-I..$(S)..$(S)..$(S)Kernel$(S)include
EXTRALDFLAGS =
EXTRAPURIFYFLAGS =
EXTRAQUANTIFYFLAGS =
EXTRAPURECOVFLAGS =
EXTRALOCALCFLAGS =

KERNEL_LIBS = \
	SyKernel$L

#ifdef SYD_CPU_SPARC
LDLIBS = \
	$(KERNEL_LIBS) \
	SyDrvCom$L \
	SyDrvInv$L \
	$(MODLIBS)
#else
LDLIBS = \
	$(KERNEL_LIBS) \
	SyDrvBtr$L \
	SyDrvCom$L \
	SyDrvInv$L \
	$(MODLIBS)
#endif

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
  Copyright (c) 1997, 2023 Ricoh Company, Ltd.
  All rights reserved.
*/
