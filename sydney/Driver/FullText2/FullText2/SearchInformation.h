// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SearchInformation.h --
// 
// Copyright (c) 2010, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_SEARCHINFORMATION_H
#define __SYDNEY_FULLTEXT2_SEARCHINFORMATION_H

#include "FullText2/Module.h"
#include "FullText2/Types.h"
#include "FullText2/FeatureSet.h"
#include "FullText2/ScoreCalculator.h"
#include "FullText2/OperatorTermNode.h"

#include "ModMap.h"
#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class FileID;
class SearchInformationConcatinate;

//
//	CLASS
//	FullText2::SearchInformation -- 
//
//	NOTES
//	転置リスト以外のデータを検索時に取得するためのインターフェース
//
class SearchInformation
{
	friend class SearchInformationConcatinate;
	
public:
	// 検索文内頻度、文書頻度、総文書内頻度と、
	// 検索ノードを格納するための構造体
	struct MapValue
	{
		MapValue()
			: m_dblQueryTermFrequency(0.0),
			  m_dblDocumentFrequency(0.0),
			  m_dblTotalTermFrequency(0.0),
			  m_bDone(false),
			  m_pTermNode(0) {}
		MapValue(OperatorTermNode* pTermNode_)
			: m_dblQueryTermFrequency(0.0),
			  m_dblDocumentFrequency(0.0),
			  m_dblTotalTermFrequency(0.0),
			  m_bDone(false),
			  m_pTermNode(pTermNode_) {}
		MapValue(double dblDocumentFrequency_, double dblTotalTermFrequency_)
			: m_dblQueryTermFrequency(0.0),
			  m_dblDocumentFrequency(dblDocumentFrequency_),
			  m_dblTotalTermFrequency(dblTotalTermFrequency_),
			  m_bDone(true),
			  m_pTermNode(0) {}

		double m_dblQueryTermFrequency;		// 検索文内頻度
		double m_dblDocumentFrequency;		// 文書頻度
		double m_dblTotalTermFrequency;		// 総文書内頻度

		bool   m_bDone;						// 取得済みかどうか

		OperatorTermNode* m_pTermNode;
	};
	
	// 件数見積りのレベル
	struct EstimateLevel
	{
		enum Value
		{
			Unknown = 0,

			Level1,			// 索引単位の登録件数のみの利用
			Level2,			// 文書IDの突合せを実施
			Level3,			// 位置情報の突合せも実施(正確な値)

			ValueNum
		};
	};
	
	// コンストラクタ
	SearchInformation(FileID& cFileID_);
	// デストラクタ
	virtual ~SearchInformation();
	// コピーコンストラクタ
	SearchInformation(const SearchInformation& src_);

	// 異表記正規化しているかどうか
	bool isNormalized() const { return m_bNormalized; }
	// 位置情報があるかどうか
	bool isNolocation() const { return m_bNolocation; }
	// 索引タイプを得る
	IndexingType::Value getIndexingType() const { return m_eIndexingType; }

	// 見積りレベルを得る
	EstimateLevel::Value getEstimateLevel();

	// オリジナルの文書長があるか
	virtual bool isOriginalLength() = 0;
	// 特徴語があるか
	virtual bool isFeatureSet() = 0;

	// キーの数
	ModSize getKeyCount() const { return m_uiKeyCount; }

	// 全文書数を設定する
	void setDocumentCount(ModSize uiDocumentCount_)
		{ m_uiDocumentCount = uiDocumentCount_; }

	// 全文書数を得る
	ModSize getDocumentCount();
	// 総文書長を得る
	ModUInt64 getTotalDocumentLength();
	// 最大文書IDを得る
	virtual DocumentID getMaxDocumentID() = 0;
	// 削除文書数を得る
	virtual ModSize getExpungeDocumentCount() = 0;

	// 文書長を得る
	virtual bool getDocumentLength(DocumentID id_, ModSize& length_) = 0;
	// オリジナルの文書長を得る
	virtual bool getOriginalLength(DocumentID id_, ModSize& length_) = 0;
	// 挿入したユニット番号を得る
	virtual bool getUnitNumber(DocumentID id_, int& unit_) = 0;
	// スコア調整値を得る
	virtual bool getScoreValue(DocumentID id_, double& score_) = 0;
	// セクションサイズを得る
	virtual bool getSectionSize(DocumentID id_,
								ModVector<ModSize>& vecSectionSize_) = 0;
	// 特徴語リストを得る
	virtual bool getFeatureSet(DocumentID id_,
							   FeatureSetPointer& pFeatureSet_) = 0;
	
	// コピーを得る
	virtual SearchInformation* copy() const = 0;

	// 各要素用の検索情報クラスを得る
	virtual SearchInformation& getElement(int n_);
	// 各要素用の検索情報クラスがnullかどうかを調べる
	virtual bool isElementNull(int n_);
	// 各要素用の検索情報クラスの数を得る
	virtual ModSize getElementSize() const;

	// 検索情報クラスの配列を得る
	virtual ModVector<SearchInformation*>& getAllElement();

	//
	// スコア計算のための数値を上書きする
	// 
	// 総文書長を設定する
	void setTotalDocumentLength(ModUInt64 uiTotalDocumentLength_)
		{ m_dblTotalDocumentLength
			  = static_cast<double>(uiTotalDocumentLength_); }
	// 平均文書長を設定する
	void setAverageLength(double dblAverageLength_)
		{ m_dblAverageLength = dblAverageLength_; }
	// 総文書数を設定する
	void setTotalDocumentFrequency(ModSize uiTotalDocumentFrequency_)
		{ m_dblTotalDocumentFrequency
			  = static_cast<double>(uiTotalDocumentFrequency_); }

	// スコア計算のための総文書数を得る
	double getTotalDocumentFrequency();

	// スコア計算に必要で、文書ごとに値が変化しないものの値を設定する
	void initializeScoreArgument(
		ModVector<ScoreCalculator::Argument>& vecScoreArg_,
		OperatorTermNode* pTermNode_);

	// DF値を求める検索語ノードを追加する
	// すでに同じキーのエントリが存在している場合には、何もしない
	MapValue* addTermNode(const ModUnicodeString& tea_,
						  OperatorTermNode* node_);
	
	// DF値を設定する
	virtual void setDocumentFrequency(const ModUnicodeString& tea_,
									  double dblDocumentFrequency_,
									  double dblTotalTermFrequency_);
	// DF値を設定する
	virtual void
	setDocumentFrequency(const ModUnicodeString& tea_,
						 OperatorTermNode::Frequency& frequency_);
	// 検索文内頻度を設定する
	virtual void setQueryTermFrequency(const ModUnicodeString& tea_,
									   double dblQueryTermFrequency_);
	
	// 検索語ノードを検索する
	MapValue* findTermNode(const ModUnicodeString& tea_);

	// DF値を求める
	void setUpDocumentFrequency(bool bTotalTermFrequency_);
	// DF値を見積もる
	void estimateDocumentFrequency();

	// 検索語ノードを得る
	void getTermNodeForDocumentFrequency(
		ModVector<OperatorTermNode*>& vecNode_);

	// 検索語ノードマップを得る
	ModMap<ModUnicodeString, MapValue,
		   ModLess<ModUnicodeString> >& getTermMap() { return *m_pTermMap; }

protected:
	// 文書頻度または総文書内頻度のデータを格納するマップ
	typedef ModMap<ModUnicodeString, MapValue,
				   ModLess<ModUnicodeString> > TermMap;

	// 全文書数を得る
	virtual ModSize getDocumentCountImpl() = 0;
	// 総文書長を得る
	virtual ModUInt64 getTotalDocumentLengthImpl() = 0;
	
	// スコア計算に必要な数を設定する
	void initializeDocumentLength();

	// 全文書数
	ModSize m_uiDocumentCount;
	
	// 総文書長
	double m_dblTotalDocumentLength;
	// 平均文書長
	double m_dblAverageLength;
	// 総文書数
	double m_dblTotalDocumentFrequency;
	// これらの数値を設定したかどうか
	bool m_bInitializedDocumentLength;

	// 検索ノードを格納するマップ
	TermMap* m_pTermMap;
	// オーナーかどうか
	bool m_bTermMapOwner;

	// 異表記正規化するかどうか
	bool m_bNormalized;
	// 位置情報が格納されていないかどうか
	bool m_bNolocation;
	// 索引タイプ
	IndexingType::Value m_eIndexingType;
	// キーの数
	ModSize m_uiKeyCount;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_SEARCHINFORMATION_H

//
//  Copyright (c) 2010, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
