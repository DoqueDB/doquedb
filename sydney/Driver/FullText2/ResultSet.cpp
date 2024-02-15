// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ResultSet.cpp --
// 
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"
#include "FullText2/ResultSet.h"

#include "FullText2/IDVectorFile.h"
#include "FullText2/OperatorTermNode.h"
#include "FullText2/Parameter.h"
#include "FullText2/SearchInformation.h"
#include "FullText2/SimpleResultSet.h"

#include "Common/Assert.h"
#include "Common/BitSet.h"
#include "Common/DoubleData.h"
#include "Common/IntegerData.h"
#include "Common/Message.h"
#include "Common/UnsignedIntegerData.h"
#include "Common/WordData.h"

#include "Exception/BadArgument.h"

#include "Os/Limits.h"

#include "Utility/SearchTermData.h"

#include "ModAlgorithm.h"
#include "ModMap.h"
#include "ModVector.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
	//
	//	VARIABLE local
	//	_$$::_Neighbor -- クラスターのマージ対象数 (2以上の整数)
	//
	ParameterIntegerInRange _cNeighbor(
		"Inverted_MergeClusterDistance", 10, 2, Os::Limits<int>::getMax());
	
	//
	//	VARIABLE local
	//	_$$::_MaxRoughClusterCount -- 一度に取得するラフクラスタの最大個数
	//
	//	NOTE
	//	クラスタリングは、一度に取得できたラフクラスタ集合を対象に、
	//	詳細クラスタリングやそれらのマージをしている。
	//	そのため、その集合をまたいだクラスタリングはできないので、
	//	あまり小さくすると、局所的なクラスタリングとなり、
	//	まとまりが悪くなる可能性がある。
	//	だからと言って大きくしても、マージ対象数は Inverted_MergeClusterDistance
	//	で設定されるので、まとまりを良くする効果は限られている。
	//	
	//	小さくする利点は、段階的クラスタリングの一回分が少なくなり、
	//	取得クラスタ数の制限がある場合は速くなる可能性がある。
	//
	ParameterIntegerInRange _cMaxRoughClusterCount(
		"Inverted_MaxRoughClusterCount",  100, 2, Os::Limits<int>::getMax());
	
	//
	//	VARIABLE local
	//	_$$::_LocalClusteredLimit -- ラフクラスタ内の詳細クラスタリングの閾値
	//
	ParameterString _cLocalClusteredLimit("Inverted_LocalClusteredLimit", "");

	//
	//	VARIABLE local
	//	_$$::_UndefinedClusterID -- 不明なクラスタID
	//
	const ModSize _UndefinedClusterID = Os::Limits<ModSize>::getMax();

	//
	//	ソートのための less
	//
	class _ClusterElementIDLess
	{
	public:
		ModBoolean operator () (const ResultSet::ClusterElement& a1,
								const ResultSet::ClusterElement& a2)
			{ return (a1.id < a2.id) ? ModTrue : ModFalse; }
	};
	class _ClusterElementIDGreater
	{
	public:
		ModBoolean operator () (const ResultSet::ClusterElement& a1,
								const ResultSet::ClusterElement& a2)
			{ return (a1.id > a2.id) ? ModTrue : ModFalse; }
	};
	class _ClusterElementScoreLess
	{
	public:
		ModBoolean operator () (const ResultSet::ClusterElement& a1,
								const ResultSet::ClusterElement& a2)
			{ return (a1.score < a2.score) ? ModTrue : ModFalse; }
	};
	class _ClusterElementScoreGreater
	{
	public:
		ModBoolean operator () (const ResultSet::ClusterElement& a1,
								const ResultSet::ClusterElement& a2)
			{ return (a1.score > a2.score) ? ModTrue : ModFalse; }
	};

	//
	//	KWIC位置を調整する係数を求める
	//
	double _getAdjustFactor(DocumentID id_, SearchInformation& cSearchInfo_,
							ModSize& uiOrigLength_)
	{
		double factor = 1;

		if (cSearchInfo_.isOriginalLength())
		{
			// 荒いKWICに関する情報がある場合
		
			// 索引に格納されている索引語の位置は、
			// 正規化後の文字位置や単語位置の場合がある。
			// 一方、上位から得られるKWICサイズや、
			// 上位に返す荒いKWICの開始位置などは、
			// 正規化前の文字列または文字位置が基準になっているので、
			// これらを変換する必要がある。

			// 単語索引の場合
			//  係数 = 文書内の総単語数 / 正規化前文字列長

			// 正規化索引の場合
			//  係数 = 正規化後の文字列長 / 正規化前文字列長
		
			// 単語数or正規化後文字列長
			ModSize uiLength = 0;
			cSearchInfo_.getDocumentLength(id_, uiLength);
			
			// 正規化前文字列長
			cSearchInfo_.getOriginalLength(id_, uiOrigLength_);
		
			// 係数の計算
			factor = static_cast<double>(uiLength) / uiOrigLength_;
			
		}
		else
		{
			// 正規化されていないので、文書長

			cSearchInfo_.getDocumentLength(id_, uiOrigLength_);
		}

		return factor;
	}
}

//
//	FUNCTION public
//	FullText2::ResultSet::ResultSet -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const ModVector<ResultSet::FieldData>& projections_
//		取得するデータ
//	FullText2::SearchInformation& cSearchInfo_
//		検索情報クラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
ResultSet::ResultSet(const ModVector<FieldData>& projections_,
					 SearchInformation& cSearchInfo_,
					 bool bGetByBitSet_)
	: m_vecProjections(projections_),
	  m_uiCurrentID(UndefinedDocumentID),
	  m_uiCurrentClusterID(_UndefinedClusterID),
	  m_uiClusteredSize(0), m_fGlobalClusteredLimit(0),
	  m_fLocalClusteredLimit(0), m_uiMaxRoughClusterCount(0),
	  m_uiNeighbor(0), m_bCluster(false), m_cSearchInfo(cSearchInfo_),
	  m_bGetByBitSet(bGetByBitSet_),
	  m_uiLimit(Os::Limits<ModSize>::getMax())
{
	// キーの数
	ModSize s = cSearchInfo_.getKeyCount();
	
	// 最大要素分の領域を確保する
	m_vecKwicSize.assign(s, 0);
	m_vecKwicMarginScaleFactor.assign(s, 0);
	m_vecMapTermLeafNode.assign(s);

	// 空の集合を設定
	m_ite = m_begin = m_end = m_vecDocIDScore.end();
}

//
//	FUNCTION public
//	FullText2::ResultSet::~ResultSet -- デストラクタ
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
ResultSet::~ResultSet()
{
	// 位置情報取得のためのノードを解放する
	ModVector<Query::TermNodeMap>::Iterator k = m_vecMapTermLeafNode.begin();
	for (; k != m_vecMapTermLeafNode.end(); ++k)
	{
		Query::TermNodeMap& mapTermLeafNode = *k;
		Query::TermNodeMap::Iterator j = mapTermLeafNode.begin();
		for (; j != mapTermLeafNode.end(); ++j)
		{
			delete (*j).second;
		}
	}
}

//
//	FUNCTION public
//	FullText2::ResultSet::setResultSet
//		-- 検索結果を設定する
//
//	NOTES
//
//	ARGUMETNS
//	const FullText2::SimpleResultSet& result_
//		検索結果
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ResultSet::setResultSet(const SimpleResultSet& result_)
{
	m_vecDocIDScore = result_;
	
	// カーソルを設定する
	m_begin = m_vecDocIDScore.begin();
	m_ite = m_begin;
	m_end = m_vecDocIDScore.end();
}

//
//	FUNCTION public
//	FullText2::ResultSet::setTermLeafNode
//		-- 位置情報取得のための検索語ノードを設定する
//
//	NOTES
//
//	ARGUMENTS
//	int n_
//		要素番号
//	FullText2::Query::TermNodeMap& mapTermLeafNode_
//		検索語ノードのマップ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ResultSet::setTermLeafNode(int n_, Query::TermNodeMap& mapTermLeafNode_)
{
	m_vecMapTermLeafNode[n_] = mapTermLeafNode_;
}

//
//	FUNCTION public
//	FullText2::ResultSet::getSearchTerm
//		-- KWICのための検索語リストを得る
//
//	NOTES
//
//	ARGUMENTS
//	int n_
//		要素番号
//	Common::DataArrayData& cTermList_
//		検索語リストを格納する配列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ResultSet::getSearchTerm(int n_, Common::DataArrayData& cTermList_)
{
	Query::TermNodeMap& m = m_vecMapTermLeafNode[n_];
	
	cTermList_.clear();
	cTermList_.reserve(m.getSize());

	Query::TermNodeMap::Iterator i = m.begin();
	for (; i != m.end(); ++i)
	{
		if ((*i).first.getSize() == 0)
			// 念のため
			continue;
		
		Common::Data::Pointer p;
		
		if ((*i).first.getSize() > 1)
		{
			// 配列
			Common::DataArrayData* pArray = new Common::DataArrayData;
			p = pArray;
			Query::TermValue::Iterator j = (*i).first.begin();
			for (; j != (*i).first.end(); ++j)
			{
				pArray->pushBack(new Utility::SearchTermData(*j));
			}
		}
		else
		{
			p = new Utility::SearchTermData(*(*i).first.begin());
		}
		
		cTermList_.pushBack(p);
	}
}

//
//	FUNCTION public
//	FullText2::ResultSet::setKwicParameter
//		-- 粗いKWIC情報取得のためのパラメータを設定する
//
//	NOTES
//
//	ARGUMENTS
//	int n_
//		要素番号
//	ModSize uiKwicSize_
//		粗いKWICのための領域サイズ
//	ModSize uiKwicMarginScaleFactor_
//		KWICで返す長さを入力値の何倍にするか
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ResultSet::setKwicParameter(int n_,
							ModSize uiKwicSize_,
							ModSize uiKwicMarginScaleFactor_)
{
	m_vecKwicSize[n_] = uiKwicSize_;
	m_vecKwicMarginScaleFactor[n_] = uiKwicMarginScaleFactor_;
}

//
//	FUNCTION public
//	FullText2::ResultSet::setClusterParameter
//		-- クラスタリングのためのパラメータを初期化する
//
//	NOTES
//
//	ARGUMENTS
//	float fGlobalClusteredLimit_
//		詳細クラスタリングの閾値
//	FullText2::ResultSet::ClusterCombiner::Value
//		クラスタ合成方法
//	const ModVector<ModPair<int, float> >& vecScale_
//		クラスタ対象フィールドの重み
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ResultSet::setClusterParameter(float fGlobalClusteredLimit_,
							   ClusterCombiner::Value eCombiner_,
							   const ModVector<ModPair<int, float> >& vecScale_)
{
	// クラスタリング済みの検索結果数
	m_uiClusteredSize = 0;

	// 詳細クラスタリングの閾値
	m_fGlobalClusteredLimit = fGlobalClusteredLimit_;

	// クラスタ合成方法
	m_eClusterCombiner = eCombiner_;
	// フィールドの重み
	m_vecClusterScale = vecScale_;
	
	// ラフクラスタ内の詳細クラスタリングの閾値
	//
	// [NOTE] キーなし、キーありで値が空文字列、値が0の場合、
	//  1.0とfGlobalClusteredLimit_の真ん中、それ以外は設定されている値を使う。
	//
	m_fLocalClusteredLimit
		= ModUnicodeCharTrait::toFloat(_cLocalClusteredLimit.get());
	if (m_fLocalClusteredLimit == 0)
	{
		m_fLocalClusteredLimit = (1 + m_fGlobalClusteredLimit) / 2;
	}
	
	// 一度に取得するラフクラスタの最大個数
	m_uiMaxRoughClusterCount = _cMaxRoughClusterCount.get();
	
	// 詳細クラスタのマージ対象数
	m_uiNeighbor = _cNeighbor.get();

	// クラスタを実施する
	m_bCluster = true;
}

//
//	FUNCTION public
//	FullText2::ResultSet::adjustScore
//		-- スコア調整カラムのスコアでスコアを調整する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::AdjustMehod::Value eMethod_
//		調整方法
//	FullText2::SortKey::Value eKey_
//		ソートキー
//	FullText2::Order::Value eOrder_
//		昇順 or 降順
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ResultSet::adjustScore(AdjustMethod::Value eMethod_,
					   SortKey::Value eKey_, Order::Value eOrder_)
{
	if (m_bCluster)
	{
		// 検索結果クラスタリングを実施するので、
		// まずは全件をクラスタリングする

		while (clustering() == true);
	}


	if (eMethod_ != AdjustMethod::Unknown)
	{
		// スコア調整が必要
		
		SimpleResultSet::Iterator i = m_vecDocIDScore.begin();
		SimpleResultSet::Iterator e = m_vecDocIDScore.end();

		for (; i < e; ++i)
		{
			// スコア調整値を得る

			double score = 0;
			m_cSearchInfo.getScoreValue((*i).first, score);

			// スコア値を変更する
		
			switch (eMethod_)
			{
			case AdjustMethod::Multiply:
				(*i).second *= score;
				break;
			case AdjustMethod::Add:
				(*i).second += score;
				break;
			case AdjustMethod::Replace:
				(*i).second = score;
				break;
			}
		}
	}

	// スコアの降順にソートする

	if (m_bCluster)
	{
		// クラスタ情報を格納する構造体の配列
		Common::LargeVector<ClusterElement> v;

		// クラスタ内をソート
		sortEachCluster(eKey_, eOrder_, v);
		// クラスタ単位でソート
		sortByCluster(eKey_, eOrder_, v);
	}
	else
	{
		// そのままソート
		m_vecDocIDScore.sort(eKey_, eOrder_);
	}

	// カーソルを設定し直す
	m_ite = m_begin = m_vecDocIDScore.begin();
	m_end = m_vecDocIDScore.end();
}

//
//	FUNCTION public
//	FullText2::ResultSet::clear -- 結果集合をクリアする
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
ResultSet::clear()
{
	m_vecDocIDScore.clear();
	m_vecClusterID.clear();
}

//
//	FUNCTION public
//	FullText2::ResultSet::reset -- カーソルを先頭に戻す
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
ResultSet::reset()
{
	m_ite = m_begin;
	m_uiCurrentID = UndefinedDocumentID;
	m_uiCurrentClusterID = _UndefinedClusterID;
}

//
//	FUNCTION public
//	FullText2::ResultSet::seek -- カーソルを指定位置に進める
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiOffset_
//		カーソル位置
//	ModSize uiLimit_
//		取得数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ResultSet::seek(ModSize uiOffset_, ModSize uiLimit_)
{
	reset();
	
	m_uiLimit = uiLimit_;
	if (uiOffset_ == 0)
		return;

	if (m_bCluster)
	{
		// クラスタリングの場合には、クラスターを求めながら offset を処理する

		while (uiOffset_ && m_ite < m_end)
		{
			ModSize n = (m_ite - m_begin);

			if (n >= m_uiClusteredSize)
			{
				// クラスタが不足しているので、追加でクラスターを求める
				clustering();
			}

			ModSize clusterID = m_vecClusterID[n];

			if (m_uiCurrentClusterID != _UndefinedClusterID &&
				m_uiCurrentClusterID != clusterID)
			{
				// クラスター番号が変わったので、uiOffset_ を減らす

				--uiOffset_;
			}

			// 今のクラスター番号を保存する
			m_uiCurrentClusterID = clusterID;

			if (uiOffset_) ++m_ite;	// 次へ
		}
	}
	else
	{
		if (m_vecDocIDScore.getSize() < uiOffset_)
		{
			// 最後より大きいので終端を設定
		
			m_ite = m_end;
		}
		else
		{
			// オフセット分進める
		
			m_ite += uiOffset_;
		}
	}
}

//
//	FUNCTION public
//	FullText2::ResultSet::next -- 次の値を取得する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::IDVectorFile* pDocIDVectorFile_,
//		文書ID -> ROWID 変換用のベクターファイル
//	Common::DataArrayData& cTuple_
//		次の値
//
//	RETURN
//	bool
//		終端に達した場合は false、データがある場合は true
//
//	EXCEPTIONS
//
bool
ResultSet::next(IDVectorFile* pDocIDVectorFile_,
				Common::DataArrayData& cTuple_)
{
	if (m_bGetByBitSet)
	{
		// ビットセットで取得する場合には別の関数
		
		return getByBitSet(pDocIDVectorFile_, cTuple_);
	}
	
	if (m_uiLimit == 0 || m_ite == m_end)
	{
		// 終わり
		return false;
	}

	if (m_bCluster)
	{
		ModSize size = m_ite - m_begin;
		while (size >= m_uiClusteredSize)
		{
			// クラスターが不足しているので、追加でクラスターを求める
			clustering();
		}
	}

	ModUInt32 uiRowID = 0;
	ModUInt32 uiDocID = 0;
	DocumentScore	dblScore = 0;

	while (m_ite < m_end)
	{
		ModPair<DocumentID, DocumentScore>& p = (*m_ite);
		
		// 文書ID -> ROWIDに変換する
		
		uiDocID = p.first;
		if (pDocIDVectorFile_->get(uiDocID, uiRowID) == false)
		{
			// 見つからないので削除されたものかもしれない
			// 通常はありえないので、メッセージを出力する
			
			SydErrorMessage << "DocID(" << uiDocID << ") not found " << ModEndl;

			// 次へ
			++m_ite;
			continue;
		}

		// スコア
		
		dblScore = p.second;

		break;
	}

	if (m_ite == m_end)
		// 終わり
		return false;

	// 検索ノードは直前に返した文書IDより小さい文書IDで下限検索を実行しても
	// 検索せずに、直前に返した文書IDを返すので、
	// 必要ならここで初期化する
	
	if (m_uiCurrentID > uiDocID)
	{
		resetNode();
	}

	m_uiCurrentID = uiDocID;

	int n = 0;
	ModVector<FieldData>::Iterator i = m_vecProjections.begin();
	for (; i != m_vecProjections.end(); ++i, ++n)
	{
		Common::Data& cData = *cTuple_.getElement(n);
		
		switch ((*i).second)
		{
		case OpenOption::Function::RowID:
			{
				// ROWIDは要素番号ごとに変化はない
				
				if (cData.getType() != Common::DataType::UnsignedInteger)
					_SYDNEY_THROW0(Exception::BadArgument);

				Common::UnsignedIntegerData& v =
					_SYDNEY_DYNAMIC_CAST(Common::UnsignedIntegerData&,
										 cData);
				v.setValue(uiRowID);
			}
			break;
			
		case OpenOption::Function::Score:
			{
				// スコアは要素番号ごとに取得はできない
				
				if (cData.getType() != Common::DataType::Double)
					_SYDNEY_THROW0(Exception::BadArgument);
				
				Common::DoubleData& v =
					_SYDNEY_DYNAMIC_CAST(Common::DoubleData&, cData);
				v.setValue(dblScore);
			}
			break;
			
		case OpenOption::Function::Section:
			{
				// ヒットしたセクションは要素番号ごとに取得可能

				ModVector<ModSize> section;
				getSection((*i).first[0], uiDocID, section);

				// ヒットセクションはUnsignedIntegerDataを要素に持つ、
				// DataArrayDataで返す

				if (cData.getType() != Common::DataType::Array)
					_SYDNEY_THROW0(Exception::BadArgument);
				
				Common::DataArrayData& v =
					_SYDNEY_DYNAMIC_CAST(Common::DataArrayData&, cData);
				v.clear();
				v.reserve(section.getSize());

				ModVector<ModSize>::Iterator j = section.begin();
				for (; j != section.end(); ++j)
				{
					// 0-base から 1-base に変換する
					v.pushBack(new Common::UnsignedIntegerData(*j + 1));
				}
			}
			break;
			
		case OpenOption::Function::Tf:
			{
				// TF値リストは要素番号ごとに取得はできない

				ModVector<ModSize> tfList;
				getTfList(uiDocID, tfList);

				// TF値リストはUnsignedIntegerDataを要素に持つ、
				// DataArrayDataで返す

				if (cData.getType() != Common::DataType::Array)
					_SYDNEY_THROW0(Exception::BadArgument);
				
				Common::DataArrayData& v =
					_SYDNEY_DYNAMIC_CAST(Common::DataArrayData&, cData);
				v.clear();
				v.reserve(tfList.getSize());

				ModVector<ModSize>::Iterator j = tfList.begin();
				for (; j != tfList.end(); ++j)
				{
					v.pushBack(new Common::UnsignedIntegerData(*j));
				}
			}
			break;
			
		case OpenOption::Function::Existence:
			{
				// EXISTENCE値リストは要素番号ごとに取得はできない

				ModVector<int> extList;
				getExistenceList(uiDocID, extList);

				// EXISTENCE値リストはIntegerDataを要素に持つ、
				// DataArrayDataで返す

				if (cData.getType() != Common::DataType::Array)
					_SYDNEY_THROW0(Exception::BadArgument);
				
				Common::DataArrayData& v =
					_SYDNEY_DYNAMIC_CAST(Common::DataArrayData&, cData);
				v.clear();
				v.reserve(extList.getSize());

				ModVector<int>::Iterator j = extList.begin();
				for (; j != extList.end(); ++j)
				{
					v.pushBack(new Common::IntegerData(*j));
				}
			}
			break;
			
		case OpenOption::Function::ClusterID:
			{
				// クラスターIDは要素番号ごとに取得はできない
				
				if (cData.getType() != Common::DataType::Integer)
					_SYDNEY_THROW0(Exception::BadArgument);
				
				Common::IntegerData& v =
					_SYDNEY_DYNAMIC_CAST(Common::IntegerData&, cData);

				ModSize pos = m_ite - m_begin;
				ModSize clusterID = m_vecClusterID[pos];

				if (m_uiCurrentClusterID != _UndefinedClusterID &&
					m_uiCurrentClusterID != clusterID)
				{
					// クラスター番号が変わったので、終了条件を確認する

					if (--m_uiLimit == 0)
					{
						// 終了
						return false;
					}
				}
				
				v.setValue(static_cast<int>(clusterID + 1));	// 1ベース

				// 今のクラスター番号を保存する
				m_uiCurrentClusterID = clusterID;
			}
			break;

		case OpenOption::Function::FeatureValue:
			{
				// 特徴語リストは要素番号ごとに取得可能

				if (cData.getType() != Common::DataType::Array)
					_SYDNEY_THROW0(Exception::BadArgument);
				
				Common::DataArrayData& v =
					_SYDNEY_DYNAMIC_CAST(Common::DataArrayData&, cData);
				v.clear();

				SearchInformation& info
					= m_cSearchInfo.getElement((*i).first[0]);
				FeatureSetPointer p;
				if (info.getFeatureSet(uiDocID, p) == false)
				{
					v.setNull();
				}
				else
				{
					ModSize count = p->getCount();
					v.reserve(count);

					const FeatureSet::Feature* pFeature = p->first();
					for (ModSize j = 0; j < count; ++j)
					{
						Common::WordData* pWord = new Common::WordData(
							ModUnicodeString(pFeature->getString(),
											 pFeature->getLength()));
						pWord->setScale(pFeature->getWeight());
						v.pushBack(pWord);

						// 次へ
						pFeature = pFeature->next();
					}
				}
			}
			break;
			
		case OpenOption::Function::RoughKwicPosition:
			{
				// 粗いKWIC情報は要素番号ごとに取得可能

				if (cData.getType() != Common::DataType::Integer)
					_SYDNEY_THROW0(Exception::BadArgument);
				
				Common::IntegerData& v =
					_SYDNEY_DYNAMIC_CAST(Common::IntegerData&, cData);

				// 0-base にする
				v.setValue(getRoughKwic((*i).first[0], uiDocID) - 1);
			}
			break;
		}
	}

	if (m_bCluster == false)
	{
		--m_uiLimit;
	}
	
	++m_ite;

	return true;
}

//
//	FUCNTION public
//	FullText2::ResultSet::MyLocationListIterator::reset
//		-- リセットする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
// 	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ResultSet::MyLocationListIterator::reset()
{
	m_vecList.erase(m_vecList.begin(), m_vecList.end());
	m_uiCurrentLocation = 0;
}

//
//	FUCNTION public
//	FullText2::ResultSet::MyLocationListIterator::pushBack
//		-- 要素を追加する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::LocationListIterator::AutoPointer& p
//		追加する要素
//
// 	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ResultSet::MyLocationListIterator::
pushBack(LocationListIterator::AutoPointer& p)
{
	m_vecList.pushBack(LocationPair(Data(), p));
}

//
//	FUCNTION public
//	FullText2::ResultSet::MyLocationListIterator::next
//		-- 次の位置情報を得る
//
//	NOTES
//
//	ARGUMENTS
//	int& length_
//		長さ
//	int& element_
//		要素番号
//
// 	RETURN
//	ModSize
//		位置情報 (1-base)
//
//	EXCEPTIONS
//
ModSize
ResultSet::MyLocationListIterator::next(int& length_, int& element_)
{
	if (m_uiCurrentLocation == UndefinedLocation)
		return UndefinedLocation;
	
	if (m_uiCurrentLocation != 0 &&
		static_cast<ModSize>(m_iCurrentElement) < (m_vecList.getSize() - 1))
	{
		// 同じ位置の違う要素がある場合には、それを返す
		int n = m_iCurrentElement + 1;
		LocationVector::Iterator i = m_vecList.begin() + n;
		for (; i < m_vecList.end(); ++i, ++n)
		{
			if ((*i).first.loc == m_uiCurrentLocation)
			{
				// 同じ位置の違う要素が見つかったので、それを返す
				m_iCurrentElement = n;
			
				length_ = (*i).first.len;
				element_ = m_iCurrentElement;
				return m_uiCurrentLocation;
			}
		}
	}
	
	return lowerBound(m_uiCurrentLocation + 1, length_, element_);
}

//
//	FUCNTION public
//	FullText2::ResultSet::MyLocationListIterator::lowerBounnd
//		-- 位置情報を下限検索する 
//
//	NOTES
//
//	ARGUMENTS
//	ModSize location_
//		検索する位置
//	int& length_
//		長さ
//	int& element_
//		要素番号
//
// 	RETURN
//	ModSize
//		位置情報 (1-base)
//
//	EXCEPTIONS
//
ModSize
ResultSet::MyLocationListIterator::lowerBound(ModSize location_,
											  int& length_, int& element_)
{
	if (location_ <= m_uiCurrentLocation)
	{
		// 前の位置を検索しているので、現在値を返す
		length_ = m_iCurrentLength;
		element_ = m_iCurrentElement;
		
		return m_uiCurrentLocation;
	}
	
	m_uiCurrentLocation = UndefinedLocation;	// 最大値

	// 最小の位置を探す
	LocationVector::Iterator i = m_vecList.begin();
	int n = 0;
	for (; i < m_vecList.end(); ++i, ++n)
	{
		if ((*i).first.loc < location_)
		{
			int len;
			ModSize loc = (*i).second->lowerBound(location_, len);
			(*i).first.loc = loc;
			(*i).first.len = len;
		}

		if ((*i).first.loc < m_uiCurrentLocation)
		{
			m_uiCurrentLocation = (*i).first.loc;
			length_ = (*i).first.len;
			element_ = n;
		}
	}

	m_iCurrentLength = length_;
	m_iCurrentElement = element_;

	return m_uiCurrentLocation;
}

//
//	FUNCTION private
//	FullText2::ResultSet::clustering -- ある一定量の検索結果をクラスタリングする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		続きがある場合には true
//		すべての検索結果をクラスタリングした場合には false
//
//	EXCEPTIONS
//
bool
ResultSet::clustering()
{
	if (m_vecClusterID.getSize() == 0)
	{
		// まだ、一度も実行されてないので、クラスタIDの配列を初期化する
			
		m_vecClusterID.assign(m_vecDocIDScore.getSize(), 0);
		Common::LargeVector<ModSize>::Iterator i = m_vecClusterID.begin();
		Common::LargeVector<ModSize>::Iterator e = m_vecClusterID.end();
		for (ModSize n = 0; i < e; ++i, ++n)
			*i = n;
	}

	if ((m_vecDocIDScore.getSize() - m_uiClusteredSize) > 1)
	{
		// 未クラスタリングな検索結果が2件以上ある場合

		if (m_fGlobalClusteredLimit > 0 && m_cSearchInfo.isFeatureSet())
		{
			//
			// 詳細クラスタの場合
			//
			
			// ラフクラスタリング格納領域
			ModVector<ModPair<ModSize, ModSize> > roughCluster;

			// clusterに併合されている文書かどうかを区別するために、
			// 現在のcluster結果を保存しておく
			Common::LargeVector<ModSize> original = m_vecClusterID;

			// 検索結果をコピーする
			SimpleResultSet workSet	= m_vecDocIDScore;

			ModVector<ModVector<FeatureSet*> > dvmap1;

			// 取得した特徴語セットを保存しておく一時領域
			FeatureSetMap dvmap;

			//
			// 1. ラフ・クラスタリング
			//

			// ラフ・クラスタを最大でクラスタ数分(requestSize)作成
			roughCluster.clear();
			doRoughClustering(m_uiClusteredSize,
							  m_uiMaxRoughClusterCount,
							  roughCluster);

			//
			// 2. 詳細クラスタ
			//
				
			// 詳細クラスタの各クラスタを表すマップ
			// 第一要素; クラスタID
			// 第二要素: そのクラスタに属するデータの初期クラスタIDのベクタ
			//  クラスタIDは、クラスタ集合の中で最も前方のデータの
			//  初期クラスタIDで表わされる。
			//  初期クラスタIDについてはinitializeClusterIDList()を参照
			ClusterMap clusterMap;
			int clusterno = -1;
			for (ModVector<ModPair<ModSize, ModSize> >::Iterator
					 iter = roughCluster.begin();
				 iter != roughCluster.end(); ++iter)
			{
				// 各ラフクラスタを順番に処理

				// 現在参照中のラフクラスタリング内の各データの
				// ドキュメントベクタを作成する。
				setDocumentVectorMap(dvmap1, original,
									 (*iter).first, (*iter).second,
									 dvmap);

				// INT_MAXは、ラフクラスタリング内の全体を見て
				// 詳細クラスタリングすることを指示している。
				doClustering(dvmap1, (*iter).first, (*iter).second,
							 m_vecClusterID, original,
							 m_fLocalClusteredLimit,
							 Os::Limits<int>::getMax());

				setClusterMap(clusterMap, m_vecClusterID, original,
							  (*iter).first, (*iter).second,
							  clusterno);
			}

			//
			// 3. 詳細クラスタのマージ
			//
				
			// 詳細クラスタリングは荒いクラスタの中で行っているので、
			// 局所的なクラスタリングになっている可能性がある。
			// そこで、荒いクラスタ内というくくりを外して、
			// 類似の詳細クラスタ同士をマージする。
				
			//
			// 各詳細クラスタの代表データを取得
			//

			Common::LargeVector<ModSize> clusterWork;
			clusterWork.reserve(clusterMap.getSize());
			ClusterMap::Iterator cluster_iter = clusterMap.begin();
			for (; cluster_iter != clusterMap.end(); ++cluster_iter)
			{
				// クラスタIDを格納
				clusterWork.pushBack((*cluster_iter).first);
			}

			//
			// 代表データの詳細クラスタリング(代表クラスタリング)
			//

			// 特徴語ベクトルを取得する
			setDocumentVectorMap(dvmap1, clusterWork,
								 0, clusterWork.getSize() - 1,
								 dvmap);

			// dvmapのインデックスは、精密クラスタリングにより得られる
			// クラスタ番号
			// 速度重視のため、m_uiNeighbor個先のclusterのみmerge対象となる
			Common::LargeVector<ModSize> workOriginal = clusterWork;
			doClustering(dvmap1, 0, clusterWork.getSize() - 1,
						 clusterWork, workOriginal,
						 m_fLocalClusteredLimit,
						 m_uiNeighbor);

			ClusterMap workMap;
			int workno = -1;
			setClusterMap(workMap, clusterWork, workOriginal,
						  0, clusterWork.getSize() - 1, workno);
				
			// 
			// マージ
			//
			for (cluster_iter = workMap.begin();
				 cluster_iter != workMap.end(); ++cluster_iter)
			{
				// 各代表クラスタを順番に処理

				ModSize no = *((*cluster_iter).second.begin());
				for (ModVector<ModSize>::Iterator iter
						 = (*cluster_iter).second.begin() + 1;
					 iter != (*cluster_iter).second.end(); ++iter)
				{
					// 代表クラスタの各データを順番に処理

					// 代表クラスタの各データとは、
					// 代表データに対応する詳細クラスタのクラスタIDである。
					// 同じ代表クラスタに属する詳細クラスタを
					// 一つにマージする

					// マージされるクラスタ
					ClusterMap::Iterator noi = clusterMap.find(no);
					// マージするクラスタ
					ClusterMap::Iterator iti = clusterMap.find(*iter);

					// マージ
					(*noi).second.insert((*noi).second.end(),
										 (*iti).second.begin(),
										 (*iti).second.end());
						
					(*iti).second.clear();
				}
			}

			//
			// 4. 最終処理、クラスタの整形
			//

			// クラスタ毎に検索結果をまとめる
			SimpleResultSet::Iterator r = m_vecDocIDScore.begin();
			r += m_uiClusteredSize;
			Common::LargeVector<ModSize>::Iterator c = m_vecClusterID.begin();
			c += m_uiClusteredSize;
			for (cluster_iter = clusterMap.begin();
				 cluster_iter != clusterMap.end(); ++cluster_iter)
			{
				// 各クラスタを順番に処理
					
				if ((*cluster_iter).second.getSize() == 0)
					// クラスタが空の場合
						
					// 詳細クラスタのマージによって、
					// クラスタが空になることがある。
					continue;

				// resultSetの文書の順番を同一のクラスタがまとまるように
				// 置き換える。

				for (ModVector<ModSize>::Iterator index_iter
						 = (*cluster_iter).second.begin();
					 index_iter != (*cluster_iter).second.end();
					 ++index_iter, ++m_uiClusteredSize, ++r, ++c)
				{
					// クラスタの各データを順番に処理
						
					(*c) = (*cluster_iter).first;
					(*r) = workSet[*index_iter];
				}
			}
		}
		else
		{
			//
			// 粗いクラスタリングの場合
			//
			
			// ランキング・スコア値だけをもとにしたクラスタリング
			// (精密クラスタリングは行わない)

			// ラフクラスタリング格納領域
			ModVector<ModPair<ModSize, ModSize> > roughCluster;

			// クラスタリング
			doRoughClustering(m_uiClusteredSize,
							  m_uiMaxRoughClusterCount,
							  roughCluster);

			// 整形
			Common::LargeVector<ModSize>::Iterator c = m_vecClusterID.begin();
			c += m_uiClusteredSize;
			for (ModVector<ModPair<ModSize, ModSize> >::Iterator iter
					 = roughCluster.begin();
				 iter != roughCluster.end(); ++iter)
			{
				// 各クラスタを順番に処理
				
				for (ModSize p = (*iter).first; p <= (*iter).second;
					 ++m_uiClusteredSize, ++c, ++p)
				{
					// あるクラスタに含まれる文書を順番に処理

					// クラスタIDリストを設定
					(*c) = iter - roughCluster.begin();
				}
			}
		}
	}
	else if ((m_vecDocIDScore.getSize() - m_uiClusteredSize) == 1)
	{
		// 残りが1件のみになった場合
		
		++m_uiClusteredSize;
	}

	return (m_uiClusteredSize < m_vecDocIDScore.getSize());
}

//
//	FUNCTION private
//	FullText2::ResultSet::doRoughClustering -- 粗いクラスタリング
//
//	NOTES
//  似たような内容の文書は、ランキングスコアも近くなることから、
//  ランキングスコアで粗く検索結果をグルーピングする。
//  類似文書は、ランキングスコアが近いが、
//  ランキングスコアが近いからといって、類似文書でない場合もある。
//	スコアの変化率が、平均変化率を超える場合、
//	その前後でクラスタをわける。
//	
//
//	ARGUMENTS
//	ModSize from
//		検索結果集合におけるクラスタリングの開始位置(0-base)
//		ちなみに終了位置は最後まで。
//	ModSize requestedRoughClusterNum
//		生成するクラスタリング数
//	ModVector<ModPair<ModSize, ModSize> >& roughCluster
//		クラスタリング結果の格納先
//		一つのModPair<ModSize, ModSize>が一つのクラスタに対応している。
//		Elementの第一要素：クラスタの先頭位置
//		Elementの第二要素：クラスタの終了位置
//		※位置はresultSetに格納されている文書の順番に対応している。
//  	requestedRoughClusterNum > roughCluster.getSize() ならば、
//		検索結果集合のすべての要素を処理したことになる。
//
//	RETURN
//
//	EXCEPTIONS
// 
void
ResultSet::
doRoughClustering(ModSize from,
				  ModSize requestedRoughClusterNum,
				  ModVector<ModPair<ModSize, ModSize> >& roughCluster)
{
	// クラスタの終了位置
	ModSize i;
	// クラスタの開始位置
	ModSize j = from;
	// クラスタリング対象の総数
	ModSize uiSize = m_vecDocIDScore.getSize();
	; _TRMEISTER_ASSERT(uiSize > 1);
	// 平均変化率の調査単位
	ModSize bias  = 1024;
	// 平均変化率の調査開始位置(0-base)
	ModSize uiStart;
	// 平均変化率の調査終了位置(0-base)
	ModSize uiEnd = from;
	// 平均変化率
	DocumentScore D = 0;
	
	do
	{
		//
		// スコアの平均変化率
		//
		do
		{
			// 平均変化率の調査範囲
			uiStart = uiEnd;
			uiEnd = ModMin(uiStart + bias, uiSize) - 1;

			if (uiStart >= uiEnd)
				// 調査範囲が1件以下
				// uiStart == uiEnd == uiSize - 1 になっている
				break;
			
			// 計算
			// スコアは昇順になっている。
			; _TRMEISTER_ASSERT(m_vecDocIDScore[uiStart].second >=
								m_vecDocIDScore[uiEnd].second);
			D = (m_vecDocIDScore[uiStart].second
				 - m_vecDocIDScore[uiEnd].second) / (uiEnd - uiStart);
			
			// scoreに変化が現れるまで検索結果のscoreを観察する
		} while (D == 0.0);

		//
		// クラスタリング
		//
		for (i = uiStart; i < uiEnd; ++i)
		{
			if ((m_vecDocIDScore[i].second - m_vecDocIDScore[i+1].second) > D)
			{
				// scoreが平均変化率を超える場合

				// [j,i] を新たなクラスタにする
				roughCluster.pushBack(ModPair<ModSize, ModSize>(j, i));
				// 次のクラスタの開始は、i + 1から
				j = i + 1;
				
				if (roughCluster.getSize() >= requestedRoughClusterNum)
					// ここでクラスタ数の上限に達した場合、
					// i < uiEnd <= uiSize - 1 が保証されるので、
					// 残りのデータをさらに追加して、
					// クラスタの上限を超えてしまうことはない。
					break;
			}
		}

		if (i >= uiSize - 1)
		{
			// 全てのデータに対してクラスタリングが終わった場合
			
			// 残りのデータを一つのクラスタにする。
			roughCluster.pushBack(ModPair<ModSize, ModSize>(j,i));
			break;
		}
		
		// requestedRoughClusterNumに到達するまでループ
	}
	while (roughCluster.getSize() < requestedRoughClusterNum);
}

//
//	FUNCTION private
//	Inverted::SearchResultSet::doClustering -- 詳細クラスタリング
//
//	NOTES
//	クラスタの識別は、クラスタ内の先頭文書の位置で示される。
//	この時点ではクラスタの識別番号は連番ではない。
//	連番にするのは、SearchResultSet::getCluster内の整形部分。
//
//	ARGUMENTS
//	ModVector<ModVector<FeatureSet*> >& dvmap
//		クラスタリング対象データの特徴語集合のリスト
//	ModSize from
//		詳細クラスタリングの開始位置
//	ModSize to
//		詳細クラスタリングの終了位置
//	Common::LargeVector<ModSize>& cluster_
//		クラスタリング結果の格納先
//	const Common::LargeVector<ModSize>& original_
//		クラスタリング前のクラスタIDの初期値リスト
//	float thresh
//		同一クラスタかどうかを判定する閾値
//		閾値を超える場合、同一クラスタとみなす。
//	ModSize neighbor
//		内積を取る範囲
//		fromとtoで指定された全てのデータと内積をとるわけではない。
//
//	RETURN
//
//	EXCEPTIONS
// 
void
ResultSet::doClustering(ModVector<ModVector<FeatureSet*> >& dvmap,
						ModSize from,
						ModSize to,
						Common::LargeVector<ModSize>& cluster_,
						const Common::LargeVector<ModSize>& original_,
						float thresh,
						ModSize neighbor)
{
	// 検査対象のイテレータ
	Common::LargeVector<ModSize>::Iterator ci = cluster_.begin() + from;
	// 検査対象の最後のイテレータ
	const Common::LargeVector<ModSize>::Iterator cie = cluster_.begin() + to;
	// 検査対象のクラスタIDの初期値を取得するイテレータ
	Common::LargeVector<ModSize>::ConstIterator oi = original_.begin() + from;
	// 検索対象の特徴語集合へのイテレータ
	ModVector<ModVector<FeatureSet*> >::Iterator fi = dvmap.begin();
	
	for (; ci <= cie; ++ci, ++oi, ++fi)
	{
		// 各データを順番に処理
		
		if (*ci != *oi)
			// 参照中のデータのクラスタIDが初期値と異なる場合
			
			// すでにどこかのclusterに併合されている文書に関しては
			// 内積をとらない
			continue;

		// 内積対象のイテレータ
		Common::LargeVector<ModSize>::Iterator cj = ci;
		// 内積対象の最後のイテレータ
		Common::LargeVector<ModSize>::Iterator cje;
		if (static_cast<unsigned int>(cie - ci) > neighbor)
			// 残りのクラスタ数がneigbor個より多い場合
			
			// neighbor個先のclusterしか見ない
			cje = ci + neighbor;
		else
			cje = cie;
		// 内積対象のクラスタIDの初期値を取得するイテレータ
		Common::LargeVector<ModSize>::ConstIterator oj = oi;
		// 内積対象の特徴語集合へのイテレータ
		ModVector<ModVector<FeatureSet*> >::Iterator fj = fi;

		for (++cj, ++oj, ++fj; cj <= cje; ++cj, ++oj, ++fj)
		{
			// cj=ci+1 から cje までの各データを順番に処理
			
			if (*cj != *oj)
				// 内積対象のデータのクラスタIDが初期値と異なる場合
				
				// すでにどこかのclusterに併合されている文書に関しては
				// 内積をとらない
				continue;

			//
			// 内積
			//
			
			if (innerProduct(*fi, *fj) > thresh)
			{
				// 閾値を超える場合
				
				// 内積対象のデータを同じクラスタにする
				*cj = *ci;
			}
		}
	}
}

//
//	FUNCTION private
//	FullText2::ResultSet::innerProduct -- 内積を求める
//
//	NOTES
//
//	ARGUMENTS
//	ModVector<FeatureSet*>& f1
//		特徴語セット
//	ModVector<FeatureSet*>& f2
//		特徴語セット
//
//	RETURN
//	float
//		内積値
//
//	EXCEPTIONS
//
float
ResultSet::innerProduct(ModVector<FeatureSet*>& f1,
						ModVector<FeatureSet*>& f2)
{
	float inner = 0;
		
	if (m_eClusterCombiner == ClusterCombiner::Ave)
	{
		// 平均
		
		ModVector<FeatureSet*>::Iterator i1 = f1.begin();
		ModVector<FeatureSet*>::Iterator i2 = f2.begin();
		ModVector<ModPair<int, float> >::Iterator s = m_vecClusterScale.begin();
		float totalScale = 0;

		for (; s != m_vecClusterScale.end(); ++s, ++i1, ++i2)
		{
			inner += ((*i1)->innerProduct(*(*i2))) * (*s).second;
			totalScale += (*s).second;
		}

		inner /= totalScale;
	}
	else if (m_eClusterCombiner == ClusterCombiner::Max)
	{
		// 最大値
		
		ModVector<FeatureSet*>::Iterator i1 = f1.begin();
		ModVector<FeatureSet*>::Iterator i2 = f2.begin();
		ModVector<ModPair<int, float> >::Iterator s = m_vecClusterScale.begin();

		for (; s != m_vecClusterScale.end(); ++s, ++i1, ++i2)
		{
			float tmp = ((*i1)->innerProduct(*(*i2))) * (*s).second;
			if (inner < tmp)
				inner = tmp;
		}
	}

	return inner;
}

//
//	FUNCTION private
//	Inverted::SearchResultSet::setDocumentVectorMap -- 
//
//	NOTES
//
//	ARGUMENTS
//	ModVector<ModVector<FeatureSet*> >& dvmap1
//		特徴語ベクトルの格納先
//	const Common::LargeVector<ModSize>& clusterWork
//		詳細クラスタリング結果
//	ModSize uiBegin_
//		詳細クラスタリングの開始位置
//	ModSize uiEnd_
//		詳細クラスタリングの終了位置
//	FullText2::ResultSet::FeatureSetMap& dvmap
//		取得した特徴語セットを保存しておく一時領域
//
//	RETURN
//
//	EXCEPTIONS
//　
void
ResultSet::setDocumentVectorMap(ModVector<ModVector<FeatureSet*> >& dvmap1,
								const Common::LargeVector<ModSize>& clusterWork,
								ModSize uiBegin_,
								ModSize uiEnd_,
								FullText2::ResultSet::FeatureSetMap& dvmap)
{
	; _TRMEISTER_ASSERT(uiBegin_ <= uiEnd_);
	; _TRMEISTER_ASSERT(clusterWork.getSize() > uiEnd_);
	
	// 初期化
	dvmap1.assign(uiEnd_ - uiBegin_ + 1, 0);

	Common::LargeVector<ModSize>::ConstIterator wi
		= clusterWork.begin() + uiBegin_;
	const Common::LargeVector<ModSize>::ConstIterator we
		= clusterWork.begin() + uiEnd_;
	ModVector<ModVector<FeatureSet*> >::Iterator dvite = dvmap1.begin();
	for (; wi <= we; ++wi, ++dvite)
	{
		makeDocumentVector(*wi, *dvite, dvmap);
	}
}

//
//	FUNCTION private
//	FullText2::ResultSet::makeDocumentVector
//		-- 特徴語リストを取得する
//
//	NOTES
//
//	ARGUMENTS
//	ModSize clusterID
//		クラスターID
//	ModVector<FullText2::FeatureSet*>& dv
//		得られた特徴語リスト
//	FullText2::ResultSet::FeatureSetMap& dvmap
//		取得した特徴語セットを保存しておく一時領域
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ResultSet::makeDocumentVector(ModSize clusterID,
							  ModVector<FeatureSet*>& dv,
							  FullText2::ResultSet::FeatureSetMap& dvmap)
{
	DocumentID id = m_vecDocIDScore[clusterID].first;
	ModVector<FeatureSetPointer>& v = dvmap[id];
	if (v.getSize() == 0)
	{
		// まだ一度も取得していないので、
		// 検索情報クラスから特徴語を取得する

		ModVector<ModPair<int, float> >::Iterator i = m_vecClusterScale.begin();
		for (; i != m_vecClusterScale.end(); ++i)
		{
			// フィールドごとに格納されている

			FeatureSetPointer p;
			SearchInformation& info = m_cSearchInfo.getElement((*i).first);
			if (info.getFeatureSet(id, p) == false)
			{
				// 特徴語が格納されていない
				_TRMEISTER_THROW0(Exception::BadArgument);
			}

			v.pushBack(p);
		}
	}

	dv.erase(dv.begin(), dv.end());
	ModVector<FeatureSetPointer>::Iterator j = v.begin();
	for (; j != v.end(); ++j)
		dv.pushBack((*j).get());
}

//
//	FUNCTION private
//	Inverted::SearchResultSet::setClusterMap -- クラスタマップの設定
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::ResultSet::ClusterMap& clusterMap,
//		詳細クラスタリング結果の格納先
//	const Common::LargeVector<ModSize>& cluster_
//		詳細クラスタリング結果
//	const Common::LargeVector<ModSize>& original_
//		クラスタIDリストの初期状態
//	ModSize uiBegin_
//		詳細クラスタリングの開始位置
//	ModSize uiEnd_
//		詳細クラスタリングの終了位置
//	ModInt32& clusterno
//		直前のクラスタID
//		初回に必ずModMap::insert()を実行することを回避するため。
//
//	RETURN
//
//	EXCEPTIONS
//　
void
ResultSet::setClusterMap(ClusterMap& clusterMap,
						 const Common::LargeVector<ModSize>& cluster_,
						 const Common::LargeVector<ModSize>& original_,
						 ModSize uiBegin_,
						 ModSize uiEnd_,
						 ModInt32& clusterno) const
{
	; _TRMEISTER_ASSERT(uiBegin_ <= uiEnd_);
	; _TRMEISTER_ASSERT(cluster_.getSize() > uiEnd_);
	; _TRMEISTER_ASSERT(original_.getSize() > uiEnd_);
	
	// 参照中のクラスタへのイテレータ
	ClusterMap::Iterator ci;

	Common::LargeVector<ModSize>::ConstIterator i
		= cluster_.begin() + uiBegin_;
	const Common::LargeVector<ModSize>::ConstIterator e
		= cluster_.begin() + uiEnd_;
	Common::LargeVector<ModSize>::ConstIterator j
		= original_.begin() + uiBegin_;
	for (; i <= e; ++i, ++j)
	{
		// ラフクラスタ内の各データを順番に処理

		//
		// 格納先のクラスタを取得
		//
		if (*i != clusterno)
		{
			// 直前のクラスタIDと異なる場合

			// 新しいクラスタを生成
			// [NOTE] すでに存在する場合は、そのクラスタへのイテレータが返る
			ModPair<ClusterMap::Iterator, ModBoolean> r
				= clusterMap.insert(*i, ModVector<ModSize>());

			// 参照中のクラスタを更新
			ci = r.first;
			clusterno = *i;
		}

		//
		// 詳細クラスタリング結果を格納
		//
		
		// 初期状態の位置を格納する
		(*ci).second.pushBack(*j);
	}
}

//
//	FUNCTION private
//	FullText2::ResultSet::sortEachCluster -- クラスタ内をソートする
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::SortKey::Value eKey_
//		キー
//	FullText2::Order::Value eOrder_
//		昇順 or 降順
//	Common::LargeVector<ClusterElement>& vecClusterElement_
//		クラスター単位を表す構造体の配列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ResultSet::
sortEachCluster(SortKey::Value eKey_, Order::Value eOrder_,
				Common::LargeVector<ClusterElement>& vecClusterElement_)
{
	Common::LargeVector<ModSize>::ConstIterator s = m_vecClusterID.begin();
	Common::LargeVector<ModSize>::ConstIterator e = m_vecClusterID.end();
	Common::LargeVector<ModSize>::ConstIterator i = s;
	Common::LargeVector<ModSize>::ConstIterator c = i;	// クラスタの開始位置

	SimpleResultSet::ConstIterator v = m_vecDocIDScore.begin();

	// 二番目から調べる
	for (++i; i < e; ++i)
	{
		if (*i != *c)
		{
			// iは次のクラスタを指す

			// 前のクラスタ、cからiの直前までをソート
			m_vecDocIDScore.sort(c - s, i - s, eKey_, eOrder_);
			
			// 前のクラスタのスコアの最大値と、クラスタの範囲を記録
			// 範囲は開始位置と、終了位置の次で表わされる。
			
			ClusterElement ele;
			ele.first = c - s;
			ele.second = i - s;
			if (eKey_ == SortKey::Score)
				ele.score = m_vecDocIDScore[ele.first].second;
			else
				ele.id = m_vecDocIDScore[ele.first].first;

			vecClusterElement_.pushBack(ele);
			
			// 次のクラスタの開始位置を更新
			c = i;
		}
	}
	
	// 最後のクラスタをソート
	m_vecDocIDScore.sort(c - s, i - s, eKey_, eOrder_);

	// 最後のクラスタを記録
	ClusterElement ele;
	ele.first = c - s;
	ele.second = i - s;
	if (eKey_ == SortKey::Score)
		ele.score = m_vecDocIDScore[ele.first].second;
	else
		ele.id = m_vecDocIDScore[ele.first].first;

	vecClusterElement_.pushBack(ele);
}

//
//	FUNCTION public
//	FullText2::ResultSet::sortByCluster
//		-- クラスター単位でソートする
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::SortKey::Value eKey_
//		ソートキー
//	FullText2::Order::Value eOrder_
//		昇順 or 降順
//	Common::LargeVector<ClusterElement>& vecClusterElement_
//		クラスター単位を表す構造体の配列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ResultSet::
sortByCluster(SortKey::Value eKey_, Order::Value eOrder_,
			  Common::LargeVector<ClusterElement>& vecClusterElement_)
{
	// クラスタの代表文書をソート
	if (eKey_ == SortKey::Score)
	{
		if (eOrder_ == Order::Desc)
			ModSort(vecClusterElement_.begin(), vecClusterElement_.end(),
					_ClusterElementScoreGreater());
		else
			ModSort(vecClusterElement_.begin(), vecClusterElement_.end(),
					_ClusterElementScoreLess());
	}
	else if (eKey_ == SortKey::DocID)
	{
		if (eOrder_ == Order::Desc)
			ModSort(vecClusterElement_.begin(), vecClusterElement_.end(),
					_ClusterElementIDGreater());
		else
			ModSort(vecClusterElement_.begin(), vecClusterElement_.end(),
					_ClusterElementIDLess());
	}
	
	// 検索結果をコピーする
	SimpleResultSet workSet	= m_vecDocIDScore;
	
	ModSize n = 0; // クラスター番号
	
	Common::LargeVector<ClusterElement>::ConstIterator ite
		= vecClusterElement_.begin();
	Common::LargeVector<ClusterElement>::ConstIterator iteEnd
		= vecClusterElement_.end();

	SimpleResultSet::Iterator r = m_vecDocIDScore.begin();
	Common::LargeVector<ModSize>::Iterator c = m_vecClusterID.begin();
	
	for (; ite < iteEnd; ++ite, ++n)
	{
		// クラスタ毎に上書きする

		ModSize j = (*ite).first;	// クラスタの開始位置
		ModSize e = (*ite).second;	// クラスタの終了位置の次
		for (; j < e; ++j, ++r, ++c)
 		{
			(*r) = workSet[j];
			(*c) = n;
		}
	}
}

//
//	FUNCTION private
//	FullText2::ResultSet::resetNode
//		-- 検索ノードをリセットする
//
//	NOTES
//
//	ARGUMETNS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ResultSet::resetNode()
{
	ModVector<OperatorTermNode*>::Iterator i = m_vecTfList.begin();
	for (; i < m_vecTfList.end(); ++i)
	{
		(*i)->reset();
	}

	ModVector<Query::TermNodeMap>::Iterator k = m_vecMapTermLeafNode.begin();
	for (; k != m_vecMapTermLeafNode.end(); ++k)
	{
		Query::TermNodeMap& mapTermLeafNode = *k;
		Query::TermNodeMap::Iterator j = mapTermLeafNode.begin();
		for (; j != mapTermLeafNode.end(); ++j)
		{
			(*j).second->reset();
		}
	}
}

//
//	FUNCTION private
//	FullText2::ResultSet::getTfList
//		-- TF値リストを取得する
//
//	NOTES
//
//	ARGUMETNS
//	FullText2::DocumentID id_
//		文書ID
//	ModVector<ModSize>& tfList_
//		TF値リスト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ResultSet::getTfList(DocumentID id_, ModVector<ModSize>& tfList_)
{
	tfList_.erase(tfList_.begin(), tfList_.end());
	tfList_.reserve(m_vecTfList.getSize());

	ModVector<OperatorTermNode*>::Iterator i = m_vecTfList.begin();
	for (; i < m_vecTfList.end(); ++i)
	{
		ModSize tf = 0;
		if ((*i)->lowerBound(m_cSearchInfo, id_, false) == id_)
		{
			// ヒットしたのでTF値を得る
			tf = (*i)->getTermFrequency();
		}

		// 引数に追加する
		tfList_.pushBack(tf);
	}
}

//
//	FUNCTION private
//	FullText2::ResultSet::getExistenceList
//		-- EXISTENCE値リストを取得する
//
//	NOTES
//
//	ARGUMETNS
//	FullText2::DocumentID id_
//		文書ID
//	ModVector<int>& extList_
//		EXISTENCE値リスト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ResultSet::getExistenceList(DocumentID id_, ModVector<int>& extList_)
{
	extList_.erase(extList_.begin(), extList_.end());
	extList_.reserve(m_vecTfList.getSize());

	ModVector<OperatorTermNode*>::Iterator i = m_vecTfList.begin();
	for (; i < m_vecTfList.end(); ++i)
	{
		int existence = 0;
		if ((*i)->lowerBound(m_cSearchInfo, id_, false) == id_)
		{
			// ヒットした
			existence = 1;
		}

		// 引数に追加する
		extList_.pushBack(existence);
	}
}

//
//	FUNCTION private
//	FullText2::ResultSet::getSection
//		-- セクション検索を実施する
//
//	NOTES
//
//	ARGUMETNS
//	int n_
//		要素番号
//	FullText2::DocumentID id_
//		文書ID
//	ModVector<ModSize>& section_
//		ヒットしたセクション(0-base)
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ResultSet::getSection(int n_, DocumentID id_, ModVector<ModSize>& section_)
{
	section_.erase(section_.begin(), section_.end());
	
	// セクションごとの文書長を得る
	ModVector<ModSize> vecSectionSize;
	SearchInformation& info = m_cSearchInfo.getElement(n_);
	info.getSectionSize(id_, vecSectionSize);

	// 位置情報へのイテレータ
	MyLocationListIterator ite;
	if (getLocationListIterator(n_, id_, ite) == false)
		return;

	// ヒットしたセクションを探す
	ModSize sectionOffset = 1;	// セクションの開始位置
	ModSize n = 0;				// セクションの要素番号
	ModVector<ModSize>::Iterator i = vecSectionSize.begin();
	for (; i != vecSectionSize.end(); ++i, ++n)
	{
		int dummy1;
		int dummy2;
		
		if (ite.lowerBound(sectionOffset, dummy1, dummy2)
			< (sectionOffset + *i))
		{
			// このセクションはヒットした (0-base)
			section_.pushBack(n);
		}

		sectionOffset += *i;
	}
}

//
//	FUNCTION private
//	FullText2::ResultSet::getRoughKwic
//		-- 粗いKWIC情報を取得する
//
//	NOTES
//
//	ARGUMENTS
//	int n_
//		要素番号
//	FullText2::DocumentID id_
//		粗いKWIC情報を取得する文書の文書ID
//
//	RETURN
//	int
//		KWIC領域の先頭位置(1-base)
//
//	EXCEPTIONS
//
int
ResultSet::getRoughKwic(int n_, DocumentID id_)
{
	// 検索情報クラスを得る
	SearchInformation& info = m_cSearchInfo.getElement(n_);

	// オリジナルの文書長
	ModSize uiOrigLength = 0;
	// 調整係数を得る
	double adjustFactor = _getAdjustFactor(id_, info, uiOrigLength);

	// 荒いKWICの中心部分となるシードKWICを取得する。
	
	// シードKWICの開始位置は、シードKWICに含まれる索引語の中で、
	// 最も先頭に出現する索引語の位置を返す。
	// シードKWICは、最も多くの相異なる索引語を含む範囲の中で、
	// 最も前方の範囲が選択される。
	
	// シードKWICの開始位置を示す索引語の開始位置
	ModSize uiPosition = 0;
	// シードKWICに含まれる異なり索引語数
	ModSize uiHitCount = 0;
	// 調整後のKWICサイズ
	ModSize uiKwicSize = static_cast<ModSize>(m_vecKwicSize[n_] * adjustFactor);
	uiKwicSize = (uiKwicSize == 0) ? 1 : uiKwicSize;

	if (uiKwicSize > uiOrigLength)
		// KWICサイズよりも短いので、先頭を返す
		return 1;

	// イテレータ
	MyLocationListIterator ite;
	if (getLocationListIterator(n_, id_, ite) == false)
		// ヒットしないので、先頭位置を返す
		return 1;
	
	ModSize uiTail = 0;
	ModVector<ModPair<ModSize, int> > vecHitPosition;
	
	// ヒット数
	ModSize uiCurrentHitCount = 0;
	ModVector<ModSize> vecHitCount(ite.getSize(), ModSize(0));

	int length;	// dummy
	int element;
	
	while ((uiTail = ite.next(length, element)) != UndefinedLocation)
	{
		// 位置情報は 1-base である
		
		// 新しい索引語を追加
		if (vecHitCount[element]++ == 0)
		{
			++uiCurrentHitCount;
		}

		// 一度取得した位置情報を覚えておく
		vecHitPosition.pushBack(ModPair<ModSize, int>(uiTail, element));
		
		// シードKWICからはみ出した索引語を削除
		while (vecHitPosition.getSize() > 1 &&
			   uiTail - (*vecHitPosition.begin()).first + 1 > uiKwicSize)
		{
			if (--vecHitCount[(*vecHitPosition.begin()).second] == 0)
			{
				--uiCurrentHitCount;
			}
			vecHitPosition.popFront();
		}

		// シードKWICを更新
		if (uiCurrentHitCount > uiHitCount)
		{
			uiPosition = (*vecHitPosition.begin()).first;
			uiHitCount = uiCurrentHitCount;
			
			if (uiHitCount == ite.getSize())
			{
				break;
			}
		}
	}

	ModSize result = 1;	// 1-base
	
	// 正規化位置or単語位置から戻すので、係数の逆数を掛け合わせる。
	ModSize uiOrigPosition = static_cast<ModSize>(uiPosition / adjustFactor);

	// マージン
	ModSize uiMargin
		= (m_vecKwicMarginScaleFactor[n_] - 1) * m_vecKwicSize[n_] / 2;
	
	// マージンを追加
	if (uiOrigPosition > uiMargin)
	{
		result = uiOrigPosition - uiMargin;

		// ラフKWICサイズ
		ModSize uiRoughKwicSize
			= m_vecKwicMarginScaleFactor[n_] * m_vecKwicSize[n_];
		// ラフKWICが文書の末尾からはみ出していたら、開始位置を前に移動する
		uiOrigLength = (uiOrigLength < uiRoughKwicSize) ?
			uiRoughKwicSize : uiOrigLength;
		result = ModMin(uiOrigLength - uiRoughKwicSize + 1, result);
	}

	return result;
}

//
//	FUNCTION private
//	FullText2::ResultSet::getLocationListIterator
//		-- 位置情報へのイテレータを得る
//
//	NOTES
//
//	ARGUMENTS
//	int n_
//		要素番号
//	FullText2::DocumentID id_
//		文書ID
//	FullText2::ResultSet::MyLocationListIterator& ite_
//		位置情報へのイテレータ
//
//	RETURN
//	bool
//		ヒットした場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
ResultSet::getLocationListIterator(int n_,
								   DocumentID id_,
								   MyLocationListIterator& ite_)
{
	Query::TermNodeMap& mapTermLeafNode = m_vecMapTermLeafNode[n_];
	
	ite_.reset();
	Query::TermNodeMap::Iterator i = mapTermLeafNode.begin();
	for (; i != mapTermLeafNode.end(); ++i)
	{
		if ((*i).second->lowerBound(m_cSearchInfo.getElement(n_),
									id_, false) == id_)
		{
			// ヒットしたので位置情報を取得する
			LocationListIterator::AutoPointer p
				= (*i).second->getLocationListIterator();
			ite_.pushBack(p);
		}
	}

	return (ite_.getSize() != 0) ? true : false;
}

//
//	FUNCTION private
//	FullText2::ResultSet::getByBitSet -- ビットセットで取得する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::IDVectorFile* pDocIDVectorFile_,
//		文書ID -> ROWID 変換用のベクターファイル
//	Common::DataArrayData& cTuple_
//		次の値
//
//	RETURN
//	bool
//		終端に達した場合は false、データがある場合は true
//
//	EXCEPTIONS
//
bool
ResultSet::getByBitSet(IDVectorFile* pDocIDVectorFile_,
					   Common::DataArrayData& cTuple_)
{
	if (m_ite == m_end)
		return false;

	// データを取り出す
	Common::Data& cData = *cTuple_.getElement(0);
	if (cData.getType() != Common::DataType::BitSet)
		_SYDNEY_THROW0(Exception::BadArgument);

	Common::BitSet& v =
		_SYDNEY_DYNAMIC_CAST(Common::BitSet&, cData);
	v.clear();
	
	for (; m_ite < m_end; ++m_ite)
	{
		// 文書ID -> ROWID に変換しながら、ビットマップに値を設定する

		ModUInt32 uiDocID = (*m_ite).first;
		ModUInt32 uiRowID = 0;
		
		if (pDocIDVectorFile_->get(uiDocID, uiRowID) == false)
		{
			// 見つからないので削除されたのかもしれないが、
			// 通常はありえないので、メッセージを出力する

			SydErrorMessage << "DocID(" << uiDocID << ") not found " << ModEndl;

			// 次へ
			continue;
		}

		// ビットセットにROWIDを設定する
		v.set(uiRowID);
	}
	
	return true;
}

//
//	Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
