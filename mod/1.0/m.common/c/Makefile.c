	/*
 * Makefile.c --- m.library/m.common/c
 * 
 * Copyright (c) 1998, 2022, 2023, 2024 Ricoh Company, Ltd.
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

HDR_DIR = ..$(S)include$(SUFFIX)
SRC_DIR = ..$(S)src

/* install headers */
EXPORT_HDRS_1 = \
	$(HDR_DIR)$(S)ModAlgorithm.h \
	$(HDR_DIR)$(S)ModArchive.h \
	$(HDR_DIR)$(S)ModAssert.h \
	$(HDR_DIR)$(S)ModAutoMutex.h \
	$(HDR_DIR)$(S)ModAutoPointer.h \
	$(HDR_DIR)$(S)ModCharString.h \
	$(HDR_DIR)$(S)ModCharTrait.h \
	$(HDR_DIR)$(S)ModCodec.h \
	$(HDR_DIR)$(S)ModCommon.h \
	$(HDR_DIR)$(S)ModCommonDLL.h \
	$(HDR_DIR)$(S)ModCommonException.h \
	$(HDR_DIR)$(S)ModCommonInitialize.h \
	$(HDR_DIR)$(S)ModCommonMutex.h \
	$(HDR_DIR)$(S)ModCompress.h \
	$(HDR_DIR)$(S)ModConditionVariable.h \
	$(HDR_DIR)$(S)ModConfig.h \
	$(HDR_DIR)$(S)ModCountry.h \
	$(HDR_DIR)$(S)ModCriticalSection.h \
	$(HDR_DIR)$(S)ModDebug.h \
	$(HDR_DIR)$(S)ModDefaultManager.h \
	$(HDR_DIR)$(S)ModError.h \
	$(HDR_DIR)$(S)ModException.h \
	$(HDR_DIR)$(S)ModExceptionMessage.h \
	$(HDR_DIR)$(S)ModFakeError.h \
	$(HDR_DIR)$(S)ModFile.h \
	$(HDR_DIR)$(S)ModHashMap.h \
	$(HDR_DIR)$(S)ModHashTable.h \
	$(HDR_DIR)$(S)ModHasher.h \
	$(HDR_DIR)$(S)ModIos.h \
	$(HDR_DIR)$(S)ModKanjiCode.h \
	$(HDR_DIR)$(S)ModLanguage.h \
	$(HDR_DIR)$(S)ModLanguageSet.h \
	$(HDR_DIR)$(S)ModLinkedList.h \
	$(HDR_DIR)$(S)ModLinkedListBase.h \
	$(HDR_DIR)$(S)ModList.h \
	$(HDR_DIR)$(S)ModMainInitialize.h \
	$(HDR_DIR)$(S)ModManager.h \
	$(HDR_DIR)$(S)ModManipulator.h \
	$(HDR_DIR)$(S)ModMap.h \
	$(HDR_DIR)$(S)ModMemory.h \
	$(HDR_DIR)$(S)ModMemoryHandle.h \
	$(HDR_DIR)$(S)ModMemoryPool.h \
	$(HDR_DIR)$(S)ModMessage.h  \
	$(HDR_DIR)$(S)ModMultiByteString.h \
	$(HDR_DIR)$(S)ModMutex.h

EXPORT_HDRS_2 = \
	$(HDR_DIR)$(S)ModObject.h \
	$(HDR_DIR)$(S)ModOs.h \
	$(HDR_DIR)$(S)ModOsDriver.h \
	$(HDR_DIR)$(S)ModOsDriverLinux.h \
	$(HDR_DIR)$(S)ModOsException.h \
	$(HDR_DIR)$(S)ModOsMutex.h \
	$(HDR_DIR)$(S)ModOstrStream.h \
	$(HDR_DIR)$(S)ModOstream.h \
	$(HDR_DIR)$(S)ModPair.h \
	$(HDR_DIR)$(S)ModParameter.h \
	$(HDR_DIR)$(S)ModParameterSource.h \
	$(HDR_DIR)$(S)ModSerial.h \
	$(HDR_DIR)$(S)ModSerialIO.h \
	$(HDR_DIR)$(S)ModSerialSize.h \
	$(HDR_DIR)$(S)ModSocket.h \
	$(HDR_DIR)$(S)ModStrStreamBuffer.h \
	$(HDR_DIR)$(S)ModString.h \
	$(HDR_DIR)$(S)ModSyncBase.h \
	$(HDR_DIR)$(S)ModThread.h \
	$(HDR_DIR)$(S)ModTime.h \
	$(HDR_DIR)$(S)ModTimeSpan.h \
	$(HDR_DIR)$(S)ModTrait.h \
	$(HDR_DIR)$(S)ModTree.h \
	$(HDR_DIR)$(S)ModTypes.h \
	$(HDR_DIR)$(S)ModUnicodeChar.h \
	$(HDR_DIR)$(S)ModUnicodeCharTrait.h \
	$(HDR_DIR)$(S)ModUnicodeCharTypes.h \
	$(HDR_DIR)$(S)ModUnicodeOstrStream.h \
	$(HDR_DIR)$(S)ModUnicodeRegularExpression.h \
	$(HDR_DIR)$(S)ModUnicodeString.h \
	$(HDR_DIR)$(S)ModUtility.h \
	$(HDR_DIR)$(S)ModVector.h \
	$(HDR_DIR)$(S)ModVersion.h \
	$(HDR_DIR)$(S)ModWideChar.h \
	$(HDR_DIR)$(S)ModWideCharIterator.h \
	$(HDR_DIR)$(S)ModWideCharTrait.h \
	$(HDR_DIR)$(S)ModWideString.h

EXPORT_HDRS = \
	$(EXPORT_HDRS_1) \
	$(EXPORT_HDRS_2)

HDRS = $(EXPORT_HDRS) \
	$(HDR_DIR)$(S)ModDetectDeadLock.h \
	$(HDR_DIR)$(S)ModMemoryNegotiate.h \
	$(HDR_DIR)$(S)ModRingLinkedList.h \
	$(HDR_DIR)$(S)ModWaitingThread.h

SRCS = \
	$(SRC_DIR)$(S)ModArchive.cpp \
	$(SRC_DIR)$(S)ModAutoMutex.cpp \
	$(SRC_DIR)$(S)ModCharString.cpp \
	$(SRC_DIR)$(S)ModCodec.cpp \
	$(SRC_DIR)$(S)ModCommonInitialize.cpp \
	$(SRC_DIR)$(S)ModCommonMutex.cpp \
	$(SRC_DIR)$(S)ModCompress.cpp \
	$(SRC_DIR)$(S)ModCountry.cpp \
	$(SRC_DIR)$(S)ModDebug.cpp \
	$(SRC_DIR)$(S)ModDefaultManager.cpp \
	$(SRC_DIR)$(S)ModDetectDeadLock.cpp \
	$(SRC_DIR)$(S)ModError.cpp \
	$(SRC_DIR)$(S)ModException.cpp \
	$(SRC_DIR)$(S)ModExceptionMessage.cpp \
	$(SRC_DIR)$(S)ModFakeError.cpp \
	$(SRC_DIR)$(S)ModFile.cpp \
	$(SRC_DIR)$(S)ModHashTable.cpp \
	$(SRC_DIR)$(S)ModKanjiCode.cpp \
	$(SRC_DIR)$(S)ModLanguage.cpp \
	$(SRC_DIR)$(S)ModLanguageSet.cpp \
	$(SRC_DIR)$(S)ModLinkedListBase.cpp \
	$(SRC_DIR)$(S)ModManager.cpp \
	$(SRC_DIR)$(S)ModMemory.cpp \
	$(SRC_DIR)$(S)ModMemoryHandle.cpp \
	$(SRC_DIR)$(S)ModMemoryNegotiate.cpp \
	$(SRC_DIR)$(S)ModMemoryPool.cpp \
	$(SRC_DIR)$(S)ModMessage.cpp \
	$(SRC_DIR)$(S)ModMultiByteString.cpp \
	$(SRC_DIR)$(S)ModOs.cpp \
	$(SRC_DIR)$(S)ModOsDriverLinux.cpp \
	$(SRC_DIR)$(S)ModOsMutex.cpp \
	$(SRC_DIR)$(S)ModOstrStream.cpp \
	$(SRC_DIR)$(S)ModOstream.cpp \
	$(SRC_DIR)$(S)ModParameter.cpp \
	$(SRC_DIR)$(S)ModParameterLinux.cpp \
	$(SRC_DIR)$(S)ModRingLinkedList.cpp \
	$(SRC_DIR)$(S)ModSerialSize.cpp \
	$(SRC_DIR)$(S)ModSocket.cpp \
	$(SRC_DIR)$(S)ModStrStreamBuffer.cpp \
	$(SRC_DIR)$(S)ModSyncBase.cpp \
	$(SRC_DIR)$(S)ModThread.cpp \
	$(SRC_DIR)$(S)ModTime.cpp \
	$(SRC_DIR)$(S)ModTree.cpp \
	$(SRC_DIR)$(S)ModUnicodeCharTrait.cpp \
	$(SRC_DIR)$(S)ModUnicodeOstrStream.cpp \
	$(SRC_DIR)$(S)ModUnicodeRegularExpression.cpp \
	$(SRC_DIR)$(S)ModUnicodeString.cpp \
	$(SRC_DIR)$(S)ModWideCharTrait.cpp \
	$(SRC_DIR)$(S)ModWideString.cpp

#ifdef OS_RHLINUX6_0
MOD_OSDRIVER = ModOsDriverLinux$O
MOD_PARAMETER = ModParameterLinux$O
MOD_INLINEDLL =
MSGRC =
MSGOBJS =
MSGDLL =
MSGDLLOBJS =
VERRES =
#endif

OBJS = \
	ModCommonMutex$O \
	ModCommonInitialize$O \
	ModCompress$O \
	ModDebug$O \
	ModDefaultManager$O \
	ModError$O \
	ModException$O \
	ModExceptionMessage$O \
	ModFakeError$O \
	ModHashTable$O \
	ModCountry$O \
	ModLanguage$O \
	ModLanguageSet$O \
	ModManager$O \
	ModMessage$O \
	ModMultiByteString$O \
	ModOstream$O \
	ModUnicodeRegularExpression$O \
	ModThread$O \
	ModTime$O \
	ModTree$O \
	ModUnicodeCharTrait$O \
	ModUnicodeString$O \
	ModUnicodeOstrStream$O \
	ModLinkedListBase$O ModCharString$O ModWideString$O \
	ModWideCharTrait$O ModKanjiCode$O \
	ModMemoryHandle$O ModMemoryPool$O \
	ModOs$O $(MOD_OSDRIVER) ModOsMutex$O ModAutoMutex$O \
	ModArchive$O ModCodec$O ModSerialSize$O \
	ModFile$O ModMemory$O ModSocket$O \
	ModParameter$O $(MOD_PARAMETER) \
	ModStrStreamBuffer$O ModOstrStream$O \
	ModDetectDeadLock$O ModSyncBase$O \
	ModMemoryNegotiate$O ModRingLinkedList$O \
	$(VERRES) \
	$(MOD_INLINEDLL)

GENERATED_FILES = ModCommonExceptionMessage.h ModUnicodeCharTrait.tbl

RX_DIR = ..$(S)..$(S)m.rx
RX_HDRDIR = $(RX_DIR)$(S)include$(SUFFIX)
RX_OBJDIR = $(RX_DIR)$(S)c.CONF
RX_OBJS = \
	$(RX_OBJDIR)$(S)rx$O \
	$(RX_OBJDIR)$(S)rxParse$O \
	$(RX_OBJDIR)$(S)rxPmm$O \
	$(RX_OBJDIR)$(S)rxchar$O \
	$(RX_OBJDIR)$(S)rxdfa$O \
	$(RX_OBJDIR)$(S)rxset$O \
	$(RX_OBJDIR)$(S)rxModUnicodeOperations$O \
	$(RX_OBJDIR)$(S)rxtree$O
SLOG_OBJS =

UNICODETOOL_DIR = $(MODTOOLSDIR)$(S)c.CONF

#ifdef MOD_DLL
EXPORTDLLFLAGS = -DMOD_EXPORT_COMMON_DLL
#else
EXPORTDLLFLAGS =
#endif
EXTRACFLAGS = $(EXPORTDLLFLAGS) -I. -I$(RX_HDRDIR)

EXTRALDFLAGS =
EXTRAPURIFYFLAGS =
EXTRAQUANTIFYFLAGS =
EXTRAPURECOVFLAGS =
#ifdef PURECOV
EXTRALOCALCFLAGS = -I../../include$(SUFFIX) -I../include$(SUFFIX) -I.
/*
INCLUDES =
*/
#else
EXTRALOCALCFLAGS =
#endif
#ifdef MOD_DLL
EXTRALOCALLDFLAGS =
#else
EXTRALOCALLDFLAGS =
#endif

/* install libraries */
#ifdef MOD_DLL
COMMON_VERSION = 1.0.0
COMMON_LIB = libcommon$(D).$(COMMON_VERSION)
EXPORT_LIB = $(COMMON_LIB)
COMMON_LINKLIB = libcommon$(D)
#else
COMMON_LIB = libcommon$(L)
EXPORT_LIB = $(COMMON_LIB)
#endif

#ifdef PURECOV
ALLTARGETS = \
	installh\
	$(COMMON_LIB)
#else
ALLTARGETS = \
	$(COMMON_LIB)
#endif

/*
 * all
 */
AllTarget($(ALLTARGETS))

LibraryTarget($(COMMON_LIB), $(OBJS) $(RX_OBJS))

/*
 * install library and header
 */
#ifdef MOD_DLL
InstallLibraryTarget($(EXPORT_LIB), $(MODINSTLIBDIR), $(COMMON_LINKLIB))
#else
InstallLibraryTarget($(EXPORT_LIB), $(MODINSTLIBDIR))
#endif
InstallHeaderTarget2($(EXPORT_HDRS_1), $(EXPORT_HDRS_2), $(MODTAPEINCLDIR))

/*
 * clean
 */
CleanTarget($(ALLTARGETS))
CleanTarget($(OBJS))
CleanTarget($(GENERATED_FILES))

UNICODE_DATA = $(UNICODETOOL_DIR)$(S)..$(S)src$(S)UnicodeData-1.1.5.txt
SL = /

$(UNICODE_DATA):
	(cd ../../tools/src; wget https:$(SL)/www.unicode.org/Public/1.1-Update/UnicodeData-1.1.5.txt)

ModUnicodeCharTrait.tbl: $(UNICODE_DATA)
	$(UNICODETOOL_DIR)$(S)maketable $(UNICODE_DATA) > ./ModUnicodeCharTrait.tbl

ModCommonExceptionMessage.h: ../src/ModCommonExceptionMessage.txt
	$(PERL) $(MKERRMSG) ../src/ModCommonExceptionMessage.txt $@

ModCommonInitialize$O: ModCommonExceptionMessage.h
ModUnicodeCharTrait$O: ModUnicodeCharTrait.tbl

#include "Makefile.h"
