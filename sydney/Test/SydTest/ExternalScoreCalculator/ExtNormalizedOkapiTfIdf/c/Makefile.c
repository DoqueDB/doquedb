/*
 * Makefile.c --- 
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

SUBDIRS =

/****************************************/
/* following variables MUST be defined  */
MODULE = ExtNormalizedOkapiTfIdf
TARGET_BASE = $(MODULE)
HDR_DIR = ..
SRC_DIR = ..

EXPORT_LIBDIR = ..$(S)..$(S)c.CONF
LOCAL_EXPORT_HDRDIR = 

/* above variables MUST be defined      */
/****************************************/

/* install headers */
EXPORT_HDRS = 

TAPE_HDRS =

HDRS = \
	$(LOCAL_EXPORT_HDRDIR) \
	$(EXPORT_HDRS)

SRCS = \
	$(SRC_DIR)$(S)$(MODULE).cpp

OBJS = \
 	$(MODULE)$O

EXTRACFLAGS = \
	$(EXPORTFLAGS)

#if defined(MOD_DLL)
EXTRALDFLAGS_MSLU = /nod:kernel32.lib
#else
EXTRALDFLAGS_MSLU =
#endif
EXTRALDFLAGS = 

EXTRAPURIFYFLAGS =
EXTRAQUANTIFYFLAGS =
EXTRAPURECOVFLAGS =

LDLIBS = \
	$(MODCLIENTLIBS)

/* install libraries */


DLLTARGET = $P$(TARGET_BASE)$D
TARGET = $P$(TARGET_BASE)$D

EXPORT_LIB = $(DLLTARGET)


ALLTARGETS = \
	$(TARGET) \
	installdll


/*
 * all
 */
AllTarget($(ALLTARGETS))
#if defined(WIN32)
LibraryTarget($(TARGET), $(OBJS))
#endif

DLLTarget($(DLLTARGET),  $(OBJS) $(TARGET_EXPORT))


/*
 * install library and header
 */
#if defined(WIN32)
InstallLibraryTarget($(EXPORT_LIB), $(EXPORT_LIBDIR))
#endif
InstallDLLTarget($(DLLTARGET), $(EXPORT_LIBDIR))

/*
 * clean
 */
CleanTarget($(ALLTARGETS))
CleanTarget($(DLLTARGET))
CleanTarget($(OBJS))
CleanTarget($(TARGET_BASE).exp)

#include "Makefile.h"

/*
  Copyright (c) 2001, 2023 Ricoh Company, Ltd.
  All rights reserved.
*/
