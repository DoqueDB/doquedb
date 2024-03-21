/*
 * Makefile.c --- Utility/UserAdd/c
 * 
 * Copyright (c) 2008, 2023, 2024 Ricoh Company, Ltd.
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

MODULE = UserAdd
TARGET_BASE1 = UserAdd
TARGET_BASE2 = UserDel
TARGET_BASE3 = UserPassword

RESOURCE1 = ResourceName($(TARGET_BASE1))
RESOURCE2 = ResourceName($(TARGET_BASE2))
RESOURCE3 = ResourceName($(TARGET_BASE3))

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
	$(HDRDIR)/Exec.h \
	$(HDRDIR)/ExecAdd.h \
	$(HDRDIR)/ExecDel.h \
	$(HDRDIR)/ExecPassword.h \
	$(HDRDIR)/Option.h

OBJS1 = \
	$(RESOURCE1) \
	Exec$O \
	ExecAdd$O \
	Option$O \
	UserAdd$O

OBJS2 = \
	$(RESOURCE2) \
	Exec$O \
	ExecDel$O \
	Option$O \
	UserDel$O

OBJS3 = \
	$(RESOURCE3) \
	Exec$O \
	ExecPassword$O \
	Option$O \
	UserPassword$O

EXTRACFLAGS =
EXTRALDFLAGS =
EXTRAPURIFYFLAGS =
EXTRAQUANTIFYFLAGS =
EXTRAPURECOVFLAGS =
EXTRALOCALCFLAGS =

#if defined(OS_WINDOWSNT4_0) || defined(OS_WINDOWS98)
KERNEL_LIBS = \
	TRMeister$(CLIENTSUFFIX)$L
#else
KERNEL_LIBS = \
	DoqueDB$(CLIENTSUFFIX)$L
#endif

LDLIBS = \
	$(KERNEL_LIBS) \
	$(MODLIBS)

#ifdef SYD_OS_LINUX
LDLIBS += $(BOOSTDLL)
#endif  
  
/********************************************************/

EXTRALOCALDLLFLAGS =
RCLOCALFLAGS = /i $(VERINFO_INCL)

TARGET1 = $(TARGET_BASE1)$E
TARGET2 = $(TARGET_BASE2)$E
TARGET3 = $(TARGET_BASE3)$E

ALLTARGETS = \
	$(TARGET1) \
	$(TARGET2) \
	$(TARGET3)

/*
 * all
 */
AllTarget($(ALLTARGETS))
clientall:: all

ProgramTarget($(TARGET1), $(OBJS1))
ProgramTarget($(TARGET2), $(OBJS2))
ProgramTarget($(TARGET3), $(OBJS3))
InstallProgramTarget($(TARGET1), $(TOP_INSTALL_DIR))
InstallProgramTarget($(TARGET2), $(TOP_INSTALL_DIR))
InstallProgramTarget($(TARGET3), $(TOP_INSTALL_DIR))

/*
 * clean
 */
CleanTarget($(ALLTARGETS))
CleanTarget($(OBJS1))
CleanTarget($(OBJS2))
CleanTarget($(OBJS3))

#if defined(OS_WINDOWSNT4_0) || defined(OS_WINDOWS98)
ResourceTarget($(RESOURCE1), $(VERINFODIR)\$(TARGET_BASE1).rc)
ResourceTarget($(RESOURCE2), $(VERINFODIR)\$(TARGET_BASE2).rc)
ResourceTarget($(RESOURCE3), $(VERINFODIR)\$(TARGET_BASE3).rc)
#endif

#include "Makefile.h"
