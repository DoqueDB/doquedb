/*
 * Makefile.c --- Kernel/Buffer/c
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

MODULE = Buffer
TARGET_BASE = SyBuffer
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
	$(HDRDIR)/AutoPool.h \
	$(HDRDIR)/Daemon.h \
	$(HDRDIR)/DaemonThread.h \
	$(HDRDIR)/Deterrent.h \
	$(HDRDIR)/Manager.h

/* headers installed in TOP */
TOP_EXPORT_HDRS = \
	$(HDRDIR)/AutoFile.h \
	$(HDRDIR)/Configuration.h \
	$(HDRDIR)/File.h \
	$(HDRDIR)/HashTable.h \
	$(HDRDIR)/List.h \
	$(HDRDIR)/Memory.h \
	$(HDRDIR)/Module.h \
	$(HDRDIR)/Page.h \
	$(HDRDIR)/Pool.h \
	$(HDRDIR)/ReplacementPriority.h \
	$(HDRDIR)/Statistics.h \
	$(MESSAGE_HDRS)

HDRS = \
	$(TOP_EXPORT_HDRS) \
	$(LOCAL_EXPORT_HDRS) \
	$(HDRDIR)/AutoPage.h \
	$(HDRDIR)/DirtyPageFlusher.h \
	$(HDRDIR)/StatisticsReporter.h

MESSAGE_OBJS =

OBJS = \
	$(MESSAGE_OBJS) \
	Buffer$O \
	Configuration$O \
	Daemon$O \
	DaemonThread$O \
	Deterrent$O \
	DirtyPageFlusher$O \
	File$O \
	Manager$O \
	Memory$O \
	Page$O \
	Pool$O \
	Statistics$O \
	StatisticsReporter$O

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
