/*
 * Makefile.c --- Driver/Inverted/c
 * 
 * Copyright (c) 1997, 2023 Ricoh Company, Ltd.
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

MODULE = Inverted
TARGET_BASE = SyDrvInv
SRCDIR = ..
HDRDIR = ../$(MODULE)
MOD_SRCDIR = $(SRCDIR)/FtsInverted
MOD_HDRDIR = $(SRCDIR)/FtsInverted/FtsInverted
MESSAGE_DEFINITION = $(SRCDIR)/MessageDefinition.xml
MESSAGE_TARGET = $(MESSAGE_DEFINITION)_

LOCAL_EXPORT_HDRDIR = ../../include/$(MODULE)
TOP_EXPORT_HDRDIR = ../../../include/$(MODULE)
TOP_INSTALL_DIR = ../../../c.CONF
VERINFODIR=..$(S)..$(S)..$(S)version$(S)c.CONF
VERINFO_INCL=..$(S)..$(S)..$(S)version$(S)include

/* above variables MUST be defined      */
/****************************************/

/* headers created for message */
MESSAGE_HDRS = \
	$(HDRDIR)/MessageAll_Class.h \
	$(HDRDIR)/MessageAll_Number.h \
	$(HDRDIR)/MessageArgument.h \
	$(HDRDIR)/MessageFormat_English.h \
	$(HDRDIR)/MessageFormat_Japanese.h \
	$(HDRDIR)/MessageNumber_CantConvertToDocID.h \
	$(HDRDIR)/MessageNumber_CantConvertToTupleID.h \
	$(HDRDIR)/MessageNumber_CorrectDisusedIdBlock.h \
	$(HDRDIR)/MessageNumber_DisusedIdBlock.h \
	$(HDRDIR)/MessageNumber_IllegalEntryCount.h \
	$(HDRDIR)/MessageNumber_IllegalIdPage.h \
	$(HDRDIR)/MessageNumber_IllegalIndex.h \
	$(HDRDIR)/MessageNumber_IllegalInvertedListType.h \
	$(HDRDIR)/MessageNumber_IllegalListCount.h \
	$(HDRDIR)/MessageNumber_IllegalLocPage.h \
	$(HDRDIR)/MessageNumber_IllegalNextLocPage.h \
	$(HDRDIR)/MessageNumber_IllegalTotalDocumentLength.h \
	$(HDRDIR)/MessageNumber_InconsistentDocIDandTupleID.h \
	$(HDRDIR)/MessageNumber_NextLinkOfLastPage.h \
	$(HDRDIR)/MessageNumber_NullStringNotInserted.h \
	$(HDRDIR)/MessageNumber_PreviousLinkOfTopPage.h \
	$(HDRDIR)/Message_CantConvertToDocID.h \
	$(HDRDIR)/Message_CantConvertToTupleID.h \
	$(HDRDIR)/Message_CorrectDisusedIdBlock.h \
	$(HDRDIR)/Message_DisusedIdBlock.h \
	$(HDRDIR)/Message_IllegalEntryCount.h \
	$(HDRDIR)/Message_IllegalIdPage.h \
	$(HDRDIR)/Message_IllegalIndex.h \
	$(HDRDIR)/Message_IllegalInvertedListType.h \
	$(HDRDIR)/Message_IllegalListCount.h \
	$(HDRDIR)/Message_IllegalLocPage.h \
	$(HDRDIR)/Message_IllegalNextLocPage.h \
	$(HDRDIR)/Message_IllegalTotalDocumentLength.h \
	$(HDRDIR)/Message_InconsistentDocIDandTupleID.h \
	$(HDRDIR)/Message_NextLinkOfLastPage.h \
	$(HDRDIR)/Message_NullStringNotInserted.h \
	$(HDRDIR)/Message_PreviousLinkOfTopPage.h

/* headers copied from MOD */
MOD_HDRS = \
	$(MOD_HDRDIR)/ModInvertedCoder.h \
	$(MOD_HDRDIR)/ModInvertedException.h \
	$(MOD_HDRDIR)/ModInvertedIDScorePair.h \
	$(MOD_HDRDIR)/ModInvertedLocationListIterator.h \
	$(MOD_HDRDIR)/ModInvertedManager.h \
	$(MOD_HDRDIR)/ModInvertedQuery.h \
	$(MOD_HDRDIR)/ModInvertedQueryBaseNode.h \
	$(MOD_HDRDIR)/ModInvertedQueryLeafNode.h \
	$(MOD_HDRDIR)/ModInvertedQueryNode.h \
	$(MOD_HDRDIR)/ModInvertedQueryParser.h \
	$(MOD_HDRDIR)/ModInvertedRankingScoreCalculator.h \
	$(MOD_HDRDIR)/ModInvertedSearchResult.h \
	$(MOD_HDRDIR)/ModInvertedSmartLocationList.h \
	$(MOD_HDRDIR)/ModInvertedTermLeafNode.h \
	$(MOD_HDRDIR)/ModInvertedTokenizer.h \
	$(MOD_HDRDIR)/ModInvertedTypes.h

/* headers installed in Driver */
LOCAL_EXPORT_HDRS = \
	$(MOD_HDRS) \
	$(HDRDIR)/AutoPointer.h \
	$(HDRDIR)/FeatureSet.h \
	$(HDRDIR)/FileID.h \
	$(HDRDIR)/GetLocationCapsule.h \
	$(HDRDIR)/IndexFile.h \
	$(HDRDIR)/IndexFileSet.h \
	$(HDRDIR)/IntermediateFile.h \
	$(HDRDIR)/IntermediateFileID.h \
	$(HDRDIR)/InvertedFile.h \
	$(HDRDIR)/InvertedList.h \
	$(HDRDIR)/InvertedUnit.h \
	$(HDRDIR)/LeafPage.h \
	$(HDRDIR)/ModInvertedFile.h \
	$(HDRDIR)/MultiListIterator.h \
	$(HDRDIR)/MultiListManager.h \
	$(HDRDIR)/OpenOption.h \
	$(HDRDIR)/OptionDataFile.h \
	$(HDRDIR)/OverflowPage.h \
	$(HDRDIR)/Page.h \
	$(HDRDIR)/PagePointer.h \
	$(HDRDIR)/SearchCapsule.h \
	$(HDRDIR)/SearchResultSet.h \
	$(HDRDIR)/SortParameter.h \
	$(HDRDIR)/FileIDNumber.h \
	$(HDRDIR)/Sign.h \
	$(HDRDIR)/FieldType.h \
	$(HDRDIR)/Types.h

/* headers installed in TOP */
TOP_EXPORT_HDRS = \
	$(HDRDIR)/Module.h \
	$(HDRDIR)/TRMExternalScoreCalculator.h \
	$(MESSAGE_HDRS)

HDRS = \
	$(TOP_EXPORT_HDRS) \
	$(LOCAL_EXPORT_HDRS) \
	$(HDRDIR)/BatchBaseList.h \
	$(HDRDIR)/BatchBaseListIterator.h \
	$(HDRDIR)/BatchList.h \
	$(HDRDIR)/BatchListIterator.h \
	$(HDRDIR)/BatchNolocationList.h \
	$(HDRDIR)/BatchNolocationListIterator.h \
	$(HDRDIR)/BatchNolocationNoTFList.h \
	$(HDRDIR)/BatchNolocationNoTFListIterator.h \
	$(HDRDIR)/BatchListMap.h \
	$(HDRDIR)/BtreeFile.h \
	$(HDRDIR)/BtreePage.h \
	$(HDRDIR)/DocumentIDVectorFile.h \
	$(HDRDIR)/FakeError.h \
	$(HDRDIR)/File.h \
	$(HDRDIR)/Hint.h \
	$(HDRDIR)/InvertedIterator.h \
	$(HDRDIR)/LeafFile.h \
	$(HDRDIR)/ListManager.h \
	$(HDRDIR)/LocationListMap.h \
	$(HDRDIR)/MiddleBaseList.h \
	$(HDRDIR)/MiddleBaseListIterator.h \
	$(HDRDIR)/MiddleList.h \
	$(HDRDIR)/MiddleListIterator.h \
	$(HDRDIR)/MiddleNolocationList.h \
	$(HDRDIR)/MiddleNolocationListIterator.h \
	$(HDRDIR)/MiddleNolocationNoTFList.h \
	$(HDRDIR)/MiddleNolocationNoTFListIterator.h \
	$(HDRDIR)/MiddleListLocationListIterator.h \
	$(HDRDIR)/ModInvertedDocumentLengthFile.h \
	$(HDRDIR)/ModInvertedFile.h \
	$(HDRDIR)/ModInvertedList.h \
	$(HDRDIR)/NormalizerManager.h \
	$(HDRDIR)/OverflowFile.h \
	$(HDRDIR)/Parameter.h \
	$(HDRDIR)/RowIDVectorFile.h \
	$(HDRDIR)/RowIDVectorFile2.h \
	$(HDRDIR)/ShortBaseList.h \
	$(HDRDIR)/ShortBaseListIterator.h \
	$(HDRDIR)/ShortList.h \
	$(HDRDIR)/ShortListIterator.h \
	$(HDRDIR)/ShortNolocationList.h \
	$(HDRDIR)/ShortNolocationListIterator.h \
	$(HDRDIR)/ShortNolocationNoTFList.h \
	$(HDRDIR)/ShortNolocationNoTFListIterator.h \
	$(HDRDIR)/ShortListLocationListIterator.h \
	$(HDRDIR)/TermResourceManager.h \
	$(HDRDIR)/UnaAnalyzerManager.h \
	$(HDRDIR)/VectorFile.h

MESSAGE_OBJS = \
	Message_CantConvertToDocID$O \
	Message_CantConvertToTupleID$O \
	Message_CorrectDisusedIdBlock$O \
	Message_DisusedIdBlock$O \
	Message_IllegalEntryCount$O \
	Message_IllegalIdPage$O \
	Message_IllegalIndex$O \
	Message_IllegalInvertedListType$O \
	Message_IllegalListCount$O \
	Message_IllegalLocPage$O \
	Message_IllegalNextLocPage$O \
	Message_IllegalTotalDocumentLength$O \
	Message_InconsistentDocIDandTupleID$O \
	Message_NextLinkOfLastPage$O \
	Message_NullStringNotInserted$O \
	Message_PreviousLinkOfTopPage$O

MOD_OBJS = \
	ModInvertedAtomicNode$O \
	ModInvertedAtomicOrNode$O \
	ModInvertedBlockedNgramTokenizer$O \
	ModInvertedSearchResult$O \
	ModInvertedBooleanResultLeafNode$O \
	ModInvertedCoder$O \
	ModInvertedDualTokenizer$O \
	ModInvertedExternalScoreCalculator$O \
	ModInvertedExtendedGolombCoder$O \
	ModInvertedFileCapsule$O \
	ModInvertedJapaneseBlocker$O \
	ModInvertedNgramTokenizer$O \
	ModInvertedNormalizedOkapiTfIdfScoreCalculator$O \
	ModInvertedNormalizedOkapiTfScoreCalculator$O \
	ModInvertedNormalizedTfIdfScoreCalculator$O \
	ModInvertedOkapiTfIdfScoreCalculator$O \
	ModInvertedOkapiTfScoreCalculator$O \
	ModInvertedOperatorAndNode$O \
	ModInvertedOperatorAndNotNode$O \
	ModInvertedOperatorOrNode$O \
	ModInvertedOperatorScaleNode$O \
	ModInvertedOperatorWordNode$O \
	ModInvertedOperatorWordNodeLocationListIterator$O \
	ModInvertedOrderedDistanceLocationListIterator$O \
	ModInvertedOrderedDistanceNode$O \
	ModInvertedParameterizedExpGolombCoder$O \
	ModInvertedQuery$O \
	ModInvertedQueryBaseNode$O \
	ModInvertedQueryInternalNode$O \
	ModInvertedQueryNode$O \
	ModInvertedQueryParser$O \
	ModInvertedRankingResultLeafNode$O \
	ModInvertedRankingScoreCalculator$O \
	ModInvertedRankingScoreCombiner$O \
	ModInvertedRankingScoreNegator$O \
	ModInvertedSimpleTokenLeafNode$O \
	ModInvertedSmartLocationList$O \
	ModInvertedTermLeafNode$O \
	ModInvertedTfIdfScoreCalculator$O \
	ModInvertedTokenizer$O \
	ModInvertedUnicodeCharBlocker$O \
	ModInvertedWindowBaseNode$O \
	ModInvertedWordBaseNode$O \
	ModInvertedWordOrderedDistanceNode$O \
	ModTerm$O \
	ModTermElement$O \
	ModTermException$O \
	ModTermMap$O \
	ModTermPattern$O \
	ModTermStringFile$O

INV_OBJS = \
	BatchBaseList$O \
	BatchList$O \
	BatchNolocationList$O \
	BatchNolocationNoTFList$O \
	BatchListMap$O \
	BtreeFile$O \
	BtreePage$O \
	DocumentIDVectorFile$O \
	FeatureSet$O \
	File$O \
	FileID$O \
	GetLocationCapsule$O \
	IndexFile$O \
	IndexFileSet$O \
	IntermediateFile$O \
	IntermediateFileID$O \
	Inverted$O \
	InvertedFile$O \
	InvertedIterator$O \
	InvertedList$O \
	InvertedUnit$O \
	LeafFile$O \
	LeafPage$O \
	ListManager$O \
	LocationListMap$O \
	MiddleBaseList$O \
	MiddleBaseListIterator$O \
	MiddleList$O \
	MiddleListIterator$O \
	MiddleNolocationList$O \
	MiddleNolocationListIterator$O \
	MiddleNolocationNoTFList$O \
	MiddleNolocationNoTFListIterator$O \
	MiddleListLocationListIterator$O \
	ModInvertedFile$O \
	MultiListIterator$O \
	MultiListManager$O \
	NormalizerManager$O \
	OpenOption$O \
	OverflowFile$O \
	OverflowPage$O \
	Page$O \
	RowIDVectorFile$O \
	RowIDVectorFile2$O \
	SearchCapsule$O \
	SearchResultSet$O \
	ShortBaseList$O \
	ShortBaseListIterator$O \
	ShortList$O \
	ShortListIterator$O \
	ShortNolocationList$O \
	ShortNolocationListIterator$O \
	ShortNolocationNoTFList$O \
	ShortNolocationNoTFListIterator$O \
	ShortListLocationListIterator$O \
	TermResourceManager$O \
	UnaAnalyzerManager$O

OBJS1 = $(MESSAGE_OBJS)
OBJS2 = $(INV_OBJS)
OBJS3 = $(MOD_OBJS)

OBJS = \
	$(OBJS1) \
	$(OBJS2) \
	$(OBJS3)

#ifdef SYD_DLL
EXPORTFLAGS = \
	-DSYD_INVERTED_EXPORT_FUNCTION
#endif
#ifdef SYD_C_GCC
USE_LARGE_VECTOR =
#else
USE_LARGE_VECTOR = \
	-DSYD_USE_LARGE_VECTOR
#endif
EXTRACFLAGS = \
	$(EXPORTFLAGS) \
	-DSYDNEY \
	-DSYD_INVERTED \
	-DSYD_CLUSTERING \
	-DRANKING_SEARCH \
	-DMOD_IMPORT_NORM_DLL \
	-DMOD_IMPORT_UNA_DLL \
	$(USE_LARGE_VECTOR) \
	-I$(SRCDIR)$(S)FtsInverted$(S)FtsInverted \
	-I..$(S)..$(S)include$(S)$(MODULE) \
	-I..$(S)..$(S)..$(S)include$(S)$(MODULE) \
	-I$(HDRDIR)
EXTRALDFLAGS =
EXTRAPURIFYFLAGS =
EXTRAQUANTIFYFLAGS =
EXTRAPURECOVFLAGS =
EXTRALOCALCFLAGS =

KERNEL_LIBS = \
	SyKernel$L

LDLIBS = \
	$(KERNEL_LIBS) \
	SyDrvCom$L \
	$(MODLIBS)

/********************************************************/

#ifdef SYD_C_MS7_0
FORCEMULTIPLE = /force:multiple /* ModObjectがなぜか多重定義になってしまう */
#endif
EXTRALOCALDLLFLAGS =  \
	$(FORCEMULTIPLE)
RCLOCALFLAGS = /i $(VERINFO_INCL)

TARGET = $P$(TARGET_BASE)$L
DLLTARGET = $P$(TARGET_BASE)$D
OLTARGET = $(MODULE).ol

ALLTARGETS = \
	$(MESSAGE_TARGET) \
	$(TARGET)

/*
 * all
 */
AllTarget($(ALLTARGETS))

LibraryTarget($(TARGET), $(OBJS))

DLLTarget($(DLLTARGET), $(RESOURCE) $(OBJS) $(TARGET_EXPORT))

ObjectListTarget3($(OLTARGET), $(OBJS1), $(OBJS2), $(OBJS3), $(TOP_INSTALL_DIR))

/*
 * message
 */
MessageTarget($(MESSAGE_TARGET), $(MESSAGE_DEFINITION), $(BUILD_JAR))

/*
 * install library and header
 */
InstallHeaderTarget($(TOP_EXPORT_HDRS), $(TOP_EXPORT_HDRDIR))
InstallHeaderTarget($(LOCAL_EXPORT_HDRS), $(LOCAL_EXPORT_HDRDIR))
InstallLibraryTarget($(TARGET), $(TOP_INSTALL_DIR))
InstallDLLTarget($(DLLTARGET), $(TOP_INSTALL_DIR))

/*
 * clean
 */
CleanTarget($(ALLTARGETS))
CleanTarget($(DLLTARGET))
CleanTarget($(INV_OBJS))
CleanTarget($(MESSAGE_OBJS))
CleanTarget($(MOD_OBJS))
CleanTarget($(TARGET_BASE).exp)

#if defined(OS_WINDOWSNT4_0) || defined(OS_WINDOWS98)
ResourceTarget($(RESOURCE), $(VERINFODIR)\$(MODULE).rc)
#endif

#include "Makefile.h"

$(MESSAGE_HDRS): $(MESSAGE_TARGET)
$(MESSAGE_OBJS): $(MESSAGE_TARGET)

/*
  Copyright 1997, 2023 Ricoh Company, Ltd.
  All rights reserved.
*/
