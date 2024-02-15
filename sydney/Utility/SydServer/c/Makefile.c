/*
 * Makefile.c --- Kernel/SydServer/c
 * 
 * Copyright (c) 2023 Ricoh Company, Ltd.
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

MODULE = SydServer
TARGET_BASE = SydServer
SRCDIR = ..
HDRDIR = ../$(MODULE)

LOCAL_EXPORT_HDRDIR = ../../include/$(MODULE)
TOP_EXPORT_HDRDIR = ../../../include/$(MODULE)
TOP_INSTALL_DIR = ../../../c.CONF
VERINFODIR=..$(S)..$(S)..$(S)version$(S)c.CONF
VERINFO_INCL=..$(S)..$(S)..$(S)version$(S)include

/* above variables MUST be defined      */
/****************************************/

HDRS = \
	$(HDRDIR)/ServiceModule.h

OBJS = \
	$(RESOURCE) \
	ServiceModule$O \
	SydServer$O

EXTRACFLAGS =
EXTRALDFLAGS =
EXTRAPURIFYFLAGS =
EXTRAQUANTIFYFLAGS =
EXTRAPURECOVFLAGS =
EXTRALOCALCFLAGS =

#ifdef SYD_OS_WINDOWS
LDLIBS = 
#else
KERNEL_LIBS = \
	SyKernel$L
LDLIBS = \
	$(KERNEL_LIBS) \
	$(MODLIBS)
#endif

#ifdef SYD_OS_LINUX
LDLIBS += $(BOOSTDLL)
#endif  

/********************************************************/

#ifdef SYD_OS_WINDOWS
EXTRALOCALDLLFLAGS = /STACK:2097152
#else
EXTRALOCALDLLFLAGS =
#endif
RCLOCALFLAGS = /i $(VERINFO_INCL)

TARGET = $(TARGET_BASE)$E

ALLTARGETS = \
	$(TARGET)

/*
 * all
 */
AllTarget($(ALLTARGETS))

ProgramTarget($(TARGET), $(OBJS))
InstallProgramTarget($(TARGET), $(TOP_INSTALL_DIR))

/*
 * clean
 */
CleanTarget($(ALLTARGETS))
CleanTarget($(OBJS))

#if defined(OS_WINDOWSNT4_0) || defined(OS_WINDOWS98)
ResourceTarget($(RESOURCE), $(VERINFODIR)\$(MODULE).rc)
#endif

#include "Makefile.h"
