// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SearchCapsule.cpp --
// 
// Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Inverted";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Inverted/SearchCapsule.h"

#include "Inverted/OpenOption.h"
#include "Inverted/InvertedFile.h"

#include "Inverted/RowIDVectorFile.h"
#include "Inverted/RowIDVectorFile2.h"
#include "Inverted/Types.h"
#include "Inverted/Parameter.h"
#include "Inverted/DocumentIDVectorFile.h"
#include "Inverted/FileID.h"

#include "Common/DataArrayData.h"
#include "Common/StringData.h"
#include "Common/UnicodeString.h"
#include "FileCommon/OpenOption.h"
#include "Exception/NotSupported.h"

#include "ModInvertedFileCapsule.h"
#include "ModInvertedQuery.h"
#include "ModInvertedQueryParser.h"
#include "ModInvertedTokenizer.h"
#include "ModOsDriver.h"
#include "ModAlgorithm.h"
#include "ModTerm.h"
#include "ModTermElement.h"

_SYDNEY_USING
_SYDNEY_INVERTED_USING

namespace
{
	//
	//  VARIABLE local
	//
	const char* _TermResourceID = "@TERMRSCID";
	const char* _UnaResourceID = "@UNARSCID";

	//
	//  VARIABLE local
	//
	const ModUnicodeChar _EstimateDF[]
	= {'e','s','t','i','m','a','t','e','d','f',0};

	const ModUnicodeChar _AND[] = {'#','a','n','d','(',0};
	const ModUnicodeChar _OR[] = {'#','o','r','(',0};

	// ソートのための関数
	class _ScaleLess
	{
	public:
		ModBoolean operator() (const Common::WordData& x,
								 const Common::WordData& y)
			{ return (x.getScale() < y.getScale()) ? ModTrue : ModFalse; }
	};
	class _ScaleGreater
	{
	public:
		ModBoolean operator() (const Common::WordData& x,
								 const Common::WordData& y)
			{ return (x.getScale() > y.getScale()) ? ModTrue : ModFalse; }
	};
	class _DfLess
	{
	public:
		ModBoolean operator() (const Common::WordData& x,
								 const Common::WordData& y)
			{ return (x.getDocumentFrequency() < y.getDocumentFrequency())
					? ModTrue : ModFalse; }
	};
	class _DfGreater
	{
	public:
		ModBoolean operator() (const Common::WordData& x,
								 const Common::WordData& y)
			{ return (x.getDocumentFrequency() > y.getDocumentFrequency())
					? ModTrue : ModFalse; }
	};

#ifdef SYD_USE_UNA_V10
	ModUnicodeString _ModNlpNormRetStemDiv = "5";
	ModUnicodeString _ModNlpNormRetDiv = "6";
#endif
}

//
//  FUNCTION public
//  Inverted::SearchCapsule::SearchCapsule -- コンストラクタ
//
//  NOTES
//  コンストラクタ
//
//  ARGUMENTS
//  ModInvertedTokenizer* tokenizer_
//		トークナイザー
//  const LogicalFile::OpenOption& cOpenOption_
//		オープンオプション
//  Inverted::OptionDataFile* pOptionDataFile_
//		オプションデータを取得するファイル
//  VectorInvertedFile
//		転置ファイルクラス
//  RETURN
//  なし
//
//  EXCEPTIONS
//  なし
//

SearchCapsule::SearchCapsule(
	const LogicalFile::OpenOption& cOpenOption_,
	OptionDataFile* pOptionDataFile_,
	IndexFileSet *pIndexFileSet_,
	ModInvertedTokenizer *pTokenizer
 	):m_cOpenOption(cOpenOption_),m_pIndexFileSet(pIndexFileSet_),
	  m_cSearchResultSet(), m_pTermPool(0), m_pExpandTermPool(0),
	  m_bEssentialTerm(false)
{
	m_bCluster = cOpenOption_.getBoolean(_SYDNEY_OPEN_PARAMETER_KEY(
								   OpenOption::KeyID::Cluster));

	m_pOptionDataFile = pOptionDataFile_;
	m_iNormalLimit  = 0;
	m_iExpandLimit = m_cOpenOption.getExpandLimit();
	m_pTerm = 0;
	m_iConditionLength = 0;
	m_cResultType = 0;	// resultType初期化、0は、booleanと同等の扱いを受ける
	m_cstrCondition    = "";
	m_cCalculator	   = m_cOpenOption.getCalculator();
	m_cExtractor       = m_cOpenOption.getExtractor();
	// 検索条件の設定
	// [YET] FREETEXTの場合は？makeTermPoolで実行している。
	if(m_cOpenOption.getSearchType() == OpenOption::Type::Normal || 
		m_cOpenOption.getSearchType() == OpenOption::Type::Equal)
	{
		m_cstrCondition    = m_cOpenOption.getConditionString();
		m_iConditionLength = m_cstrCondition.getLength();
	}
	m_bDoSectionSearch = false;
	// 総文書数と平均文書長さを求める
	m_uiAverageLength		= m_cOpenOption.getAverageLength();
	m_uiDocumentFrequency	= m_cOpenOption.getDocumentFrequency();
	m_uiTotalDocumentLength = m_pIndexFileSet->getTotalDocumentLength();

	// 総文書数と平均文書長さがまだ計算されていない場合は
	// InvertedFileから計算する
	if(m_uiDocumentFrequency == 0)
		m_uiDocumentFrequency = (ModSize)m_pIndexFileSet->getCount();
	if(m_uiAverageLength == 0)
	{
		if(m_uiDocumentFrequency)
			m_uiAverageLength = (ModSize)m_pIndexFileSet
				->getTotalDocumentLength() / m_uiDocumentFrequency;
		else
			m_uiAverageLength = 0;
	}
	m_pIndexFileSet->setTokenizer(pTokenizer);
}

//
//  FUNCTION public
//  Inverted::SearchCapsule::~SearchCapsule -- デストラクタ
//
//  NOTES
//  デストラクタ
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//  なし
//
SearchCapsule::~SearchCapsule()
{
	delete m_pTerm;
	delete m_pTermPool;
	delete m_pExpandTermPool;
}

//
//  FUNCTION public
//  Inverted::SearchCapsule::execute -- 文書検索
//
//  NOTES
//
//  ARGUMENTS
//   limit_:SQL文でlimit句が指定された場合、その数値がセットされる
//   eSort_:SQL文でorder句が指定された場合、ソートの種類がセットされる
//   ResultSet& pSearchResult
//		最終検索結果格納場所
//		上位に返す構造をもつ(内部型ではない)
//
//  RETURN
//  const Inverted::SearchCapsule::ResultSet&
//	  検索結果集合
//
//  EXCEPTINS
//  なし
//
void
SearchCapsule::execute(ModSize limit,
					   SortParameter::Value eSort_,
					   ModSize &nTerm,
					   ResultSet &pSearchResult,
					   ClusterIDList& cClusterIDList_)
{
	nTerm = 0;
	m_cResultType = pSearchResult->getType();
	; _TRMEISTER_ASSERT(
		!(m_cResultType & (1 <<_SYDNEY::Inverted::FieldType::Internal)));
	m_uiLimit = limit;
	
	try
	{
		_retrieve = &SearchCapsule::__nonbooleanRetrieve;
		if (m_cOpenOption.getSearchType() == OpenOption::Type::Equal)
		{
			// 整合性検査時のequal検索
			ModUInt32 rowID	= m_cOpenOption.getRowID();
			ModVector<ModSize> vecSectionByteOffset
				= m_cOpenOption.getSectionByteOffset();
			ModVector<ModLanguageSet>vecSectionLanguage
				= m_cOpenOption.getLanguageSet();

			if (m_pIndexFileSet->contains(rowID) == true)
			{
				if (m_pIndexFileSet->check( m_cstrCondition,
										   rowID,
										   vecSectionLanguage,
										   vecSectionByteOffset) == true)
				{
					pSearchResult->pushBack(rowID);
				}
			}
		}
		else if (m_cOpenOption.getSearchType() == OpenOption::Type::Normal)
		{
			// 他の主要なデータベースでも行われている伝統的な条件式による
			// キーワード検索
#ifndef DEL_BOOL
			if(m_cResultType == (1 << _SYDNEY::Inverted::FieldType::Rowid))
				_retrieve = &SearchCapsule::__booleanRetrieve;
#endif
			search(limit,eSort_,pSearchResult);
		}
		else
		{
			// 自然言語処理、Sydney特有の検索
			search(nTerm,limit,eSort_,pSearchResult);
		}

	}
	catch (...)
	{
		detach();
		_SYDNEY_RETHROW;
	}

	detach();

	if (m_bCluster == true)
	{
		// クラスタリング検索の場合

		// スコア調整前にクラスタリングする。
		// 荒いクラスタリングは、文書内容が似ていればスコアも近い、
		// という仮定を使っているため。

		m_uiClusterCount = 0;
		m_uiClusteredSize = 0;
	 	SearchResultSet searchResultSet(m_pIndexFileSet, m_cResultType);
		// ソート後回し条件の場合、limit処理は行わない(limit=0にする)
		searchResultSet.getCluster(m_pOptionDataFile,
								   pSearchResult,
								   cClusterIDList_,
								   m_cOpenOption.getClusteredLimit(),
								   isDelayedSort(eSort_) ? 0 : limit,
								   m_uiClusterCount,
								   m_uiClusteredSize,
								   isPhasedClustering(eSort_));
	}
	
	if (m_pOptionDataFile->isModifierValue())
	{
		// スコアの調整
		modifyValue(pSearchResult);

		if (isDelayedSort(eSort_) == true)
		{
			// 後回しにしたソートとlimit処理を実行する。
			doDelayedSort(eSort_, pSearchResult, cClusterIDList_);
		}
	}
}

//
//  FUNCTION public
//  Inverted::SearchCapsule::execute -- 単語検索
//
//  NOTES
//
//  ARGUMENTS
//  ModSize limit_
//		SQL文でlimit句が指定された場合、その数値がセットされる
//  SortParameter eSort_
//		SQL文でorder句が指定された場合、ソートの種類がセットされる
//  WordSet* pWordSet
//		単語集合格納場所
//
//  RETURN
//
//  EXCEPTINS
//
void
SearchCapsule::execute(ModSize limit_,
					   SortParameter::Value eSort_,
					   WordSet * pWordSet)
{
	// 基本的に、(Sydne固有の)文書検索と処理は同じ。
	
	try
	{
		// 実行条件の確認
		// [NOTE] 検索条件は、OpenOption::parse()で確認し、
		//  取得列は、FullText::LogicalInterface::open()で確認しているが、
		//  その組み合わせについては、検索実行時(ここ)で確認する。
		if (m_cOpenOption.getSearchType() == OpenOption::Type::Normal)
		{
			// (Tea構文がある)通常の検索は未サポート
			_SYDNEY_THROW0(Exception::NotSupported);
		}

		bool isEssential;
		
		// 質問処理器を得る
		// [NOTE] 文書検索と異なり、初期検索語数の上限を指定できる。
		m_iNormalLimit = limit_;
		initializeTerm(m_cExtractor,
					   m_uiDocumentFrequency,
					   m_uiAverageLength);
		int matchMode = getTermMatchMode();
		// 検索結果一時格納領域
		// [NOTE] 文書検索と異なり、DFを取得できれば十分。
 		SearchResultSet searchResultSet(
			m_pIndexFileSet,
			1 << _SYDNEY::Inverted::FieldType::Rowid);
		// 初期検索語用ModTermPoolを作成
		ModAutoPointer<ModTermPool> pool = makeTermPool(isEssential);
		// 拡張検索語用ModTermPool
		ModAutoPointer<ModTermPool> pool2;

		// 全削除文書の取得
		retrieve("", searchResultSet, Inverted::Sign::DELETE_MASK);

		// 初期検索語のDFを取得
		// [NOTE] 文書検索と異なり、retrieveで取得しないため。
		ModTermPool::Iterator iter;
		for (iter = pool->begin(); iter != pool->end(); ++iter)
		{
			iter->setDf(
				getDocumentFrequency(
					m_bEstimateDF,
					iter->getFormula(matchMode, m_cCalculator, ModFalse),
					searchResultSet));
		}

		ModSize size = 0;
		if (m_vecDocument.getSize())
		{
			// 拡張検索語を取得
			pool2 = makeTermPoolFromExpandDocument(
				matchMode, searchResultSet, pool);
			// 拡張検索語数を取得
			size = pool2->getSize();
		}

		//
		// 検索語の設定
		//
		
		// 検索語の格納領域確保
		pWordSet->reserve(pool->getSize() + size);
		
		// 初期検索語の設定
		Common::WordData::Category::Value eCategory = 
			(m_cOpenOption.getSearchType() == OpenOption::Type::FreeText) ?
			Common::WordData::Category::Helpful :
			Common::WordData::Category::Undefined;
		for (iter = pool->begin(); iter != pool->end(); ++iter)
		{
			setWordData(pWordSet, *iter, eCategory);
		}

		if (m_vecDocument.getSize())
		{
			// 拡張検索語の設定
			for (iter = pool2->begin(); iter != pool2->end(); iter++)
			{
				setWordData(pWordSet, *iter,
							Common::WordData::Category::HelpfulRelated);
			}
		}
	}
	catch (...)
	{
		detach();
		_SYDNEY_RETHROW;
	}

	detach();

	// ソート
	sortWordSet(pWordSet, eSort_);
}

//
//  FUNCTION public
//  Inverted::SearchCapsule::pushExpandDocument -- 拡張文書を設定する
//
//  NOTES
//
//  ARGUMENTS
//  const ModUncodeString& cstrDocument_
//	  文書データ
//  const ModLanguageSet& cLanguage_
//	  言語データ
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
SearchCapsule::pushExpandDocument(const ModUnicodeString& cstrDocument_,
								 const ModLanguageSet& cLanguage_)
{
	m_vecDocument.pushBack(cstrDocument_);
	m_vecLanguage.pushBack(cLanguage_);
}

//
//  FUNCTION public
//  Inverted::SearchCapsule::getCondition
//	  -- tea構文形式の条件式を取得する
//  executeでモード指定に変更する
//
//  NOTES
//  SearchCapsule::execute 実行前にこのメソッドを実行しても、
//  得られる条件式は不定である。
//
//  ARGUMENTS
//  bool isGetCount_
//		件数取得のために条件を取得するかどうか
//
//  RETURN
//  const ModUnicodeString&
//	  tea構文形式の条件式
//
//  EXCEPTIONS
//
const ModUnicodeString&
SearchCapsule::getCondition(bool isGetEstimateCount_)
{
	if (m_cstrCondition.getLength() == 0)
	{
		// executeが実行されていないので、ここで実行する
		// [NOTE] Sydney固有の検索以外は、コンストラクタで設定済み
		; _TRMEISTER_ASSERT(
			m_cOpenOption.getSearchType() == OpenOption::Type::FreeText ||
			m_cOpenOption.getSearchType() == OpenOption::Type::WordList);
		
		if (isGetEstimateCount_ == true)
		{
			// 件数見積りために検索条件を取得する場合
			convertToTeaFormatForGetCount();
		}
		else
		{
			ResultSet resultSet = new ModInvertedSearchResult;
			try
			{
				ModSize nTerm;
				_retrieve = &SearchCapsule::__booleanRetrieve;
				search(nTerm,1,SortParameter::None,resultSet);
			}
			catch (...)
			{
				delete resultSet;
				detach();
				_SYDNEY_RETHROW;
			}
			delete resultSet;
			detach();
		}
	}
	return m_cstrCondition;
}

//
//	FUNCTION public
//	Inverted::SearchCapsule::getEstimateCount
//		-- ヒット件数の見積もりを行う
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModInt64
//		見積もった件数
//
//	EXCEPTIONS
//
ModSize
SearchCapsule::getEstimateCount()
{
	ModSize uiDF = 0;
	try
	{
		// tea構文形式の条件を得る
		const ModUnicodeString& condition = getCondition(true);
		if (condition.getLength() == 0)
		{
			// 条件がないので、0件とする
			detach();
			return uiDF;
		}
		
		// 件数見積もりを行う
			
		SearchResultSet searchResultSet(
			m_pIndexFileSet, 1 << _SYDNEY::Inverted::FieldType::Rowid);
			
		// [NOTE] 見積りは削除文書数を考慮しないので、
		//  searchResultSetに削除文書集合を設定しない。

		uiDF = getDocumentFrequency(true, condition, searchResultSet,
									true);
	}
	catch (...)
	{
		detach();
		_SYDNEY_RETHROW;
	}
	detach();
	
	return uiDF;
}

//
//  FUNCTION public
//  Inverted::SearchCapsule::getCluster -- クラスタIDリストの取得
//
//  NOTES
//	スコア調整なし、かつ、スコア降順の場合、クラスタリングを一度に
//	処理する必要はない。必要な時にその都度、取得する。
//
//  ARGUMENTS
//	ClusterIDList& cCluster_
//	ResultSet& pSearchResult_
//	ModSize uiPos_
//		クラスタIDを取得するデータの検索結果における位置(0-base)
//
//  RETURN
//	bool
//		クラスタIDリストを取得できた
//
//  EXCEPTIONS
//
bool
SearchCapsule::getCluster(ClusterIDList& cCluster_,
						  ResultSet& pSearchResult_,
						  ModSize uiPos_)
{
	; _TRMEISTER_ASSERT(m_bCluster == true);
	; _TRMEISTER_ASSERT(m_uiClusteredSize > 0);
	; _TRMEISTER_ASSERT(cCluster_.getSize() > 0);
	; _TRMEISTER_ASSERT(cCluster_.getSize() == pSearchResult_->getSize());
	; _TRMEISTER_ASSERT(cCluster_.getSize() >= m_uiClusteredSize);

	// [NOTE] 段階的クラスタリング時以外でも呼ばれる。
	// [NOTE] uiPos_ >= cCluster_.getSize() の場合、
	//  つまり、クラスタIDを取得するデータが検索結果に存在しない場合もある。
	
	bool result = true;
	
	// [NOTE] クラスタリング済みのクラスタIDを取得する場合は、
	//  SearchResultSetをコンストラクタせずに結果を返す。
	if (uiPos_ >= m_uiClusteredSize)
	{
		SearchResultSet searchResultSet(m_pIndexFileSet, m_cResultType);
		while (uiPos_ >= m_uiClusteredSize)
		{
			// 対象データが未クラスタリングの場合
			
			if (m_uiClusteredSize >= cCluster_.getSize())
			{
				// 全ての検索結果がクラスタリング済みの場合
				
				// ここに来る時は等しい時だけのはず。
				; _TRMEISTER_ASSERT(m_uiClusteredSize == cCluster_.getSize());
				
				// [NOTE] 対象データが、検索結果に存在しないケースは二通りある。
				//  1. 対象データの位置が、始めから検索結果サイズ以上だった場合
				//  2. 検索結果サイズがgetCluster()により小さくなった(※)ため、
				//   対象データの位置が、検索結果サイズ以上になってしまった場合
				//  ※ クラスタIDリストを追加取得した結果、
				//   出力するクラスタ数の上限に達すると、
				//   出力されないクラスタIDリストと検索結果は削除されるため。
				
				result = false;
				break;
			}
			
			searchResultSet.getCluster(
				m_pOptionDataFile,
				pSearchResult_,
				cCluster_,
				m_cOpenOption.getClusteredLimit(),
				m_uiLimit,
				m_uiClusterCount,
				m_uiClusteredSize,
				true);
		}
		
		// [NOTE] 再ソートは不要。
		//  段階的クラスタリングは、再ソートが不要な場合しか実行されないため。
		//  通常のクラスタリングは、全データのクラスタリングが終わっているので、
		//  ここで新たなクラスタIDが取得されることはないため。
	}
	return result;
}

//
//  FUNCTION public
//  Inverted::SearchCapsule::getSearchTermList -- 
//
//  NOTES
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
//	The type of the Lists are DataArrayData.
//
//  ARGUMENTS
//
//  RETURN
//
//  EXCEPTIONS
//
void
SearchCapsule::getSearchTermList(Common::DataArrayData& cSearchTermList_,
								 ModUInt32 uiResultType_)
{
	; _TRMEISTER_ASSERT(cSearchTermList_.getCount() == 0);

	if (m_cOpenOption.getSearchType() == OpenOption::Type::FreeText ||
		m_cOpenOption.getSearchType() == OpenOption::Type::WordList)
	{
		// prepareForSydneySearch()でm_pTermPoolを設定し、
		// そこから検索語リストを取得する
		
		initializeTerm(m_cExtractor,
					   m_uiDocumentFrequency,
					   m_uiAverageLength);
		try
		{
			prepareForSydneySearch(uiResultType_);
		}
		catch(...)
		{
			detach();
			_SYDNEY_RETHROW;
		}
		detach();

		getSearchTermListForSydneySearch(cSearchTermList_);
	}
	else
	{
		; _TRMEISTER_ASSERT(
			m_cOpenOption.getSearchType() == OpenOption::Type::Normal &&
			m_cstrCondition.getLength() > 0);

		// Parser::parse()で一旦Queryを生成し、そこから検索語リストを取得する
		
		// [YET] prepareForNormalSearch()を作ってQueryをキャッシュする？
		
		ModInvertedQuery cQuery;
		ModInvertedQueryParser cParser;
		cParser.parse(m_cstrCondition, cQuery);
		
		ModInvertedQuery::SearchTermList vecSearchTerm;
		cQuery.getSearchTermList(vecSearchTerm);
		
		getSearchTermListForNormalSearch(cSearchTermList_, vecSearchTerm);
	}
}

//
//  FUNCTION public
//  Inverted::RankingSearchCapsult::setSectionSearchFlag
//	  -- このカプセルの実行後にセクション検索を行うかどうかを設定する
//
//  NOTES
//
//  ARGUMENTS
//  bool flag_
//	  セクション検索を行う場合はtrue、それ以外の場合はfalseを設定する
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
SearchCapsule::setSectionSearchFlag(bool flag_)
{
	m_bDoSectionSearch = flag_;
}

//
//  FUNCTION public
//  Inverted::SearchCapsule::detach
//	  -- すべてのファイルのページをdetachする
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
SearchCapsule::detach()
{
	m_pIndexFileSet->detachAllPages();
}

//
//  FUNCTION private
//  Inverted::SearchCapsule::search -- 通常(非Sydney固有)の検索
//
//  NOTES
//
//  ARGUMENTS
//	ModSize limit_
//		SQL文でlimit句が指定された場合、その数値がセットされる
//  SortParameter::Value eSort_
//		SQL文でorder句が指定された場合、ソートの種類がセットされる
//  ResultSet& pSearchResult_
//		最終検索結果格納場所
//
//  RETURN
//
//  EXCEPTIONS
//
void
SearchCapsule::search(ModSize  limit_,
					  SortParameter::Value eSort_,
					  ResultSet &pSearchResult_)
{
	// [NOTE] 呼び出し側でdetach()すること
	
	// 実行条件の確認
	// [NOTE] 検索条件は、OpenOption::parse()で確認し、
	//  取得列は、FullText::LogicalInterface::open()で確認しているが、
	//  その組み合わせについては、検索実行時(ここ)で確認する。
	if(m_cResultType & (1 <<_SYDNEY::Inverted::FieldType::Tf))
	{
		// [NOTE] 伝統的な検索条件は検索語を取得できないので未サポート。
		//  参照 単語検索用execute()
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	// 検索結果一時格納領域
 	SearchResultSet searchResultSet(m_pIndexFileSet,m_cResultType);

	// 前削除文書の取得
	retrieve("",searchResultSet,Inverted::Sign::DELETE_MASK);

	// 検索
	retrieve(m_cstrCondition,searchResultSet,Inverted::Sign::INSERT_MASK);
	
	// クラスリング時は、composeでlimitによる打ち切り処理をしない
	// getCluster時の打ち切り処理を行う
	// またソートを後回しする条件の場合も打ち切り処理をしない
	// executeで打ち切り処理を行う
	searchResultSet.compose(m_pIndexFileSet,
							pSearchResult_,
							(m_bCluster || isDelayedSort(eSort_)) ? 0 : limit_,
							eSort_);
}

//
//  FUNCTION private
//  Inverted::SearchCapsule::search -- Sydney固有(FREETEXT,WORDLIST)の文書検索
//
//  NOTES
//
//  ARGUMENTS
//  ModSize& nTerm
//		検索文に含まれる検索語の数
//  ModSize limit_
//		SQL文でlimit句が指定された場合、その数値がセットされる
//  SortParameter::Value eSort_
//		SQL文でorder句が指定された場合、ソートの種類がセットされる
//  ResultSet& pSearchResult_
//		最終検索結果格納場所
//
//  RETURN
//
//  EXCEPTIONS
//
void
SearchCapsule::search(ModSize &nTerm,
					  ModSize limit_,
					  SortParameter::Value eSort_,
					  ResultSet &pSearchResult_)
{
	// [NOTE] 呼び出し側でdetach()すること
	
	// [NOTE] 拡張検索語ありの検索結果は、
	//  パラメータ補正された初期検索語による検索結果と、
	//  拡張検索語による検索結果とをマージして得られる。
	//  参考 libTerm仕様書
	
	if (m_bDoSectionSearch)
	{
		// [NOTE] getCondition()で見積もり用のTea構文が格納されているかも。
		m_cstrCondition.clear();
	}
	
	bool isEssential = false;
	
	// 質問処理器を得る
	initializeTerm(m_cExtractor,
				   m_uiDocumentFrequency,
				   m_uiAverageLength);
	// Sydney固有の検索のための準備
	prepareForSydneySearch(pSearchResult_->getType());
	int matchMode = getTermMatchMode();
	bool bcond = true;

	//
	// 検索
	//
	if (m_bEssentialTerm == true)
	{
		// 必須語ありの場合
		retrieveAND(matchMode, bcond, *m_pTermPool,m_cSearchResultSet);	// intersection
		// [NOTE] 初期検索語はTSVが0でも検索に使われる。
		//  拡張検索語ありの場合、初期検索語が0に補正される場合がある。
		//  通常は、TSVが0の検索語は検索に使う必要はないが、
		//  拡張検索語ありにした結果、
		//  今まで使われていた初期検索語が使われなくなり、
		//  さらに、今までヒットしていた文書がヒットしなくなるのは回避したい。
		retrieveADD(matchMode, bcond, *m_pTermPool, m_cSearchResultSet, true);// merge
		if (m_vecDocument.getSize())
		{
			// 拡張検索語の分もmerge
			retrieveADD(matchMode, bcond, *m_pExpandTermPool,
						m_cSearchResultSet);
		}
	}
	else
	{
		// 必須語なしの場合
		// [NOTE] 初期検索語はTSVが0でも検索に使われる。
		retrieve(matchMode,bcond,*m_pTermPool,m_cSearchResultSet, true);
		if (m_vecDocument.getSize())
		{
			// 拡張検索語の分もOR
			retrieve(matchMode, bcond, *m_pExpandTermPool, m_cSearchResultSet);
		}
	}

	//
	// section検索用の処理
	//

	if (m_bDoSectionSearch)
	{
		if (bcond == false)
			m_cstrCondition.append(Common::UnicodeChar::usRparent);
	}

	//
	// 単語数の更新
	//

	nTerm = m_pTermPool->getSize();

	//
	// 検索結果をまとめる
	//
	
	// クラスリング時は、composeでlimitによる打ち切り処理をしない
	// getCluster時の打ち切り処理を行う
	// またソートを後回しする条件の場合も打ち切り処理をしない
	// executeで打ち切り処理を行う
	m_cSearchResultSet.compose(
		m_pIndexFileSet,
		pSearchResult_,
		(m_bCluster || isDelayedSort(eSort_)) ? 0 : limit_,
		eSort_);

	//
	// 後処理
	//
	if (m_bEssentialTerm == true)
	{
		// TFを取得している場合、個々の結果内を単語番号順にソート
		pSearchResult_->sort(_SYDNEY::Inverted::SortParameter::None,
							 _SYDNEY::Inverted::SortParameter::TermNoAsc);
	}
	// [NOTE] m_bEssentialTermもクリアされるので、一番最後に実行する。
	clearForSydneySearch();
}

////////////////////////////////////////////////////////////////////////////
// 全件検索
// 検索条件がない場合は、
// 全件検索、必然的にbooleanになる
// 全件検索(特殊な検索、めったに使われない？)高速性が要求される
// ARGUMENT
//	searchResultSet:検索結果集合格納クラス
//	sig:転置ファイルのsignature
// RETURN
//  なし
void
SearchCapsule::search(IndexFileSet::Iterator iter,
					  ResultSet& pResultSet)
{
	ModSize docCount;
	if (docCount = ModSize(iter->getCount()))
	{
		pResultSet->reserve(static_cast<ModSize>(docCount));
		RowIDVectorFile* pVectorFile = iter->getRowIDVectorFile();
		if (pVectorFile)
		{
			RowIDVectorFile::Iterator i = pVectorFile->begin();
			for (; i != pVectorFile->end(); ++i)
			{
				pResultSet->pushBack((*i).key);
			}
		}
		else
		{
			RowIDVectorFile2* pVectorFile2 = iter->getRowIDVectorFile2();
			RowIDVectorFile2::Iterator i = pVectorFile2->begin();
			for (; i != pVectorFile2->end(); ++i)
			{
				pResultSet->pushBack((*i).key);
			}
		}
	}
}

//
//  FUNCTION private
//  Inverted::SearchCapsule::search --
//		全件検索のヘルパー関数(複数転置ファイル指定可)
//
//  NOTES
//
//  ARGUMENTS
//	SearchResultSet& searchResultSet
//		検索結果集合格納クラス
//	const ModUInt32 sig
//		転置ファイルのsignature
//
//  RETURN
//  ModSize
//
//  EXCEPTIONS
//
ModSize
SearchCapsule::search(SearchResultSet& searchResultSet,
					  const ModUInt32 sig)
{
	// [NOTE] sigにはマスク等を利用して複数転置ファイルを指定できる。
	//  参考 execute()の文書全件取得用
	
	ModSize n = 0;
	for (IndexFileSet::Iterator iter = m_pIndexFileSet->begin();
		 iter != m_pIndexFileSet->end(); ++iter)
	{
		// 各転置ファイルを順番に処理
		
		if (iter->signature() & sig)
		{
			// シグネチャの条件を満たした場合
			
			ModInvertedSearchResult* pResultSet
				= searchResultSet.get(iter->signature());
			if (pResultSet)
			{
				// 全件取得
				search(iter, pResultSet);
				n += pResultSet->getSize();
			}
		}
	}
	return n;
}

//
//  FUNCTION private
//  Inverted::SearchCapsule::getDocumentFrequency -- 検索語のDFの取得
//
//  NOTES
//
//  ARGUMENTS
// 	isEstimate_
//		文書頻度を求めるためのflag
//		true,falseに従い、DFの値を、大体の数値と正確な数値を切り替える
//	cstrCondition_
//		検索条件
//	searchResultSet
//		検索結果集合格納クラス
//	bool isGetCount_
//		getEstimateCount()からの呼び出しの場合、
//		vaidate結果によって、estimateをtrueに切り替える。
//
//  RETURN
//  ModSize
//		文書数
//
//  EXCEPTIONS
//
ModSize
SearchCapsule::getDocumentFrequency(bool isEstimate_,
									const ModUnicodeString& cstrCondition_,
									SearchResultSet& searchResultSet,
									bool isGetCount_)
{
	// [NOTE] 見積り(isEstimate_==true)の場合、多目の件数を返す。
	//  削除文書は考慮しない。
	//
	// 条件			見積り方法
	// ------------------------------
	// OR			総和
	// AND			最小値
	// AND-NOT		NOT条件を無視
	// AtomicOR		ORで代用(総和)
	// WindowBase	ANDで代用(最小値)
	
	// [NOTE] getEstimateCount()から呼ばれる場合は、
	//  searchResultSetに削除文書集合を設定していない。
	
	ModSize df = 0;
	ModInvertedQuery query;
	ModInvertedQueryParser cParser(m_cResultType);
	cParser.parse(cstrCondition_, query);
	
	for( IndexFileSet::Iterator iter = m_pIndexFileSet->begin();
				iter != m_pIndexFileSet->end();iter++)
	{
		// 各索引ファイルを順番に処理
		
		if(iter->signature() & Inverted::Sign::INSERT_MASK)
		{
			// 挿入索引の場合
			
			ModInvertedBooleanResult *expungedDocumentId = 0;
			if (isEstimate_ == false)
			{
				expungedDocumentId = searchResultSet.getExpungedDoc(
					m_pIndexFileSet,iter->signature());
			}
			ModInvertedDocumentID upperBoundDoc =
				iter->signature() == Inverted::Sign::_FullInvert ?
				iter->getLastDocumentID():
				ModInvertedUpperBoundDocumentID ;
			ModInvertedQuery _query = query;

			// [NOTE] iterが指すファイルにおける各索引語のDFと全文書数、
			//  ModInvertedQuery::totalDocumentFrequencyが設定される。
			//  このDFや全文書数は、削除文書を考慮しない。
			//  参考
			//   ModInvertedQuery::getSimpleTokenNode() でDFを取得
			//   InvertedFile::getDocumentFrequency() で全文書数を取得
			_query.validate(
				iter->getInvertedFile(),
				ModInvertedFileCapsuleBooleanSearch::defaultValidateMode);

			// getCountでなるべく正確な数値を高速に処理するための処理
			bool estimate = isEstimate_;
			if (estimate == false && isGetCount_ == true &&
				_query.isShortWord())
			{
				estimate = true;
			}

			//
			// DFの取得
			//
			df += _query.getDocumentFrequency((estimate) ? ModTrue : ModFalse,
											  expungedDocumentId,
											  upperBoundDoc);
		}
	}
	return df;
}

//
//  FUNCTION private
//  Inverted::SearchCapsule::retrieve -- 通常(非Sydney固有)の検索のロジック本体
//
//  NOTES
//
//  ARGUMENTS
//	const ModUnicodeString& cstrCondition_
//		検索条件
//	SearchResultSet& searchResultSet
//		検索結果集合格納クラス
//	ModUInt32 sig
//		転置ファイルのsignature
//  ModInt32 queryTermNo
//		検索語番号
//  ModUInt32 queryTermFrequency
//		検索語の検索文内における頻度
//
//  RETURN
//  ModSize
//		文書数
//
//  EXCEPTIONS
//
ModSize
SearchCapsule::retrieve(const ModUnicodeString& cstrCondition_,
						SearchResultSet &searchResultSet,
						const ModUInt32 sig,
						ModInt32 queryTermNo,
						ModUInt32 queryTermFrequency)
{
	if (cstrCondition_.getLength() == 0)
	{
		// 全件検索の場合
		
		// 通常、削除文書検索時に全文検索になる
		// この版では、削除文書の検索は必ず全件検索であることを前提とする。
		// それにより、コードが簡略化できる
		return search(searchResultSet,sig);
	}
	else if(sig & Inverted::Sign::INSERT_MASK)
	{
		// 挿入済み索引ファイルに対する検索の場合

		QueryList cQueryList;
		ModSize DF = 0;

		// 二次検索で利用。cQueryListの各要素と対応
		QueryNodeMapList vecQueryNodeMap;
		// 二次検索で利用
		TermMap cTermMap;

		// 一次検索
		retrieveFirstStep(cstrCondition_,
						  searchResultSet,
						  sig,
						  cQueryList,
						  vecQueryNodeMap,
						  cTermMap);
		
		// 二次検索
		retrieveSecondStep(searchResultSet,
						   cQueryList,
						   vecQueryNodeMap,
						   cTermMap,
						   DF,
						   queryTermFrequency);

		//
		// 最終検索結果を得る
		//
		ModInvertedSearchResult *result;
		for(QueryList::Iterator query_iter = cQueryList.begin();
			query_iter != cQueryList.end(); ++query_iter)
		{
			// 各Queryの検索結果を順番に処理

			// 検索結果の取得
#ifndef DEL_BOOL
			if(_retrieve == &SearchCapsule::__booleanRetrieve)		
			{
				// boolean
				query_iter->second->getBooleanResult((ModInvertedBooleanResult *&)result);
			}
			else
#endif
			{	// boolean以外
				query_iter->second->doSecondStepInRetrieveScore(result);
			}
			
			// 検索結果のOR
			
			// 同じ文書IDに関して、索引ファイル毎にまとめる。
			// 索引ファイル間については、SearchResultSet::composeを参照
			searchResultSet.add(query_iter->first,result,queryTermNo);

			// 後処理
			// [NOTE] retrieveFirstStep()でnewしている。
			delete query_iter->second;
		}
		return DF;
	}else
		return 0;	// temp
}

//
//  FUNCTION private
//  Inverted::SearchCapsule::retrieve -- Sydney固有検索のロジック本体(OR検索)
//
//  NOTES
//
//  ARGUMENTS
//  int matchMode
//	bool& bcond
//	ModTermPool& pool_
//		検索語pool
//	SearchResultSet& searchResultSet_
//		検索結果集合格納クラス
//	bool bIgnoreTsv_
//		選択値を無視するかどうか
//
//  RETURN
//
//  EXCEPTIONS
//
void
SearchCapsule::retrieve(int matchMode,
						bool& bcond,
						ModTermPool& pool_,
						SearchResultSet &searchResultSet_,
						bool bIgnoreTsv_)
{
	// [NOTE] 条件には、必須語が一つもないので、
	//  いずれかの検索語が含まれれば十分、つまりOR検索になる。
	
	for (ModTermPool::Iterator iter = pool_.begin(); iter != pool_.end(); ++iter)
	{
		// 各検索語を順番に処理
		
		if (iter->getTsv() || bIgnoreTsv_ == true)
		{
			ModUnicodeString condition = iter->getFormula(matchMode,m_cCalculator);

			//
			// 検索
			//

			// ModTermPoolに単語のDFも設定(SearchResultに設定するわけではない)
			iter->setDf(
				retrieve(condition,searchResultSet_,
						 Inverted::Sign::INSERT_MASK,
						 iter - pool_.begin(),
						 ModUInt32(iter->getTf())));

			//
			// セクション検索用の処理
			//
			if (m_bDoSectionSearch)
			{
				if (bcond == true)
				{// 検索語をＯＲでつなぐ
					m_cstrCondition.append(_OR);	// #or(
					bcond = false;
				}
				else
					m_cstrCondition.append(Common::UnicodeChar::usComma); // ,
				 m_cstrCondition.append(iter->getFormula(matchMode,
												m_cCalculator,
												ModFalse));
			}
		}
	}
}

//
//  FUNCTION private
//  Inverted::SearchCapsule::retrieveAND -- Sydney固有の検索のロジック本体(AND検索)
//
//  NOTES
//
//  ARGUMENTS
//  int matchMode
//		...
//	bool bcond_
//		...
//	ModTermPool& pool_
//		検索語pool
//	SearchResultSet& searchResultSet_
//		検索結果集合格納クラス
//
//  RETURN
//
//  EXCEPTIONS
//
void
SearchCapsule::retrieveAND(int matchMode,
						   bool& bcond_,
						   ModTermPool& pool_,
						   SearchResultSet &searchResultSet_)
{
	// [NOTE] 条件には、１つ以上の必須語(Essential属性の検索語)が含まれる。
	//  必須語は、必ず文書に含まれることを意味する。
	//  ここでは必須語部分だけを処理し、それらはAND条件になる。
	
	// x
	// [NOTE] xにデータは格納されない
	SearchResultSet searchResultSet(m_pIndexFileSet,m_cResultType);
	// y
	ModUInt32 resultType = m_cResultType;
	if(resultType & (1 <<_SYDNEY::Inverted::FieldType::Tf))
	{
		// TF は、list型なのでInternal属性を設定
		resultType |= (1 <<_SYDNEY::Inverted::FieldType::Internal);
	}
	SearchResultSet y(m_pIndexFileSet,resultType);
	// z
	// searchResultSet_
	
	// 削除済みデータを取得
	retrieve("",y,Inverted::Sign::DELETE_MASK);

	// 検索
	bool bfirst = true;
	for (ModTermPool::Iterator iter = pool_.begin(); iter != pool_.end(); ++iter)
	{
		// 各検索語を順番に処理
		
		if (iter->getType() == Common::WordData::Category::Essential ||
			iter->getType() == Common::WordData::Category::EssentialRelated)
		{
			// 必須語の場合
			
			ModUnicodeString condition = iter->getFormula(matchMode,m_cCalculator);

			//
			// 検索
			//
			
			// まずはzに初期値を設定し、
			// 二回目以降は、yに設定した後、yとzの積集合をzに設定し直す。
			// yが内部型の場合は、常にyに設定して、yとzの積集合をzに設定し直す。

			iter->setDf(
				retrieve(condition,
						 (bfirst == false ||
						  (resultType & (1 <<_SYDNEY::Inverted::FieldType::Internal))) ?
						 y :searchResultSet_,
						 Inverted::Sign::INSERT_MASK,
						 -1,
						 ModUInt32(iter->getTf())));

			// 積集合の生成
			if(bfirst == false || (resultType & (1 <<_SYDNEY::Inverted::FieldType::Internal)))
			{
				// itr - pool_.begin()は単語番号
				// y and z の結果を z に格納 (xは使われない)
				searchResultSet.intersection(
					y,searchResultSet_,iter - pool_.begin(), bfirst);
				// 挿入転置ファイルの中身だけをクリア
				y.clear(Inverted::Sign::INSERT_MASK);
			}

			//
			// セクション検索用の処理
			//
			if (m_bDoSectionSearch)
			{
				// 検索条件の追加
				
				if (m_cstrCondition.getLength() == m_iConditionLength)
				{
					// [NOTE] 個々のセクションの計算には、
					//  いずれかの条件を満たせばよいのでORでつなげば十分。
					m_cstrCondition.append(_OR);						// #or(
					// [NOTE] m_bEssentialTerm==true で、この関数が呼ばれても、
					//  必須語がpoolの中にあるとは限らないので、bcond_は必要。
					//  参照：WORDLIST用の SearchCapsule::makePool()
					bcond_ = false;
				}
				else
					m_cstrCondition.append(Common::UnicodeChar::usComma); // ,
				 m_cstrCondition.append(iter->getFormula(matchMode,
												m_cCalculator,
												ModFalse));
			}
			
			bfirst = false;
		}
	}
}

//
//  FUNCTION private
//  Inverted::SearchCapsule::retrieveADD
//		-- 必須語が指定された自然言語検索 (ADD検索)
//
//  NOTES
//	SQL文にて検索語にEssential属性をつけるとこの関数が呼ばれる
//
//  ARGUMENTS
//  int matchMode
//		
//	ModTermPool& pool_
//		検索語pool
//	SearchResultSet& searchResultSet_
//		検索結果集合格納クラス
//
//  RETURN
//
//  EXCEPTIONS
//
void
SearchCapsule::retrieveADD(int matchMode,
						   bool&bcond_,
						   ModTermPool& pool_,
						   SearchResultSet& searchResultSet_,
						   bool bIgnoreTsv_)
{
	// [NOTE] 条件には、１つ以上の必須語(Essential属性の検索語)が含まれる。
	//  ここではretrieveAND()で処理していない必須語以外の部分を処理する。
	//  ADD条件は、searchResultSet_に含まれる文書については、
	//  得られた検索結果を足し合わせるが、
	//  含まれない文書については無視する検索条件である。
	
	// x
	SearchResultSet searchResultSet(m_pIndexFileSet,m_cResultType);
	// y
	ModUInt32 resultType = m_cResultType;
	if(resultType & (1 <<_SYDNEY::Inverted::FieldType::Tf))
	{
		// TF は、list型なのでInternal属性を設定
		resultType |= (1 <<_SYDNEY::Inverted::FieldType::Internal);
	}
	SearchResultSet y(m_pIndexFileSet,resultType);
	// z
	// searchResultSet_
	
	// 削除済みデータを取得
	retrieve("",y,Inverted::Sign::DELETE_MASK);

	// 検索
	for (ModTermPool::Iterator iter = pool_.begin(); iter != pool_.end(); ++iter)
	{
		// 各検索語を順番に処理
		
		if (iter->getType() != Common::WordData::Category::Essential &&
			iter->getType() != Common::WordData::Category::EssentialRelated &&
			(iter->getTsv() != 0 || bIgnoreTsv_ == true))
		{
			ModUnicodeString condition = iter->getFormula(matchMode,m_cCalculator);
			
			//
			// 検索
			//

			// 検索結果を y に格納し、空でなければ z にマージする。
			
			iter->setDf(
				retrieve(condition,
						 y,
						 Inverted::Sign::INSERT_MASK,
						 -1,
						 ModUInt32(iter->getTf())));

			// マージ
			// 和集合と異なり、y のみに含まれる要素は z に含まれない。
			if(y.isEmpty() == false	)
			{
				searchResultSet.merge(y,searchResultSet_,iter - pool_.begin());
				// 挿入転置ファイルの中身だけをクリア
				y.clear(Inverted::Sign::INSERT_MASK);
			}

			//
			// セクション検索用の処理
			//
			if (m_bDoSectionSearch)
			{
				if(bcond_ == true){
					m_cstrCondition.append(_OR);	// #or(
					bcond_ = false;
				}else
					m_cstrCondition.append(Common::UnicodeChar::usComma); // ,	

				m_cstrCondition.append(iter->getFormula(matchMode,
												m_cCalculator,
												ModFalse));
			}
		}
	}
}

//
//  FUNCTION private
//  Inverted::SearchCapsule::__booleanRetrieve --
//		通常の検索のロジック本体の下請け(Boolean検索用)
//
//  NOTES
//
//  ARGUMENTS
//
//  RETURN
//
//  EXCEPTIONS
//
void
SearchCapsule::__booleanRetrieve(
	InvertedFile* invertedFile,
	const ModUnicodeString& cstrCondition_,
	ModInvertedQueryParser& cParser,	
	ModInvertedQuery* query,
	ModInvertedBooleanResult* expungedDocumentId,
	ModInvertedDocumentID upperBoundDoc)
{
	cParser.parse(cstrCondition_, *query);
	query->validate(invertedFile,
	// <- ModInvertedFileCapsuleSearchに変更
		ModInvertedFileCapsuleBooleanSearch::defaultValidateMode);
	query->retrieveBoolean(expungedDocumentId,
		ModInvertedFileCapsuleBooleanSearch::defaultEvaluateMode);
}

//
//  FUNCTION private
//  Inverted::SearchCapsule::__nonbooleanRetrieve --
//		通常の検索のロジック本体の下請け(非Boolean検索用)
//
//  NOTES
//
//  ARGUMENTS
//
//  RETURN
//
//  EXCEPTIONS
//
void
SearchCapsule::__nonbooleanRetrieve(
	InvertedFile* invertedFile,
	const ModUnicodeString& cstrCondition_,
	ModInvertedQueryParser& cParser,	
	ModInvertedQuery* query,
	ModInvertedBooleanResult* expungedDocumentId,
	ModInvertedDocumentID upperBoundDoc)
{

	cParser.parse(cstrCondition_, *query);
	if (m_cCalculator.getLength() != 0 &&
		query->getDefaultScoreCalculator() == 0)
		query->setDefaultScoreCalculator(m_cCalculator.getString());
	query->validate(invertedFile,
	// <- ModInvertedFileCapsuleSearchに変更
		ModInvertedFileCapsuleRankingSearch::defaultValidateMode,
		m_uiAverageLength);
	query->retrieve(expungedDocumentId,upperBoundDoc);					
}

//
//  FUNCTION private
//  Inverted::SearchCapsule::retrieveFirstStep
//		-- 一次検索(文書ID, TF等の取得)
//
//  NOTES
//
//  ARGUMENTS
//
//  RETURN
//
//  EXCEPTIONS
//
void
SearchCapsule::retrieveFirstStep(const ModUnicodeString& cstrCondition_,
								 SearchResultSet &searchResultSet,
								 ModUInt32 uiSignature_,
								 QueryList& cQueryList_,
								 QueryNodeMapList& vecQueryNodeMap_,
								 TermMap& cTermMap_)
{
	; _TRMEISTER_ASSERT(uiSignature_ & Inverted::Sign::INSERT_MASK);

	ModUInt32 resultType = m_cResultType;
	if(resultType & 1 <<_SYDNEY::Inverted::FieldType::Tf)
	{
		// [NOTE] TFと位置リストは、上位に返す時はlist型だが、
		//  転置リストから取得する際は、list型ではないので、
		//  Internal属性を設定する。
		resultType |= (1 <<_SYDNEY::Inverted::FieldType::Internal);
	}
	ModInvertedQueryParser cParser(resultType);
	QueryNodeMap querynodeMap;
		
	for(IndexFileSet::Iterator iter = m_pIndexFileSet->begin();
		iter != m_pIndexFileSet->end();iter++)
	{
		// 各索引ファイルを順番に処理する
			
		if( iter->getCount() && (iter->signature() & uiSignature_))
		{
			//
			// 条件を満たす文書IDを取得
			//
				
			ModAutoPointer<ModInvertedQuery> query = new ModInvertedQuery;
			
			// 検索結果から削除される削除済み文書
			ModInvertedBooleanResult *expungedDocumentId = 
				searchResultSet.getExpungedDoc(
					m_pIndexFileSet,iter->signature());
			ModInvertedDocumentID upperBoundDoc = 
				iter->signature() == Inverted::Sign::_FullInvert ? 
				iter->getLastDocumentID():ModInvertedUpperBoundDocumentID ;
			(this->*_retrieve)(
				iter->getInvertedFile(),cstrCondition_,cParser,query,
				expungedDocumentId,upperBoundDoc);

			//
			// 二次検索用の後処理
			//
				
			if(resultType & (1 <<_SYDNEY::Inverted::FieldType::Score))
			{
				// queryの検索語マップを、vecQueryNodeMap_に格納
				query->getTermNodes(querynodeMap);
				vecQueryNodeMap_.pushBack(querynodeMap);

				// queryの検索語を、cTermMap_に格納
				for (QueryNodeMap::Iterator iter = querynodeMap.begin();
					 iter != querynodeMap.end(); ++iter)
				{
					// (検索語,0)をpush
					cTermMap_.insert((*iter).first,0);
				}
			}

			// queryを、cQueryList_に格納
			// [NOTE] queryはオートポインタなのでreleaseしてから格納するが、
			//  呼び出し元のretrieve()で解放すること。
			cQueryList_.pushBack(
				QueryElement(iter->signature(),query.release()));
		}
	}
}

//
//  FUNCTION private
//  Inverted::SearchCapsule::retrieveSecondStep
//		-- 二次検索(DFの取得、スコア計算)
//
//  NOTES
//
//  ARGUMENTS
//
//  RETURN
//
//  EXCEPTIONS
//
void
SearchCapsule::retrieveSecondStep(SearchResultSet &searchResultSet_,
								  const QueryList& cQueryList_,
								  const QueryNodeMapList& vecQueryNodeMap_,
								  const TermMap& cTermMap_,
								  ModSize& uiDF_,
								  ModUInt32& uiQueryTermFrequency_)
{
	// retrieveFirstStepで検索結果にscore計算を含む場合に
	// クエリノードmapを取得したので、スコア計算が必要な場合に実行される。
	
	// コレクション内における検索語の総出現頻度
	ModUInt64 totalTermFrequency = 0;

	for (TermMap::ConstIterator iter = cTermMap_.begin();
		 iter  != cTermMap_.end(); iter++)
	{
		// 各検索語を順番に処理する
			
		// 検索語
		const ModUnicodeString& cstrKey = (*iter).first;
		// 当該検索語を含む文書数（DF値）
		ModSize df = 0;
		// この検索語を含むQueryNodeへのポインタのベクタ
		ModVector < ModInvertedQueryNode *> vecQueryNode;
		
		// DF値が必要かどうかを示すフラグ
		bool bNeed = false;
		bool bExtendedScore = false;
			
		// この検索語が含まれないQueryNodeMapに対応する索引ファイルの
		// シグネチャのベクタ
		SignatureList vecSignature;

		// cstrKeyのQueryNodeを取得
		getQueryNode(cstrKey, vecQueryNode, df, totalTermFrequency,
					 cQueryList_, vecQueryNodeMap_,
					 bNeed, bExtendedScore, vecSignature);

		// df,totalTermFrequencyの補正
		adjustDFAndTotalTF(cstrKey, searchResultSet_, df, totalTermFrequency,
						   bNeed, bExtendedScore, vecSignature);
		
		// dfが確定したので合計(DF)を更新
		uiDF_ += df;

		//
		// スコア計算
		//
			
		// 検索時のquery node をすべて検査する
		// 文書スコア計算を正しく行うために、DF値を正しく計算する。
		// そのために、各転置ファイルのクエリノード内の検索結果数を足して、
		// 検索語毎のDF値を計算する必要がある。TFについても同じ
			
		for(ModVector <ModInvertedQueryNode*>::Iterator 
				querynode_iter = vecQueryNode.begin();
			querynode_iter != vecQueryNode.end();
			querynode_iter++)
		{
			// 検索語を含むQueryNodeを順番に処理
				
			// second stepをここで計算する
			// 文書スコア計算は、firstStep(),secondStep()の順に計算する
			// 索引がwordの場合は、totalDocumentLengthはdocument collection中の全単語数を指す
				
			(*querynode_iter)->prepareScoreCalculatorEx(totalTermFrequency,
														m_uiTotalDocumentLength,
														uiQueryTermFrequency_);
			//実質的なsecondStep
			(*querynode_iter)->prepareScoreCalculator(m_uiDocumentFrequency,df);
		}
	}
}

//
//  FUNCTION private
//  Inverted::SearchCapsule::getQueryNode --
//
//  NOTES
//
//  ARGUMENTS
//
//  RETURN
//
//  EXCEPTIONS
//
void
SearchCapsule::getQueryNode(const ModUnicodeString& cstrKey_,
							ModVector<ModInvertedQueryNode*>& vecQueryNode_,
							ModSize& uiDF_,
							ModUInt64& ui64TotalTermFrequency_,
							const QueryList& cQueryList_,
							const QueryNodeMapList& vecQueryNodeMap_,
							bool& bNeed_,
							bool& bExtendedScore_,
							SignatureList& vecSignature_)
{
	const QueryNodeMapList::ConstIterator iteBegin = vecQueryNodeMap_.begin();
	const QueryNodeMapList::ConstIterator iteEnd = vecQueryNodeMap_.end();
	QueryNodeMapList::ConstIterator ite = iteBegin;
	for(; ite != iteEnd; ++ite)
	{
		// 索引ファイル毎に生成されたQueryNodeMapを順番に処理する

		// QueryNodeMapから、検索語のQueryNodeを取得
		QueryNodeMap::ConstIterator i = ite->find(cstrKey_);
		if(i != ite->end())
		{
			// 検索語のリーフノードが見つかった場合

			ModInvertedQueryNode* pQueryNode = (*i).second;

			// リーフノードを格納
			vecQueryNode_.pushBack(pQueryNode);

			// リーフノードの検索結果を取得
			ModInvertedSearchResult *result = pQueryNode->getRankingResult();
			// 検索語のDF値を得る
			uiDF_ += result->getSize();
			// コレクション内における検索語の総出現頻度
			ui64TotalTermFrequency_ += pQueryNode->getTotalTermFrequency();
					
			if(bNeed_ == false)
				bNeed_ = pQueryNode->needDocumentFrequency();
			if(bExtendedScore_ == false)
				bExtendedScore_ = pQueryNode->isExtendedScoreCalculator();
		}
		else
		{
			// 検索語のリーフノードが見つからなかった場合
					
			// この検索語は検索条件式のoptimize過程で削除されたもの
			// 検索語が見つからなかった転置のsignatureを再検索に備えて覚えておく
			ModUInt32 sig = cQueryList_.at(ite - iteBegin).first;
			vecSignature_.pushBack(sig);
		}
	}
}

//
//  FUNCTION private
//  Inverted::SearchCapsule::adjustDFAndTotalTF --
//
//  NOTES
//
//  ARGUMENTS
//
//  RETURN
//
//  EXCEPTIONS
//
void
SearchCapsule::adjustDFAndTotalTF(const ModUnicodeString& cstrKey_,
								  SearchResultSet& searchResultSet_,
								  ModSize& uiDF_,
								  ModUInt64& ui64TotalTermFrequency_,
								  bool bNeed_,
								  bool bExtendedScore_,
								  const SignatureList& vecSignature_)
{
	if(bNeed_ == false && bExtendedScore_ ==false)
		return;
	
	// optimize過程で削除された検索語があるので、もう一度検索して、
	// df値、必要に応じてtotalTermFrequencyを求める

	ModInvertedSearchResult *result;
	ModUInt32 resultType;
	if(bExtendedScore_ == true)
	{
		// 拡張モードのスコア計算には、totalTermFrequencyが必要
		// totalTermFrequency 取得は、全てのqueryNodeを取得し、
		// tfを計算する必要がある。
		resultType = (1 << _SYDNEY::Inverted::FieldType::Rowid) |
			(1 <<_SYDNEY::Inverted::FieldType::Tf) |
			(1 << _SYDNEY::Inverted::FieldType::Internal);
	}
	else
	{
		// DF だけ必要なのでboolean検索で十分
		resultType = (1 << _SYDNEY::Inverted::FieldType::Rowid);
	}
	ModInvertedQueryParser cParser(resultType);
	ModInvertedQuery query;

	for(SignatureList::ConstIterator sig_iter = vecSignature_.begin();
		sig_iter != vecSignature_.end(); ++sig_iter)
	{
		// getQueryNode()で省略されているQueryNodeを含むことがわかっている
		// 索引ファイルを順番に処理
		
		IndexFileSet::Iterator i = m_pIndexFileSet->find(*sig_iter);
		if(i != m_pIndexFileSet->end())
		{
			// シグネチャに対応する索引ファイルが存在した
			// [YET] 存在しないことはある？念のため？
			
			// 検索
			ModInvertedBooleanResult *expungedDocumentId = 
				searchResultSet_.getExpungedDoc(m_pIndexFileSet,*sig_iter);
			ModInvertedDocumentID upperBoundDoc = 
				i->signature() == Inverted::Sign::_FullInvert ? 
				i->getLastDocumentID() : ModInvertedUpperBoundDocumentID;
			(this->*_retrieve)(i->getInvertedFile(), cstrKey_, cParser, &query,
							   expungedDocumentId,	upperBoundDoc);

			// df, totalTermFrequency の補正
			result = query.getRoot()->getRankingResult();
			uiDF_ += result->getSize();
			if(bExtendedScore_ == true)
			{
				for(ModSize j = 0 ; j < result->getSize() ; j++)
				{
					ui64TotalTermFrequency_ += result->_getTF(j);
				}
			}
		}
	}
}

//
//  FUNCTION private
//  Inverted::SearchCapsule::isDelayedSort -- ソートを後回しにするか？
//
//  NOTES
//
//  ARGUMENTS
//  SortParameter::Value eSort_
//
//  RETURN
//  bool
//
//  EXCEPTIONS
//
bool
SearchCapsule::isDelayedSort(SortParameter::Value eSort_) const
{
	// [NOTE] m_pOptionDataFile == 0 の場合がある。
	//  FullText::DelayedIndexFile::verify を参照。
	
	bool result = false;

	if ((eSort_ == SortParameter::ScoreDesc ||
		 eSort_ == SortParameter::ScoreAsc) &&
		m_pOptionDataFile != 0 &&
		m_pOptionDataFile->isModifierValue() == true &&
		m_cOpenOption.getScoreMethod() != OpenOption::ScoreMethod::Unknown)
	{
		// スコアでソートし、スコア調整器を使う場合
		
		// スコア調整後にソートする必要があるので、
		// ソートを後回しにしなくてはならない。
		result = true;
	}
	return result;
}

//
//  FUNCTION private
//  Inverted::SearchCapsule::doDelayedSort -- 後回しにしたソートを実行する
//
//  NOTES
//
//  ARGUMENTS
//	ModSize uiLimit_
//  SortParameter::Value eSort_
//	ResultSet& pSearchResult_
//
//  RETURN
//
//  EXCEPTIONS
//
void
SearchCapsule::doDelayedSort(SortParameter::Value eSort_,
							 ResultSet &pSearchResult_,
							 ClusterIDList& cClusterIDList_)
{
	// [NOTE] m_uiClusteredSizeを修正するので、非const。
	//  limitの処理で出力されないクラスタIDや検索結果の削除にともない、
	//  クラスタリング済みの検索結果数も変わるため。
	
	// limit処理はソートの単位がタプルかクラスタかで異なるので、
	// 検索の種類によりuiLimit_の値が変わる可能性がある。
	ModSize limit = m_uiLimit;
	
	//
	// ソート
	//
	
	if (m_bCluster == true)
	{
		// クラスタリング検索の場合

		// [NOTE] 本来はソートの単位(タプル単位、クラスタ単位)と
		//  ソート順を考慮する必要があるが、
		//  現状は、常にクラスタ単位、かつ、降順で返す。
		SortParameter::Value eSort = SortParameter::ScoreDesc;

		// 全体をソートするための一時作業領域(クラスタの代表文書を格納)
		ModVector<ClusterElement> v;

		// クラスタ内のソート
		sortEachCluster(eSort, pSearchResult_, cClusterIDList_, v);

		// 全体のソート
		ModSize uiTupleCount = sortByCluster(eSort, pSearchResult_, v);
		
		// limit値の設定
		if (limit > 0 && v.getSize() > limit)
		{
			// limitの値は、現状、ソート単位が常にクラスタなので、
			// limit番目のクラスタの最後のタプルまでを残す。
			limit = uiTupleCount;
		}
		else
		{
			// 全てのクラスタを出力するのでlimit処理は不要
			limit = 0;
		}
	}
	else
	{
		// 非クラスタリング検索の場合
		
		pSearchResult_->sort(eSort_);
	}

	//
	// limit処理
	//
	
	if (limit > 0)
	{
		// limitが指定されている
		
		ModSize size = pSearchResult_->getSize();
		if (size > limit)
		{
			// limit件のデータを残したいので、
			// limit+1番目以降のデータを削除する。
			// 下記のeraseは、引数を0-baseの数値とみなして、
			// startからend-1までのデータを削除する。
			pSearchResult_->erase(limit, size);
			if (m_bCluster == true)
			{
				cClusterIDList_.erase(cClusterIDList_.begin() + limit,
									  cClusterIDList_.end());
				m_uiClusteredSize = limit;
			}
		}
	}

	// [NOTE] クラスタIDリスト数は、検索結果数と必ず等しい。
	; _TRMEISTER_ASSERT(
		m_bCluster == false ||
		(pSearchResult_->getSize() == cClusterIDList_.getSize() &&
		 pSearchResult_->getSize() == m_uiClusteredSize));
}

//
//  FUNCTION private
//  Inverted::SearchCapsule::modifyValue -- スコアを調整
//
//  NOTES
//
//  ARGUMENTS
//
//  RETURN
//
//  EXCEPTIONS
//
void
SearchCapsule::modifyValue(ResultSet pSearchResult_)
{
	; _TRMEISTER_ASSERT(m_pOptionDataFile->isModifierValue());

	// [NOTE] m_pOptionDataFileの非const関数を使うので、これも非const。
	
	double m;
	if (m_cOpenOption.getScoreMethod()
		== OpenOption::ScoreMethod::Sum)
	{
		for (ModSize i = 0; i < pSearchResult_->getSize();i++ )
		{
			if (m_pOptionDataFile->getModifierValue(
					pSearchResult_->getDocID(i), m))
				// 登録されているので、調整する
				pSearchResult_->setScore(i, pSearchResult_->getScore(i)+m);
		}
	}
	else if (m_cOpenOption.getScoreMethod()
			 == OpenOption::ScoreMethod::Multiply)
	{
		for (ModSize i = 0; i < pSearchResult_->getSize();i++ )
		{
			if (m_pOptionDataFile->getModifierValue(
					pSearchResult_->getDocID(i), m))
				// 登録されているので、調整する
				pSearchResult_->setScore(i, pSearchResult_->getScore(i)*m);
		}
	}
	else if (m_cOpenOption.getScoreMethod()
			 == OpenOption::ScoreMethod::Replace)
	{
		for (ModSize i = 0; i < pSearchResult_->getSize();i++ )
		{
			if (m_pOptionDataFile->getModifierValue(
					pSearchResult_->getDocID(i), m) == false)
				m = 0.0;
			// 置き換える
			pSearchResult_->setScore(i, m);
		}
	}
	else
	{
		// ScoreMethod追加時の処理漏れを防ぐため。
		;_TRMEISTER_ASSERT(m_cOpenOption.getScoreMethod()
						   == OpenOption::ScoreMethod::Unknown);
	}
		
	m_pOptionDataFile->detachAllPages();
}

//
//  FUNCTION private
//  Inverted::SearchCapsule::sortEachCluster -- 各クラスタ内をソートする
//
//  NOTES
//
//  ARGUMENTS
//  SortParameter::Value eSort_
//	ResultSet& pSearchResult_
//	ModVector<ClusterElement>& vecClusterElement_
//
//  RETURN
//
//  EXCEPTIONS
//
void
SearchCapsule::sortEachCluster(
	SortParameter::Value eSort_,
	ResultSet& pSearchResult_,
	ClusterIDList& cClusterIDList_,
	ModVector<ClusterElement>& vecClusterElement_) const
{
	// 現状、常にスコアの降順でソートされる
	; _TRMEISTER_ASSERT(eSort_ == SortParameter::ScoreDesc);
	
	ClusterIDList::ConstIterator i = cClusterIDList_.begin();
	const ClusterIDList::ConstIterator s = i;
	const ClusterIDList::ConstIterator e = cClusterIDList_.end();
	ClusterIDList::ConstIterator c = i;	// クラスタ開始位置
	// 2番目から調べる
	for (++i; i < e; ++i)
	{
		if (*i != *c)
		{
			// iは次のクラスタを指す

			// 前のクラスタ、cからiの直前までをソート
			pSearchResult_->sort(c - s, i - s, eSort_);
			// 前のクラスタのスコアの最大値と、クラスタの範囲を記録
			// 範囲は開始位置と、終了位置の次で表わされる。
			vecClusterElement_.pushBack(ClusterElement(
											pSearchResult_->getScore(c - s),
											ClusterRange(c - s, i - s)));
			// 次のクラスタの開始位置を更新
			c = i;
		}
	}
	// 最後のクラスタをソート
	pSearchResult_->sort(c - s, i - s, eSort_);
	// 最後のクラスタを記録
	vecClusterElement_.pushBack(ClusterElement(pSearchResult_->getScore(c - s),
											   ClusterRange(c - s, i - s)));
}

//
//  FUNCTION private
//  Inverted::SearchCapsule::sortByCluster -- クラスタ単位でソートする
//
//  NOTES
//
//  ARGUMENTS
//  SortParameter::Value eSort_
//	ResultSet& pSearchResult_
//	ModVector<ClusterElement>& vecClusterElement_
//
//  RETURN
//	ModSize
//
//  EXCEPTIONS
//
ModSize
SearchCapsule::sortByCluster(
	SortParameter::Value eSort_,
	ResultSet& pSearchResult_,
	ModVector<ClusterElement>& vecClusterElement_) const
{
	// 現状、常にスコアの降順でソートされる
	; _TRMEISTER_ASSERT(eSort_ == SortParameter::ScoreDesc);

	// クラスタの代表文書をソート
	// [NOTE] 常にスコアの降順でソートされる
	ModSort(vecClusterElement_.begin(), vecClusterElement_.end(),
			_ClusterElementGreater());
	
	// 全体をソートする前の状態をコピーする
	ModUInt32 uiResultType = pSearchResult_->getType();
	AutoPointer<ModInvertedSearchResult> pPrev
		= ModInvertedSearchResult::factory(uiResultType);
	pPrev->reserve(pSearchResult_->getSize());
	pPrev->copy(pSearchResult_);

	ModSize i = 0; // タプルの通し番号(出力するタプル数に等しい)
	
	ModVector<ClusterElement>::ConstIterator ite = vecClusterElement_.begin();
	// 現状、ソート単位は常にクラスタなので、
	// limit処理は出力するクラスタ数で制限される。
	ModVector<ClusterElement>::ConstIterator iteEnd =
		(m_uiLimit > 0 && vecClusterElement_.getSize() > m_uiLimit) ?
		ite + m_uiLimit : vecClusterElement_.end();
	for (; ite < iteEnd; ++ite)
	{
		// クラスタ毎に上書きする

		ModSize j = (*ite).second.first;	// クラスタの開始位置
		ModSize e = (*ite).second.second;	// クラスタの終了位置の次
		for (; j < e; ++j, ++i)
 		{
			pSearchResult_->setDocID(i, pPrev->getDocID(j));
			pSearchResult_->setScore(i, pPrev->getScore(j));
			if (uiResultType & (1 <<_SYDNEY::Inverted::FieldType::Tf))
			{
				pSearchResult_->setTF(i, *pPrev->getTF(j));
			}
		}
	}

	return i;
}

//
//  FUNCTION private
//  Inverted::SearchCapsule::isPhasedClustering
//		-- 段階的クラスタリングが可能か？
//
//  NOTES
//
//  ARGUMENTS
//
//  RETURN
//
//  EXCEPTIONS
//
bool
SearchCapsule::isPhasedClustering(SortParameter::Value eSort_) const
{
	// スコアの昇順でソートし、スコア調整列がないかスコア調整器を使わない場合、
	// 上位からN件ずつクラスタリングできる。
	
	// [NOTE] m_pOptionDataFile==0 の場合がある。
	//  (その状態では呼ばれないはずだが、念のためチェックしておく。)
	//  参照 FullText::DelayedIndexFile::verify()
	
	return
		m_pOptionDataFile != 0 &&
		(m_pOptionDataFile->isModifierValue() == false ||
		 m_cOpenOption.getScoreMethod() == OpenOption::ScoreMethod::Unknown) &&
		eSort_ == SortParameter::ScoreDesc;
}

//
//  FUNCTION private
//  Inverted::SearchCapsule::initializeTerm -- 質問処理器を得る
//
//  NOTES
//
//  ARGUMENTS
//  const ModUnicodeString& extractor_
//	  質問処理器パラメータ
//  ModSize collectionSize_
//	  登録文書数
//  ModSize averageLength_
//	  平均文書長
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
SearchCapsule::initializeTerm(const ModUnicodeString& extractor_,
							  ModSize collectionSize_,
							  ModSize averageLength_)
{
	if (m_pTerm == 0)
	{
		ModUnicodeString tmp(extractor_);
		if (extractor_.getLength() == 0)
		{
			// FileIDから得る
			tmp = m_pIndexFileSet->getFullInvert()->getExtractor();
		}
		m_bEstimateDF = (tmp.search(_EstimateDF, ModFalse) != 0);
		ModSize id = ModInvertedTokenizer::getResourceID(
			tmp.getString(),
			_TermResourceID);
		ModSize unaid = ModInvertedTokenizer::getResourceID(
			tmp.getString(),
			_UnaResourceID);
		if (unaid == 0)
			unaid = UndefinedResourceID;

		m_pTerm = m_pIndexFileSet->getFullInvert()->getLibTerm(id, unaid,
										collectionSize_, averageLength_);

		ModBoolean isNormalized
			= (m_pIndexFileSet->getFullInvert()->isNormalized() == true ) ? ModTrue : ModFalse;
		m_pTerm->useNormalizer1 = isNormalized;
		m_pTerm->useNormalizer2 = isNormalized;

#ifdef SYD_USE_UNA_V10
		if (m_pIndexFileSet->getFullInvert()->isStemming() == true)
			m_pTerm->termUnit1 = _ModNlpNormRetStemDiv;
		else
			m_pTerm->termUnit1 = _ModNlpNormRetDiv;
#else
		if (m_pIndexFileSet->getFullInvert()->isStemming() == true)
			m_pTerm->termUnit1 = ModNlpNormRetStemDiv;
		else
			m_pTerm->termUnit1 = ModNlpNormRetDiv;
#endif

		if (m_iNormalLimit != 0)
		{
			// select ... order by word(xxx).xxx xxx limit N のように指定された場合
			m_pTerm->maxTerm1 = m_iNormalLimit;
		}
		if (m_iExpandLimit != 0)
			// expand ( from(...) order by word(xxx).xxx limit N)
			// のように指定された場合
			m_pTerm->maxTerm2 = m_iExpandLimit;
	}
}

//
//  FUNCTION private
//  Inverted::SearchCapsule::makePool -- ModTermPoolを自然文から作成
//
//  NOTES
//
//  ARGUMENTS
//  const ModUnicodeString& freetext_
//	  自然文
//  const ModLanguageSet& lang_
//	  言語情報
//
//  RETURN
//  ModTermPool*
//	  作成されたプール
//
//  EXCEPTIONS
//
ModTermPool*
SearchCapsule::makePool(const ModUnicodeString& freetext_,
						const ModLanguageSet& lang_)
{
	ModTermPool* pool = new ModTermPool(m_pTerm->maxTerm1);
	m_pTerm->poolTerm(freetext_, lang_, *pool, 0);
	return pool;
}

//
//  FUNCTION private
//  Inverted::SearchCapsule::makePool -- ModTermPoolを単語リストから作成
//
//  NOTES
//
//  ARGUMENTS
//  const WordSetInternal& wordSet_
//	  単語セット
//  bool& isEssential_
//	  必須語が含まれるかどうか
//
//  RETURN
//  ModTermPool*
//	  作成されたプール
//
//  EXCEPTIONS
//
ModTermPool*
SearchCapsule::makePool(const WordSetInternal& wordSet_,
						 bool& isEssential_)
{
	isEssential_ = false;
	ModTermPool* pool = new ModTermPool(m_pTerm->maxTerm1);

	for (WordSetInternal::ConstIterator iter = wordSet_.begin(); iter != wordSet_.end(); ++iter)
	{
		if ((*iter).first.getCategory() == Common::WordData::Category::Essential ||
			(*iter).first.getCategory() == Common::WordData::Category::EssentialRelated)
		{
			// [NOTE] insertTerm()で、検索語候補が上限(maxTerm1)を超えた場合、
			//  必須語かどうかを考慮せずにスケール値で足切りするので、
			//  必ず必須語を含むとは限らない。
			isEssential_ = true;
		}

		ModTermElement element((*iter).first.getTerm(), (*iter).first.getCategory(), 1);
		element.setScale((*iter).first.getScale());
		element.setLangSpec((*iter).first.getLanguage());
		element.setMatchMode((*iter).second);

		pool->insertTerm(element);
	}
	m_pTerm->validatePool(*pool);
	return pool;
}

//
//  FUNCTION private
//  Inverted::SearchCapsule::makeTermPool --
//		ModTermPoolを自然文または単語リストから作成
//
//  NOTES
//
//  ARGUMENTS
//
//  RETURN
//
//  EXCEPTIONS
//
ModTermPool*
SearchCapsule::makeTermPool(bool &isEssential)
{
	ModTermPool *pool;

	if (m_cOpenOption.getSearchType() == OpenOption::Type::FreeText)
	{
		// 自然文から作成
		pool = makePool(m_cOpenOption.getConditionString(),
						m_cOpenOption.getFreeTextLanguage());
	}
	else
	{
		int size = m_cOpenOption.getWordCount();
		WordSetInternal cSearchWord;
		for (int i = 0; i < size; ++i)
		{
			Common::WordData::Category::Value category
				= m_cOpenOption.getWordCategory(i);
			if (category == Common::WordData::Category::Prohibitive ||
				category == Common::WordData::Category::ProhibitiveRelated)
			{
				// 禁止語、この版では何もしない
			}
			else
			{
				Common::WordData cWordData;
				cWordData.setTerm(m_cOpenOption.getWordPattern(i));
				cWordData.setLanguage(m_cOpenOption.getWordLanguage(i));
				cWordData.setCategory(category);
				cWordData.setScale(m_cOpenOption.getWordScale(i));
				cSearchWord.pushBack(
					ModPair<Common::WordData, int>(
						cWordData, m_cOpenOption.getWordMatchMode(i)));
			}
		}
		// 単語リストから作成
		pool = makePool(cSearchWord, isEssential);
	}
	return pool;
}

//
//  FUNCTION private
//  Inverted::SearchCapsule::makeTermPoolFromExpandDocument --
//		ModTermPoolを拡張文書から作成
//
//  NOTES
//
//  ARGUMENTS
//
//  RETURN
//
//  EXCEPTIONS
//
ModTermPool *
SearchCapsule::makeTermPoolFromExpandDocument(
	int matchMode,
	SearchResultSet &searchResultSet,
	ModTermPool *pool)
{
	ModTermMap termMap;
	// 拡張する
	ModVector<ModUnicodeString>::Iterator d = m_vecDocument.begin();
	ModVector<ModLanguageSet>::Iterator l = m_vecLanguage.begin();
	ModSize id = 0;
	for (; d != m_vecDocument.end(); ++d, ++l, ++id)
	{
		m_pTerm->mapTerm(*d, *l, id, termMap);
	}

	// 初期検索語の重みづけ等
	m_pTerm->weightTerm(termMap, *pool);

	// 拡張語の候補用プール
	ModTermPool cand(m_pTerm->maxCandidate);
	// 拡張語の候補をプール
	m_pTerm->poolTerm(termMap, cand);

	// 各候補にDFを設定
	for (ModTermPool::Iterator cand_iter = cand.begin();
		 cand_iter != cand.end();
		 ++cand_iter)
	{
		cand_iter->setDf(
			getDocumentFrequency(
				m_bEstimateDF,
				cand_iter->getFormula(matchMode, m_cCalculator, ModFalse),
				searchResultSet));
	}
	{
	// 拡張検索用プール
		ModAutoPointer<ModTermPool> pool2 = new ModTermPool(m_pTerm->maxTerm2);
	// 候補群から拡張検索語を選択
		m_pTerm->selectTerm(termMap, cand, *pool2);
		return pool2.release();
	}
}

//
//  FUNCTION private
//  Inverted::SearchCapsule::prepareForSydneySearch --
//
//  NOTES
//
//  ARGUMENTS
//
//  RETURN
//
//  EXCEPTIONS
//
void
SearchCapsule::prepareForSydneySearch(ModUInt32 uiResultType_)
{
	// [NOTE] FullText::LogicalInterfaceのgetProperty()とget()が
	//  連続して呼ばれることと対応して、
	//  getSearchTermList()とsearch()も連続して呼ばれる。
	//  同じ処理を繰り返さないために実行結果をキャッシュする。
	
	// [NOTE] getSearchTermList()時の検索結果型と、
	//  get()時の検索結果型は一致するはず。
	; _TRMEISTER_ASSERT(
		m_pTermPool == 0 ||
		(m_cSearchResultSet.getSize() > 0 &&
		 (m_cSearchResultSet[0].second == 0 ||
		  m_cSearchResultSet[0].second->getType()== uiResultType_)));

	// [NOTE] m_cSearchResultSet.getSize() > 0 は、
	//  prepare済みかどうかの判定には使えない。
	//  SearchResultSet::clear()はSearchResultSet自体をclearしないため。
	if (m_pTermPool == 0)
	{
		; _TRMEISTER_ASSERT(m_pExpandTermPool == 0 &&
							m_bEssentialTerm == false);
		
		// 検索結果一時格納領域
		if (m_cSearchResultSet.getSize() == 0)
		{
			m_cSearchResultSet.set(m_pIndexFileSet, uiResultType_);
		}
		// 初期検索語用ModTermPoolを作成
		m_pTermPool = makeTermPool(m_bEssentialTerm);
		// 全削除文書の取得
		// [NOTE] 拡張語ありの場合の初期検索語のDF取得前に実行する
		retrieve("", m_cSearchResultSet, Inverted::Sign::DELETE_MASK);

		if (m_vecDocument.getSize())
		{
			// 拡張検索語ありの場合

			// 初期検索語のDFを取得
			// [NOTE] 初期検索語のパラメータ補正に必要。
			int iMatchMode = getTermMatchMode();
			for (ModTermPool::Iterator i = m_pTermPool->begin();
				 i != m_pTermPool->end(); ++i)
			{
				i->setDf(getDocumentFrequency(
							 m_bEstimateDF,
							 i->getFormula(iMatchMode, m_cCalculator, ModFalse),
							 m_cSearchResultSet));
			}
			// 拡張検索語を取得
			// [NOTE] 同時に初期検索語のパラメータも補正される。
			m_pExpandTermPool = makeTermPoolFromExpandDocument(
				iMatchMode, m_cSearchResultSet, m_pTermPool);
		}
	}
}

//
//  FUNCTION private
//  Inverted::SearchCapsule::clearForSydneySearch --
//
//  NOTES
//
//  ARGUMENTS
//
//  RETURN
//
//  EXCEPTIONS
//
void
SearchCapsule::clearForSydneySearch()
{
	// [NOTE] FullText::LogicalInterfaceのget()は、
	//  途中でreset()が呼ばれることがある（と思われる）。
	//  その場合、prepareForSydneySearch()を再実行する必要があるので、
	//  search()の最後でキャッシュした実行結果をクリアしておく。
	
	m_cSearchResultSet.clear();
	delete m_pTermPool, m_pTermPool = 0;
	delete m_pExpandTermPool, m_pExpandTermPool = 0;
	m_bEssentialTerm = false;
}

//
//  FUNCTION private
//  Inverted::SearchCapsule::getTermMatchMode -- マッチモードを取得
//
//  NOTES
//
//  ARGUMENTS
//
//  RETURN
//	int
//		ModTermElement::***Match
//
//  EXCEPTIONS
//
int
SearchCapsule::getTermMatchMode()
{
	// マッチモードを決定する
	int matchMode;
	switch (m_pIndexFileSet->getIndexingType())
	{
	case ModInvertedFileDualIndexing:
		matchMode = ModTermElement::multiMatch;
		break;
	case ModInvertedFileWordIndexing:
		matchMode = ModTermElement::exactMatch;
		break;
	case ModInvertedFileNgramIndexing:
		matchMode = ModTermElement::stringMatch;
	}
	return matchMode;
}

//
//  FUNCTION private static
//  Inverted::SearchCapsule::convertTermMatchMode --
//
//  NOTES
//
//  ARGUMENTS
//
//  RETURN
//
//  EXCEPTIONS
//
Utility::SearchTermData::MatchMode::Value
SearchCapsule::convertTermMatchMode(ModInvertedTermMatchMode eTermMatchMode_)
{
	// [NOTE] eTermMatchMode_ is NOT a ModTermElement::***Match.
	
	using namespace Utility;
	
	SearchTermData::MatchMode::Value eMatchMode =
		SearchTermData::MatchMode::Unknown;
	
	switch (eTermMatchMode_)
	{
	case ModInvertedTermStringMode:
		eMatchMode = SearchTermData::MatchMode::String;
		break;
	case ModInvertedTermWordHead:
		eMatchMode = SearchTermData::MatchMode::WordHead;
		break;
	case ModInvertedTermWordTail:
		eMatchMode = SearchTermData::MatchMode::WordTail;
		break;
	case ModInvertedTermMultiLanguageMode:
		eMatchMode = SearchTermData::MatchMode::MultiLanguage;
		break;
	case ModInvertedTermSimpleWordMode:
		eMatchMode = SearchTermData::MatchMode::SimpleWord;
		break;
	case ModInvertedTermExactWordMode:
		eMatchMode = SearchTermData::MatchMode::ExactWord;
		break;
	default:
		; _TRMEISTER_ASSERT(false);
		break;
	}
	
	return eMatchMode;
}

//
//  FUNCTION private static
//  Inverted::SearchCapsule::convertTermMatchMode --
//
//  NOTES
//
//  ARGUMENTS
//
//  RETURN
//	int
//		
//
//  EXCEPTIONS
//
Utility::SearchTermData::MatchMode::Value
SearchCapsule::convertTermMatchMode(ModInvertedFileIndexingType eIndexingType_)
{
	using namespace Utility;
	
	SearchTermData::MatchMode::Value eMatchMode =
		SearchTermData::MatchMode::Unknown;
	
	switch (eIndexingType_)
	{
	case ModInvertedFileNgramIndexing:
		eMatchMode = SearchTermData::MatchMode::String;
		break;
	case ModInvertedFileDualIndexing:
		eMatchMode = SearchTermData::MatchMode::MultiLanguage;
		break;
	case ModInvertedFileWordIndexing:
		eMatchMode = SearchTermData::MatchMode::ExactWord;
		break;
	default:
		; _TRMEISTER_ASSERT(false);
		break;
	}
	
	return eMatchMode;
}

//
//  FUNCTION static private
//  Inverted::SearchCapsule::setWordData -- WordDataの設定
//
//  NOTES
//
//  ARGUMENTS
//
//  RETURN
//
//  EXCEPTIONS
//
void
SearchCapsule::setWordData(WordSet* pWordSet_,
						   const ModTermIndex& cTermIndex_,
						   Common::WordData::Category::Value eCategory_)
{
	Common::WordData cData(cTermIndex_.getOriginalString());
	cData.setLanguage(cTermIndex_.getLangSpec());
	cData.setCategory(
		(eCategory_ == Common::WordData::Category::Undefined) ?
		static_cast<Common::WordData::Category::Value>(cTermIndex_.getType()) :
		eCategory_);
	cData.setScale(cTermIndex_.getScale());
	cData.setDocumentFrequency(static_cast<int>(cTermIndex_.getDf()));
	pWordSet_->pushBack(cData);
}

//
//  FUNCTION static private
//  Inverted::SearchCapsule::sortWordSet -- WordSetをソート
//
//  NOTES
//
//  ARGUMENTS
//
//  RETURN
//
//  EXCEPTIONS
//
void
SearchCapsule::sortWordSet(WordSet* pWordSet_,
						   SortParameter::Value eSort_)
{
	// [NOTE] WordSetは選択値の降順になっている。
	
	switch (eSort_)
	{
	case SortParameter::ScaleAsc:
		ModSort(pWordSet_->begin(), pWordSet_->end(), _ScaleLess());
		break;
	case SortParameter::ScaleDesc:
		ModSort(pWordSet_->begin(), pWordSet_->end(), _ScaleGreater());
		break;
	case SortParameter::DfAsc:
		ModSort(pWordSet_->begin(), pWordSet_->end(), _DfLess());
		break;
	case SortParameter::DfDesc:
		ModSort(pWordSet_->begin(), pWordSet_->end(), _DfGreater());
		break;
	}
}

//
//  FUNCTION private
//  Inverted::SearchCapsule::convertToTeaFormatForGetCount --
//		件数見積もりのためにSydney固有の検索の条件をTea構文に変換
//
//  NOTES
//
//  ARGUMENTS
//
//  RETURN
//
//  EXCEPTIONS
//
void
SearchCapsule::convertToTeaFormatForGetCount()
{
	// [NOTE] Sydney固有の検索の条件をTea構文に変換する時のみ呼ばれる。
	; _TRMEISTER_ASSERT(
		m_cOpenOption.getSearchType() == OpenOption::Type::FreeText ||
		m_cOpenOption.getSearchType() == OpenOption::Type::WordList);
	// [NOTE] 条件は未設定。(Sydney非固有の場合、コンストラクタで設定される)
	; _TRMEISTER_ASSERT(m_cstrCondition.getLength() == 0);

	bool isEssential = false;

	// 質問処理器を得る
	initializeTerm(m_cExtractor,
				   m_uiDocumentFrequency,
				   m_uiAverageLength);
	// マッチモードを取得
	int matchMode = getTermMatchMode();
	// 初期検索語用ModTermPoolを作成
	ModAutoPointer<ModTermPool> pool = makeTermPool(isEssential);

	// [NOTE] 関連語拡張検索時は、拡張語も考慮した件数見積もりは不要。
	//  件数見積もりに、関連語を取得するための検索コストとシード文書挿入コストを
	//	かけられないので、シード文書挿入前に呼ばれるため。
	
	//
	// 変換
	//
	if (m_cOpenOption.getSearchType() == OpenOption::Type::FreeText)
	{
		// 条件が自然文の場合
		convertFreeTextToTeaFormat(pool, matchMode);
	}
	else
	{
		// 条件が単語リストの場合
		
		// [NOTE] 件数見積もりのためにTea構文に変換するので、等価変換ではない。
		//  必須語があれば、必須語以外を無視している。
		
		if (isEssential == true)
		{
			// 必須語ありの場合
			convertEssentialToTeaFormat(pool, matchMode);
		}
		if (m_cstrCondition.getLength() == 0)
		{
			// 必須語なし、または、必須語ありだが必須語が選択から漏れた場合
			convertNonEssentialToTeaFormat(pool, matchMode);
		}
	}
}

//
//  FUNCTION private
//  Inverted::SearchCapsule::convertFreeTextToTeaFormat --
//		Sydney固有の検索の自然文の条件をTea構文に変換
//
//  NOTES
//
//  ARGUMENTS
//
//  RETURN
//
//  EXCEPTIONS
//
void
SearchCapsule::convertFreeTextToTeaFormat(const ModTermPool* pPool_,
										  int iMatchMode_)
{
	// [YET] for内のわずかな処理以外は他のconvert*ToTeaFormatと同じ
 	
	ModUnicodeOstrStream cStream;
	ModSize uiCount = 0;
	for (ModTermPool::ConstIterator i = pPool_->begin(); i != pPool_->end(); ++i)
	{
		// 検索語をORで連結
		
		if (++uiCount == 1)
		{
			m_cstrCondition =
				i->getFormula(iMatchMode_, m_cCalculator, ModFalse);
		}
		else
		{
			if (uiCount == 2)
			{
				cStream << _OR << m_cstrCondition;
			}
			cStream	<< Common::UnicodeChar::usComma
					<< i->getFormula(iMatchMode_, m_cCalculator, ModFalse);
		}
	}
	if (uiCount >= 2)
	{
		// 検索語が2件以上だった場合
		// ')'で閉じる
		cStream << Common::UnicodeChar::usRparent;
		m_cstrCondition = cStream.getString();
	}
}

//
//  FUNCTION private
//  Inverted::SearchCapsule::convertEssentialToTeaFormat --
//		Sydney固有の検索の必須条件をTea構文に変換
//
//  NOTES
//
//  ARGUMENTS
//
//  RETURN
//
//  EXCEPTIONS
//
void
SearchCapsule::convertEssentialToTeaFormat(const ModTermPool* pPool_,
										   int iMatchMode_)
{
	// [YET] for内のわずかな処理以外は他のconvert*ToTeaFormatと同じ
	
	ModUnicodeOstrStream cStream;
	ModSize uiCount = 0;
	Common::WordData::Category::Value eCategory;
	for (ModTermPool::ConstIterator i = pPool_->begin();
		 i != pPool_->end(); ++i)
	{
		eCategory =
			static_cast<Common::WordData::Category::Value>(i->getType());
		if (eCategory == Common::WordData::Category::Essential ||
			eCategory == Common::WordData::Category::EssentialRelated)
		{
			if (++uiCount == 1)
			{
				m_cstrCondition =
					i->getFormula(iMatchMode_, m_cCalculator, ModFalse);
			}
			else
			{
				if (uiCount == 2)
				{
					cStream << _AND << m_cstrCondition;
				}
				cStream	<< Common::UnicodeChar::usComma
						<< i->getFormula(iMatchMode_, m_cCalculator, ModFalse);
			}
		}
	}
	if (uiCount >= 2)
	{
		// 必須語が2件以上だった場合
		// ')'で閉じる
		cStream << Common::UnicodeChar::usRparent;
		m_cstrCondition = cStream.getString();
	}
}

//
//  FUNCTION private
//  Inverted::SearchCapsule::convertNonEssentialToTeaFormat --
//		Sydney固有の検索の必須条件以外をTea構文に変換
//
//  NOTES
//
//  ARGUMENTS
//
//  RETURN
//
//  EXCEPTIONS
//
void
SearchCapsule::convertNonEssentialToTeaFormat(const ModTermPool* pPool_,
											  int iMatchMode_)
{
	// [YET] for内のわずかな処理以外は他のconvert*ToTeaFormatと同じ
	
	ModUnicodeOstrStream cStream;
	ModSize uiCount = 0;
	Common::WordData::Category::Value eCategory;
	for (ModTermPool::ConstIterator i = pPool_->begin();
		 i != pPool_->end(); ++i)
	{
		eCategory =
			static_cast<Common::WordData::Category::Value>(i->getType());
		if (eCategory != Common::WordData::Category::Essential &&
			eCategory != Common::WordData::Category::EssentialRelated)
		{
			if (++uiCount == 1)
			{
				m_cstrCondition =
					i->getFormula(iMatchMode_, m_cCalculator, ModFalse);
			}
			else
			{
				if (uiCount == 2)
				{
					cStream << _OR << m_cstrCondition;
				}
				cStream	<< Common::UnicodeChar::usComma
						<< i->getFormula(iMatchMode_, m_cCalculator, ModFalse);
			}
		}
	}
	if (uiCount >= 2)
	{
		// 検索語が2件以上だった場合
		// ')'で閉じる
		cStream << Common::UnicodeChar::usRparent;
		m_cstrCondition = cStream.getString();
	}
}

//
//  FUNCTION static private
//  Inverted::SearchCapsule::getSearchTermListForSydneySearch --
//
//  NOTES
//
//  ARGUMENTS
//
//  RETURN
//
//  EXCEPTIONS
//
void
SearchCapsule::getSearchTermListForSydneySearch(
	Common::DataArrayData& cSearchTermList_) const
{
	OpenOption::Type::Value eType = m_cOpenOption.getSearchType();
	; _TRMEISTER_ASSERT(eType == OpenOption::Type::WordList ||
						eType == OpenOption::Type::FreeText);

	Utility::SearchTermData::MatchMode::Value eMatchMode =
		convertTermMatchMode(m_pIndexFileSet->getIndexingType());
	
	ModTermPool::ConstIterator i = m_pTermPool->begin();
	const ModTermPool::ConstIterator e = m_pTermPool->end();
	for (; i != e; ++i)
	{
		if (eType == OpenOption::Type::WordList)
		{
			Common::WordData::Category::Value eCategory =
				static_cast<Common::WordData::Category::Value>(i->getType());
			if (eCategory == Common::WordData::Category::Prohibitive ||
				eCategory == Common::WordData::Category::ProhibitiveRelated)
			{
				continue;
			}
		}
		
		const ModTermElement& term = (*i).getTerm();
		Utility::SearchTermData* pSearchTermData =
			new Utility::SearchTermData(term.getString());
		pSearchTermData->setLanguage(term.getLangSpec());
		pSearchTermData->setMatchMode(eMatchMode);
				
		cSearchTermList_.pushBack(pSearchTermData);
	}
}

//
//  FUNCTION private static
//  Inverted::SearchCapsule::getSearchTermListForNormalSearch --
//
//  NOTES
//
//  ARGUMENTS
//
//  RETURN
//
//  EXCEPTIONS
//
void
SearchCapsule::getSearchTermListForNormalSearch(
	Common::DataArrayData& cSearchTermList_,
	const ModInvertedQuery::SearchTermList& vecSearchTerm_)
{
	ModSize uiPrevSynonymID = 0;
	Common::DataArrayData* pSynonymList = 0;
	
	ModInvertedQuery::SearchTermList::ConstIterator i =
		vecSearchTerm_.begin();
	const ModInvertedQuery::SearchTermList::ConstIterator e =
		vecSearchTerm_.end();
	for (; i != e; ++i)
	{
		Utility::SearchTermData* p =
			new Utility::SearchTermData((*i).m_cstrSearchTerm);
		p->setLanguage((*i).m_cLanguageSet);
		p->setMatchMode(convertTermMatchMode((*i).m_eMatchMode));
		
		ModSize uiSynonymID = (*i).m_uiSynonymID;
		if (uiSynonymID == 0)
		{
			// 同義語ではないので、直接格納する
			cSearchTermList_.pushBack(p);
		}
		else
		{
			// 同義語なので、同義語集合でまとめてから格納する
			if (uiPrevSynonymID != uiSynonymID)
			{
				// 初めて or 今までとは別の同義語集合
				if (pSynonymList != 0)
				{
					// 今までの同義語集合を格納する
					cSearchTermList_.pushBack(pSynonymList);
				}
				pSynonymList = new Common::DataArrayData();
			}
			; _TRMEISTER_ASSERT(pSynonymList != 0);
			pSynonymList->pushBack(p);
		}
		uiPrevSynonymID = uiSynonymID;
	}
	if (pSynonymList != 0)
	{
		cSearchTermList_.pushBack(pSynonymList);
	}
}

//
//	Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
