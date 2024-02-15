/*
 * Makefile.c --- Kernel/Checkpoint/c
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

MODULE = Checkpoint
TARGET_BASE = SyCkpt
SRCDIR = ..
HDRDIR = ../$(MODULE)
MESSAGE_DEFINITION =
MESSAGE_TARGET =

LOCAL_EXPORT_HDRDIR = ../../include/$(MODULE)
TOP_EXPORT_HDRDIR = ../../../include/$(MODULE)
TOP_INSTALL_DIR = ../../../c.CONF

/* above variables MUST be defined      */
/****************************************/

/* headers created for message */
MESSAGE_HDRS =

/* headers installed in Kernel */
LOCAL_EXPORT_HDRS = \
	$(HDRDIR)/Configuration.h \
	$(HDRDIR)/Executor.h \
	$(HDRDIR)/Externalizable.h \
	$(HDRDIR)/FileDestroyer.h \
	$(HDRDIR)/LogData.h \
	$(HDRDIR)/Manager.h \
	$(HDRDIR)/TimeStamp.h

/* headers installed in TOP */
TOP_EXPORT_HDRS = \
	$(HDRDIR)/Daemon.h \
	$(HDRDIR)/Database.h \
	$(HDRDIR)/Module.h \
	$(MESSAGE_HDRS)

HDRS = \
	$(TOP_EXPORT_HDRS) \
	$(LOCAL_EXPORT_HDRS) \
	$(HDRDIR)/Buffer.h \
	$(HDRDIR)/Debug.h \
	$(HDRDIR)/FileMover.h \
	$(HDRDIR)/FileSynchronizer.h \
	$(HDRDIR)/LogicalLog.h

MESSAGE_OBJS =

OBJS = \
	$(MESSAGE_OBJS) \
	Buffer$O \
	Checkpoint$O \
	Configuration$O \
	Daemon$O \
	Database$O \
	Executor$O \
	Externalizable$O \
	FileDestroyer$O \
	FileMover$O \
	FileSynchronizer$O \
	LogData$O \
	LogicalLog$O \
	Manager$O \
	TimeStamp$O

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
/* MessageTarget($(MESSAGE_TARGET), $(MESSAGE_DEFINITION), $(BUILD_JAR)) */

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

/*
  Copyright (c) 1999, 2023 Ricoh Company, Ltd.
  All rights reserved.
*/
