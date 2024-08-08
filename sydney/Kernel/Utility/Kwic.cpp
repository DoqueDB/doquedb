// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// Kwic.cpp --
// 
// Copyright (c) 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Utility";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "Utility/Kwic.h"
#include "Utility/Parameter.h"

#include "Common/Assert.h"
#include "Common/DataArrayData.h"
#include "Common/StringData.h"
#include "Common/UnicodeString.h"
#include "Common/UnsignedIntegerData.h"
#include "Exception/BadArgument.h"

#include "ModUnicodeOstrStream.h"

_TRMEISTER_USING
_TRMEISTER_UTILITY_USING

namespace
{
	//
	//	VARIABLE local
	//	_$$::_cEscapeMethod_HTML --
	//
	const ModUnicodeString _cEscapeMethod_None("NONE");
	
	//
	//	VARIABLE local
	//	_$$::_cEscapeMethod_HTML --
	//
	const ModUnicodeString _cEscapeMethod_HTML("HTML");
	const ModUnicodeString _cEscapeMethod_HTML_usWquate("&quot;");
	const ModUnicodeString _cEscapeMethod_HTML_usAmpersand("&amp;");
	const ModUnicodeString _cEscapeMethod_HTML_usLcompare("&lt;");
	const ModUnicodeString _cEscapeMethod_HTML_usRcompare("&gt;");
	
	//
	//	VARIABLE local
	//
	const ModUnicodeString _cRoughKwicSize("RoughKwicSize");
	const ModUnicodeString _cSearchTermList("SearchTermList");
	const ModUnicodeString _cUnaParameterKey("UnaParameterKey");
	const ModUnicodeString _cUnaParameterValue("UnaParameterValue");
	
	//
	//	VARIABLE local
	//	_$$::_cKwicMarginPercent --
	//
	ParameterInteger _cKwicMarginPercent("Utility_KwicMarginPercent", 10);

	//
	//	VARIABLE local
	//	_$$::_cKwicUseMatchMode --
	//
	//	ARGUMENTS
	//	true: The mode which is set in SearchTermData is used.
	//	false: The string mode is always used.
	//
	ParameterBoolean _cKwicUseMatchMode("Utility_KwicUseMatchMode", true);
	
	//
	//	VARIABLE local
	//	_$$::_cKwicWordHeadMode --
	//
	//	NOTE
	//	This value is used when the mode is word head and _cKwicUseMatchMode is
	//	true.
	//
	//	ARGUMENTS
	//	true: The word head mode is always used.
	//	false: The string mode is always used.
	//
	ParameterBoolean _cKwicWordHeadMode("Utility_KwicWordHeadMode", true);
	
	//
	//	VARIABLE local
	//	_$$::_cKwicWordTailMode --
	//
	ParameterBoolean _cKwicWordTailMode("Utility_KwicWordTailMode", true);
	
	//
	//	VARIABLE local
	//	_$$::_cKwicSimpleWordMode --
	//
	ParameterBoolean _cKwicSimpleWordMode("Utility_KwicSimpleWordMode", true);
	
	//
	//	VARIABLE local
	//	_$$::_cKwicExactWordMatchMode --
	//
	ParameterBoolean _cKwicExactWordMode("Utility_KwicExactWordMode", true);
	
	//
	//	VARIABLE local
	//	_$$::_cKwicMultiMatchMode --
	//
	//	NOTE
	//	This value is used when the mode is multi language, _cKwicUseMatchMode
	//	is true and _cKwicExactWordMode is true.
	//
	//	ARGUMENTS
	//	true: The mode is decided by the character type of the search term.
	//		including CJK		-> string
	//		not including CJK	-> exact word
	//	false: The string mode is always used.
	//
	ParameterBoolean _cKwicMultiMatchMode("Utility_KwicMultiMatchMode", true);
	
	//
	//	VARIABLE local
	//	_$$::_cKwicExpandLimit --
	//
	ParameterInteger _cKwicExpandLimit("Utility_KwicExpandLimit", 16);
	
	//
	//	VARIABLE local
	//	_$$::_cKwicExpandWordSeparator --
	//
	//	NOTE
	//	This value is used when the mode is word.
	//
	//	ARGUMENTS
	//	true: The search term is expanded with a word separator.
	//	false: The search term is NOT expanded.
	//
	ParameterBoolean _cKwicExpandWordSeparator(
		"Utility_KwicExpandWordSeparator", true);
	
	//
	//	CLASS local
	//	_$$::_WordInfoNormalizedTailLess
	//
	class _WordInfoNormalizedTailLess
	{
	public:
		ModBoolean operator()(const Tokenizer::WordInfoElement& x,
							  const Tokenizer::WordInfoElement& y)
		{
			return (x.first.second < y.first.second) ? ModTrue : ModFalse;
		}
	};
	
	//
	//	FUNCTION local
	//	_$$::_lowerBoundWithWordInfoNormalizedTail --
	//
	Tokenizer::WordInfoVec::ConstIterator
	_lowerBoundWithWordInfoNormalizedTail(
		const Tokenizer::WordInfoVec::ConstIterator& iteBegin_,
		const Tokenizer::WordInfoVec::ConstIterator& iteEnd_,
		ModSize uiNormalizedTail_)
	{
		const Tokenizer::WordInfoElement i =
			Tokenizer::WordInfoElement(
				Tokenizer::WordInfoLocation(0, uiNormalizedTail_),
				Tokenizer::WordInfoSubElement(
					Tokenizer::WordInfoLocation(0, 0), 0));
		return ModLowerBound(iteBegin_, iteEnd_, i, _WordInfoNormalizedTailLess());
	}
	
	//
	//	FUNCTION local
	//	_$$::_increment --
	//
	//	RETURN
	//	bool
	//		false means an overflow occurred.
	//
	bool _increment(ModVector<ModSize>& v_,
					const ModVector<ModSize>& vmax_)
	{
		bool result = false;
		
		ModVector<ModSize>::Iterator i = v_.begin();
		const ModVector<ModSize>::ConstIterator e = v_.end();
		ModVector<ModSize>::ConstIterator max = vmax_.begin();
		for (; i != e; ++i, ++max)
		{
			if (++(*i) < *max)
			{
				result = true;
				break;
			}
			
			*i = 0;
		}
		
		return result;
	}
	
	//
	//	FUNCTION local
	//	_$$::_release --
	//
	void _release(ModVector<Common::Data::Pointer>& v_)
	{
		ModVector<Common::Data::Pointer>::Iterator i = v_.begin();
		const ModVector<Common::Data::Pointer>::ConstIterator e = v_.end();
		for(; i != e; ++i)
		{
			(*i).release();
		}
	}
}

//
//	FUNCTION public
//	Utility::Kwic::Kwic -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
Kwic::Kwic()
	: m_pCondition(0), m_pChecker(0), m_pTokenizer(0),
	  m_cstrStartTag(), m_cstrEndTag(), m_cstrEllipsis(),
	  m_eEscape(EscapeMethod::Unknown), m_uiSize(0), m_uiContentSize(0)
{}

//
//	FUNCTION public
//	Utility::Kwic::~Kwic -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
Kwic::~Kwic()
{
	delete m_pCondition;
	delete m_pChecker;
	delete m_pTokenizer;
}

//
//	FUNCTION public
//	Utility::Kwic::set -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
Kwic::set(const Common::DataArrayData* pPropertyKey_,
		  const Common::DataArrayData* pPropertyValue_,
		  const ModUnicodeString& cstrStartTag_,
		  const ModUnicodeString& cstrEndTag_,
		  const ModUnicodeString& cstrEllipsis_,
		  const ModUnicodeString& cstrEscapeMethod_,
		  ModSize uiSize_)
{
	const Common::DataArrayData* pSearchTermList = 0;
	const Common::DataArrayData* pUnaParameterKey = 0;
	const Common::DataArrayData* pUnaParameterValue = 0;
	
	// Set member variables.

	if (m_uiSize == 0)
	{
		// first time
		
		// Analyze parameters.
		ModVector<ModSize> vecRoughSize;
		analyzeProperty(pPropertyKey_, pPropertyValue_, &vecRoughSize,
						pSearchTermList, pUnaParameterKey, pUnaParameterValue);

		m_vecRoughSize = vecRoughSize;

		if (m_pTokenizer == 0)
		{
			m_pTokenizer = new Tokenizer();
		}
		m_pTokenizer->set(pUnaParameterKey, pUnaParameterValue);
	
		if (m_pChecker == 0)
		{
			m_pChecker = new PatternChecker();
		}
		m_pChecker->initialize();
	
		if (m_pCondition == 0)
		{
			m_pCondition = new Condition();
		}
		m_pCondition->set(pSearchTermList, m_pChecker, m_pTokenizer);

		m_cstrStartTag = cstrStartTag_;
		m_cstrEndTag =
			(cstrEndTag_.getLength() > 0) ? cstrEndTag_ : cstrStartTag_;
		m_cstrEllipsis = cstrEllipsis_;
		m_eEscape = getEscapeMethod(cstrEscapeMethod_);
	
		if (uiSize_ == 0)
		{
			_TRMEISTER_THROW0(Exception::BadArgument);
		}
		m_uiSize = uiSize_;
	
		m_uiContentSize = getContentSize(uiSize_);
	}
	else
	{
		// Analyze parameters.
		analyzeProperty(pPropertyKey_, pPropertyValue_, 0,
						pSearchTermList, pUnaParameterKey, pUnaParameterValue);
		
		// second time or later
		m_pCondition->set(pSearchTermList);
	}
}

//
//	FUNCTION public
//	Utility::Kwic::clear -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
Kwic::clear()
{
	m_vecRoughSize.clear();
	if (m_pCondition != 0)
	{
		m_pCondition->clear();
	}
	if (m_pChecker != 0)
	{
		m_pChecker->clear();
	}
	if (m_pTokenizer != 0)
	{
		m_pTokenizer->clear();
	}
	m_cstrStartTag.clear();
	m_cstrEndTag.clear();
	m_cstrEllipsis.clear();
	m_eEscape = EscapeMethod::Unknown;
	m_uiSize = 0;
	m_uiContentSize = 0;
}

//
//	FUNCTION public
//	Utility::Kwic::getRoughSize -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
ModSize
Kwic::getRoughSize(int fieldNumber_) const
{
	return m_vecRoughSize[fieldNumber_];
}

//
//	FUNCTION public
//	Utility::Kwic::generate -- 
//
//	NOTES
//
//	ARGUMENTS
//	const ModLanguageSet& cLanguageSet_
//		When empty, default language set is used.
//
//	RETURN
//
//	EXCEPTIONS
//
bool
Kwic::generate(const ModUnicodeString& cstrSrc_,
			   ModSize uiRoughSize_,
			   const ModLanguageSet& cLanguageSet_,
			   ModUnicodeString& cstrDst_,
			   const bool retry = false)
{
	; _TRMEISTER_ASSERT(cstrSrc_.getLength() > 0);

	if (m_pCondition->isPrepared() == false)
	{
		m_pCondition->prepare();
		m_pChecker->prepare();
	}
	
	// Get WordInfo
	ModUnicodeString cstrNormalized = ModUnicodeString();
	Tokenizer::WordInfoVec vecWordInfo;
	ModUnicodeChar usWordSeparator = (m_pChecker->isWordChecker() == true) ?
		PatternChecker::getWordSeparator() : Common::UnicodeChar::usNull;
	m_pTokenizer->getWordInfo(cstrSrc_, cLanguageSet_, usWordSeparator,
							 cstrNormalized, vecWordInfo);
	
	// Get location of patterns
	// [YET] Not need to check all string if seed of kwic is found during check.
	PatternChecker::PatternLocationVec vecPatternLocation;
	m_pChecker->check(cstrNormalized, vecPatternLocation);

	// Get seed of kwic
	ModSize uiWordSeparatorCount = (m_pChecker->isWordChecker() == true) ?
									vecWordInfo.getSize() + 1 : 0;
	double dNormalizedRatio = (cstrSrc_.getLength() > 0) ?
		static_cast<double>(cstrNormalized.getLength() - uiWordSeparatorCount)
		/ cstrSrc_.getLength() :
		0;
	ModSize uiNormalizedSeedOffset = 0;
	ModSize uiNormalizedSeedSize = 0;
	ModSize uiNormalizedContentSize =
		ModMax(ModMin(cstrNormalized.getLength() - uiWordSeparatorCount,
					  static_cast<ModSize>(m_uiContentSize * dNormalizedRatio)),
			   ModSize(1));
	getSeed(vecPatternLocation, uiNormalizedContentSize,
			uiNormalizedSeedOffset, uiNormalizedSeedSize);
	
	// Determine location of kwic and words
	ModSize uiOffset = 0;
	ModSize uiSize = 0;
	bool bHeadEllipsis = true;
	bool bTailEllipsis = true;
	ModVector<ModSize> vecWordRange;
	determine(vecPatternLocation, vecWordInfo,
			  uiNormalizedSeedOffset, uiNormalizedSeedSize,
			  dNormalizedRatio,
			  uiOffset, uiSize, bHeadEllipsis, bTailEllipsis,
			  vecWordRange);

	// Generate kwic
	return doGenerate(cstrSrc_, uiOffset, uiSize, uiRoughSize_,
			   bHeadEllipsis, bTailEllipsis, vecWordRange,
			   cstrDst_, retry);
}


//
//	FUNCTION private
//	Utility::Kwic::Condition::~Condition -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
Kwic::Condition::~Condition()
{
	_release(m_vecSearchTermList);
}

//
//	FUNCTION private
//	Utility::Kwic::Condition::set -- 
//
//	NOTES
//	SearchTermList
//		|- SynonymList or SearchTerm
//		...
//		+- SynonymList or SearchTerm
//
//	SynonymList
//		|- SearchTerm
//		...
//		+- SearchTerm
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
Kwic::Condition::set(const Common::DataArrayData* pSearchTermList_,
					 PatternChecker* pChecker_,
					 Tokenizer* pTokenizer_)
{
	; _TRMEISTER_ASSERT(pSearchTermList_ != 0);

	// Set variable
	if (pChecker_ != 0)
	{
		m_pChecker = pChecker_;
	}
	if (pTokenizer_ != 0)
	{
		m_pTokenizer = pTokenizer_;
	}
	if (m_bWordSearch == false)
	{
		m_bWordSearch = isWordSearch(pSearchTermList_);
	}
	m_vecSearchTermList.pushBack(pSearchTermList_->copy());
}

//
//	FUNCTION private
//	Utility::Kwic::Condition::isPrepared -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
bool
Kwic::Condition::isPrepared() const
{
	return (m_uiPerfectScore > 0) ? true : false;
}

//
//	FUNCTION private
//	Utility::Kwic::Condition::prepare -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
Kwic::Condition::prepare()
{
	; _TRMEISTER_ASSERT(m_pChecker != 0 && m_pTokenizer != 0);
	
	if (isPrepared() == false)
	{
		// Set search terms
		ModSize uiRootID = addNode(Node::Type::Root);
		ModVector<Common::Data::Pointer>::ConstIterator i =
			m_vecSearchTermList.begin();
		const ModVector<Common::Data::Pointer>::ConstIterator e =
			m_vecSearchTermList.end();
		for (; i != e; ++i)
		{
			// each search term list
			const Common::DataArrayData* pSearchTermList =
				_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*, (*i).get());
			for (int i = 0; i < pSearchTermList->getCount(); ++i)
			{
				// each search term or synonym list
				const Common::Data* pData =
					pSearchTermList->getElement(i).get();
				; _TRMEISTER_ASSERT(pData->isNull() == false);
				
				if (pData->getType() == Common::DataType::SearchTerm)
				{
					// Search Term
					setSearchTerm(
						_SYDNEY_DYNAMIC_CAST(const SearchTermData*, pData),
						uiRootID);
				}
				else
				{
					// The list of the synonyms
					; _TRMEISTER_ASSERT(
						pData->getType() == Common::DataType::Array);
					const Common::DataArrayData* pSynonymList =
						_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*,
											 pData);
			
					// add node
					ModSize uiParentID = addNode(
						Node::Type::Disjunction, uiRootID);
			
					for (int i = 0; i < pSynonymList->getCount(); ++i)
					{
						const Common::Data* pData1 =
							pSynonymList->getElement(i).get();
						; _TRMEISTER_ASSERT(
							pData1->isNull() == false &&
							pData1->getType() == Common::DataType::SearchTerm);

						// One of the synonyms
						setSearchTerm(
							_SYDNEY_DYNAMIC_CAST(const SearchTermData*, pData1),
							uiParentID);
					}
				}
			}
		}

		// Set the perfect score.
		for (ModSize i = 0; i < m_vecLeaf.getSize(); ++i)
		{
			increment(i);
		}
		m_uiPerfectScore = getScore();
	}
}

//
//	FUNCTION private
//	Utility::Kwic::Condition::clear -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
Kwic::Condition::clear()
{
	m_vecNode.erase(m_vecNode.begin(), m_vecNode.end());
	m_vecLeaf.erase(m_vecLeaf.begin(), m_vecLeaf.end());
	m_uiPerfectScore = 0;
	m_pChecker = 0;
	m_pTokenizer = 0;
	_release(m_vecSearchTermList);
	m_vecSearchTermList.erase(m_vecSearchTermList.begin(),
							  m_vecSearchTermList.end());
	m_bWordSearch = false;
}

//
//	FUNCTION private
//	Utility::Kwic::Condition::initializeStatus -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
Kwic::Condition::initializeStatus()
{
	ModVector<Node>::Iterator i = m_vecNode.begin();
	for (; i != m_vecNode.end(); ++i)
	{
		(*i).m_uiCount = 0;
	}
	ModVector<Leaf>::Iterator j = m_vecLeaf.begin();
	for (; j != m_vecLeaf.end(); ++j)
	{
		(*j).first = 0;
	}
}

//
//	FUNCTION private
//	Utility::Kwic::Condition::increment -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
Kwic::Condition::increment(ModSize uiLeafID_)
{
	; _TRMEISTER_ASSERT(uiLeafID_ < m_vecLeaf.getSize());
	if (++(m_vecLeaf[uiLeafID_].first) == 1)
	{
		incrementNode(m_vecLeaf[uiLeafID_].second);
	}
}

//
//	FUNCTION private
//	Utility::Kwic::Condition::decrement -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
Kwic::Condition::decrement(ModSize uiLeafID_)
{
	; _TRMEISTER_ASSERT(uiLeafID_ < m_vecLeaf.getSize());
	; _TRMEISTER_ASSERT(m_vecLeaf[uiLeafID_].first > 0);
	if (--(m_vecLeaf[uiLeafID_].first) == 0)
	{
		decrementNode(m_vecLeaf[uiLeafID_].second);
	}
}

//
//	FUNCTION private
//	Utility::Kwic::Condition::setSearchTerm -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
Kwic::Condition::setSearchTerm(const SearchTermData* pSearchTerm_,
							   ModSize uiParentID_)
{
	; _TRMEISTER_ASSERT(pSearchTerm_ != 0);

	// Get separator modes
	Separator::Value eHead = Separator::None;
	Separator::Value eMid = Separator::None;
	Separator::Value eTail = Separator::None;
	getSeparatorMode(pSearchTerm_, eHead, eMid, eTail);
	ModSize uiExpandSeparator = (eMid == Separator::Expand) ? 2 : 1;
	
	// Get expanded morphemes
	ModSize uiRemainder = 1;
	ModVector<ModVector<ModUnicodeString> > vecExpand;
	ModVector<ModUnicodeString> vecMorpheme;
	m_pTokenizer->setSearchTerm(pSearchTerm_);
	while (m_pTokenizer->getExpandWords(vecMorpheme) == true)
	{
		if (vecMorpheme.getSize() > 0 &&
			(*(vecMorpheme.begin())).getLength() > 0)
		{
			uiRemainder *= (vecExpand.getSize() == 0 ? 1 : uiExpandSeparator)
				* vecMorpheme.getSize();
			vecExpand.pushBack(vecMorpheme);
			vecMorpheme.clear();
		}
	}
	if (vecExpand.getSize() == 0)
	{
		return;
	}

	// The expanded patterns for synonyms are here:
	// (A,A',B,B' are morphemes.)
	// AB, A'B, AB', A'B'
	// And when eMid is Separator::Expand, the patterns are here:
	// (/ is a word separator.)
	// AB, A'B, AB', A'B', A/B, A'/B, A/B', A'/B'

	ModVector<ModVector<ModUnicodeString> >::ConstIterator i =
		vecExpand.begin();
	ModVector<ModVector<ModUnicodeString> >::ConstIterator j = i + 1;
	const ModVector<ModVector<ModUnicodeString> >::ConstIterator s = i;
	const ModVector<ModVector<ModUnicodeString> >::ConstIterator e =
		vecExpand.end();
	ModSize uiParentID = uiParentID_;
	ModSize uiCombination = (*i).getSize();
	uiRemainder /= (uiCombination * uiExpandSeparator);
	ModSize uiExpandLimit =
		(_cKwicExpandLimit > 0) ? static_cast<ModSize>(_cKwicExpandLimit): 0;
	for (; j != e; ++j)
	{
		if (uiCombination * uiExpandSeparator * uiRemainder <= uiExpandLimit)
		{
			break;
		}
		
		uiCombination *= uiExpandSeparator * (*j).getSize();
		uiRemainder /= (uiExpandSeparator * (*j).getSize());
		if (uiCombination > uiExpandLimit || uiCombination > 2 * uiRemainder)
		{
			// [NOTE] The '2' of the above second condition means that
			//  uiCombination is MUCH larger than uiRemainder.
			
			if (i == s)
			{
				// First time to divide the search term
				uiParentID = addNode(Node::Type::Conjunction, uiParentID);
			}
			
			// The devided patterns with separators are here:
			//  ABCD,ABC/D,AB/CD,AB/C/D,A/BCD,A/BC/D,A/B/CD,A/B/C/D
			//  -> AB,A/B CD,C/D
			// AB/, for example, does not generate, but AB includes it.
			
			setExpandSearchTerm(
				i, j, (i == s ? eHead : Separator::None), eMid, Separator::None,
				uiCombination, uiParentID);
			
			uiCombination = 1;
		}
	}
	if (i != e)
	{
		setExpandSearchTerm(
			i, e, (i == s ? eHead : Separator::None), eMid, eTail,
			uiCombination * (i + 1 == e ? 1 : uiExpandSeparator) * uiRemainder,
			uiParentID);
	}
}

//
//	FUNCTION private
//	Utility::Kwic::Condition::setExpandSearchTerm -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
Kwic::Condition::setExpandSearchTerm(
	ModVector<ModVector<ModUnicodeString> >::ConstIterator& i,
	const ModVector<ModVector<ModUnicodeString> >::ConstIterator& j,
	Separator::Value eHead_,
	Separator::Value eMid_,
	Separator::Value eTail_,
	ModSize uiCombination,
	ModSize uiParentID_)
{
	; _TRMEISTER_ASSERT(i != j);
	
	// Morphemes + separator
	ModVector<ModSize> vmax;
	createExpandTable(vmax, i, j, eMid_);
	ModVector<ModSize> v(vmax.getSize(), 0);
	
	// add node
	ModSize uiParentID = uiParentID_;
	; _TRMEISTER_ASSERT(uiParentID < m_vecNode.getSize());
	if (uiCombination > 1 &&
		m_vecNode[uiParentID].m_eType != Node::Type::Disjunction)
	{
		uiParentID = addNode(Node::Type::Disjunction, uiParentID);
	}
	
	// add leaf
	ModUnicodeChar usSeparator(PatternChecker::getWordSeparator());
	ModUnicodeOstrStream stream;
	const ModVector<ModVector<ModUnicodeString> >::ConstIterator s = i;
	ModVector<ModSize>::ConstIterator vite = v.begin();
	const ModVector<ModSize>::ConstIterator vstart = vite;
	do
	{
		if (eHead_ == Separator::Insert)
		{
			stream << usSeparator;
		}
		for (i = s, vite = vstart; i != j; ++i, ++vite)
		{
			if (i != s)
			{
				if (eMid_ == Separator::Insert)
				{
					stream << usSeparator;
				}
				else if (eMid_ == Separator::Expand)
				{
					if (*vite == 1)
					{
						stream << usSeparator;
					}
					++vite;
				}
			}
			stream << (*i)[(*vite)];
		}
		if (eTail_ == Separator::Insert)
		{
			stream << usSeparator;
		}
		addLeaf(uiParentID);
		m_pChecker->addPattern(ModUnicodeString(stream.getString()));
		stream.clear();
		
	} while (_increment(v, vmax));
}

//
//	FUNCTION private
//	Utility::Kwic::Condition::isWordSearch -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
bool
Kwic::Condition::isWordSearch(
	const Common::DataArrayData* pSearchTermList_) const
{
	; _TRMEISTER_ASSERT(pSearchTermList_ != 0);

	bool result = false;

	if (_cKwicUseMatchMode == true)
	{
		for (int i = 0; i < pSearchTermList_->getCount(); ++i)
		{
			const Common::Data* pData = pSearchTermList_->getElement(i).get();
			; _TRMEISTER_ASSERT(pData->isNull() == false);
		
			if (pData->getType() == Common::DataType::SearchTerm)
			{
				// Search Term
				const SearchTermData* pSearchTerm =
					_SYDNEY_DYNAMIC_CAST(const SearchTermData*, pData);
				if (getMatchMode(pSearchTerm)
					!= SearchTermData::MatchMode::String)
				{
					result = true;
					break;
				}
			}
			else
			{
				// The list of the synonyms
				; _TRMEISTER_ASSERT(
					pData->getType() == Common::DataType::Array);
				const Common::DataArrayData* pSynonymList
					= _SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*, pData);
			
				for (int i = 0; i < pSynonymList->getCount(); ++i)
				{
					const Common::Data* pData1 =
						pSynonymList->getElement(i).get();
					; _TRMEISTER_ASSERT(
						pData1->isNull() == false &&
						pData1->getType() == Common::DataType::SearchTerm);

					// One of the synonyms
					const SearchTermData* pSynonymData
						= _SYDNEY_DYNAMIC_CAST(const SearchTermData*, pData1);
					if (getMatchMode(pSynonymData)
						!= SearchTermData::MatchMode::String)
					{
						result = true;
						break;
					}
				}
				if (result == true)
				{
					break;
				}
			}
		}
	}

	return result;
}

//
//	FUNCTION private
//	Utility::Kwic::Condition::getMatchMode -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
SearchTermData::MatchMode::Value
Kwic::Condition::getMatchMode(const SearchTermData* pSearchTerm_) const
{
	; _TRMEISTER_ASSERT(pSearchTerm_ != 0);
	
	SearchTermData::MatchMode::Value eMatchMode =
		SearchTermData::MatchMode::String;

	if (_cKwicUseMatchMode == true)
	{
		eMatchMode = pSearchTerm_->getMatchMode();
		switch (eMatchMode)
		{
		case SearchTermData::MatchMode::String:
			break;
		case SearchTermData::MatchMode::WordHead:
			if (_cKwicWordHeadMode == false)
			{
				eMatchMode = SearchTermData::MatchMode::String;
			}
			break;
		case SearchTermData::MatchMode::WordTail:
			if (_cKwicWordTailMode == false)
			{
				eMatchMode = SearchTermData::MatchMode::String;
			}
			break;
		case SearchTermData::MatchMode::SimpleWord:
			if (_cKwicSimpleWordMode == false)
			{
				eMatchMode = SearchTermData::MatchMode::String;
			}
			break;
		case SearchTermData::MatchMode::ExactWord:
			if (_cKwicExactWordMode == false)
			{
				eMatchMode = SearchTermData::MatchMode::String;
			}
			break;
		case SearchTermData::MatchMode::MultiLanguage:
			if (_cKwicMultiMatchMode == true && _cKwicExactWordMode == true &&
				pSearchTerm_->getCharacterType()
				== SearchTermData::CharacterType::NoCJK)
			{
				eMatchMode = SearchTermData::MatchMode::ExactWord;
			}
			else
			{
				eMatchMode = SearchTermData::MatchMode::String;
			}
			break;
		default:
			; _TRMEISTER_ASSERT(false);
			break;
		}
	}

	; _TRMEISTER_ASSERT(eMatchMode != SearchTermData::MatchMode::MultiLanguage);
	return eMatchMode;
}

//
//	FUNCTION private
//	Utility::Kwic::Condition::getSeparatorMode -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
Kwic::Condition::getSeparatorMode(const SearchTermData* pSearchTerm_,
								  Separator::Value& eHead_,
								  Separator::Value& eMid_,
								  Separator::Value& eTail_) const
{
	; _TRMEISTER_ASSERT(pSearchTerm_ != 0 &&
						eHead_ == Separator::None &&
						eMid_ == Separator::None &&
						eTail_ == Separator::None);
	
	if (m_bWordSearch)
	{
		// The expanded patterns for a word search are here:
		// (A,B,C are morphemes, / is a word separater.)
		// String:		ABC, AB/C, A/BC, A/B/C
		// ExactWord:	/A/B/C/
		// SimpleWord:	/ABC/, /AB/C/, /A/BC/, /A/B/C/
		// WordHead:	/ABC, /AB/C, /A/BC, /A/B/C
		// WordTail:	ABC/, AB/C/, A/BC/, A/B/C/
		
		switch (getMatchMode(pSearchTerm_))
		{
		case SearchTermData::MatchMode::String:
			eMid_ = Separator::Expand;
			break;
		case SearchTermData::MatchMode::WordHead:
			eHead_ = Separator::Insert;
			eMid_ = Separator::Expand;
			break;
		case SearchTermData::MatchMode::WordTail:
			eMid_ = Separator::Expand;
			eTail_ = Separator::Insert;
			break;
		case SearchTermData::MatchMode::SimpleWord:
			eHead_ = Separator::Insert;
			eMid_ = Separator::Expand;
			eTail_ = Separator::Insert;
			break;
		case SearchTermData::MatchMode::ExactWord:
			eHead_ = Separator::Insert;
			eMid_ = Separator::Insert;
			eTail_ = Separator::Insert;
			break;
		case SearchTermData::MatchMode::MultiLanguage:
			// See getMatchMode() for details.
			; _TRMEISTER_ASSERT(false);
			break;
		default:
			; _TRMEISTER_ASSERT(false);
			break;
		}
		
		if (eMid_ == Separator::Expand && _cKwicExpandWordSeparator == false)
		{
			// The search term is NOT expanded. So the pattern is here:
			// (A,B,C are morphemes, / is a word separater.)
			// String:		A/B/C
			// ExactWord:	/A/B/C/
			// SimpleWord:	/A/B/C/
			// WordHead:	/A/B/C
			// WordTail:	A/B/C/
			eMid_ = Separator::Insert;
		}
	}
}

//
//	FUNCTION private
//	Utility::Kwic::Condition::createExpandTable -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
Kwic::Condition::createExpandTable(
	ModVector<ModSize>& vmax_,
	ModVector<ModVector<ModUnicodeString> >::ConstIterator i,
	const ModVector<ModVector<ModUnicodeString> >::ConstIterator& e,
	Separator::Value eMid_) const
{
	ModSize uiSize = e - i;
	if (eMid_ == Separator::Expand)
	{
		uiSize = uiSize * 2 - 1;
	}
	vmax_.reserve(uiSize);
	
	const ModVector<ModVector<ModUnicodeString> >::ConstIterator s = i;
	for (; i != e; ++i)
	{
		if (eMid_ == Separator::Expand && i != s)
		{
			// 2 patterns: with a separator and without a separator
			vmax_.pushBack(2);
		}
		vmax_.pushBack((*i).getSize());
	}
}

//
//	FUNCTION private
//	Utility::Kwic::Condition::addNode -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	ModSize
//		My Node ID
//
//	EXCEPTIONS
//
ModSize
Kwic::Condition::addNode(Kwic::Condition::Node::Type::Value eType_,
						 ModSize uiParentID_)
{
	Node node;
	node.m_uiParentID = uiParentID_;
	node.m_eType = eType_;
	m_vecNode.pushBack(node);

	if (eType_ != Node::Type::Root)
	{
		; _TRMEISTER_ASSERT(uiParentID_ < m_vecNode.getSize());
		++(m_vecNode[uiParentID_].m_uiChildrenCount);
	}
	
	return m_vecNode.getSize() - 1;
}

//
//	FUNCTION private
//	Utility::Kwic::Condition::addLeaf -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
Kwic::Condition::addLeaf(ModSize uiParentID_)
{
	Leaf leaf;
	leaf.first = 0;
	leaf.second = uiParentID_;
	m_vecLeaf.pushBack(leaf);

	; _TRMEISTER_ASSERT(uiParentID_ < m_vecNode.getSize());
	++(m_vecNode[uiParentID_].m_uiChildrenCount);
}

//
//	FUNCTION private
//	Utility::Kwic::Condition::incrementNode -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
Kwic::Condition::incrementNode(ModSize uiNodeID_)
{
	; _TRMEISTER_ASSERT(uiNodeID_ < m_vecNode.getSize());

	Node& cNode = m_vecNode[uiNodeID_];

	++(cNode.m_uiCount);
	switch(cNode.m_eType)
	{
	case Node::Type::Root:
		break;
	case Node::Type::Conjunction:
		if (cNode.m_uiCount == cNode.m_uiChildrenCount)
		{
			incrementNode(cNode.m_uiParentID);
		}
		break;
	case Node::Type::Disjunction:
		if (cNode.m_uiCount == 1)
		{
			incrementNode(cNode.m_uiParentID);
		}
		break;
	default:
		; _TRMEISTER_ASSERT(false);
		break;
	}
}

//
//	FUNCTION private
//	Utility::Kwic::Condition::decrementNode -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
Kwic::Condition::decrementNode(ModSize uiNodeID_)
{
	; _TRMEISTER_ASSERT(uiNodeID_ < m_vecNode.getSize());
	; _TRMEISTER_ASSERT(m_vecNode[uiNodeID_].m_uiCount > 0);

	Node& cNode = m_vecNode[uiNodeID_];

	--(cNode.m_uiCount);
	switch(cNode.m_eType)
	{
	case Node::Type::Root:
		break;
	case Node::Type::Conjunction:
		if (cNode.m_uiCount == cNode.m_uiChildrenCount - 1)
		{
			decrementNode(cNode.m_uiParentID);
		}
		break;
	case Node::Type::Disjunction:
		if (cNode.m_uiCount == 0)
		{
			decrementNode(cNode.m_uiParentID);
		}
		break;
	default:
		; _TRMEISTER_ASSERT(false);
		break;
	}
}

//
//	FUNCTION private
//	Utility::Kwic::analyzeProperty -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
Kwic::analyzeProperty(const Common::DataArrayData* pPropertyKey_,
					  const Common::DataArrayData* pPropertyValue_,
					  ModVector<ModSize>* pvecRoughSize_,
					  const Common::DataArrayData*& pSearchTermList_,
					  const Common::DataArrayData*& pUnaParameterKey_,
					  const Common::DataArrayData*& pUnaParameterValue_) const
{
	; _TRMEISTER_ASSERT(pPropertyKey_ != 0 && pPropertyValue_ != 0);
	; _TRMEISTER_ASSERT(pPropertyKey_->isNull() == false
						&& pPropertyValue_->isNull() == false);
	; _TRMEISTER_ASSERT(pPropertyKey_->getCount() ==
						pPropertyValue_->getCount());

	for (int i = 0; i < pPropertyKey_->getCount(); ++i)
	{
		const Common::Data* pKeyData = pPropertyKey_->getElement(i).get();
		const Common::Data* pValueData = pPropertyValue_->getElement(i).get();

		; _TRMEISTER_ASSERT(pKeyData->isNull() == false &&
							pValueData->isNull() == false);

		// Get key
		; _TRMEISTER_ASSERT(pKeyData->getType() == Common::DataType::String);
		const Common::StringData* pKeyStringData
			= _SYDNEY_DYNAMIC_CAST(const Common::StringData*, pKeyData);
		const ModUnicodeString& cstrKey = pKeyStringData->getValue();

		// Get value
		if (cstrKey.compare(_cRoughKwicSize, ModFalse) == 0)
		{
			if (pvecRoughSize_ == 0)
				continue;

			; _TRMEISTER_ASSERT(pValueData->getType() ==
								Common::DataType::Array);
			const Common::DataArrayData* pArrayData
				= _SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*,
									   pValueData);
			int n = pArrayData->getCount();
			pvecRoughSize_->reserve(n);
			for (int i = 0; i < n; ++i)
			{
				Common::Data::Pointer p = pArrayData->getElement(i);
				ModSize size = 0;
				if (p.get())
				{
					const Common::UnsignedIntegerData* pUnsignedIntegerData
						= _SYDNEY_DYNAMIC_CAST(
							const Common::UnsignedIntegerData*, p.get());
					size = pUnsignedIntegerData->getValue();
				}
				pvecRoughSize_->pushBack(size);
			}
		}
		else if (cstrKey.compare(_cSearchTermList, ModFalse) == 0)
		{
			; _TRMEISTER_ASSERT(
				pValueData->getType() == Common::DataType::Array);
			pSearchTermList_ =
				_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*,
									 pValueData);
		}
		else if (cstrKey.compare(_cUnaParameterKey, ModFalse) == 0)
		{
			; _TRMEISTER_ASSERT(
				pValueData->getType() == Common::DataType::Array);
			pUnaParameterKey_ =
				_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*,
									 pValueData);
		}
		else if (cstrKey.compare(_cUnaParameterValue, ModFalse) == 0)
		{
			; _TRMEISTER_ASSERT(
				pValueData->getType() == Common::DataType::Array);
			pUnaParameterValue_ =
				_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*,
									 pValueData);
		}
		else
		{
			; _TRMEISTER_ASSERT(false);
		}
	}
	
	// [NOTE] pSearchTermList_ is always set. See Kwic::set() for details.
	; _TRMEISTER_ASSERT(pSearchTermList_ != 0);
}


//
//	FUNCTION private
//	Utility::Kwic::getEscapeMethod -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
Kwic::EscapeMethod::Value
Kwic::getEscapeMethod(const ModUnicodeString& cstrEscapeMethod_) const
{
	EscapeMethod::Value result = EscapeMethod::None;
	
	if (cstrEscapeMethod_.getLength() > 0)
	{
		if (cstrEscapeMethod_.compare(_cEscapeMethod_HTML, ModFalse) == 0)
		{
			result = EscapeMethod::HTML;
		}
		else if (cstrEscapeMethod_.compare(_cEscapeMethod_None, ModFalse) == 0)
		{
			; _TRMEISTER_ASSERT(result == EscapeMethod::None);
		}
		else
		{
			_TRMEISTER_THROW0(Exception::BadArgument);
		}
	}
	
	return result;
}

//
//	FUNCTION private
//	Utility::Kwic::getContentSize -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
ModSize
Kwic::getContentSize(ModSize uiSize_) const
{
	; _TRMEISTER_ASSERT(uiSize_ > 0);
	
	int temp = _cKwicMarginPercent.get();
	if (temp < 0 || 100 <= temp)
	{
		_TRMEISTER_THROW0(Exception::BadArgument);
	}
	
	ModSize uiContentSize = uiSize_ * (100 - static_cast<ModSize>(temp)) / 100;
	return ModMax(uiContentSize, ModSize(1));
}

//
//	FUNCTION private
//	Utility::Kwic::getSeed -- 
//
//	NOTES
//
//	ARGUMENTS
//	ModSize& uiSeedSize_
//		When 0, not hit any pattern.
//
//	RETURN
//
//	EXCEPTIONS
//
void
Kwic::getSeed(
	const PatternChecker::PatternLocationVec& vecPatternLocation_,
	ModSize uiContentSize_,
	ModSize& uiSeedOffset_,
	ModSize& uiSeedSize_)
{
	; _TRMEISTER_ASSERT(uiSeedOffset_ == 0 && uiSeedSize_ == 0);

	const PatternChecker::PatternLocationVec::ConstIterator s =
		vecPatternLocation_.begin();
	const PatternChecker::PatternLocationVec::ConstIterator e =
		vecPatternLocation_.end();
	// iterator of head
	PatternChecker::PatternLocationVec::ConstIterator i = s;
	// iterator of tail
	PatternChecker::PatternLocationVec::ConstIterator j = s;

	ModSize uiScore = 0;
	m_pCondition->initializeStatus();
	for (; j != e; ++j)
	{
		ModSize uiTail = (*j).first;
		ModSize uiTailPatternID = (*j).second;

		// Increase hit count
		m_pCondition->increment(uiTailPatternID);
		
		// Decrease hit count
		for (; i != j; ++i)
		{
			ModSize uiHead = (*i).first;
			ModSize uiHeadPatternID = (*i).second;
			
			ModSize uiTempSeedSize = uiTail - uiHead +
				m_pChecker->getPatternLength(uiTailPatternID);
			if (uiTempSeedSize <= uiContentSize_)
			{
				// [NOTE] Seed KWIC size may be greater than uiContentSize_
				//  when i == j and the size of i's pattern is very long.
				break;
			}
			
			m_pCondition->decrement(uiHeadPatternID);
		}

		// Set seed range
		if (j == s || m_pCondition->getScore() > uiScore)
		{
			; _TRMEISTER_ASSERT(uiSeedOffset_ <= (*i).first);
			uiSeedOffset_ = (*i).first;
			uiSeedSize_ = uiTail - (*i).first +
				m_pChecker->getPatternLength(uiTailPatternID);
			uiScore = m_pCondition->getScore();
			
			if (uiScore == m_pCondition->getPerfectScore())
			{
				break;
			}
		}
	}
}

//
//	FUNCTION private
//	Utility::Kwic::determine -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
Kwic::determine(const PatternChecker::PatternLocationVec& vecPatternLocation_,
				const Tokenizer::WordInfoVec& vecWordInfo_,
				ModSize uiNormalizedSeedOffset_,
				ModSize uiNormalizedSeedSize_,
				double dNormalizedRatio_,
				ModSize& uiOffset_,
				ModSize& uiSize_,
				bool& bHeadEllipsis_,
				bool& bTailEllipsis_,
				ModVector<ModSize>& vecWordRange_) const
{
	; _TRMEISTER_ASSERT(
		vecPatternLocation_.getSize() > 0 || uiNormalizedSeedSize_ == 0);
	; _TRMEISTER_ASSERT(uiOffset_ == 0 && uiSize_ == 0);
	; _TRMEISTER_ASSERT(bHeadEllipsis_ == true && bTailEllipsis_ == true);
	
	uiOffset_ = 0;
	uiSize_ = m_uiSize;
	
	if (vecPatternLocation_.getSize() > 0)
	{
		const Tokenizer::WordInfoVec::ConstIterator iteBegin =
			vecWordInfo_.begin();
		const Tokenizer::WordInfoVec::ConstIterator iteEnd =
			vecWordInfo_.end();
		Tokenizer::WordInfoVec::ConstIterator i = iteBegin;
		Tokenizer::WordInfoVec::ConstIterator iteTailWord = iteBegin;
		
		// Set kwic offset
		determineOffset(iteBegin, iteEnd,
						uiNormalizedSeedOffset_, uiNormalizedSeedSize_,
						dNormalizedRatio_,
						i, iteTailWord, uiOffset_);
		
		// Set head ellipsis
		if (i != iteBegin && isPeriod((*(i - 1)).second.second) == true)
		{
			bHeadEllipsis_ = false;
		}
	
		// Set word ranges
		ModSize uiWordTail = 0;
		ModSize uiNormalizedPatternTail = 0;
		bool bPatternInside = false;
		const ModVector<ModSize>& vecPatternLength =
			m_pChecker->getPatternLength();
		PatternChecker::PatternLocationVec::ConstIterator j =
			vecPatternLocation_.begin();
		const PatternChecker::PatternLocationVec::ConstIterator e =
			vecPatternLocation_.end();
		for (; i != iteEnd; ++i)
		{
			if (j != e || bPatternInside == true)
			{
				; _TRMEISTER_ASSERT(vecPatternLocation_.getSize() > 0);
				setWordRange(*i, vecPatternLength,
							 j, e, uiNormalizedPatternTail, bPatternInside,
							 vecWordRange_);
			}

			uiWordTail = (*i).second.first.second;
			if (iteTailWord <= i && uiWordTail - uiOffset_ + 1 >= m_uiSize)
			{
				// [NOTE] The former condition is not enough.
				//  See determineOffset() for details.
				break;
			}
		}
		if (bPatternInside == true && vecWordRange_.getSize() > 0)
		{
			; _TRMEISTER_ASSERT(vecWordRange_.getSize() % 2 == 1);
			vecWordRange_.popBack();
		}
		; _TRMEISTER_ASSERT(vecWordRange_.getSize() > 0 ||
							vecPatternLocation_.getSize() == 0);
	
		// Set tail ellipsis
		if (iteEnd != iteBegin &&
			isPeriod((*(i == iteEnd ? i - 1 : i)).second.second) == true)
		{
			bTailEllipsis_ = false;
		}

		// Set kwic size
		uiSize_ = uiWordTail - uiOffset_ + 1;
	}
}

//
//	FUNCTION private
//	Utility::Kwic::determineOffset -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
Kwic::determineOffset(
	const Tokenizer::WordInfoVec::ConstIterator& iteBegin_,
	const Tokenizer::WordInfoVec::ConstIterator& iteEnd_,
	ModSize uiNormalizedSeedOffset_,
	ModSize uiNormalizedSeedSize_,
	double dNormalizedRatio_,
	Tokenizer::WordInfoVec::ConstIterator& iteHeadWord_,
	Tokenizer::WordInfoVec::ConstIterator& iteTailWord_,
	ModSize& uiOffset_) const
{
	; _TRMEISTER_ASSERT(uiNormalizedSeedSize_ > 0);
	; _TRMEISTER_ASSERT(iteBegin_ != iteEnd_);

	ModSize uiNormalizedMargin =
		(static_cast<ModSize>(dNormalizedRatio_ * m_uiSize)
		 >= uiNormalizedSeedSize_) ?
		static_cast<ModSize>(
			(dNormalizedRatio_ * m_uiSize - uiNormalizedSeedSize_) / 2) : 0;

	// Get tail
	ModSize uiNormalizedSeedTail =
		uiNormalizedSeedOffset_ + uiNormalizedSeedSize_ - 1;
	// [NOTE] Get word whose tail is greater than or equal to the argument,
	//  and whose head is less than or equal to the argument.
	//  head <= the argument <= tail
	iteTailWord_ = _lowerBoundWithWordInfoNormalizedTail(
		iteBegin_, iteEnd_, uiNormalizedSeedTail + uiNormalizedMargin);
	if (iteTailWord_ == iteEnd_)
	{
		--iteTailWord_;
	}
	ModSize uiTail = (*iteTailWord_).second.first.second;

	// Get head
	iteHeadWord_ = iteBegin_;
	if (uiTail + 1 > m_uiSize)
	{
		ModSize uiHead = uiTail + 1 - m_uiSize;
		iteHeadWord_ = _lowerBoundWithWordInfoNormalizedTail(
			iteBegin_, iteTailWord_,
			static_cast<ModSize>(dNormalizedRatio_ * uiHead));
	}
	// [NOTE] KWIC should include the seed KWIC.
	for (; iteHeadWord_ != iteBegin_; --iteHeadWord_)
	{
		if (iteHeadWord_ != iteEnd_ &&
			(*iteHeadWord_).first.first <= uiNormalizedSeedOffset_)
		{
			break;
		}
	}
	; _TRMEISTER_ASSERT(iteHeadWord_ != iteEnd_);
	if (isPeriod((*iteHeadWord_).second.second) == true &&
		iteHeadWord_ + 1 != iteEnd_)
	{
		// The head of KWIC is not period.
		++iteHeadWord_;
	}
	// [NOTE] iteTailWord_ may not indicate the tail word of KWIC.
	//  Because, the size from iteHeadWord_ to iteTailWord_ may not be
	//  equal to m_uiSize.
	
	// Get offset
	uiOffset_ = (*iteHeadWord_).second.first.first;
}

//
//	FUNCTION private
//	Utility::Kwic::setWordRange -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
Kwic::setWordRange(const Tokenizer::WordInfoElement& cWordInfoElement_,
				   const ModVector<ModSize>& vecPatternLength_,
				   PatternChecker::PatternLocationVec::ConstIterator& j_,
				   const PatternChecker::PatternLocationVec::ConstIterator& e_,
				   ModSize& uiNormalizedPatternTail_,
				   bool& bPatternInside_,
				   ModVector<ModSize>& vecWordRange_) const
{
	ModSize uiNormalizedWordHead = cWordInfoElement_.first.first;
	ModSize uiNormalizedWordTail = cWordInfoElement_.first.second;
	ModSize uiWordHead = cWordInfoElement_.second.first.first;
	ModSize uiWordTail = cWordInfoElement_.second.first.second;
	
	ModSize uiNormalizedPatternHead = 0;
	while (true)
	{
		if (bPatternInside_ == true)
		{
			; _TRMEISTER_ASSERT(uiNormalizedWordHead <= uiNormalizedPatternTail_);
			if (uiNormalizedWordTail < uiNormalizedPatternTail_)
			{
				// Get next word
				break;
			}
			
			// Set tail
			pushPatternTail(uiWordHead, uiWordTail,
							uiNormalizedWordHead, uiNormalizedWordTail,
							uiNormalizedPatternTail_, vecWordRange_);
			bPatternInside_ = false;
			
			if (j_ == e_)
			{
				// Not exist any pattern
				break;
			}
		}
		else
		{
			// Get head of pattern which is included in the word

			// Skip patterns
			for(; j_ != e_; ++j_)
			{
				uiNormalizedPatternHead = (*j_).first;
				if (uiNormalizedWordHead <= uiNormalizedPatternHead)
				{
					break;
				}
			}
			if (j_ == e_)
			{
				// Not found any pattern
				break;
			}

			if (uiNormalizedWordTail < uiNormalizedPatternHead)
			{
				// Get next word
				break;
			}
			
			// Set head
			pushPatternHead(uiWordHead, uiWordTail,
							uiNormalizedWordHead, uiNormalizedWordTail, 
							uiNormalizedPatternHead, vecWordRange_);
			bPatternInside_ = true;
			
			// Select the longest pattern,
			// and overwrpped patterns are concatinated.
			// [NOTE] Contignuous patterns are not concatinated.
			uiNormalizedPatternTail_ =
				uiNormalizedPatternHead + vecPatternLength_[(*(j_++)).second] - 1;
			for (; j_ != e_; ++j_)
			{
				if (uiNormalizedPatternTail_ < (*j_).first)
				{
					// Patterns are not overwapped.
					break;
				}
				uiNormalizedPatternTail_ = ModMax(
					(*j_).first + vecPatternLength_[(*j_).second] - 1,
					uiNormalizedPatternTail_);
			}
		}
	}
}

//
//	FUNCTION private
//	Utility::Kwic::getPatternHead -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
Kwic::pushPatternHead(ModSize uiWordHead_,
					  ModSize uiWordTail_,
					  ModSize uiNormalizedWordHead_,
					  ModSize uiNormalizedWordTail_,
					  ModSize uiNormalizedPatternHead_,
					  ModVector<ModSize>& vecWordRange_) const
{
	; _TRMEISTER_ASSERT(uiWordHead_ <= uiWordTail_);
	; _TRMEISTER_ASSERT(uiNormalizedWordHead_ <= uiNormalizedWordTail_);
	; _TRMEISTER_ASSERT(uiNormalizedWordHead_ <= uiNormalizedPatternHead_);
	; _TRMEISTER_ASSERT(uiNormalizedPatternHead_ <= uiNormalizedWordTail_);
	; _TRMEISTER_ASSERT(vecWordRange_.getSize() % 2 == 0);

	// [NOTE] The pattern head is not appearent when the normalized pattern head
	//  does not correspond with the normalized word head. 
	//  So, basically, the pattern head is calculated with 
	//  the ratio of the original word length and the normalized word length.
	
	double dRatio = static_cast<double>(uiWordTail_ - uiWordHead_ + 1) /
		(uiNormalizedWordTail_ - uiNormalizedWordHead_ + 1);
	
	// Calculate the pattern head using the distance from
	// the normlaized word head or tail.
	ModSize uiPatternHead = 0;
	if ((uiNormalizedWordHead_ + uiNormalizedWordTail_) / 2.0 <=
		static_cast<double>(uiNormalizedPatternHead_))
	{
		// round up
		// [NOTE] Japanese long chracter is removed when normalizing,
		//  and the character often locates at the end of words.
		//  So the length is rounded up.
		double dLength =
			dRatio * (uiNormalizedWordTail_ - uiNormalizedPatternHead_ + 1);
		ModSize uiLength = static_cast<ModSize>(dLength);
		uiLength += (dLength > static_cast<double>(uiLength)) ? 1 : 0;
		uiLength = ModMin(uiWordTail_ - uiWordHead_ + 1, uiLength);
		uiPatternHead = uiWordTail_ - uiLength + 1;
	}
	else
	{
		// round down
		ModSize uiDiff = static_cast<ModSize>(
			dRatio * (uiNormalizedPatternHead_ - uiNormalizedWordHead_));
		uiDiff = ModMin(uiWordTail_ - uiWordHead_, uiDiff);
		uiPatternHead = uiWordHead_ + uiDiff;
	}
	
	if (vecWordRange_.getSize() > 0 && uiPatternHead <= vecWordRange_.getBack())
	{
		// Patterns are overwrapped.
		vecWordRange_.popBack();
	}
	else
	{
		vecWordRange_.pushBack(uiPatternHead);
	}
	
	; _TRMEISTER_ASSERT(vecWordRange_.getSize() % 2 == 1);
}

//
//	FUNCTION private
//	Utility::Kwic::getPatternTail -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
Kwic::pushPatternTail(ModSize uiWordHead_,
					  ModSize uiWordTail_,
					  ModSize uiNormalizedWordHead_,
					  ModSize uiNormalizedWordTail_,
					  ModSize uiNormalizedPatternTail_,
					  ModVector<ModSize>& vecWordRange_) const
{
	; _TRMEISTER_ASSERT(uiWordHead_ <= uiWordTail_);
	; _TRMEISTER_ASSERT(uiNormalizedWordHead_ <= uiNormalizedWordTail_);
	; _TRMEISTER_ASSERT(uiNormalizedWordHead_ <= uiNormalizedPatternTail_);
	; _TRMEISTER_ASSERT(uiNormalizedPatternTail_ <= uiNormalizedWordTail_);
	; _TRMEISTER_ASSERT(vecWordRange_.getSize() % 2 == 1);

	double dRatio = static_cast<double>(uiWordTail_ - uiWordHead_ + 1) /
		(uiNormalizedWordTail_ - uiNormalizedWordHead_ + 1);

	ModSize uiPatternTail = 0;
	if ((uiNormalizedWordHead_ + uiNormalizedWordTail_) / 2.0 <=
		static_cast<double>(uiNormalizedPatternTail_))
	{
		double dDiff =
			dRatio * (uiNormalizedWordTail_ - uiNormalizedPatternTail_);
		ModSize uiDiff = static_cast<ModSize>(dDiff);
		uiDiff += (dDiff > static_cast<double>(uiDiff)) ? 1 : 0;
		uiDiff = ModMin(uiWordTail_ - uiWordHead_, uiDiff);
		uiPatternTail = uiWordTail_ - uiDiff;
	}
	else
	{
		ModSize uiLength = static_cast<ModSize>(
			dRatio * (uiNormalizedPatternTail_ - uiNormalizedWordHead_ + 1));
		uiLength = ModMin(uiWordTail_ - uiWordHead_ + 1, uiLength);
		uiPatternTail = uiWordHead_ + uiLength - 1;
	}
	
	// [NOTE] The order in vecWordRange_ must be uiPatternHead <= uiPatternTail.
	vecWordRange_.pushBack(ModMax(uiPatternTail, vecWordRange_.getBack()));
}

//
//	FUNCTION private
//	Utility::Kwic::doGenerate -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
bool
Kwic::doGenerate(const ModUnicodeString& cstrSrc_,
				 ModSize uiOffset_,
				 ModSize uiSize_,
				 ModSize uiRoughSize_,
				 bool bHeadEllipsis_,
				 bool bTailEllipsis_,
				 const ModVector<ModSize>& vecWordRange_,
				 ModUnicodeString& cstrDst_,
				 const bool retry) const
{
	; _TRMEISTER_ASSERT(vecWordRange_.getSize() % 2 == 0);
	
	ModUnicodeOstrStream cStream;
	bool ret = false;

	if (cstrSrc_.getLength() > 0)
	{
		// first occurrence place
		if (retry)
		{
			uiSize_ = uiRoughSize_;  
			if (vecWordRange_.getSize() >= 1)
			{
				uiOffset_ = ModMax(static_cast<ModSize>(0), vecWordRange_[0] - ((uiSize_ > 1) ? 1 : 0));
			}
		}

		// Write head ellipsis
		if (bHeadEllipsis_ == true &&
			(uiOffset_ > 0 || cstrSrc_.getLength() >= uiRoughSize_))
		{
			cStream << m_cstrEllipsis;
		}

		// Write body
		const ModUnicodeChar* s = &cstrSrc_[0];
		const ModUnicodeChar* e = cstrSrc_.getTail();
		const ModUnicodeChar* p = s + uiOffset_;

		ModSize uiTotal = 0;
		ModSize prev = uiOffset_;
		bool bWordInside = false;
		for (ModVector<ModSize>::ConstIterator i = vecWordRange_.begin();
			 i != vecWordRange_.end(); ++i)
		{
			if (bWordInside == true)
			{
				// Write pattern
				cStream << m_cstrStartTag;
				; _TRMEISTER_ASSERT(*i + 1 >= prev);
				writeToStream(p, e, *i - prev + 1, cStream);
				cStream << m_cstrEndTag;
				
				uiTotal += *i - prev + 1;
				prev = *i + 1;
				bWordInside = false;
				ret = true;
			}
			else
			{
				// Write strings other than patterns
				; _TRMEISTER_ASSERT(*i >= prev);
				writeToStream(p, e, *i - prev, cStream);
				
				uiTotal += *i - prev;
				prev = *i;
				bWordInside = true;
			}
		}
		ModSize uiSize = ModMin(uiSize_, cstrSrc_.getLength());
		; _TRMEISTER_ASSERT(uiOffset_ + uiSize <= cstrSrc_.getLength());
		if (uiSize > uiTotal)
		{
			writeToStream(p, e, uiSize - uiTotal, cStream);
		}
		
		// Write tail ellipsis
		if (bTailEllipsis_ == true &&
			(p < e || cstrSrc_.getLength() >= uiRoughSize_))
		{
			cStream << m_cstrEllipsis;
		}
	}

	// Convert to string
	cstrDst_ = ModUnicodeString(cStream.getString());
	return ret;
}

//
//	FUNCTION private
//	Utility::Kwic::writeToStream -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
Kwic::writeToStream(const ModUnicodeChar*& p_,
					const ModUnicodeChar* e_,
					ModSize uiSize_,
					ModUnicodeOstrStream& cStream_) const
{
	const ModUnicodeChar* e = (p_ + uiSize_ < e_)  ? p_ + uiSize_ : e_;
	
	if (m_eEscape == EscapeMethod::HTML)
	{
		for(; p_ < e; ++p_)
		{
			switch(*p_)
			{
			case Common::UnicodeChar::usWquate:
				cStream_ << _cEscapeMethod_HTML_usWquate;
				break;
			case Common::UnicodeChar::usAmpersand:
				cStream_ << _cEscapeMethod_HTML_usAmpersand;
				break;
			case Common::UnicodeChar::usLcompare:
				cStream_ << _cEscapeMethod_HTML_usLcompare;
				break;
			case Common::UnicodeChar::usRcompare:
				cStream_ << _cEscapeMethod_HTML_usRcompare;
				break;
			default:
				cStream_ << *p_;
				break;
			}
		}
	}
	else if (m_eEscape == EscapeMethod::None)
	{
		for(; p_ < e; ++p_)
		{
			cStream_ << *p_;
		}
	}
	else
	{
		; _TRMEISTER_ASSERT(false);
	}
}

//
// Copyright (c) 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
