/*
 * Makefile.c --- Test/SydTest/c
 * 
 * Copyright (c) 2001, 2023 Ricoh Company, Ltd.
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

#ifdef SYD_DLL
SUBDIRS = \
	ExternalScoreCalculator
#else
SUBDIRS =
#endif

/****************************************/
/* following variables MUST be defined  */

MODULE = SydTest
TARGET_BASE = SydTest
SRCDIR = ..
HDRDIR = ../$(MODULE)

LOCAL_EXPORT_HDRDIR = ../../include/$(MODULE)
TOP_EXPORT_HDRDIR = ../../../include/$(MODULE)
TOP_INSTALL_DIR = ../../../c.CONF

/* above variables MUST be defined      */
/****************************************/

HDRS = \
	$(HDRDIR)/Command.h \
	$(HDRDIR)/Executor.h \
	$(HDRDIR)/FileBuffer.h \
	$(HDRDIR)/Item.h \
	$(HDRDIR)/Map.h \
	$(HDRDIR)/Monitor.h \
	$(HDRDIR)/Number.h \
	$(HDRDIR)/Option.h \
	$(HDRDIR)/Parameter.h \
	$(HDRDIR)/Parser.h \
	$(HDRDIR)/CascadeConf.h \
	$(HDRDIR)/StopWatch.h \
	$(HDRDIR)/String.h \
	$(HDRDIR)/SydTestException.h \
	$(HDRDIR)/SydTestMessage.h \
	$(HDRDIR)/Thread.h

OBJS = \
	$(RESOURCE) \
	Command$O \
	Executor$O \
	FileBuffer$O \
	Item$O \
	Monitor$O \
	Number$O \
	Option$O \
	Parameter$O \
	Parser$O \
	CascadeConf$O \
	StopWatch$O \
	String$O \
	SydTest$O \
	Thread$O

EXTRACFLAGS =
EXTRALDFLAGS =
EXTRAPURIFYFLAGS =
EXTRAQUANTIFYFLAGS =
EXTRAPURECOVFLAGS =
EXTRALOCALCFLAGS =

KERNEL_LIBS = \
	SyKernel$L

#ifdef SYD_OPENSSL
LDLIBS = \
	$(KERNEL_LIBS) \
	$(MODLIBS) \
	$(OPENSSLDLL)
#else
LDLIBS = \
	$(KERNEL_LIBS) \
	$(MODLIBS)
#endif

#ifdef SYD_OS_LINUX
LDLIBS += $(BOOSTDLL)
#endif  
  
/********************************************************/

#ifdef SYD_C_MS7_0
FORCEMULTIPLE = /force:multiple /* ModObjectがなぜか多重定義になってしまう */
#endif
#ifdef SYD_OS_WINDOWS
EXTRALOCALDLLFLAGS = \
	/STACK:2097152 \
	$(FORCEMULTIPLE)
#else
EXTRALOCALDLLFLAGS =
#endif

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

#include "Makefile.h"

/*
  Copyright (c) 2001, 2023 Ricoh Company, Ltd.
  All rights reserved.
*/
