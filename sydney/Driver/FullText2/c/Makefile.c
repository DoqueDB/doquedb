/*
 * Makefile.c --- Driver/FullText2/c
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

MODULE = FullText2
TARGET_BASE = SyDrvFts2
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
	$(HDRDIR)/Message_PreviousLinkOfTopPage.h \
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
	$(HDRDIR)/MessageNumber_PreviousLinkOfTopPage.h

/* headers installed in TOP */
TOP_EXPORT_HDRS = \
	$(HDRDIR)/KeyID.h \
	$(HDRDIR)/Module.h \
	$(HDRDIR)/Types.h \
	$(HDRDIR)/ScoreCalculator.h \
	$(MESSAGE_HDRS)

HDRS = \
	$(TOP_EXPORT_HDRS) \
	$(LOCAL_EXPORT_HDRS) \
	$(HDRDIR)/AndLeafNode.h \
	$(HDRDIR)/ArrayLeafNode.h \
	$(HDRDIR)/ASumScoreCombiner.h \
	$(HDRDIR)/AtomicOrLeafLocationListIterator.h \
	$(HDRDIR)/AtomicOrLeafNode.h \
	$(HDRDIR)/AutoPointer.h \
	$(HDRDIR)/BatchBaseList.h \
	$(HDRDIR)/BatchList.h \
	$(HDRDIR)/BatchListManager.h \
	$(HDRDIR)/BatchListMap.h \
	$(HDRDIR)/BatchNolocationList.h \
	$(HDRDIR)/BatchNolocationNoTFList.h \
	$(HDRDIR)/Blocker.h \
	$(HDRDIR)/BtreeFile.h \
	$(HDRDIR)/BtreePage.h \
	$(HDRDIR)/DelayListIterator.h \
	$(HDRDIR)/DelayListManager.h \
	$(HDRDIR)/DoSearch.h \
	$(HDRDIR)/DualTokenizer.h \
	$(HDRDIR)/DummyListIterator.h \
	$(HDRDIR)/DummyListManager.h \
	$(HDRDIR)/ExactWordLeafLocationListIterator.h \
	$(HDRDIR)/Executor.h \
	$(HDRDIR)/ExpungeIDVectorFile.h \
	$(HDRDIR)/ExpungeMP.h \
	$(HDRDIR)/ExternalScoreCalculator.h \
	$(HDRDIR)/FakeError.h \
	$(HDRDIR)/FeatureList.h \
	$(HDRDIR)/FeatureSet.h \
	$(HDRDIR)/FieldMask.h \
	$(HDRDIR)/FieldType.h \
	$(HDRDIR)/File.h \
	$(HDRDIR)/FileDriver.h \
	$(HDRDIR)/FileID.h \
	$(HDRDIR)/FullTextFile.h \
	$(HDRDIR)/GeneralBlocker.h \
	$(HDRDIR)/GetCount.h \
	$(HDRDIR)/GetDocumentFrequency.h \
	$(HDRDIR)/GetDocumentFrequencyForEstimate.h \
	$(HDRDIR)/GetResult.h \
	$(HDRDIR)/HeadLeafLocationListIterator.h \
	$(HDRDIR)/HeadLeafNode.h \
	$(HDRDIR)/IDVectorFile.h \
	$(HDRDIR)/IndexFile.h \
	$(HDRDIR)/InsertMP.h \
	$(HDRDIR)/InvertedBatch.h \
	$(HDRDIR)/InvertedCountUnit.h \
	$(HDRDIR)/InvertedExpungeBatch.h \
	$(HDRDIR)/InvertedExpungeUnit.h \
	$(HDRDIR)/InvertedFile.h \
	$(HDRDIR)/InvertedIterator.h \
	$(HDRDIR)/InvertedList.h \
	$(HDRDIR)/InvertedListCore.h \
	$(HDRDIR)/InvertedMultiUnit.h \
	$(HDRDIR)/InvertedSection.h \
	$(HDRDIR)/InvertedUnit.h \
	$(HDRDIR)/InvertedUpdateFile.h \
	$(HDRDIR)/JapaneseBlocker.h \
	$(HDRDIR)/JapaneseBlocker1.h \
	$(HDRDIR)/JapaneseBlocker2.h \
	$(HDRDIR)/JapaneseBlocker3.h \
	$(HDRDIR)/LeafFile.h \
	$(HDRDIR)/LeafNode.h \
	$(HDRDIR)/LeafPage.h \
	$(HDRDIR)/ListIterator.h \
	$(HDRDIR)/ListIteratorWithExpungeList.h \
	$(HDRDIR)/ListIteratorWithMax.h \
	$(HDRDIR)/ListIteratorWithWhiteList.h \
	$(HDRDIR)/ListManager.h \
	$(HDRDIR)/ListManagerWithExpungeList.h \
	$(HDRDIR)/ListManagerWithWhiteList.h \
	$(HDRDIR)/LocationList.h \
	$(HDRDIR)/LocationListIterator.h \
	$(HDRDIR)/LocationListManager.h \
	$(HDRDIR)/LogicalInterface.h \
	$(HDRDIR)/LogicalOperatorNode.h \
	$(HDRDIR)/MainFile.h \
	$(HDRDIR)/MaxScoreCombiner.h \
	$(HDRDIR)/MergeDaemon.h \
	$(HDRDIR)/MergeReserve.h \
	$(HDRDIR)/MiddleBaseList.h \
	$(HDRDIR)/MiddleBaseListIterator.h \
	$(HDRDIR)/MiddleList.h \
	$(HDRDIR)/MiddleListIterator.h \
	$(HDRDIR)/MiddleListLocationListIterator.h \
	$(HDRDIR)/MiddleNolocationList.h \
	$(HDRDIR)/MiddleNolocationListIterator.h \
	$(HDRDIR)/MiddleNolocationNoTFList.h \
	$(HDRDIR)/MiddleNolocationNoTFListIterator.h \
	$(HDRDIR)/MinScoreCombiner.h \
	$(HDRDIR)/MultiFile.h \
	$(HDRDIR)/MultiListIterator.h \
	$(HDRDIR)/MultiListIteratorImpl.h \
	$(HDRDIR)/MultiListManager.h \
	$(HDRDIR)/MultiVectorFile.h \
	$(HDRDIR)/NgramTokenizer.h \
	$(HDRDIR)/NormalizedOkapiTfIdfScoreCalculator.h \
	$(HDRDIR)/NormalizedOkapiTfScoreCalculator.h \
	$(HDRDIR)/NormalizedTfIdfScoreCalculator.h \
	$(HDRDIR)/NormalizerManager.h \
	$(HDRDIR)/NormalLeafLocationListIterator.h \
	$(HDRDIR)/NormalLeafNode.h \
	$(HDRDIR)/NormalLocationList.h \
	$(HDRDIR)/NormalLocationListIterator.h \
	$(HDRDIR)/NormalShortLeafLocationListIterator.h \
	$(HDRDIR)/NormalShortLeafNode.h \
	$(HDRDIR)/OkapiTfIdfScoreCalculator.h \
	$(HDRDIR)/OkapiTfScoreCalculator.h \
	$(HDRDIR)/OpenOption.h \
	$(HDRDIR)/OperatorAddNode.h \
	$(HDRDIR)/OperatorAndNode.h \
	$(HDRDIR)/OperatorAndNotNode.h \
	$(HDRDIR)/OperatorNode.h \
	$(HDRDIR)/OperatorOrNode.h \
	$(HDRDIR)/OperatorTermNode.h \
	$(HDRDIR)/OperatorTermNodeAnd.h \
	$(HDRDIR)/OperatorTermNodeArray.h \
	$(HDRDIR)/OperatorTermNodeOr.h \
	$(HDRDIR)/OperatorTermNodeScore.h \
	$(HDRDIR)/OperatorTermNodeSingle.h \
	$(HDRDIR)/OperatorTermNodeTf.h \
	$(HDRDIR)/OperatorWeightNode.h \
	$(HDRDIR)/OptionDataFile.h \
	$(HDRDIR)/OrderedDistanceLeafLocationListIterator.h \
	$(HDRDIR)/OrderedDistanceLeafNode.h \
	$(HDRDIR)/OrLeafNode.h \
	$(HDRDIR)/OtherInformationFile.h \
	$(HDRDIR)/OverflowFile.h \
	$(HDRDIR)/OverflowPage.h \
	$(HDRDIR)/Page.h \
	$(HDRDIR)/PagePointer.h \
	$(HDRDIR)/Parameter.h \
	$(HDRDIR)/ProdScoreCombiner.h \
	$(HDRDIR)/Query.h \
	$(HDRDIR)/ResourceManager.h \
	$(HDRDIR)/ResultSet.h \
	$(HDRDIR)/RowIDVectorFile.h \
	$(HDRDIR)/RowIDVectorFile2.h \
	$(HDRDIR)/ScoreCalculator.h \
	$(HDRDIR)/ScoreCalculatorImpl.h \
	$(HDRDIR)/ScoreCombiner.h \
	$(HDRDIR)/SearchCapsule.h \
	$(HDRDIR)/SearchInformation.h \
	$(HDRDIR)/SearchInformationArray.h \
	$(HDRDIR)/SearchInformationConcatinate.h \
	$(HDRDIR)/SearchInformationInSection.h \
	$(HDRDIR)/SearchResultSet.h \
	$(HDRDIR)/ShortBaseList.h \
	$(HDRDIR)/ShortBaseListIterator.h \
	$(HDRDIR)/ShortLeafLocationListIterator.h \
	$(HDRDIR)/ShortLeafNode.h \
	$(HDRDIR)/ShortLeafNodeCompatible.h \
	$(HDRDIR)/ShortList.h \
	$(HDRDIR)/ShortListIterator.h \
	$(HDRDIR)/ShortListLocationListIterator.h \
	$(HDRDIR)/ShortNolocationList.h \
	$(HDRDIR)/ShortNolocationListIterator.h \
	$(HDRDIR)/ShortNolocationNoTFList.h \
	$(HDRDIR)/ShortNolocationNoTFListIterator.h \
	$(HDRDIR)/SimpleLeafNode.h \
	$(HDRDIR)/SimpleListManager.h \
	$(HDRDIR)/SimpleResultSet.h \
	$(HDRDIR)/SimpleWordLeafLocationListIterator.h \
	$(HDRDIR)/SmartLocationList.h \
	$(HDRDIR)/SmartLocationListIterator.h \
	$(HDRDIR)/SumScoreCombiner.h \
	$(HDRDIR)/TailLeafLocationListIterator.h \
	$(HDRDIR)/TailLeafNode.h \
	$(HDRDIR)/TfIdfScoreCalculator.h \
	$(HDRDIR)/Tokenizer.h \
	$(HDRDIR)/UnaryLeafNode.h \
	$(HDRDIR)/UpdateListManager.h \
	$(HDRDIR)/VariableFile.h \
	$(HDRDIR)/VectorFile.h \
	$(HDRDIR)/WithinOrderedLeafLocationListIterator.h \
	$(HDRDIR)/WithinOrderedLeafNode.h \
	$(HDRDIR)/WithinUnorderedLeafLocationListIterator.h \
	$(HDRDIR)/WithinUnorderedLeafNode.h \
	$(HDRDIR)/WordLeafLocationListIterator.h \
	$(HDRDIR)/WordLeafNode.h

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
	ModInvertedCoder$O \
	ModInvertedExtendedGolombCoder$O \
	ModInvertedParameterizedExpGolombCoder$O

INV_OBJS = \
	AndLeafNode$O \
	ArrayLeafNode$O \
	ASumScoreCombiner$O \
	AtomicOrLeafNode$O \
	BatchBaseList$O \
	BatchList$O \
	BatchListManager$O \
	BatchListMap$O \
	BatchNolocationList$O \
	BatchNolocationNoTFList$O \
	Blocker$O \
	BtreeFile$O \
	BtreePage$O \
	DBGetFileDriver$O \
	DelayListIterator$O \
	DelayListManager$O \
	DoSearch$O \
	DualTokenizer$O \
	DummyListIterator$O \
	DummyListManager$O \
	ExactWordLeafLocationListIterator$O \
	Executor$O \
	ExpungeIDVectorFile$O \
	ExpungeMP$O \
	ExternalScoreCalculator$O \
	FeatureSet$O \
	FieldMask$O \
	File$O \
	FileDriver$O \
	FileID$O \
	FullTextFile$O \
	GeneralBlocker$O \
	GetCount$O \
	GetDocumentFrequency$O \
	GetDocumentFrequencyForEstimate$O \
	GetResult$O \
	HeadLeafLocationListIterator$O \
	HeadLeafNode$O \
	IDVectorFile$O \
	IndexFile$O \
	InsertMP$O \
	Inverted$O \
	InvertedBatch$O \
	InvertedCountUnit$O \
	InvertedExpungeBatch$O \
	InvertedExpungeUnit$O \
	InvertedFile$O \
	InvertedIterator$O \
	InvertedList$O \
	InvertedListCore$O \
	InvertedMultiUnit$O \
	InvertedSection$O \
	InvertedUnit$O \
	InvertedUpdateFile$O \
	JapaneseBlocker$O \
	JapaneseBlocker1$O \
	JapaneseBlocker2$O \
	JapaneseBlocker3$O \
	LeafFile$O \
	LeafNode$O \
	LeafPage$O \
	ListIterator$O \
	ListIteratorWithExpungeList$O \
	ListIteratorWithMax$O \
	ListIteratorWithWhiteList$O \
	ListManager$O \
	ListManagerWithExpungeList$O \
	ListManagerWithWhiteList$O \
	LocationList$O \
	LocationListIterator$O \
	LocationListManager$O \
	LogicalInterface$O \
	LogicalOperatorNode$O \
	MaxScoreCombiner$O \
	MergeDaemon$O \
	MergeReserve$O \
	MiddleBaseList$O \
	MiddleBaseListIterator$O \
	MiddleList$O \
	MiddleListIterator$O \
	MiddleListLocationListIterator$O \
	MiddleNolocationList$O \
	MiddleNolocationListIterator$O \
	MiddleNolocationNoTFList$O \
	MiddleNolocationNoTFListIterator$O \
	MinScoreCombiner$O \
	MultiFile$O \
	MultiListIterator$O \
	MultiListIteratorImpl$O \
	MultiListManager$O \
	MultiVectorFile$O \
	NgramTokenizer$O \
	NormalizedOkapiTfIdfScoreCalculator$O \
	NormalizedOkapiTfScoreCalculator$O \
	NormalizedTfIdfScoreCalculator$O \
	NormalLeafLocationListIterator$O \
	NormalLeafNode$O \
	NormalLocationList$O \
	NormalShortLeafLocationListIterator$O \
	NormalShortLeafNode$O \
	OkapiTfIdfScoreCalculator$O \
	OkapiTfScoreCalculator$O \
	OpenOption$O \
	OperatorAddNode$O \
	OperatorAndNode$O \
	OperatorAndNotNode$O \
	OperatorNode$O \
	OperatorOrNode$O \
	OperatorTermNode$O \
	OperatorTermNodeAnd$O \
	OperatorTermNodeArray$O \
	OperatorTermNodeOr$O \
	OperatorTermNodeScore$O \
	OperatorTermNodeSingle$O \
	OperatorTermNodeTf$O \
	OperatorWeightNode$O \
	OrderedDistanceLeafLocationListIterator$O \
	OrderedDistanceLeafNode$O \
	OrLeafNode$O \
	OtherInformationFile$O \
	OverflowFile$O \
	OverflowPage$O \
	Page$O \
	ProdScoreCombiner$O \
	Query$O \
	ResultSet$O \
	ScoreCalculatorImpl$O \
	SearchInformation$O \
	SearchInformationArray$O \
	SearchInformationConcatinate$O \
	SearchInformationInSection$O \
	ShortBaseList$O \
	ShortBaseListIterator$O \
	ShortLeafLocationListIterator$O \
	ShortLeafNode$O \
	ShortLeafNodeCompatible$O \
	ShortList$O \
	ShortListIterator$O \
	ShortListLocationListIterator$O \
	ShortNolocationList$O \
	ShortNolocationListIterator$O \
	ShortNolocationNoTFList$O \
	ShortNolocationNoTFListIterator$O \
	SimpleLeafNode$O \
	SimpleListManager$O \
	SimpleResultSet$O \
	SimpleWordLeafLocationListIterator$O \
	SmartLocationList$O \
	SumScoreCombiner$O \
	TailLeafLocationListIterator$O \
	TailLeafNode$O \
	TfIdfScoreCalculator$O \
	Tokenizer$O \
	UnaryLeafNode$O \
	UpdateListManager$O \
	VariableFile$O \
	VectorFile$O \
	WithinOrderedLeafLocationListIterator$O \
	WithinOrderedLeafNode$O \
	WithinUnorderedLeafLocationListIterator$O \
	WithinUnorderedLeafNode$O \
	WordLeafLocationListIterator$O \
	WordLeafNode$O

OBJS1 = $(MESSAGE_OBJS)
OBJS2 = $(INV_OBJS)
OBJS3 = $(MOD_OBJS)

OBJS = \
	$(OBJS1) \
	$(OBJS2) \
	$(OBJS3)

#ifdef SYD_DLL
EXPORTFLAGS = \
	-DSYD_FULLTEXT2_EXPORT_FUNCTION \
	-DSYD_EXPORT_FUNCTION
#endif
USE_LARGE_VECTOR = \
	-DSYD_USE_LARGE_VECTOR
EXTRACFLAGS = \
	$(EXPORTFLAGS) \
	-DSYDNEY \
	-DSYD_INVERTED \
	-DSYD_CLUSTERING \
	-DRANKING_SEARCH \
	-DMOD_IMPORT_NORM_DLL \
	-DMOD_IMPORT_UNA_DLL \
	$(USE_LARGE_VECTOR) \
	-I..$(S)..$(S)..$(S)Kernel$(S)include \
	-I$(SRCDIR)$(S)FtsInverted$(S)FtsInverted
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
    Copyright (c) 1997, 2023 Ricoh Company, Ltd.
    All rights reserved.
*/
