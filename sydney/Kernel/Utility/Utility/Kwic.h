// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// Kwic.h -- 
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

#ifndef	__SYDNEY_UTILITY_KWIC_H__
#define	__SYDNEY_UTILITY_KWIC_H__

#include "Utility/Module.h"
#include "Utility/PatternChecker.h"
#include "Utility/SearchTermData.h"
#include "Utility/Tokenizer.h"
#include "Utility/UNA.h"

#include "ModUnicodeString.h"
#include "ModVector.h"

class ModLanguageSet;
class ModUnicodeOstrStream;

_TRMEISTER_BEGIN

namespace Common
{
	class Data;
	class DataArrayData;
}

_TRMEISTER_UTILITY_BEGIN

//
// CLASS
// Kwic -- Key Word In Context
//
// NOTES
//
// HOW TO USE
// Kwic k;
// k.set(...);
// k.set(...);
//  Ignore data except for search terms in PropertyValue at second time or later
// ...
// ModSize s = k.getRoughSize();
// k.generate(cstSrc0_, ..., cstrDst0_);
// k.generate(cstSrc1_, ..., cstrDst1_);
// ...
// k.clear();
// k.set(...);
// ...
// s = k.getRoughSize();
// k.generate(...);
// ...
//
class Kwic
{
public:
	Kwic();
	~Kwic();

	void set(const Common::DataArrayData* pPropertyKey_,
			 const Common::DataArrayData* pPropertyValue_,
			 const ModUnicodeString& cstrStartTag_,
			 const ModUnicodeString& cstrEndTag_,
			 const ModUnicodeString& cstrEllipsis_,
			 const ModUnicodeString& cstrEscapeMethod_,
			 ModSize uiSize_);

	void clear();
	
	ModSize getRoughSize(int fieldNumber_) const;

	void generate(const ModUnicodeString& cstrSrc_,
				  ModSize uiRoughSize_,
				  const ModLanguageSet& cLanguageSet_,
				  ModUnicodeString& cstrDst_);

protected:
private:
	//
	//	STRUCT
	//	Utility::Kwic::EscapeMethod --
	//
	struct EscapeMethod
	{
		//	ENUM
		//	Utility::Kwic::EscapeMethod::Value --
		//
		enum Value
		{
			None =			0,
			HTML,
			
			Count,
			Unknown =		Count
		};
	};

	//
	//	CLASS
	//	Utility::Kwic::Condition --
	//
	class Condition
	{
	public:
		//
		//	STRUCT
		//	Utility::Kwic::Condition::Node --
		//
		struct Node
		{
			//
			//	STRUCT
			//	Utility::Kwic::Condition::Node::Type --
			//
			struct Type
			{
				enum Value
				{
					// The score is not propageted.
					Root,
					// The score is propageted when all the children are hit.
					Conjunction,
					// The score is propageted
					// when one or more children are hit.
					Disjunction,

					Undefined
				};
			};

			Node()
				: m_uiCount(0), m_uiParentID(0), m_eType(Type::Undefined),
				  m_uiChildrenCount(0) {}
			virtual ~Node() {}

			ModSize m_uiCount;
			ModSize m_uiParentID;
			Type::Value m_eType;
			ModSize m_uiChildrenCount;
		};
	
		//
		//	TYPEDEF
		//	Leaf -- 1: hit count, 2: Parent Node ID
		//
		typedef ModPair<ModSize, ModSize> Leaf;

		Condition() : m_vecNode(), m_vecLeaf(), m_uiPerfectScore(0),
			m_pChecker(0), m_pTokenizer(0), m_bWordSearch(false),
			m_vecSearchTermList() {}
		virtual ~Condition();

		void set(const Common::DataArrayData* pSearchTermList_,
				 PatternChecker* pChecker_ = 0,
				 Tokenizer* pTokenizer_ = 0);
		bool isPrepared() const;
		void prepare();
		void clear();
		
		void initializeStatus();
		void increment(ModSize uiLeafID_);
		void decrement(ModSize uiLeafID_);
		ModSize getScore() const { return m_vecNode[0].m_uiCount; }
		ModSize getPerfectScore() const { return m_uiPerfectScore; }
		
	private:
		//
		//	STRUCT
		//	Utility::Kwic::Condition::Separator --
		//
		struct Separator
		{
			enum Value
			{
				None,
				Insert,
				Expand
			};
		};

		void setSearchTerm(const SearchTermData* pSearchTerm_,
						   ModSize uiParentID_);
		
		void setExpandSearchTerm(
			ModVector<ModVector<ModUnicodeString> >::ConstIterator& i,
			const ModVector<ModVector<ModUnicodeString> >::ConstIterator& j,
			Separator::Value eHead_,
			Separator::Value eMid_,
			Separator::Value eTail_,
			ModSize uiCombination,
			ModSize uiParentID_);

		bool isWordSearch(const Common::DataArrayData* pSearchTermList_) const;
		
		SearchTermData::MatchMode::Value getMatchMode(
			const SearchTermData* pSearchTerm_) const;
		
		void getSeparatorMode(const SearchTermData* pSearchTerm_,
							  Separator::Value& eHead_,
							  Separator::Value& eMid_,
							  Separator::Value& eTail_) const;
		
		void createExpandTable(
			ModVector<ModSize>& vmax_,
			ModVector<ModVector<ModUnicodeString> >::ConstIterator i,
			const ModVector<ModVector<ModUnicodeString> >::ConstIterator& j,
			Separator::Value eMid_) const;
		
		ModSize addNode(Node::Type::Value eType_,
						ModSize uiParentID_ = 0);
		void addLeaf(ModSize uiParentID_);
		
		void incrementNode(ModSize uiNodeID_);
		void decrementNode(ModSize uiNodeID_);
		
		ModVector<Node> m_vecNode;
		ModVector<Leaf> m_vecLeaf;
		ModSize m_uiPerfectScore;
		PatternChecker* m_pChecker;
		Tokenizer* m_pTokenizer;
		ModVector<Common::Data::Pointer> m_vecSearchTermList;
		bool m_bWordSearch;
	};
	
	void analyzeProperty(
		const Common::DataArrayData* pPropertyKey_,
		const Common::DataArrayData* pPropertyValue_,
		ModVector<ModSize>* pvecRoughSize_,
		const Common::DataArrayData*& pSearchTermList_,
		const Common::DataArrayData*& pUnaParameterKey_,
		const Common::DataArrayData*& pUnaParameterValue_) const;

	EscapeMethod::Value
	getEscapeMethod(const ModUnicodeString& cstrEscapeMethod_) const;
	
	ModSize getContentSize(ModSize uiSize_) const;

	void getSeed(
		const PatternChecker::PatternLocationVec& vecPatternLocation_,
		ModSize uiContentSize_,
		ModSize& uiOffset_,
		ModSize& uiSize_);

	void determine(
		const PatternChecker::PatternLocationVec& vecPatternLocation_,
		const Tokenizer::WordInfoVec& vecWordInfo_,
		ModSize uiNormalizedSeedOffset_,
		ModSize uiNormalizedSeedSize_,
		double dNormalizedRatio_,
		ModSize& uiOffset_,
		ModSize& uiSize_,
		bool& bHeadEllipsis_,
		bool& bTailEllipsis_,
		ModVector<ModSize>& vecWordRange_) const;

	void determineOffset(
		const Tokenizer::WordInfoVec::ConstIterator& iteBegin_,
		const Tokenizer::WordInfoVec::ConstIterator& iteEnd_,
		ModSize uiNormalizedSeedOffset_,
		ModSize uiNormalizedSeedSize_,
		double dNormalizedRatio_,
		Tokenizer::WordInfoVec::ConstIterator& iteHeadWord_,
		Tokenizer::WordInfoVec::ConstIterator& iteTailWord_,
		ModSize& uiOffset_) const;

	bool isPeriod(int iPOS_) const
	{
		return (iPOS_ == Una::POS::Period::Japanese);
	}

	void setWordRange(
		const Tokenizer::WordInfoElement& cWordInfoElement_,
		const ModVector<ModSize>& vecPatternLength_,
		PatternChecker::PatternLocationVec::ConstIterator& j_,
		const PatternChecker::PatternLocationVec::ConstIterator& e_,
		ModSize& uiNormalizedPatternTail_,
		bool& bPatternInside_,
		ModVector<ModSize>& vecWordRange_) const;

	void pushPatternHead(ModSize uiWordHead_,
						 ModSize uiWordTail_,
						 ModSize uiNormalizedWordHead_,
						 ModSize uiNormalizedWordTail_,
						 ModSize uiNormalizedPatternHead_,
						 ModVector<ModSize>& vecWordRange_) const;
	
	void pushPatternTail(ModSize uiWordHead_,
						 ModSize uiWordTail_,
						 ModSize uiNormalizedWordHead_,
						 ModSize uiNormalizedWordTail_,
						 ModSize uiNormalizedPatternTail_,
						 ModVector<ModSize>& vecWordRange_) const;
	
	void doGenerate(const ModUnicodeString& cstrSrc_,
					ModSize uiOffset_,
					ModSize uiSize_,
					ModSize uiRoughSize_,
					bool bHeadEllipsis_,
					bool bTailEllipsis_,
					const ModVector<ModSize>& vecWordRange_,
					ModUnicodeString& cstrDst_) const;
	
	void writeToStream(const ModUnicodeChar*& p_,
					   const ModUnicodeChar* e_,
					   ModSize uiSize_,
					   ModUnicodeOstrStream& cStream_) const;
	
	// size of kwic which is generated by file driver
	ModVector<ModSize> m_vecRoughSize;
	Condition* m_pCondition;
	PatternChecker* m_pChecker;
	Tokenizer* m_pTokenizer;
	ModUnicodeString m_cstrStartTag;
	ModUnicodeString m_cstrEndTag;
	ModUnicodeString m_cstrEllipsis;
	EscapeMethod::Value m_eEscape;
	// size of kwic which is specified by user
	ModSize m_uiSize;
	// size of kwic whose margin is removed
	ModSize m_uiContentSize;
};

_TRMEISTER_UTILITY_END
_TRMEISTER_END

#endif	// __SYDNEY_UTILITY_KWIC_H__

//
// Copyright (c) 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
