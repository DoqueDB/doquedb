// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// Tokenizer.cpp --
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

#include "Utility/SearchTermData.h"
#include "Utility/Tokenizer.h"
#include "Utility/UNA.h"

#include "Common/Assert.h"
#include "Common/DataArrayData.h"
#include "Common/LanguageData.h"
#include "Common/StringData.h"
#include "Common/UnicodeString.h"
#include "Common/UnsignedIntegerData.h"

#include "ModUnicodeOstrStream.h"
#include "ModUnicodeString.h"

#include "ModNLP.h"

_TRMEISTER_USING
_TRMEISTER_UTILITY_USING

namespace
{
	//
	//	VARIABLE
	//
	const ModUnicodeString _cUnaRscID = _TRMEISTER_U_STRING("UnaRscID");
	const ModUnicodeString _cDefaultLanguageSet = _TRMEISTER_U_STRING(
		"DefaultLanguageSet");
	const ModUnicodeString _cDoNorm = _TRMEISTER_U_STRING("donorm");
#ifdef DEBUG
	const ModUnicodeString _cCarriage = _TRMEISTER_U_STRING("carriage");
	const ModUnicodeString _cCompound = _TRMEISTER_U_STRING("compound");
	const ModUnicodeString _cMaxWordLen = _TRMEISTER_U_STRING("maxwordlen");
	const ModUnicodeString _cSpace = _TRMEISTER_U_STRING("space");
	const ModUnicodeString _cStem = _TRMEISTER_U_STRING("stem");
#endif
	
	const ModUnicodeString _cTrue = _TRMEISTER_U_STRING("true");
}

//
//	FUNCTION public
//	Utility::Tokenizer::Tokenizer -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
Tokenizer::Tokenizer()
	: m_pAnalyzer(0), m_cDefaultLanguageSet(), m_bNormalize(false)
{}

//
//	FUNCTION public
//	Utility::Tokenizer::~Tokenizer -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
Tokenizer::~Tokenizer()
{
	clear();
}

//
//	FUNCTION public
//	Utility::Tokenizer::set -- 
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
Tokenizer::set(const Common::DataArrayData* pPropertyKey_,
			   const Common::DataArrayData* pPropertyValue_)
{
	; _TRMEISTER_ASSERT(
		pPropertyKey_ != 0 && pPropertyValue_ != 0 &&
		pPropertyKey_->isNull() == false && 
		pPropertyValue_->isNull() == false &&
		pPropertyKey_->getCount() == pPropertyValue_->getCount());

	// Analyze properties
	ModMap<ModUnicodeString, ModUnicodeString, ModLess<ModUnicodeString> > cMap;
	Una::ResourceID::Value uiUnaRscID = Una::ResourceID::Unknown;
	ModLanguageSet cDefaultLanguageSet = ModLanguageSet();
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
		if (cstrKey.compare(_cUnaRscID) == 0)
		{
			; _TRMEISTER_ASSERT(pValueData->getType() ==
								Common::DataType::UnsignedInteger);
			const Common::UnsignedIntegerData* pUnsignedIntegerData
				= _SYDNEY_DYNAMIC_CAST(const Common::UnsignedIntegerData*,
									   pValueData);
			uiUnaRscID = pUnsignedIntegerData->getValue();
			if (uiUnaRscID == 0)
			{
				// [NOTE] Resource for Like operator is used
				//  when resource ID is not defined.
				uiUnaRscID = Una::Manager::getResourceForLikeOperator();
			}
		}
		else if (cstrKey.compare(_cDefaultLanguageSet) == 0)
		{
			; _TRMEISTER_ASSERT(pValueData->getType() ==
								Common::DataType::Language);
			const Common::LanguageData* pLanguageData
				= _SYDNEY_DYNAMIC_CAST(const Common::LanguageData*, pValueData);
			cDefaultLanguageSet = pLanguageData->getValue();
		}
		else
		{
			; _TRMEISTER_ASSERT(cstrKey.compare(_cCarriage) == 0 ||
								cstrKey.compare(_cCompound) == 0||
								cstrKey.compare(_cDoNorm) == 0||
								cstrKey.compare(_cMaxWordLen) == 0 ||
								cstrKey.compare(_cSpace) == 0 ||
								cstrKey.compare(_cStem) == 0);
			
			; _TRMEISTER_ASSERT(pValueData->getType() ==
								Common::DataType::String);
			const Common::StringData* pStringData
				= _SYDNEY_DYNAMIC_CAST(const Common::StringData*, pValueData);
			
			if (cstrKey.compare(_cDoNorm) == 0 &&
				pStringData->getValue().compare(_cTrue) == 0)
			{
				m_bNormalize = true;
			}
			
			cMap.insert(cstrKey, pStringData->getValue());
		}
	}
	; _TRMEISTER_ASSERT(uiUnaRscID != Una::ResourceID::Unknown);
	// [NOTE] Default language may be empty.
	//  In that case, UNA's default language is used.
	
	// Set propertyies
	m_pAnalyzer = Una::Manager::getModNlpAnalyzer(uiUnaRscID);
	m_pAnalyzer->prepare(cMap);
	m_cDefaultLanguageSet = cDefaultLanguageSet;
}

//
//	FUNCTION public
//	Utility::Tokenizer::clear -- 
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
Tokenizer::clear()
{
	if (m_pAnalyzer != 0)
	{
		m_pAnalyzer->releaseResource();
		delete m_pAnalyzer, m_pAnalyzer = 0;
	}
	if (m_cDefaultLanguageSet.getSize() > 0)
	{
		m_cDefaultLanguageSet.clear();
	}
	m_bNormalize = false;
}

//
//	FUNCTION public
//	Utility::Tokenizer::getWordInfo -- 
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
Tokenizer::getWordInfo(const ModUnicodeString& cstrSrc_,
					   const ModLanguageSet& cLanguageSet_,
					   ModUnicodeChar usWordSeparator_,
					   ModUnicodeString& cstrDst_,
					   WordInfoVec& vecWordInfo_) const
{
	ModUnicodeOstrStream cStream;
	
	if (cstrSrc_.getLength() > 0)
	{
		m_pAnalyzer->set(cstrSrc_,
						 (cLanguageSet_.getSize() > 0) ?
						 cLanguageSet_ : m_cDefaultLanguageSet);

		ModVector<ModUnicodeString> vecNormalized;
		ModVector<ModUnicodeString> vecOriginal;
		ModVector<int> vecPOS;
		ModVector<ModUnicodeString>::ConstIterator iteNormalized;
		ModVector<ModUnicodeString>::ConstIterator iteOriginal;
		ModVector<int>::ConstIterator itePOS;
		int iNormalizedHeadPos = 0;
		int iNormalizedTailPos = -1;
		int iOriginalHeadPos = 0;
		int iOriginalTailPos = -1;
		const ModUnicodeChar* p = &cstrSrc_[0];
		const ModUnicodeChar* e = cstrSrc_.getTail();
		while(m_pAnalyzer->getBlock(
				  vecNormalized, vecOriginal, vecPOS) == ModTrue)
		{
			iteNormalized = vecNormalized.begin();
			iteOriginal = vecOriginal.begin();
			itePOS = vecPOS.begin();
			for (; itePOS != vecPOS.end();
				 ++itePOS, ++iteNormalized, ++iteOriginal)
			{
				; _TRMEISTER_ASSERT((*iteNormalized).getLength() > 0);
				; _TRMEISTER_ASSERT((*iteOriginal).getLength() > 0);

				if (usWordSeparator_ != Common::UnicodeChar::usNull)
				{
					cStream << usWordSeparator_;
				}
				cStream << *iteNormalized;

				// Set positions
				iNormalizedHeadPos = iNormalizedTailPos + 1;
				iNormalizedTailPos =
					iNormalizedHeadPos + (*iteNormalized).getLength() - 1;
				// [NOTE] Ingnored characters may exist between Originals.
				iOriginalHeadPos =
					iOriginalTailPos + 1 +
					getIgnoredCharacterCount(p, e, *iteOriginal);
				iOriginalTailPos =
					iOriginalHeadPos + (*iteOriginal).getLength() - 1;
				// [NOTE] Original does not include any ignored character.
				; _TRMEISTER_ASSERT(p < e);
				p += (*iteOriginal).getLength();
				; _TRMEISTER_ASSERT(p <= e);

				// Set vecWordInfo_
				vecWordInfo_.pushBack(
					WordInfoElement(
						WordInfoLocation(
							static_cast<ModSize>(iNormalizedHeadPos),
							static_cast<ModSize>(iNormalizedTailPos)),
						WordInfoSubElement(
							WordInfoLocation(
								static_cast<ModSize>(iOriginalHeadPos),
								static_cast<ModSize>(iOriginalTailPos)),
							*itePOS)));
			}
			vecNormalized.erase(vecNormalized.begin(), vecNormalized.end());
			vecOriginal.erase(vecOriginal.begin(), vecOriginal.end());
			vecPOS.erase(vecPOS.begin(), vecPOS.end());
		}
	}

	// Set cstrDst_
	if (cStream.isEmpty() == ModFalse &&
		usWordSeparator_ != Common::UnicodeChar::usNull)
	{
		cStream << usWordSeparator_;
	}
	cstrDst_ = cStream.getString();
}

//
//	FUNCTION public
//	Utility::Tokenizer::setSearchTerm -- 
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
Tokenizer::setSearchTerm(const SearchTermData* pSearchTerm_)
{
	; _TRMEISTER_ASSERT(m_pAnalyzer != 0);
	m_pAnalyzer->set(pSearchTerm_->getTerm(),
					 (pSearchTerm_->getLanguage().getSize() > 0 ?
					  pSearchTerm_->getLanguage() : m_cDefaultLanguageSet));
}

//
//	FUNCTION public
//	Utility::Tokenizer::setSearchTerm -- 
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
Tokenizer::getExpandWords(ModVector<ModUnicodeString>& vecMorpheme_)
{
	; _TRMEISTER_ASSERT(m_pAnalyzer != 0);
	; _TRMEISTER_ASSERT(vecMorpheme_.getSize() == 0);

	ModBoolean result = ModFalse;
	if (m_bNormalize == true)
	{
		ModUnicodeString dummy1;
		int dummy2;
		result = m_pAnalyzer->getExpandWords(vecMorpheme_, dummy1, dummy2);
	}
	else
	{
		// [NOTE] UNA::ModNlpAnalyze::getExpandWords() needs a normalized string
		//  which has been set in UNA::ModNlpAnalyze::set().
		//  So you can NOT expand it when it has been not normalized.
		ModUnicodeString cstrMorpheme;
		result = m_pAnalyzer->getWord(cstrMorpheme);
		if (result == ModTrue)
		{
			vecMorpheme_.pushBack(cstrMorpheme);
		}
	}
	return (result == ModTrue) ? true : false;
}

//
//	FUNCTION private
//	Utility::Tokenizer::getIgnoredCharacterCount -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
int
Tokenizer::getIgnoredCharacterCount(const ModUnicodeChar*& p_,
									const ModUnicodeChar* e_,
									const ModUnicodeString& cstrWord_) const
{
	; _TRMEISTER_ASSERT(cstrWord_.getLength() > 0);
	const ModUnicodeChar c = cstrWord_[0];
	
	const ModUnicodeChar* s = p_;
	for (; p_ < e_; ++p_)
	{
		if (*p_ == c)
		{
			break;
		}
		// [NOTE] *p_ is an ignored character, when not equal to c.
	}
	; _TRMEISTER_ASSERT(p_ < e_);

	return static_cast<int>(p_ - s);
}

//
// Copyright (c) 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
