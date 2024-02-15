// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SearchCapsule.h --
// 
// Copyright (c) 2010, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_SEARCHCAPSULE_H
#define __SYDNEY_FULLTEXT2_SEARCHCAPSULE_H

#include "FullText2/Module.h"

#include "FullText2/SortParameter.h"
#include "FullText2/OptionDataFile.h"
#include "FullText2/OpenOption.h"

#include "LogicalFile/FileID.h"
#include "FullText2/IntermediateFileID.h"

#include "FullText2/IndexFile.h"
#include "FullText2/IndexFileSet.h"
#include "FullText2/SearchResultSet.h"

#include "LogicalFile/OpenOption.h"

#include "Common/Object.h"
#include "Common/WordData.h"

#include "ModInvertedQuery.h"

#include "ModLanguageSet.h"
#include "ModPair.h"

class ModInvertedQueryParser;
class ModInvertedSearchResult;
class ModInvertedTokenizer;

_SYDNEY_BEGIN

namespace Common
{
	class DataArrayData;
}

namespace Utility
{
	class ModTerm;
	class ModTermPool;
	class ModTermIndex;
}

_SYDNEY_FULLTEXT2_BEGIN

//
//  CLASS
//  FullText2::SearchCapsule --
//
//  NOTES
//
class SearchCapsule
{
public:
	typedef ModInvertedVector<Common::WordData> WordSet;
	typedef ModInvertedVector<ModPair<Common::WordData, int> > WordSetInternal;
	typedef ModInvertedSearchResult* ResultSet;

	typedef SearchResultSet::ClusterIDList ClusterIDList;
	
	// コンストラクタ
	SYD_FULLTEXT2_FUNCTION
	SearchCapsule(const LogicalFile::OpenOption& cOpenOption_,
				  OptionDataFile* pOptionDataFile_,
				  IndexFileSet *pIndexFileSet_,
				  ModInvertedTokenizer *pTokenizer);
	// デストラクタ
	SYD_FULLTEXT2_FUNCTION
	virtual ~SearchCapsule();

	// 文書取得
	SYD_FULLTEXT2_FUNCTION
	void execute(ModSize limit,
				 SortParameter::Value eSort_,
				 ModSize &nTerm,
				 ResultSet &pSearchResult,
				 ClusterIDList& cClusterIDList_);

	// 文書全件取得(転置ファイル単位)
	SYD_FULLTEXT2_FUNCTION
	void execute(ModUInt32 sign,
				 ResultSet resultSet)
	{
		// [NOTE] signは一つの転置ファイルを指している。マスクは不可。
		IndexFileSet::Iterator iter = m_pIndexFileSet->findEntity(sign);
		if(iter != m_pIndexFileSet->end())
		{
			search(iter,resultSet);
		}
	}

	// 単語取得
	// exclusively for the ranking search
	SYD_FULLTEXT2_FUNCTION
	void execute(ModSize limit_,
				SortParameter::Value eSort_,
				WordSet *pWordSet );

	// 拡張文書を設定する
	SYD_FULLTEXT2_FUNCTION
	void pushExpandDocument(const ModUnicodeString& cstrDocument_,
							const ModLanguageSet& cLanguage_);

	// IndexFileSetを取得
	IndexFileSet *getIndexFileSet(){ return m_pIndexFileSet; }

	// tea構文形式の条件式を取得する
	SYD_FULLTEXT2_FUNCTION
	const ModUnicodeString& getCondition();

	// ヒット件数の見積もり
	SYD_FULLTEXT2_FUNCTION
	ModSize getEstimateCount();

	// クラスタIDリストを取得
	SYD_FULLTEXT2_FUNCTION
	bool getCluster(ClusterIDList &cCluster_,
					ResultSet& pSearchResult_,
					ModSize uiPos_);

	// 検索語リストを取得
	SYD_FULLTEXT2_FUNCTION
	void getSearchTermList(Common::DataArrayData& cSearchTermList_,
						   ModUInt32 uiResultType_);

	// このカプセルの実行後にセクション検索を行うかどうかを設定する
	SYD_FULLTEXT2_FUNCTION
	void setSectionSearchFlag(bool flag_);

	// ページをdetachする
	void detach();

private:
	
	// クラスタのソート用
	// [スコア, [クラスタ開始位置, クラスタ終了位置]]
	typedef ModPair<ModSize, ModSize> ClusterRange;
	typedef ModPair<ModInvertedDocumentScore, ClusterRange> ClusterElement;
	class _ClusterElementGreater
	{
	public:
		ModBoolean operator() (const ClusterElement& x, const ClusterElement& y)
			{ return (x.first > y.first) ? ModTrue : ModFalse; }
	};

	// 索引ファイルのシグネチャーリスト
	typedef ModVector<ModUInt32> SignatureList;
	// <索引ファイルのシグネチャー, Queryへのポインタ>
	typedef ModPair<ModUInt32,ModInvertedQuery *> QueryElement;
	typedef ModVector<QueryElement> QueryList;
	// <検索語, QueryNodeへのポインタ>のマップのベクタ
	typedef ModInvertedQuery::QueryNodeMap QueryNodeMap;
	typedef ModVector <QueryNodeMap> QueryNodeMapList;
	// <検索語, (未使用？)>のマップ
	typedef ModMap <ModUnicodeString,int ,ModLess<ModUnicodeString> > TermMap;

	//////////////////////////////////////////////////////////
	//  method section

	//////////////////////////////////////////////////////////
	//  検索関数
	//
 
	// 通常(非Sydney固有)の検索
	void search(ModSize  limit_,
				SortParameter::Value eSort_,
				ResultSet &pSearchResult_);
	// Sydney固有(FREETEXT,WORDLIST)の検索
	void search(ModSize &nTerm,
				ModSize  limit_,
				SortParameter::Value eSort_,
				ResultSet &pSearchResult_);
	// 全件検索(転置ファイル単位)
	void search(IndexFileSet::Iterator iter,
				ResultSet & pResultSet);
	// 全件検索のヘルパー関数(複数転置ファイル指定可)
	ModSize search(SearchResultSet &searchResultSet,
				   const ModUInt32 sig);

	// DFの取得
	ModSize getDocumentFrequency(bool isEstimate_,
								 const ModUnicodeString& cstrCondition_,
								 SearchResultSet& SearchResultSet,
								 bool isGetCount_ = false);

	/////////////////////////////////////////////////////////////////////////
	//  検索用の内部関数
	//  search()から呼ばれる

	// 通常(非Sydney固有)の検索のロジック本体
	ModSize retrieve(const ModUnicodeString& cstrCondition_,
					 SearchResultSet& SearchResultSet,
					 const ModUInt32 sig,
					 ModInt32 queryTermNo = 0,
					 ModUInt32 queryTermFrequency = 1);

	// Sydney固有検索のロジック本体(OR検索)
	void retrieve(int matchMode,
				  bool& bcond,
				  Utility::ModTermPool& pool,
				  SearchResultSet& searchResultSet_,
				  bool bIgnoreTsv_ = false);
	
	// Sydney固有検索のロジック本体(AND検索)
	void retrieveAND(int matchMode,
					 Utility::ModTermPool& pool,
					 SearchResultSet& searchResultSet_);
	// Sydney固有検索のロジック本体(ADD検索)
	void retrieveADD(int matchMode,
					 bool& bcond,
					 Utility::ModTermPool& pool,
					 SearchResultSet& searchResultSet_,
					 bool bIgnoreTsv_ = false);

	// 通常(非Sydney固有)の検索のロジック本体の下請け
	void (SearchCapsule::*_retrieve)(
		InvertedFile* invertedFile,
		const ModUnicodeString& cstrCondition_,
		ModInvertedQueryParser& cParser,	
		ModInvertedQuery* query,
		ModInvertedBooleanResult* expungedDocumentId,
		ModInvertedDocumentID upperDoc);
	void __booleanRetrieve(InvertedFile* invertedFile,
						   const ModUnicodeString& cstrCondition_,
						   ModInvertedQueryParser& cParser,
						   ModInvertedQuery* query,
						   ModInvertedBooleanResult* expungedDocumentId,
						   ModInvertedDocumentID upperDoc);
	void __nonbooleanRetrieve(InvertedFile* invertedFile,
							  const ModUnicodeString& cstrCondition_,
							  ModInvertedQueryParser& cParser,
							  ModInvertedQuery* query,
							  ModInvertedBooleanResult* expungedDocumentId,
							  ModInvertedDocumentID upperDoc);

	void retrieveFirstStep(const ModUnicodeString& cstrCondition_,
						   SearchResultSet &searchResultSet,
						   ModUInt32 uiSignature_,
						   QueryList& cQueryList_,
						   QueryNodeMapList& vecQueryNodeMap_,
						   TermMap& cTermMap_);
	
	void retrieveSecondStep(SearchResultSet &searchResultSet,
							const QueryList& cQueryList_,
							const QueryNodeMapList& vecQueryNodeMap_,
							const TermMap& cTermMap_,
							ModSize& uiDF_,
							ModUInt32& uiQueryTermFrequency_);

	void getQueryNode(const ModUnicodeString& cstrKey_,
					  ModVector<ModInvertedQueryNode*>& vecQueryNode_,
					  ModSize& uiDF_,
					  ModUInt64& ui64TotalTermFrequency,
					  const QueryList& cQueryList_,
					  const QueryNodeMapList& vecQueryNodeMap_,
					  bool& bNeed_,
					  bool& bExtendedScore_,
					  SignatureList& vecSigunature_);

	void adjustDFAndTotalTF(const ModUnicodeString& cstrKey_,
							SearchResultSet &searchResultSet,
							ModSize& uiDF_,
							ModUInt64& ui64TotalTermFrequency,
							bool bNeed_,
							bool bExtendedScore_,
							const SignatureList& vecSigunature_);


	/////////////////////////////////////////////////////////////////////////
	//	ソート

	// ソートを後回しにするか？
	bool isDelayedSort(SortParameter::Value eSort_) const;
	// 後回しにしたソートを実行する
	void doDelayedSort(SortParameter::Value eSort_,
					   ResultSet& pSearchResult_,
					   ClusterIDList& cClusterIDList_) const;
	
	// スコアの調整
	void modifyValue(ResultSet pSearchResult_);
	
	/////////////////////////////////////////////////////////////////////////
	//  クラスタリング関連
	
	// 各クラスタ内をソートする
	void sortEachCluster(SortParameter::Value eSort_,
						 ResultSet& pSearchResult_,
						 ClusterIDList& cClusterIDList_,
						 ModVector<ClusterElement>& vecClusterElement_) const;
	// クラスタ単位でソートする
	ModSize sortByCluster(SortParameter::Value eSort_,
						  ResultSet& pSearchResult_,
						  ModVector<ClusterElement>& vecClusterElement_) const;

	// 段階的クラスタリングが可能か？
	bool isPhasedClustering(SortParameter::Value eSort_) const;

	//////////////////////////////////////////////////////////
	// libTermの検索
	//
	// freetext ->(libterm)->tea statment
	//
	// 質問処理ライブラリーを得る
	void initializeTerm(const ModUnicodeString& extractor_,
						ModSize collectionSize_,
						ModSize averageLength_);

	// DFを推定で実行するかどうか
	bool isEstimateDF() const { return m_bEstimateDF; }
	
	// ModTermPoolを自然文または単語リストから作成
	Utility::ModTermPool* makeTermPool(bool &isEssential);
	// ModTermPoolを拡張文書から作成
	Utility::ModTermPool* makeTermPoolFromExpandDocument(
		int matchMode,
		SearchResultSet &searchResultSet,
		Utility::ModTermPool *pool);
	
	// ModTermPoolを自然文から作成 (makeTermPoolの下請け)
	Utility::ModTermPool* makePool(const ModUnicodeString& freetext_,
								   const ModLanguageSet& lang_);
	// ModTermPoolを単語リストから作成 (makeTermPoolの下請け)
	Utility::ModTermPool* makePool(const WordSetInternal& wordSet_,
								   bool& isEssential_);

	//////////////////////////////////////////////////////////
	// その他
	
	// Sydney固有の検索の準備
	void prepareForSydneySearch(ModUInt32 uiResultType_);
	// Sydney固有の検索の後処理
	void clearForSydneySearch();

	// マッチモードを取得
	int getTermMatchMode();

	// WordDataの設定
	static void setWordData(WordSet* pWordSet_,
							const Utility::ModTermIndex& cTermIndex_,
							Common::WordData::Category::Value eCategory_);
	// WordSetをソート
	static void sortWordSet(WordSet* pWordSet_,
							SortParameter::Value eSort_);

	// Tea構文に変換
	void convertToTeaFormatForGetSearchTerm(
		int iMatchMode_,
		ModUnicodeString& cstrCondition_) const;
	
	//////////////////////////////////////////////////////////
	//  data section

	// オプションデータを取得するファイル
	OptionDataFile* m_pOptionDataFile;
	// 転置ファイルのカプセルクラス
	IndexFileSet* m_pIndexFileSet;

	// tea構文形式の検索文
	ModUnicodeString m_cstrCondition;
	int m_iConditionLength;
	// スコア計算器
	ModUnicodeString m_cCalculator;
	// 総文書長(Word索引以外)　全単語数（Word索引）
	ModUInt64 m_uiTotalDocumentLength;
	// 平均文書長
	ModSize m_uiAverageLength;
	// 文書頻度
	ModSize m_uiDocumentFrequency;
	// Extractor
	ModUnicodeString m_cExtractor;
	// 質問処理ライブラリー
	Utility::ModTerm* m_pTerm;
	// DFを推定で実行するかどうか
	bool m_bEstimateDF;
	// 通常語上限
	int m_iNormalLimit;
	// 拡張語上限
	int m_iExpandLimit;
	// 拡張用文字列
	ModVector<ModUnicodeString> m_vecDocument;
	// 言語情報
	ModVector<ModLanguageSet> m_vecLanguage;
	// 後でセクション検索を実行するかどうか
	bool m_bDoSectionSearch;
	// クラスリングを行うかどうか
	bool m_bCluster;
	// 検索条件
	OpenOption	m_cOpenOption;
	// 検索結果型
	ModUInt32 m_cResultType;
	// 検索結果(or 検索結果クラスタ)数の上限
	ModSize m_uiLimit;
	// 取得クラスタ数
	ModSize m_uiClusterCount;
	// クラスタリング済みの検索結果数
	ModSize m_uiClusteredSize;

	// prepareForSydneySearch()の結果のキャッシュ
	SearchResultSet m_cSearchResultSet;
	Utility::ModTermPool* m_pTermPool;
	Utility::ModTermPool* m_pExpandTermPool;
	bool m_bEssentialTerm;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END
#endif //__SYDNEY_FULLTEXT2_SEARCHCAPSULE_H

//
//  Copyright (c) 2010, 2013, 2023 Ricoh Company, Ltd.
//  All rights reserved.
//
