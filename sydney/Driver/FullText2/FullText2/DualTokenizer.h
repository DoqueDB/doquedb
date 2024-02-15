// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DualTokenizer.h --
// 
// Copyright (c) 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_DUALTOKENIZER_H
#define __SYDNEY_FULLTEXT2_DUALTOKENIZER_H

#include "FullText2/Module.h"
#include "FullText2/Types.h"
#include "FullText2/NgramTokenizer.h"

#include "FullText2/FeatureList.h"
#include "FullText2/AtomicOrLeafNode.h"
#include "FullText2/OrderedDistanceLeafNode.h"
#include "FullText2/WordLeafNode.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::DualTokenizer	-- DUALトークナイザー
//
//	NOTES
//
class DualTokenizer : public NgramTokenizer
{
public:
	// コンストラクタ
	DualTokenizer(FullTextFile& cFile_,
				  UNA::ModNlpAnalyzer* pAnalyzer_,
				  const ModUnicodeChar* pParameter_);
	// デストラクタ
	virtual ~DualTokenizer();

	// 特徴語取得のための情報を付加する
	void setFeatureParameter(ModSize uiFeatureSize_, // 特徴語の数
							 const Utility::ModTermResource* pTermResource_)
		{
			m_uiFeatureSize = uiFeatureSize_;
			m_pTermResource = pTermResource_;
		}

	// 特徴語リストを得る
	void getFeatureList(FeatureList& vecFeature_);

protected:
	// 検索用のLeafNodeを作成する
	LeafNode* createLeafNodeImpl(ListManager& cManager_,
								 const ModUnicodeString& cTerm_,
								 const ModLanguageSet& cLang_,
								 MatchMode::Value eMatchMode_);

	// 正規化する
	void normalize(const ModUnicodeString& cTarget_,	// 対象の文字列
				   const ModLanguageSet& cLang_,		// 対象の言語
				   ModSize uiStartPosition_,			// 対象文字列の先頭位置
				   SmartLocationListMap& cResult_,		// 結果
				   ModSize& uiSize_,					// 正規化後のサイズ
				   ModSize& uiOriginalSize_);			// 正規化前のサイズ
	// トークナイズする
	void tokenize(SmartLocationListMap& cResult_);		// 結果

	// 初期化する
	void initialize();

	// 展開語を取得する
	static ModBoolean getExpandWords(UNA::ModNlpAnalyzer* pAnalyzer_,
									 bool bNormalized_,
									 ModVector<ModUnicodeString>& vecExpanded_);

	// first が TF で、second が生起コスト
	typedef ModPair<int, int>	FeatureCandidateElement;
	
	typedef ModMap<ModUnicodeString, FeatureCandidateElement,
				   ModLess<ModUnicodeString> >		FeatureMap;

	// 正規化する - 特徴語取得用
	void normalizeFeature(const ModUnicodeString& cTarget_,
						  const ModLanguageSet& cLang_,
						  ModSize uiStartPosition_,
						  SmartLocationListMap& cResult_,
						  ModSize& uiSize_,
						  ModSize& uiOriginalSize_);
	// 正規化する - 通常用
	void normalizeNormal(const ModUnicodeString& cTarget_,
						 const ModLanguageSet& cLang_,
						 ModSize uiStartPosition_,
						 SmartLocationListMap& cResult_,
						 ModSize& uiSize_,
						 ModSize& uiOriginalSize_);

	// 検索ノードを作成する - DUAL索引用
	LeafNode* createLeafNodeForDual(ListManager& cManager_,
									const ModUnicodeString& cTerm_,
									const ModLanguageSet& cLang_,
									MatchMode::Value eMatchMode_);
	// 検索ノードを作成する - WORD索引用
	LeafNode* createLeafNodeForWord(ListManager& cManager_,
									const ModUnicodeString& cTerm_,
									const ModLanguageSet& cLang_,
									MatchMode::Value eMatchMode_);

	// 特徴語候補を設定する
	void setFeatureCandidate(const ModUnicodeString& cstrMorpheme_,
							 int iUnifinedPOS_,
							 int iCost_,
							 int iPOS_);

	// 展開された形態素を追加する - DUAL用
	bool appendExpandedString(
		const ModVector<ModUnicodeString>& expanded_,
		ModVector<ModUnicodeString>& vecToken_,
		ModVector<ModVector<ModSize> >& vecBoundary_);
	// 展開された形態素を追加する - WORD用
	bool appendExpandedWord(
		const ModVector<ModUnicodeString>& expanded_,
		ModVector<LeafNode::LocationVector>& vecToken_);
	
	// 特徴語を取得する数
	ModSize m_uiFeatureSize;
	// 特徴語を取得するためのTermResource
	const Utility::ModTermResource* m_pTermResource;

	// 特徴語候補のマップ
	FeatureMap m_mapFeatureCandidate;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_DUALTOKENIZER_H

//
//	Copyright (c) 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
