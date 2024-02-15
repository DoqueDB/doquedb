/*
 * Makefile.c --- Driver/Record/c
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

MODULE = Record
TARGET_BASE = SyDrvRcd
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
	$(HDRDIR)/MessageNumber_BadFreeObjectID.h \
	$(HDRDIR)/MessageNumber_DiscordObjectNum.h \
	$(HDRDIR)/MessageNumber_ExistLastObject.h \
	$(HDRDIR)/MessageNumber_ExistTopObject.h \
	$(HDRDIR)/MessageNumber_InconsistentHeader.h \
	$(HDRDIR)/MessageNumber_InconsistentPageObjectNumber.h \
	$(HDRDIR)/MessageNumber_InconsistentVariableSize.h \
	$(HDRDIR)/MessageNumber_ObjectNotFound.h \
	$(HDRDIR)/MessageNumber_VerifyFailed.h \
	$(HDRDIR)/MessageNumber_VerifyOnGoing.h \
	$(HDRDIR)/MessageNumber_VerifyPhysicalFileFinished.h \
	$(HDRDIR)/MessageNumber_VerifyPhysicalFileStarted.h \
	$(HDRDIR)/Message_BadFreeObjectID.h \
	$(HDRDIR)/Message_DiscordObjectNum.h \
	$(HDRDIR)/Message_ExistLastObject.h \
	$(HDRDIR)/Message_ExistTopObject.h \
	$(HDRDIR)/Message_InconsistentHeader.h \
	$(HDRDIR)/Message_InconsistentPageObjectNumber.h \
	$(HDRDIR)/Message_InconsistentVariableSize.h \
	$(HDRDIR)/Message_ObjectNotFound.h \
	$(HDRDIR)/Message_VerifyFailed.h \
	$(HDRDIR)/Message_VerifyOnGoing.h \
	$(HDRDIR)/Message_VerifyPhysicalFileFinished.h \
	$(HDRDIR)/Message_VerifyPhysicalFileStarted.h

/* headers installed in Driver */
LOCAL_EXPORT_HDRS =

/* headers installed in TOP */
TOP_EXPORT_HDRS = \
	$(HDRDIR)/FileDriver.h \
	$(HDRDIR)/FileOption.h \
	$(HDRDIR)/Module.h \
	$(HDRDIR)/OpenOption.h \
	$(MESSAGE_HDRS)

HDRS = \
	$(TOP_EXPORT_HDRS) \
	$(LOCAL_EXPORT_HDRS) \
	$(HDRDIR)/Debug.h \
	$(HDRDIR)/DirectField.h \
	$(HDRDIR)/DirectFile.h \
	$(HDRDIR)/DirectIterator.h \
	$(HDRDIR)/File.h \
	$(HDRDIR)/FileBase.h \
	$(HDRDIR)/FileDriver.h \
	$(HDRDIR)/FileInformation.h \
	$(HDRDIR)/FreeAreaManager.h \
	$(HDRDIR)/LinkedObject.h \
	$(HDRDIR)/MetaData.h \
	$(HDRDIR)/OpenParameter.h \
	$(HDRDIR)/Parameter.h \
	$(HDRDIR)/PhysicalPosition.h \
	$(HDRDIR)/TargetFields.h \
	$(HDRDIR)/Tools.h \
	$(HDRDIR)/UseInfo.h \
	$(HDRDIR)/VariableField.h \
	$(HDRDIR)/VariableFile.h \
	$(HDRDIR)/VariableIterator.h

MESSAGE_OBJS = \
	Message_BadFreeObjectID$O \
	Message_DiscordObjectNum$O \
	Message_ExistLastObject$O \
	Message_ExistTopObject$O \
	Message_InconsistentHeader$O \
	Message_InconsistentPageObjectNumber$O \
	Message_InconsistentVariableSize$O \
	Message_ObjectNotFound$O \
	Message_VerifyFailed$O \
	Message_VerifyOnGoing$O \
	Message_VerifyPhysicalFileFinished$O \
	Message_VerifyPhysicalFileStarted$O

OBJS = \
	$(MESSAGE_OBJS) \
	DBGetFileDriver$O \
	DirectField$O \
	DirectFile$O \
	DirectIterator$O \
	File$O \
	FileBase$O \
	FileDriver$O \
	FileInformation$O \
	FreeAreaManager$O \
	LinkedObject$O \
	MetaData$O \
	OpenParameter$O \
	PhysicalPosition$O \
	Record$O \
	TargetFields$O \
	Tools$O \
	UseInfo$O \
	VariableField$O \
	VariableFile$O \
	VariableIterator$O

#ifdef SYD_DLL
EXPORTFLAGS = \
	-DSYD_RECORD_EXPORT_FUNCTION \
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
