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

LIBNAME = libunajp
TARGET_BASE = $(LIBNAME)
MODULE = ModNlpUnaJp
BASE_MODULE = UnaBase
HDR_DIR = ..$(S)$(MODULE)
SRC_DIR = ..

BASE_DIR = $(SRC_DIR)$(S)$(BASE_MODULE)
BASE_HDRDIR = $(BASE_DIR)$(S)$(BASE_MODULE)
BASE_OBJDIR = $(BASE_DIR)$(S)c.CONF

EXPORT_LIBDIR = ..$(S)..$(S)..$(S)c.CONF
LOCAL_EXPORT_HDRDIR = ..$(S)..$(S)include$(S)$(MODULE)
LOCAL_EXPORT_BASE_HDRDIR = ..$(S)..$(S)include$(S)$(BASE_MODULE)$(S)$(BASE_MODULE)

/* above variables MUST be defined      */
/****************************************/

/* install headers for LibUna */
EXPORT_HDRS = \
	$(HDR_DIR)$(S)ModNlpAnalyzerUnaJp.h \
	$(HDR_DIR)$(S)ModNlpResourceUnaJp.h \
	$(HDR_DIR)$(S)ModNlpUnaJp.h \
	$(HDR_DIR)$(S)ModNLPX.h \
	$(HDR_DIR)$(S)ModNormalizer.h \
	$(HDR_DIR)$(S)ModNormC.h \
	$(HDR_DIR)$(S)ModNormChar.h \
	$(HDR_DIR)$(S)ModNormDLL.h \
	$(HDR_DIR)$(S)ModNormRule.h \
	$(HDR_DIR)$(S)ModNormString.h \
	$(HDR_DIR)$(S)ModNormType.h \
	$(HDR_DIR)$(S)ModNormUNA.h \
	$(HDR_DIR)$(S)Module.h \
	$(HDR_DIR)$(S)ModUNA.h \
	$(HDR_DIR)$(S)ModUnaMiddle.h \
	$(HDR_DIR)$(S)ModUNANorm.h \
	$(HDR_DIR)$(S)ModNlpExpStr.h \
	$(HDR_DIR)$(S)unakapi.h \
	$(HDR_DIR)$(S)unamdeng.h

EXPORT_BASE_HDRS = \
	$(BASE_HDRDIR)$(S)una.h \
	$(BASE_HDRDIR)$(S)unaapinf.h \
	$(BASE_HDRDIR)$(S)unabns.h \
	$(BASE_HDRDIR)$(S)unamdtri.h \
	$(BASE_HDRDIR)$(S)unamdunk.h \
	$(BASE_HDRDIR)$(S)unamorph.h \
	$(BASE_HDRDIR)$(S)unastd.h


TAPE_HDRS =

HDRS = \
	$(EXPORT_HDRS)


SRCS = \
	$(SRC_DIR)$(S)GetFactory.cpp \
	$(SRC_DIR)$(S)ModNlpAnalyzerUnaJp.cpp \
	$(SRC_DIR)$(S)ModNlpResourceUnaJp.cpp \
	$(SRC_DIR)$(S)ModNlpUnaJp.cpp \
	$(SRC_DIR)$(S)ModNLPX.cpp \
	$(SRC_DIR)$(S)ModNormalizer.cpp \
	$(SRC_DIR)$(S)ModNormC.cpp \
	$(SRC_DIR)$(S)ModNormChar.cpp \
	$(SRC_DIR)$(S)ModNormRule.cpp \
	$(SRC_DIR)$(S)ModNormString.cpp \
	$(SRC_DIR)$(S)ModNormUNA.cpp \
	$(SRC_DIR)$(S)ModUNA.cpp \
	$(SRC_DIR)$(S)ModUnaMiddle.cpp \
	$(SRC_DIR)$(S)ModNlpExpStr.cpp \
	$(SRC_DIR)$(S)unakapi.cpp \
	$(SRC_DIR)$(S)unamdeng.cpp \
	$(BASE_DIR)$(S)unaapinf.cpp \
	$(BASE_DIR)$(S)unabns.cpp \
	$(BASE_DIR)$(S)unamdtri.cpp \
	$(BASE_DIR)$(S)unamdunk.cpp \
	$(BASE_DIR)$(S)unamorph.cpp \
	$(BASE_DIR)$(S)unastd.cpp

OBJS = \
	GetFactory$O \
	ModNlpAnalyzerUnaJp$O \
	ModNlpResourceUnaJp$O \
	ModNlpUnaJp$O \
	ModNLPX$O \
	ModNormalizer$O \
	ModNormC$O \
	ModNormChar$O \
	ModNormRule$O \
	ModNormString$O \
	ModNormUNA$O \
	ModUNA$O \
	ModUnaMiddle$O \
	ModNlpExpStr$O \
	unakapi$O \
	unamdeng$O \
	unaapinf$O \
	unabns$O \
	unamdtri$O \
	unamdunk$O \
	unamorph$O \
	unastd$O

EXTRACFLAGS =  -DUNAUNKKATAKANA=255 -DUNA_HIN_DEBUG -I$(BASE_HDRDIR)

#if defined(OS_RHLINUX6_0)
EXTRALDFLAGS_UNA = -luna -lenstem
EXTRALDFLAGS = $(EXTRALDFLAGS_UNA)
#endif

EXTRAPURIFYFLAGS =
EXTRAQUANTIFYFLAGS =
EXTRAPURECOVFLAGS =
#ifdef MOD_DLL
EXTRALOCALCFLAGS = -DMOD_IMPORT_COMMON_DLL  -DMOD_EXPORT_UNA_DLL -DMOD_EXPORT_NORM_DLL  -DMOD_IMPORT_XXSTEM_DLL -DUNA_DLL -DUNA_UNAJP_EXPORT_FUNCTION  -DUNA_EXPORT_FUNCTION -DUNA_LOCAL_EXPORT_FUNCTION
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
UNA_VERSION = 1.0.0
UNA_LIB = $(TARGET_BASE)$(D).$(UNA_VERSION)
EXPORT_LIB = $(UNA_LIB)
UNA_LINKLIB = $(TARGET_BASE)$(D)
#endif

ALLTARGETS = \
	$(UNA_LIB) \
	install \
	installh

/*
 * all
 */
AllTarget($(ALLTARGETS))
LibraryTarget($(UNA_LIB), $(OBJS))

/*
 * install library and header
 */
InstallLibraryTarget($(EXPORT_LIB), $(EXPORT_LIBDIR), $(UNA_LINKLIB))
InstallHeaderTarget($(EXPORT_HDRS), $(LOCAL_EXPORT_HDRDIR))
InstallHeaderTarget($(EXPORT_BASE_HDRS), $(LOCAL_EXPORT_BASE_HDRDIR))

/*
 * clean
 */
CleanTarget($(LOCAL_EXPORT_HDRDIR)$(S)*.h)
CleanTarget($(OBJS))
CleanTarget($(EXPORT_LIB))

#include "Makefile.h"
