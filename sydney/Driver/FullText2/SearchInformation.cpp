// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SearchInformation.cpp --
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "FullText2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"
#include "FullText2/SearchInformation.h"

#include "FullText2/FileID.h"
#include "FullText2/GetDocumentFrequency.h"
#include "FullText2/GetDocumentFrequencyForEstimate.h"
#include "FullText2/OperatorTermNode.h"
#include "FullText2/Parameter.h"
#include "FullText2/ScoreCalculator.h"

#include "Exception/BadArgument.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace {

	//
	//	VARIABLE local
	//	_$$::_cEstimateLevel -- 見積りレベル
	//
	//	NOTES
	//
	ParameterInteger _cEstimateLevel("FullText2_EstimateLevel", 1);
	
}

//
//	FUNCTION public
//	FullText2::SearchInformation::SearchInformation -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::FileID& cFileID_
//		ファイルID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
SearchInformation::SearchInformation(FileID& cFileID_)
	: m_bTermMapOwner(true), m_pTermMap(0),
	  m_uiDocumentCount(0),
	  m_dblTotalDocumentLength(0),
	  m_dblAverageLength(0),
	  m_dblTotalDocumentFrequency(0),
	  m_bInitializedDocumentLength(false)
{
	m_pTermMap = new TermMap;

	// 高速化のためキャッシュする
	m_bNormalized = cFileID_.isNormalized();
	m_bNolocation = cFileID_.isNolocation();
	m_eIndexingType = cFileID_.getIndexingType();
	m_uiKeyCount = cFileID_.getKeyCount();
}

//
//	FUNCTION public
//	FullText2::SearchInformation::~SearchInformation -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
SearchInformation::~SearchInformation()
{
	if (m_bTermMapOwner) delete m_pTermMap;
}

//
//	FUNCTION public
//	FullText2::SearchInformation::SearchInformation -- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::SearchInformation& src_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
SearchInformation::SearchInformation(const SearchInformation& src_)
	: m_bTermMapOwner(false), m_pTermMap(src_.m_pTermMap),
	  m_uiDocumentCount(src_.m_uiDocumentCount),
	  m_dblTotalDocumentLength(src_.m_dblTotalDocumentLength),
	  m_dblAverageLength(src_.m_dblAverageLength),
	  m_dblTotalDocumentFrequency(src_.m_dblTotalDocumentFrequency),
	  m_bInitializedDocumentLength(src_.m_bInitializedDocumentLength),
	  m_bNormalized(src_.m_bNormalized),
	  m_bNolocation(src_.m_bNolocation),
	  m_eIndexingType(src_.m_eIndexingType),
	  m_uiKeyCount(src_.m_uiKeyCount)
{
	// コピーはOpenMPを使ってマルチスレッドで検索されるときにしか呼ばれない
	// 検索語ノードのマップをコピーするのは無駄なので、参照するのみ
}

//
//	FUNCTION public
//	FullText2::SearchInformation::getDocumentCount -- 全文書数を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		文書数
//
//	EXCEPTIONS
//
ModSize
SearchInformation::getDocumentCount()
{
	return (m_uiDocumentCount != 0) ?
		m_uiDocumentCount : getDocumentCountImpl();
}

//
//	FUNCTION public
//	FullText2::SearchInformation::getTotalDocumentLength -- 総文書長を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt64
//		総文書長
//
//	EXCEPTIONS
//
ModUInt64
SearchInformation::getTotalDocumentLength()
{
	ModUInt64 length = 0;
	
	if (m_uiDocumentCount != 0)
	{
		// double で計算する
		
		double ratio = static_cast<double>(m_uiDocumentCount)
			/ getDocumentCountImpl();
		length = static_cast<ModUInt64>(
			static_cast<double>(getTotalDocumentLengthImpl()) * ratio);
	}
	else
	{
		length = getTotalDocumentLengthImpl();
	}

	return length;
}

//
//	FUNCTION public
//	FullText2::SearchInformation::getEstimateLevel
//		-- 見積りレベルを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::SearchInformation::EstimateLevel::Value
//		見積レベル
//
//	EXCEPTIONS
//
SearchInformation::EstimateLevel::Value
SearchInformation::getEstimateLevel()
{
	SearchInformation::EstimateLevel::Value r = EstimateLevel::Level1;
	
	int level = _cEstimateLevel.get();
	switch (level)
	{
	case EstimateLevel::Level2:
		r = EstimateLevel::Level2;
		break;
	case EstimateLevel::Level3:
		r = EstimateLevel::Level3;
		break;
	default:
		;
	}

	return r;
}

//
//	FUNCTION public
//	FullText2::SearchInformation::getElement -- 各要素用の検索情報クラスを得る
//
//	NOTES
//
//	ARGUMENTS
//	int n_
//		要素番号
//
//	RETURN
//	FullText2::SearchInformation&
//		各要素用の検索情報クラス
//
//	EXCEPTIONS
//
SearchInformation&
SearchInformation::getElement(int n_)
{
	// 複合索引用のクラスは要素を返すが、この基底クラスでは自分自身を返す
	return *this;
}

//
//	FUNCTION public
//	FullText2::SearchInformation::isElementNull
//		-- 各要素用の検索情報クラスがnullかどうかを調べる
//
//	NOTES
//
//	ARGUMENTS
//	int n_
//		要素番号
//
//	RETURN
//	bool
//		各要素用の検索情報クラスがnullの場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
SearchInformation::isElementNull(int n_)
{
	// 複合索引用のクラスは要素を返すが、この基底クラスでは常にfalse
	return false;
}

//
//	FUNCTION public
//	FullText2::SearchInformation::getElementSize
//		-- 各要素用の検索情報クラスの数を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		検索情報クラスの数
//
//	EXCEPTIONS
//
ModSize
SearchInformation::getElementSize() const
{
	return 1;
}

//
//	FUNCTION public
//	FullText2::SearchInformation::getAllElement
//		-- すべての要素の検索情報クラスを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModVector<FullText2::SearchInformation*>&
//		すべての要素の検索情報クラス
//
//	EXCEPTIONS
//
ModVector<SearchInformation*>&
SearchInformation::getAllElement()
{
	_TRMEISTER_THROW0(Exception::BadArgument);
}

//
//	FUNCTION public
//	FullText2::SearchInformation::getTotalDocumentFrequency
//		-- スコア計算のための総文書数を得る
//
//	NOTES
//
//	ARGUMETNS
//	なし
//
//	RETURN
//	double
//		スコア計算のための総文書数
//
//	EXCEPTIONS
//
double
SearchInformation::getTotalDocumentFrequency()
{
	// 平均文書長などのデータを初期化する
	initializeDocumentLength();
	
	return m_dblTotalDocumentFrequency;
}

//
//	FUNCTION public
//	FullText2::SearchInformation::initializeScoreArgument
//		-- スコア計算に必要で、文書ごとに変化しないものの値を設定する
//
//	NOTES
//
//	ARGUMENTS
//	ModVector<FullText2::ScoreCalculator::Argument>& vecScoreArg_
//		スコア計算のための引数
//	FullText2::OperatorTermNode* node_
//		スコア計算を行う端末ノード
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SearchInformation::
initializeScoreArgument(ModVector<ScoreCalculator::Argument>& vecScoreArg_,
						OperatorTermNode* pTermNode_)
{
	// 平均文書長などのデータを初期化する
	initializeDocumentLength();
	
	TermMap::Iterator j = m_pTermMap->end();
		
	ModVector<ScoreCalculator::Argument>::Iterator i = vecScoreArg_.begin();
	for (; i != vecScoreArg_.end(); ++i)
	{
		switch ((*i).m_eType)
		{
		case ScoreCalculator::Argument::QueryTermFrequency:
			{
				// 検索文内頻度
				
				if (j == m_pTermMap->end())
				{
					j = m_pTermMap->find(pTermNode_->getString());
					if (j == m_pTermMap->end())
						// ありえない
						_TRMEISTER_THROW0(Exception::BadArgument);
				}

				(*i).m_dblValue = (*j).second.m_dblQueryTermFrequency;
			}
			break;
		case ScoreCalculator::Argument::DocumentFrequency:
			{
				// 文書頻度
				
				if (j == m_pTermMap->end())
				{
					j = m_pTermMap->find(pTermNode_->getString());
					if (j == m_pTermMap->end())
						// ありえない
						_TRMEISTER_THROW0(Exception::BadArgument);
				}

				(*i).m_dblValue = (*j).second.m_dblDocumentFrequency;
			}
			break;
		case ScoreCalculator::Argument::TotalDocumentLength:
			{
				// 総文書長
				
				(*i).m_dblValue = m_dblTotalDocumentLength;
			}
			break;
		case ScoreCalculator::Argument::TotalTermFrequency:
			{
				// 総文書内頻度
				
				if (j == m_pTermMap->end())
				{
					j = m_pTermMap->find(pTermNode_->getString());
					if (j == m_pTermMap->end())
						// ありえない
						_TRMEISTER_THROW0(Exception::BadArgument);
				}

				(*i).m_dblValue = (*j).second.m_dblTotalTermFrequency;
			}
			break;
		case ScoreCalculator::Argument::AverageDocumentLength:
			{
				// 平均文書長
				
				(*i).m_dblValue = m_dblAverageLength;
			}
			break;
		case ScoreCalculator::Argument::TotalDocumentFrequency:
			{
				// 総文書数
				
				(*i).m_dblValue = m_dblTotalDocumentFrequency;
			}
			break;
		}
	}
}

//
//	FUNCTION public
//	FullText2::SearchInformation::addTermNode
//		-- DF値を求める検索語ノードを追加する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& tea_
//		検索語ノードを表すtea構文
//	FullText2::OperatorTermNode* node_
//		検索語ノード
//
//	RETURN
//	FullText2::SearchInformation::MapValue*
//		エントリへのポインタ
//
//	EXCEPTIONS
//
SearchInformation::MapValue*
SearchInformation::addTermNode(const ModUnicodeString& tea_,
							   OperatorTermNode* node_)
{
	// 重複があったら、無視する
	ModPair<TermMap::Iterator, ModBoolean> r
		= m_pTermMap->insert(tea_, MapValue(node_));
	return &((*r.first).second);
}

//
//	FUNCTION public
//	FullText2::SearchInformation::setDocumentFrequency
//		-- DF値を設定する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& tea_
//		検索語ノードを表すtea構文
//	double dblDocumentFrequency_
//		文書頻度
//	double dblTotalTermFrequency_
//		総文書内頻度
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SearchInformation::setDocumentFrequency(const ModUnicodeString& tea_,
										double dblDocumentFrequency_,
										double dblTotalTermFrequency_)
{
	//【注意】	基底クラスの実装では複合カラムは考慮しない
	
	ModPair<TermMap::Iterator, ModBoolean> r
		= m_pTermMap->insert(tea_, MapValue(dblDocumentFrequency_,
											dblTotalTermFrequency_));
	if (r.second == ModFalse)
	{
		// すでに登録済みのエントリがあったので、
		// そのデータを書き換える

		(*r.first).second.m_dblDocumentFrequency = dblDocumentFrequency_;
		(*r.first).second.m_dblTotalTermFrequency = dblTotalTermFrequency_;
		(*r.first).second.m_bDone = true;
		(*r.first).second.m_pTermNode = 0;
	}
}

//
//	FUNCTION public
//	FullText2::SearchInformation::setDocumentFrequency
//		-- DF値を設定する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& tea_
//		検索語ノードを表すtea構文
// 	FullText2::OperatorTermNode::Frequency& frequency_
//		頻度情報
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SearchInformation::setDocumentFrequency(const ModUnicodeString& tea_,
										OperatorTermNode::Frequency& frequency_)
{
	//【注意】	基底クラスの実装では複合カラムは考慮しない
	
	double dblDocumentFrequency
		= static_cast<double>(frequency_.m_uiDocumentFrequency);
	double dblTotalTermFrequency
		= static_cast<double>(frequency_.m_ulTotalTermFrequency);

	SearchInformation::setDocumentFrequency(tea_,
											dblDocumentFrequency,
											dblTotalTermFrequency);
}

//
//	FUNCTION public
//	FullText2::SearchInformation::setQueryTermFrequency
//		-- 検索文内頻度を設定する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& tea_
//		検索語ノードを表すtea構文
//	double dblQueryTermFrequency_
//		検索文内頻度
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SearchInformation::setQueryTermFrequency(const ModUnicodeString& tea_,
										 double dblQueryTermFrequency_)
{
	//【注意】	基底クラスの実装では複合カラムは考慮しない
	//【注意】	すでにMapValueが存在していることを前提としている

	TermMap::Iterator i = m_pTermMap->find(tea_);
	if (i == m_pTermMap->end())
		// 想定外
		_TRMEISTER_THROW0(Exception::BadArgument);
	
	// ヒットしたので設定する

	(*i).second.m_dblQueryTermFrequency = dblQueryTermFrequency_;
}

//
//	FUNCTION public
//	FullText2::SearchInformation::findTermNode
//		-- 検索語ノードを検索する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& tea_
//		検索語ノードを表すtea構文
//
//	RETURN
//	FullText2::SearchInformation::MapValue*
//		エントリへのポインタ。見つからない場合は0を返す
//
//	EXCEPTIONS
//
SearchInformation::MapValue*
SearchInformation::findTermNode(const ModUnicodeString& tea_)
{
	MapValue* r = 0;
	TermMap::Iterator i = m_pTermMap->find(tea_);
	if (i != m_pTermMap->end())
	{
		// ヒットした

		r = &((*i).second);
	}

	return r;
}

//
//	FUNCTION public
//	FullText2::SearchInformation::setUpDocumentFrequency
//		-- 文書頻度と総文書内頻度を求める
//
//	NOTES
//
//	ARGUMENTS
//	bool bTotalTermFrequency_
//		総文書内頻度が必要な場合にtrueを設定する
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SearchInformation::setUpDocumentFrequency(bool bTotalTermFrequency_)
{
	if (getDocumentCount() != 0)
	{
		// OpenMPを用いて並列化する
		GetDocumentFrequency cExecute(*this,
									  bTotalTermFrequency_);
		cExecute.run();
	}
}

//
//	FUNCTION public
//	FullText2::SearchInformation::estimateDocumentFrequency
//		-- 文書頻度を見積もる
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SearchInformation::estimateDocumentFrequency()
{
	if (getDocumentCount() != 0)
	{
		// OpenMPを用いて並列化する
		GetDocumentFrequencyForEstimate cExecute(*this);
		cExecute.run();
	}
}

//
//	FUNCTION public
//	FullText2::SearchInformation::getTermNodeForDocumentFrequency
//		-- 文書頻度を求める対象の検索ノードを得る
//
//	NOTES
//
//	ARGUMENTS
//	ModVector<FullText2::OperatorTermNode*>& vecNode_
//		対象検索ノード
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SearchInformation::
getTermNodeForDocumentFrequency(ModVector<OperatorTermNode*>& vecNode_)
{
	vecNode_.clear();
	
	TermMap::Iterator i = m_pTermMap->begin();
	TermMap::Iterator e = m_pTermMap->end();
	for (; i != e; ++i)
	{
		if ((*i).second.m_bDone == true)
			continue;

		vecNode_.pushBack((*i).second.m_pTermNode);
	}
}

//
//	FUNCTION protected
//	FullText2::SearchInformation::initializeDocumentLength
//		-- スコア計算に必要な平均文書長などのデータを初期化する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SearchInformation::initializeDocumentLength()
{
	if (m_bInitializedDocumentLength == false)
	{
		if (m_dblTotalDocumentFrequency == 0)
		{
			m_dblTotalDocumentFrequency
				= static_cast<double>(getDocumentCount());
		}

		// 件数が0件なら調べる必要なし
		if (m_dblTotalDocumentFrequency != 0)
		{
			if (m_dblTotalDocumentLength == 0)
			{
				m_dblTotalDocumentLength
					= static_cast<double>(getTotalDocumentLength());
			}

			if (m_dblAverageLength == 0)
			{
				m_dblAverageLength
					= m_dblTotalDocumentLength / m_dblTotalDocumentFrequency;
			}
		}

		m_bInitializedDocumentLength = true;
	}
}

//
//	Copyright (c) 2010, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
