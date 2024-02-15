/*
 * Makefile.c --- 
 * 
 * Copyright (c) 1998, 2022, 2023 Ricoh Company, Ltd.
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

/* install headers */
EXPORT_HDRS =

HDRS = \
	rx.h \
	rxDefs.h \
	rxLocal.h \
	rxParse.h \
	rxPmm.h \
	rxPmmLocal.h \
	rxchar.h \
	rxdfa.h \
	rxset.h \
	rxTmpModUnicodeChar.h \
	rxModUnicodeOperations.h \
	rxtree.h

SRCS = \
	rx.c \
	rxParse.c \
	rxPmm.c \
	rxTest.c \
	rxchar.c \
	rxdfa.c \
	rxset.c \
	rxModUnicodeOperations.c \
	rxtree.c

OBJS = \
	rx$O \
	rxParse$O \
	rxPmm$O \
	rxchar$O \
	rxdfa$O \
	rxset$O \
	rxModUnicodeOperations$O \
	rxtree$O

#ifdef OS_RHLINUX6_0
#ifdef CC_GCC
CC = /usr/bin/gcc
#endif
#endif

EXTRACFLAGS = -I.
EXTRALDFLAGS = 
EXTRAPURIFYFLAGS =
EXTRAQUANTIFYFLAGS =
EXTRAPURECOVFLAGS =
EXTRALOCALCFLAGS = -DRX_UNICODE
EXTRALOCALLDFLAGS = -DRX_UNICODE

ALLTARGETS = $(OBJS)

/*
 * all
 */
AllTarget($(ALLTARGETS))

/*
 * clean
 */
CleanTarget($(ALLTARGETS))

#include "Makefile.h"
