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

LIBNAME = libuna
TARGET_BASE = $(LIBNAME)
MODULE = LibUna
HDR_DIR = ..$(S)$(MODULE)
JP_MODULE = ModNlpUnaJp
JP_HDR_DIR = ..$(S)..$(S)$(JP_MODULE)$(S)$(JP_MODULE)
SRC_DIR = ..

EXPORT_LIBDIR = ..$(S)..$(S)..$(S)c.CONF
EXPORT_HDRDIR = ..$(S)..$(S)include
LOCAL_EXPORT_HDRDIR = $(EXPORT_HDRDIR)$(S)$(MODULE)

/* above variables MUST be defined      */
/****************************************/

/* install headers for libUna */
EXPORT_HDRS = \
	$(HDR_DIR)$(S)UnaDLL.h \
	$(HDR_DIR)$(S)ModNLPText.h \
	$(HDR_DIR)$(S)ModNLP.h \
	$(HDR_DIR)$(S)UnaNameSpace.h \
	$(HDR_DIR)$(S)UnaAssert.h \
	$(HDR_DIR)$(S)ModNLPLocal.h \
	$(HDR_DIR)$(S)Algorithm.h \
	$(HDR_DIR)$(S)Morph.h \
	$(HDR_DIR)$(S)Type.h \
	$(HDR_DIR)$(S)UnicodeChar.h \
	$(HDR_DIR)$(S)UnaDynamicCast.h \
	$(HDR_DIR)$(S)UnaReinterpretCast.h \
	$(HDR_DIR)$(S)UnaParam.h \
	$(HDR_DIR)$(S)UnaVersion.h \
	$(HDR_DIR)$(S)UNA_UNIFY_TAG.h 

LOCAL_EXPORT_HDRS = \
	$(HDR_DIR)$(S)DoubleArray.h \
	$(HDR_DIR)$(S)DicSet.h \
	$(HDR_DIR)$(S)Keyword.h \
	$(HDR_DIR)$(S)ModStemDLL.h \
	$(HDR_DIR)$(S)Module.h \
	$(HDR_DIR)$(S)ModWordStemmer.h \
	$(HDR_DIR)$(S)ModTerm.h \
	$(HDR_DIR)$(S)ModTermStringFile.h \
	$(HDR_DIR)$(S)ModTermElement.h \
	$(HDR_DIR)$(S)ModNlpNpCost.h

HDRS = \
	$(EXPORT_HDRS)	\
	$(LOCAL_EXPORT_HDRS) \
	$(HDR_DIR)$(S)AutoArrayPointer.h \
	$(HDR_DIR)$(S)AutoMapPointer.h \
	$(HDR_DIR)$(S)Bitset.h \
	$(HDR_DIR)$(S)Data.h \
	$(HDR_DIR)$(S)Executer.h \
	$(HDR_DIR)$(S)Keyword.h \
	$(HDR_DIR)$(S)ModNlpUserDic.h \
	$(HDR_DIR)$(S)ModNlpUserDicStem.h \
	$(HDR_DIR)$(S)Rule.h \
	$(HDR_DIR)$(S)RuleApplier.h \
	$(HDR_DIR)$(S)RuleElementSet.h \
	$(HDR_DIR)$(S)RuleHolder.h \
	$(HDR_DIR)$(S)RuleMaker.h \
	$(HDR_DIR)$(S)RuleScanner.h \
	$(HDR_DIR)$(S)RxTools.h \
	$(HDR_DIR)$(S)SmartPointer.h \
	$(HDR_DIR)$(S)UnicodeFile.h

TAPE_HDRS = \
	$(HDR_DIR)$(S)UnaDLL.h \
	$(HDR_DIR)$(S)ModNLPText.h \
	$(HDR_DIR)$(S)ModNLP.h \
	$(HDR_DIR)$(S)UnaVersion.h \
	$(HDR_DIR)$(S)UNA_UNIFY_TAG.h 
	
SRCS = \
	$(SRC_DIR)$(S)Bitset.cpp \
	$(SRC_DIR)$(S)Data.cpp \
	$(SRC_DIR)$(S)Executer.cpp \
	$(SRC_DIR)$(S)Keyword.cpp \
	$(SRC_DIR)$(S)ModNlp.cpp \
	$(SRC_DIR)$(S)ModNLPLocal.cpp \
	$(SRC_DIR)$(S)ModWordStemmer.cpp \
	$(SRC_DIR)$(S)Rule.cpp \
	$(SRC_DIR)$(S)RuleApplier.cpp \
	$(SRC_DIR)$(S)RuleHolder.cpp \
	$(SRC_DIR)$(S)RuleMaker.cpp \
	$(SRC_DIR)$(S)RuleParser.tab.cpp \
	$(SRC_DIR)$(S)RuleScanner.cpp \
	$(SRC_DIR)$(S)RxTools.cpp \
	$(SRC_DIR)$(S)Type.cpp \
	$(SRC_DIR)$(S)UnicodeFile.cpp \
	$(SRC_DIR)$(S)ModTerm.cpp \
	$(SRC_DIR)$(S)ModTermStringFile.cpp \
	$(SRC_DIR)$(S)ModTermElement.cpp \
	$(SRC_DIR)$(S)ModNlpNpCost.cpp

OBJS = \
	Bitset$O \
	Data$O \
	Executer$O \
	Keyword$O \
	ModNlp$O \
	ModNLPLocal$O \
	ModWordStemmer$O \
	Rule$O \
	RuleApplier$O \
	RuleHolder$O \
	RuleMaker$O \
	RuleParser.tab$O \
	RuleScanner$O \
	RxTools$O \
	Type$O \
	UnicodeFile$O \
	ModTerm$O \
	ModTermStringFile$O \
	ModTermElement$O \
	ModNlpNpCost$O

EXTRACFLAGS = -DUNAUNKKATAKANA=255 -DUNA_HIN_DEBUG -DUNA_LOCAL_EXPORT_FUNCTION

EXTRAPURIFYFLAGS =
EXTRAQUANTIFYFLAGS =
EXTRAPURECOVFLAGS =
#ifdef MOD_DLL
EXTRALOCALCFLAGS = -DMOD_IMPORT_COMMON_DLL -DUNA_EXPORT_FUNCTION -DMOD_EXPORT_NORM_DLL -DMOD_EXPORT_STEM_DLL -DUNA_DLL
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
InstallLibraryTarget($(EXPORT_LIB),$(EXPORT_LIBDIR),$(UNA_LINKLIB))
InstallHeaderTarget($(EXPORT_HDRS),$(EXPORT_HDRDIR))
InstallHeaderTarget($(LOCAL_EXPORT_HDRS),$(LOCAL_EXPORT_HDRDIR))
TapeHeaderTarget($(TAPE_HDRS),$(UNATAPEINCLDIR))

/*
 * clean
 */
CleanTarget($(EXPORT_LIB))
CleanTarget($(EXPORT_HDRDIR)$(S)*.h)
CleanTarget($(OBJS))

#include "Makefile.h"
