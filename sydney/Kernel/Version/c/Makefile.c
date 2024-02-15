/*
 * Makefile.c --- Kernel/Version/c
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

/****************************************/
/* following variables MUST be defined  */

MODULE = Version
TARGET_BASE = SyVers
SRCDIR = ..
HDRDIR = ../$(MODULE)
MESSAGE_DEFINITION = $(SRCDIR)/MessageDefinition.xml
MESSAGE_TARGET = $(MESSAGE_DEFINITION)_

LOCAL_EXPORT_HDRDIR = ../../include/$(MODULE)
TOP_EXPORT_HDRDIR = ../../../include/$(MODULE)
TOP_INSTALL_DIR = ../../../c.CONF

/* above variables MUST be defined      */
/****************************************/

/* headers created for message */
MESSAGE_HDRS = \
	$(HDRDIR)/MessageAll_Class.h \
	$(HDRDIR)/MessageAll_Number.h \
	$(HDRDIR)/MessageArgument.h \
	$(HDRDIR)/MessageFormat_English.h \
	$(HDRDIR)/MessageFormat_Japanese.h \
	$(HDRDIR)/MessageNumber_AllocationBitInconsistent.h \
	$(HDRDIR)/MessageNumber_BlockCountInconsistent.h \
	$(HDRDIR)/MessageNumber_ChildCountInconsistent.h \
	$(HDRDIR)/MessageNumber_LatestCountInconsistent.h \
	$(HDRDIR)/MessageNumber_MasterDataFileNotFound.h \
	$(HDRDIR)/MessageNumber_OlderNotIdentical.h \
	$(HDRDIR)/MessageNumber_OlderTimeStampInconsistent.h \
	$(HDRDIR)/MessageNumber_OldestTimeStampInconsistent.h \
	$(HDRDIR)/MessageNumber_PhysicalLogIDInconsistent.h \
	$(HDRDIR)/MessageNumber_PreservedDifferentPage.h \
	$(HDRDIR)/MessageNumber_SyncLogFileFound.h \
	$(HDRDIR)/MessageNumber_VersionLogFileNotFound.h \
	$(HDRDIR)/MessageNumber_VersionLogIDInconsistent.h \
	$(HDRDIR)/MessageNumber_VersionPageCountInconsistent.h \
	$(HDRDIR)/Message_AllocationBitInconsistent.h \
	$(HDRDIR)/Message_BlockCountInconsistent.h \
	$(HDRDIR)/Message_ChildCountInconsistent.h \
	$(HDRDIR)/Message_LatestCountInconsistent.h \
	$(HDRDIR)/Message_MasterDataFileNotFound.h \
	$(HDRDIR)/Message_OlderNotIdentical.h \
	$(HDRDIR)/Message_OlderTimeStampInconsistent.h \
	$(HDRDIR)/Message_OldestTimeStampInconsistent.h \
	$(HDRDIR)/Message_PhysicalLogIDInconsistent.h \
	$(HDRDIR)/Message_PreservedDifferentPage.h \
	$(HDRDIR)/Message_SyncLogFileFound.h \
	$(HDRDIR)/Message_VersionLogFileNotFound.h \
	$(HDRDIR)/Message_VersionLogIDInconsistent.h \
	$(HDRDIR)/Message_VersionPageCountInconsistent.h

/* headers installed in Kernel */
LOCAL_EXPORT_HDRS = \
	$(HDRDIR)/AutoFile.h \
	$(HDRDIR)/Configuration.h \
	$(HDRDIR)/Manager.h

/* headers installed in TOP */
TOP_EXPORT_HDRS = \
	$(HDRDIR)/Block.h \
	$(HDRDIR)/File.h \
	$(HDRDIR)/Module.h \
	$(HDRDIR)/Page.h \
	$(MESSAGE_HDRS)

HDRS = \
	$(TOP_EXPORT_HDRS) \
	$(LOCAL_EXPORT_HDRS) \
	$(HDRDIR)/AutoPage.h \
	$(HDRDIR)/AutoVerification.h \
	$(HDRDIR)/Daemon.h \
	$(HDRDIR)/Debug.h \
	$(HDRDIR)/DetachedPageCleaner.h \
	$(HDRDIR)/HashTable.h \
	$(HDRDIR)/List.h \
	$(HDRDIR)/MasterData.h \
	$(HDRDIR)/PathParts.h \
	$(HDRDIR)/SyncLog.h \
	$(HDRDIR)/Verification.h \
	$(HDRDIR)/VersionLog.h

MESSAGE_OBJS = \
	Message_AllocationBitInconsistent$O \
	Message_BlockCountInconsistent$O \
	Message_ChildCountInconsistent$O \
	Message_LatestCountInconsistent$O \
	Message_MasterDataFileNotFound$O \
	Message_OlderNotIdentical$O \
	Message_OlderTimeStampInconsistent$O \
	Message_OldestTimeStampInconsistent$O \
	Message_PhysicalLogIDInconsistent$O \
	Message_PreservedDifferentPage$O \
	Message_SyncLogFileFound$O \
	Message_VersionLogFileNotFound$O \
	Message_VersionLogIDInconsistent$O \
	Message_VersionPageCountInconsistent$O

OBJS = \
	$(MESSAGE_OBJS) \
	Block$O \
	Configuration$O \
	Daemon$O \
	File$O \
	Manager$O \
	MasterData$O \
	Page$O \
	SyncLog$O \
	Verification$O \
	Version$O \
	VersionLog$O

#ifdef SYD_DLL
EXPORTFLAGS = \
	-DSYD_KERNEL_EXPORT_FUNCTION
#endif
EXTRACFLAGS = \
	$(EXPORTFLAGS)
EXTRALDFLAGS =
EXTRAPURIFYFLAGS =
EXTRAQUANTIFYFLAGS =
EXTRAPURECOVFLAGS =
EXTRALOCALCFLAGS =

/********************************************************/

TARGET = $(MODULE).ol

ALLTARGETS = \
	$(MESSAGE_TARGET) \
	$(TARGET)

/*
 * all
 */
AllTarget($(ALLTARGETS))

ObjectListTarget($(TARGET), $(OBJS), $(TOP_INSTALL_DIR))

/*
 * message
 */
MessageTarget($(MESSAGE_TARGET), $(MESSAGE_DEFINITION), $(BUILD_JAR))

/*
 * install library and header
 */
InstallHeaderTarget($(TOP_EXPORT_HDRS), $(TOP_EXPORT_HDRDIR))
InstallHeaderTarget($(LOCAL_EXPORT_HDRS), $(LOCAL_EXPORT_HDRDIR))

/*
 * clean
 */
CleanTarget($(ALLTARGETS))
CleanTarget($(OBJS))

#include "Makefile.h"

$(MESSAGE_HDRS): $(MESSAGE_TARGET)
$(MESSAGE_OBJS): $(MESSAGE_TARGET)

/*
  Copyright (c) 1999, 2023 Ricoh Company, Ltd.
  All rights reserved.
*/
