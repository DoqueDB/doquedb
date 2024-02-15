/*
 * Makefile.c --- Kernel/Os/c
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

MODULE = Os
TARGET_BASE = SyOs
SRCDIR = ..
HDRDIR = ../$(MODULE)
MESSAGE_DEFINITION =
MESSAGE_TARGET =

LOCAL_EXPORT_HDRDIR = ../../include/$(MODULE)
TOP_EXPORT_HDRDIR = ../../../include/$(MODULE)
TOP_INSTALL_DIR = ../../../c.CONF
PACKAGE_HDRDIR = $(TOP_INSTALL_DIR)/$(PACKAGE_NAME)/include/$(MODULE)

/* above variables MUST be defined      */
/****************************************/

/* headers created for message */
MESSAGE_HDRS =

#ifdef SYD_OS_LINUX
LINUX_HDRS = \
	$(HDRDIR)/CriticalSectionManager.h 
#else
LINUX_HDRS = 
#endif
  
/* headers installed in tape image */
PACKAGE_HDRS = \
	$(HDRDIR)/CriticalSection.h \
	$(HDRDIR)/Module.h

/* headers installed in Kernel */
LOCAL_EXPORT_HDRS = \
	$(HDRDIR)/AsyncStatus.h \
	$(HDRDIR)/AutoEvent.h \
	$(HDRDIR)/Host.h \
	$(HDRDIR)/Manager.h \
	$(HDRDIR)/Math.h \
	$(HDRDIR)/SysConf.h

/* headers installed in TOP */
TOP_EXPORT_HDRS = \
	$(PACKAGE_HDRS) \
	$(LINUX_HDRS) \
	$(HDRDIR)/AutoCriticalSection.h \
	$(HDRDIR)/AutoRWLock.h \
	$(HDRDIR)/AutoSemaphore.h \
	$(HDRDIR)/AutoSynchronization.h \
	$(HDRDIR)/Event.h \
	$(HDRDIR)/File.h \
	$(HDRDIR)/InterlockedVariable.h \
	$(HDRDIR)/Library.h \
	$(HDRDIR)/Limits.h \
	$(HDRDIR)/Memory.h \
	$(HDRDIR)/Mutex.h \
	$(HDRDIR)/Path.h \
	$(HDRDIR)/Process.h \
	$(HDRDIR)/RWLock.h \
	$(HDRDIR)/Semaphore.h \
	$(HDRDIR)/Thread.h \
	$(HDRDIR)/ThreadLocalStorage.h \
	$(HDRDIR)/Timer.h \
	$(HDRDIR)/Unicode.h \
	$(HDRDIR)/Uuid.h \
	$(MESSAGE_HDRS)

HDRS = \
	$(TOP_EXPORT_HDRS) \
	$(LOCAL_EXPORT_HDRS) \
	$(HDRDIR)/Assert.h \
	$(HDRDIR)/FakeError.h \
	$(HDRDIR)/MiniDump.h

MESSAGE_OBJS =

#ifdef SYD_OS_LINUX
LINUX_OBJS = CriticalSectionManager$O
#else
LINUX_OBJS =  
#endif
  
OBJS = \
	$(MESSAGE_OBJS) \
	$(LINUX_OBJS) \
	AsyncStatus$O \
	CriticalSection$O \
	Event$O \
	File$O \
	Library$O \
	Manager$O \
	Memory$O \
	MiniDump$O \
	Mutex$O \
	Os$O \
	Path$O \
	Process$O \
	RWLock$O \
	Semaphore$O \
	SysConf$O \
	ThreadLocalStorage$O \
	Timer$O \
	Unicode$O \
	Uuid$O

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
clientall:: all

ObjectListTarget($(TARGET), $(OBJS), $(TOP_INSTALL_DIR))
clientobjlist:: objlist

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
 * install header for package
 */
TapeHeaderTarget($(PACKAGE_HDRS), $(PACKAGE_HDRDIR))

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
