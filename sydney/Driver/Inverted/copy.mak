MOD_VERSION=v1.7.dist
SRC_PATH1=s:/mod/obj/$(MOD_VERSION)/windows/m.library/m.driver/m.inverted
SRC_PATH2=s:/mod/obj/$(MOD_VERSION)/windows/m.library/m.manager
SRC_PATH3=s:/mod/obj/$(MOD_VERSION)/windows/m.library/m.term
DIST_PATH=FtsInverted
CP=cp

CPP_FILES = \
	ModInvertedAtomicNode.cpp \
	ModInvertedAtomicOrNode.cpp \
	ModInvertedBlockedNgramTokenizer.cpp \
	ModInvertedBooleanResult.cpp \
	ModInvertedBooleanResultLeafNode.cpp \
	ModInvertedCoder.cpp \
	ModInvertedDualTokenizer.cpp \
	ModInvertedExtendedGolombCoder.cpp \
	ModInvertedFileCapsule.cpp \
	ModInvertedJapaneseBlocker.cpp \
	ModInvertedNgramTokenizer.cpp \
	ModInvertedNormalizedOkapiTfIdfScoreCalculator.cpp \
	ModInvertedNormalizedOkapiTfScoreCalculator.cpp \
	ModInvertedNormalizedTfIdfScoreCalculator.cpp \
	ModInvertedOkapiTfIdfScoreCalculator.cpp \
	ModInvertedOkapiTfScoreCalculator.cpp \
	ModInvertedOperatorAndNode.cpp \
	ModInvertedOperatorAndNotNode.cpp \
	ModInvertedOperatorOrNode.cpp \
	ModInvertedOperatorScaleNode.cpp \
	ModInvertedOperatorWordNode.cpp \
	ModInvertedOperatorWordNodeLocationListIterator.cpp \
	ModInvertedOrderedDistanceLocationListIterator.cpp \
	ModInvertedOrderedDistanceNode.cpp \
	ModInvertedParameterizedExpGolombCoder.cpp \
	ModInvertedQuery.cpp \
	ModInvertedQueryBaseNode.cpp \
	ModInvertedQueryInternalNode.cpp \
	ModInvertedQueryNode.cpp \
	ModInvertedQueryParser.cpp \
	ModInvertedRankingResult.cpp \
	ModInvertedRankingResultLeafNode.cpp \
	ModInvertedRankingScoreCalculator.cpp \
	ModInvertedRankingScoreCombiner.cpp \
	ModInvertedRankingScoreNegator.cpp \
	ModInvertedSimpleTokenLeafNode.cpp \
	ModInvertedSmartLocationList.cpp \
	ModInvertedTermLeafNode.cpp \
	ModInvertedTfIdfScoreCalculator.cpp \
	ModInvertedTokenizer.cpp \
	ModInvertedUnicodeCharBlocker.cpp \
	ModInvertedWindowBaseNode.cpp \
	ModInvertedWordBaseNode.cpp \
	ModInvertedWordOrderedDistanceNode.cpp
INV_H_FILES = \
	ModInvertedASumScoreCombiner.h \
	ModInvertedAndNotScoreNegator.h \
	ModInvertedAtomicNode.h \
	ModInvertedAtomicOrNode.h \
	ModInvertedBlockedNgramTokenizer.h \
	ModInvertedBooleanResult.h \
	ModInvertedBooleanResultLeafNode.h \
	ModInvertedCoder.h \
	ModInvertedCompressedLocationListIterator.h \
	ModInvertedDocumentElement.h \
	ModInvertedDualTokenizer.h \
	ModInvertedEndNodeLocationListIterator.h \
	ModInvertedException.h \
	ModInvertedExpGolombCoder.h \
	ModInvertedExtendedGolombCoder.h \
	ModInvertedExtendedGolombCoderArgument.h \
	ModInvertedFileCapsule.h \
	ModInvertedIDScorePair.h \
	ModInvertedIterator.h \
	ModInvertedJapaneseBlocker.h \
	ModInvertedLocationListIterator.h \
	ModInvertedLocationNodeLocationListIterator.h \
	ModInvertedLocationNodeTemplate.h \
	ModInvertedManager.h \
	ModInvertedNgramTokenizer.h \
	ModInvertedNormalizedOkapiTfIdfScoreCalculator.h \
	ModInvertedNormalizedOkapiTfScoreCalculator.h \
	ModInvertedNormalizedTfIdfScoreCalculator.h \
	ModInvertedOkapiTfIdfScoreCalculator.h \
	ModInvertedOkapiTfScoreCalculator.h \
	ModInvertedOperatorAndNode.h \
	ModInvertedOperatorAndNotNode.h \
	ModInvertedOperatorEndNode.h \
	ModInvertedOperatorLocationNode.h \
	ModInvertedOperatorOrNode.h \
	ModInvertedOperatorScaleNode.h \
	ModInvertedOperatorWindowNode.h \
	ModInvertedOperatorWordNode.h \
	ModInvertedOperatorWordNodeLocationListIterator.h \
	ModInvertedOrLocationListIterator.h \
	ModInvertedOrderedDistanceLocationListIterator.h \
	ModInvertedOrderedDistanceNode.h \
	ModInvertedOrderedOperatorWindowLocationListIterator.h \
	ModInvertedOrderedSimpleWindowLocationListIterator.h \
	ModInvertedOverflowFileHeader.h \
	ModInvertedParameterizedExpGolombCoder.h \
	ModInvertedParameterizedExpGolombCoderArgument.h \
	ModInvertedProdScoreCombiner.h \
	ModInvertedQuery.h \
	ModInvertedQueryBaseNode.h \
	ModInvertedQueryInternalNode.h \
	ModInvertedQueryLeafNode.h \
	ModInvertedQueryNode.h \
	ModInvertedQueryParser.h \
	ModInvertedRankingResult.h \
	ModInvertedRankingResultLeafNode.h \
	ModInvertedRankingScoreCalculator.h \
	ModInvertedRankingScoreCombiner.h \
	ModInvertedRankingScoreNegator.h \
	ModInvertedRegexLeafNode.h \
	ModInvertedSimpleTokenLeafNode.h \
	ModInvertedSimpleWindowNode.h \
	ModInvertedSmartLocationList.h \
	ModInvertedSumScoreCombiner.h \
	ModInvertedTermElement.h \
	ModInvertedTermLeafNode.h \
	ModInvertedTfIdfScoreCalculator.h \
	ModInvertedTokenizer.h \
	ModInvertedTypes.h \
	ModInvertedUnaryCoder.h \
	ModInvertedUncompressedLocationListIterator.h \
	ModInvertedUnicodeCharBlocker.h \
	ModInvertedUnorderedOperatorWindowLocationListIterator.h \
	ModInvertedUnorderedSimpleWindowLocationListIterator.h \
	ModInvertedVoidCoder.h \
	ModInvertedWindowBaseNode.h \
	ModInvertedWindowLocationListIterator.h \
	ModInvertedWindowNodeTemplate.h \
	ModInvertedWordBaseNode.h \
	ModInvertedWordOrderedDistanceNode.h
MANAGER_H_FILES = \
	ModCheck.h \
	ModExtendedList.h \
	ModLogicalFile.h \
	ModLogicalFileCatalog.h \
	ModLogicalFileID.h \
	ModLogicalFileVersion.h \
	ModLogicalLogID.h \
	ModObjectID.h \
	ModPhysicalFileID.h \
	ModRequest.h \
	ModRequestID.h \
	ModSystemCatalogEntryID.h
TERM_CPP_FILES = \
	ModTerm.cpp \
	ModTermElement.cpp \
	ModTermException.cpp \
	ModTermMap.cpp \
	ModTermPattern.cpp \
	ModTermStringFile.cpp
TERM_H_FILES = \
	ModTerm.h \
	ModTermDocument.h \
	ModTermElement.h \
	ModTermException.h \
	ModTermMap.h \
	ModTermPattern.h \
	ModTermStringFile.h
TERM_AUTO_H_FILES = \
	ModTermExceptionMessage.h

all: install_inv install_term
install_inv: install_cpp install_inv_h install_manager_h
install_term: install_term_cpp install_term_h install_term_auto_h
install_cpp: $(CPP_FILES)
$(CPP_FILES):
	$(CP) $(SRC_PATH1)/c.O/$@ $(DIST_PATH)
install_inv_h: $(INV_H_FILES)
$(INV_H_FILES):
	$(CP) $(SRC_PATH1)/includewin/$@ $(DIST_PATH)/FtsInverted
install_manager_h: $(MANAGER_H_FILES)
$(MANAGER_H_FILES):
	$(CP) $(SRC_PATH2)/includewin/$@ $(DIST_PATH)/FtsInverted
install_term_cpp: $(TERM_CPP_FILES)
$(TERM_CPP_FILES):
	$(CP) $(SRC_PATH3)/c.O/$@ $(DIST_PATH)
install_term_h: $(TERM_H_FILES)
$(TERM_H_FILES):
	$(CP) $(SRC_PATH3)/includewin/$@ $(DIST_PATH)/FtsInverted
install_term_auto_h: $(TERM_AUTO_H_FILES)
$(TERM_AUTO_H_FILES):
	$(CP) $(SRC_PATH3)/c.O/$@ $(DIST_PATH)/FtsInverted
