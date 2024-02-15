// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// NgramTokenizer.h --
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_NGRAMTOKENIZER_H
#define __SYDNEY_FULLTEXT2_NGRAMTOKENIZER_H

#include "FullText2/Module.h"
#include "FullText2/Types.h"
#include "FullText2/Tokenizer.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::NgramTokenizer	-- NGRAMトークナイザー
//
//	NOTES
//
class NgramTokenizer : public Tokenizer
{
public:
	// コンストラクタ
	NgramTokenizer(FullTextFile& cFile_,
				   UNA::ModNlpAnalyzer* pAnalyzer_,
				   const ModUnicodeChar* pParameter_);
	// デストラクタ
	virtual ~NgramTokenizer();

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

	// 1つのLeafNodeを作成する
	LeafNode* createOneLeafNode(ListManager& cManager_,
								const ModUnicodeString& cTerm_);
	
	// 包含関係にあるものが存在しているか否か
	bool isContains(ModVector<ModUnicodeString>& vecToken_,
					ModVector<ModUnicodeString>::Iterator i_,
					MatchMode::Value eMatchMode_);
	
	// 正規化後の文字列
	ModUnicodeString m_cstrNormalized;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_NGRAMTOKENIZER_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
