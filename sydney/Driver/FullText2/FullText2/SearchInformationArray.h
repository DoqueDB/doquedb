// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SearchInformationArray.h --
// 
// Copyright (c) 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_SEARCHINFORMATIONARRAY_H
#define __SYDNEY_FULLTEXT2_SEARCHINFORMATIONARRAY_H

#include "FullText2/Module.h"
#include "FullText2/SearchInformation.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::SearchInformationArray -- 
//
//	NOTES
//	複数セクション用の検索情報クラス
//
class SearchInformationArray : public SearchInformation
{
public:
	// コンストラクタ
	SearchInformationArray(FileID& cFileID_);
	// デストラクタ
	virtual ~SearchInformationArray();
	// コピーコンストラクタ
	SearchInformationArray(const SearchInformationArray& cSrc_);

	// 各要素用の検索情報クラスを加える
	void pushElement(SearchInformation* pElement_);
	// 各要素用の検索情報クラスを得る
	SearchInformation& getElement(int n_);
	// 各要素用の検索情報クラスがnullかどうかを調べる
	bool isElementNull(int n_);
	// 各要素用の検索情報クラスの数を得る
	ModSize getElementSize() const { return m_vecpElement.getSize(); }

	// 検索情報クラスの配列を得る
	ModVector<SearchInformation*>& getAllElement() { return m_vecpElement; }

	// オリジナルの文書長があるか
	bool isOriginalLength();
	// 特徴語があるか
	bool isFeatureSet();

	// 最大文書IDを得る
	DocumentID getMaxDocumentID();
	// 削除文書数を得る
	ModSize getExpungeDocumentCount();

	// 文書長を得る
	bool getDocumentLength(DocumentID id_, ModSize& length_);
	
	// オリジナルの文書長を得る
	bool getOriginalLength(DocumentID id_, ModSize& length_);
	// 挿入したユニット番号を得る
	bool getUnitNumber(DocumentID id_, int& unit_);
	// スコア調整値を得る
	bool getScoreValue(DocumentID id_, double& score_);
	// セションサイズを得る
	bool getSectionSize(DocumentID id_,
						ModVector<ModSize>& vecSectionSize_);
	// 特徴語リストを得る
	bool getFeatureSet(DocumentID id_,
					   FeatureSetPointer& pFeatureSet_);

	// コピーを得る
	SearchInformation* copy() const;
	
	// DF値を設定する
	void setDocumentFrequency(const ModUnicodeString& tea_,
							  double dblDocumentFrequency_,
							  double dblTotalTermFrequency_);
	// DF値を設定する
	void setDocumentFrequency(const ModUnicodeString& tea_,
							  OperatorTermNode::Frequency& frequency_);
	// 検索文内頻度を設定する
	void setQueryTermFrequency(const ModUnicodeString& tea_,
							   double dblQueryTermFrequency_);
	
protected:
	// 全文書数を得る
	ModSize getDocumentCountImpl();
	// 総文書長を得る
	ModUInt64 getTotalDocumentLengthImpl();
	
	// 各要素用の検索情報クラス
	ModVector<SearchInformation*> m_vecpElement;
	// 先頭のnullではない要素
	int m_iNotNull;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_SEARCHINFORMATIONARRAY_H

//
//  Copyright (c) 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
