/*
 * Makefile.c --- 
 * 
 * Copyright (c) 2008, 2022, 2023 Ricoh Company, Ltd.
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

LIBNAME = libenstem
TARGET_BASE = $(LIBNAME)
MODULE = EnStem
HDR_DIR = ..$(S)$(MODULE)
SRC_DIR = ..

EXPORT_LIBDIR = ..$(S)..$(S)..$(S)c.CONF
LOCAL_EXPORT_HDRDIR = ..$(S)..$(S)include$(S)$(MODULE)

/* above variables MUST be defined      */
/****************************************/

/* install headers for libUna */
EXPORT_HDRS = \
	$(HDR_DIR)$(S)Module.h \
	$(HDR_DIR)$(S)ModEnglishWordStemmer.h

TAPE_HDRS =

HDRS = \
	$(LOCAL_EXPORT_HDRDIR) \
	$(EXPORT_HDRS)

SRCS = \
	$(SRC_DIR)$(S)ModEnglishWordStemmer.cpp

OBJS = \
 	ModEnglishWordStemmer$O

EXTRACFLAGS = -DUNAUNKKATAKANA=255 -DUNA_HIN_DEBUG -DMOD_NLP_USER_STEM_DIC -DMOD_EXPORT_STEM_DLL


#if defined(OS_RHLINUX6_0)
EXTRALDFLAGS = $(EXTRALDFLAGS_UNA)
#endif

EXTRAPURIFYFLAGS =
EXTRAQUANTIFYFLAGS =
EXTRAPURECOVFLAGS =
#ifdef MOD_DLL
EXTRALOCALCFLAGS = -DMOD_IMPORT_COMMON_DLL  -DMOD_IMPORT_STEM_DLL  -DMOD_EXPORT_XXSTEM_DLL -DUNA_DLL
#else
#ifdef MCH_INTEL
EXTRALOCALCFLAGS = -DUNA_DLL
#else
EXTRALOCALCFLAGS =
#endif
#endif
#ifdef MCH_INTEL
EXTRALOCALLDFLAGS = 
#else
EXTRALOCALLDFLAGS = 
#endif


/* install libraries */
#ifdef MOD_DLL
ENSTEM_VERSION = 1.0.0
ENSTEM_LIB = $(TARGET_BASE)$(D).$(ENSTEM_VERSION)
EXPORT_LIB = $(ENSTEM_LIB)
ENSTEM_LINKLIB = $(TARGET_BASE)$(D)
#endif

ALLTARGETS = \
	$(ENSTEM_LIB) \
	install \
	installh

/*
 * all
 */
AllTarget($(ALLTARGETS))
LibraryTarget($(ENSTEM_LIB), $(OBJS))

/*
 * install library and header
 */
InstallLibraryTarget($(EXPORT_LIB), $(EXPORT_LIBDIR), $(ENSTEM_LINKLIB))
InstallHeaderTarget($(EXPORT_HDRS), $(LOCAL_EXPORT_HDRDIR))

/*
 * clean
 */
CleanTarget($(EXPORT_LIB))
CleanTarget($(LOCAL_EXPORT_HDRDIR)$(S)*.h)
CleanTarget($(OBJS))

#include "Makefile.h"
