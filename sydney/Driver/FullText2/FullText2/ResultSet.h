// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ResultSet.h --
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

#ifndef __SYDNEY_FULLTEXT2_RESULTSET_H
#define __SYDNEY_FULLTEXT2_RESULTSET_H

#include "FullText2/Module.h"
#include "FullText2/FeatureSet.h"
#include "FullText2/LocationListIterator.h"
#include "FullText2/OpenOption.h"
#include "FullText2/Query.h"
#include "FullText2/SimpleResultSet.h"

#include "Common/DataArrayData.h"
#include "Common/LargeVector.h"

#include "ModHashMap.h"
#include "ModPair.h"
#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class IDVectorFile;
class OperatorTermNode;
class SearchInformation;

//
//	CLASS
//	FullText2::ResultSet -- 検索結果クラスの基底クラス
//
//	NOTES
//
class ResultSet
{
public:
	//
	// 	フィールドデータ
	//
	typedef ModPair<ModVector<int>, OpenOption::Function::Value>	FieldData;

	//
	//	クラスタの合成方法
	struct ClusterCombiner
	{
		enum Value
		{
			Ave,		// 平均
			Max			// 最大値
		};
	};

	// クラスタのソート用の構造体
	struct ClusterElement
	{
		union {
			DocumentID		id;			// 文書ID
			DocumentScore	score;		// スコア
		};
		ModSize				first;		// クラスタの先頭要素番号
		ModSize				second;		// クラスタの終端要素番号
	};

	// コンストラクタ
	ResultSet(const ModVector<FieldData>& projections_,
			  SearchInformation& cSearchInfo_,
			  bool isGetByBitSet_);
	// デストラクタ
	virtual ~ResultSet();

	// 検索結果を設定する
	void setResultSet(const SimpleResultSet& result_);

	// TFリスト取得のための検索語ノードを設定する
	void setTfListNode(const ModVector<OperatorTermNode*>& vecNode_)
		{ m_vecTfList = vecNode_; }
	
	// 位置情報取得のための検索語ノードを設定する
	void setTermLeafNode(int n_,
						 Query::TermNodeMap& mapTermLeafNode_);
	// 位置情報取得のための検索語リストを得る
	void getSearchTerm(int n_, Common::DataArrayData& cTermList_);

	// 粗いKWIC情報のためのパラメータを設定する
	void setKwicParameter(int n_,
						  ModSize uiKwicSize_,
						  ModSize uiKwicMarginScaleFactor_);

	// クラスタリングのためのパラメータを初期化する
	void setClusterParameter(float fClusteredLimit_,
							 ClusterCombiner::Value eCombiner_,
							 const ModVector<ModPair<int, float> >& vecScale_);
	
	// スコア調整カラムのスコアでスコアを調整する
	void adjustScore(AdjustMethod::Value eMethod_,
					 SortKey::Value eKey_, Order::Value eOrder_);

	// 要素数を得る
	ModSize getSize() const { return m_vecDocIDScore.getSize(); }
	
	// 結果集合をクリアする
	void clear();

	// カーソルを先頭に戻す
	void reset();
	// カーソルを指定位置に進める(戻すことはできない)
	void seek(ModSize uiOffset_, ModSize uiLimit_);
	// 次の値を取得する
	bool next(IDVectorFile* pDocIDVectorFile_,
			  Common::DataArrayData& cTuple_);
	
private:
	// 長いのでtypedefする
	typedef ModHashMap<DocumentID, ModVector<FeatureSetPointer>,
					   ModHasher<DocumentID> > FeatureSetMap;
	typedef ModMap<ModInt32, ModVector<ModSize>,
				   ModLess<ModInt32> > ClusterMap;

	// 位置情報へのイテレータ
	class MyLocationListIterator
	{
	public:
		// コンストラクタ
		MyLocationListIterator()
			: m_uiCurrentLocation(0) {}
		// デストラクタ
		~MyLocationListIterator() {}

		// 要素配列を確保する
		void reserve(ModSize n_) { m_vecList.reserve(n_); }
		// 要素配列をリセットする
		void reset();
		// 位置情報を追加する
		void pushBack(LocationListIterator::AutoPointer& list_);

		// 次の位置を得る
		ModSize next(int& length_, int& element_);
		// 下限検索する
		ModSize lowerBound(ModSize location_, int& length_, int& element_);

		// 要素数
		ModSize getSize() const { return m_vecList.getSize(); }

	private:
		struct Data
		{
			Data() : loc(0), len(0) {}
		
			ModSize	loc;	// 位置情報
			int		len;	// 検索語長
		};

		typedef ModPair<Data,
						LocationListIterator::AutoPointer>	LocationPair;
		typedef ModVector<LocationPair>						LocationVector;

		// 要素の配列
		LocationVector m_vecList;

		// 現在の位置
		ModSize m_uiCurrentLocation;
		// 現在の長さ
		int m_iCurrentLength;
		// 現在の要素番号
		int m_iCurrentElement;
	};

	// ある一定量の検索結果をクラスタリングする
	bool clustering();
	// 粗いクラスタリング
	void doRoughClustering(ModSize from, ModSize requestedRoughClusterNum,
						   ModVector<ModPair<ModSize, ModSize> >& roughCluster);
	// 詳細クラスタリング
	void doClustering(ModVector<ModVector<FeatureSet*> >& dvmap,
					  ModSize from,
					  ModSize to,
					  Common::LargeVector<ModSize>& cluster_,
					  const Common::LargeVector<ModSize>& original_,
					  float thresh,
					  ModSize neighbor);
	// 内積を求める
	float innerProduct(ModVector<FeatureSet*>& f1,
					   ModVector<FeatureSet*>& f2);
	// 特徴語リストを設定する
	void setDocumentVectorMap(ModVector<ModVector<FeatureSet*> >& dvmap1,
							  const Common::LargeVector<ModSize>& clusterWork,
							  ModSize uiBegin_,
							  ModSize uiEnd_,
							  FeatureSetMap& dvmap);
	// 特徴語リストを得る
	void makeDocumentVector(ModSize clusterID,
							ModVector<FeatureSet*>& dv,
							FeatureSetMap& dvmap);
	// クラスタマップの設定
	void setClusterMap(ClusterMap& clusterMap,
					   const Common::LargeVector<ModSize>& cluster_,
					   const Common::LargeVector<ModSize>& original_,
					   ModSize uiBegin_,
					   ModSize uiEnd_,
					   ModInt32& clusterno) const;

	// クラスタ内をソートする
	void sortEachCluster(
		SortKey::Value eKey_, Order::Value eOrder_,
		Common::LargeVector<ClusterElement>& vecClusterElement_);
	// クラスタ単位でソートする
	void sortByCluster(
		SortKey::Value eKey_, Order::Value eOrder_,
		Common::LargeVector<ClusterElement>& vecClusterElement_);

	// 検索ノードをリセットする
	void resetNode();
	// TFリストを取得する
	void getTfList(DocumentID id_, ModVector<ModSize>& tfList_);
	// EXISTENCEリストを取得する
	void getExistenceList(DocumentID id_, ModVector<int>& extList_);
	// セクション検索を実施する
	void getSection(int n_, DocumentID id_, ModVector<ModSize>& section_);
	// 粗いKWIC情報を取得する
	int getRoughKwic(int n_, DocumentID id_);

	// 位置情報へのイテレータを得る
	bool getLocationListIterator(int n_, DocumentID id_,
								 MyLocationListIterator& ite_);

	// ビットセットで取得する
	bool getByBitSet(IDVectorFile* pDocIDVectorFile_,
					 Common::DataArrayData& cTuple_);

	// 取得するデータ
	ModVector<FieldData> m_vecProjections;
	
	// 文書IDとスコア -- 文書IDのみの場合もこの配列を使う
	SimpleResultSet m_vecDocIDScore;
	// クラスターID
	Common::LargeVector<ModSize> m_vecClusterID;
	// 直前に返した文書ID
	DocumentID m_uiCurrentID;
	// 直前に反したクラスターID
	ModSize m_uiCurrentClusterID;

	// 検索結果集合のカーソル位置
	SimpleResultSet::Iterator m_ite;
	// 検索結果集合の開始位置
	SimpleResultSet::Iterator m_begin;
	// 検索結果集合の終端位置
	SimpleResultSet::Iterator m_end;

	// クラスタリング済みの検索結果数
	ModSize m_uiClusteredSize;	// uiPos_
	// クラスタリングの閾値
	float m_fGlobalClusteredLimit;
	float m_fLocalClusteredLimit;
	// 一度に取得するラフクラスタの最大個数
	ModSize m_uiMaxRoughClusterCount;	// requestSize
	// 詳細クラスタのマージ対象数
	ModSize m_uiNeighbor;
	// クラスタ対象フィールドの重み
	ModVector<ModPair<int, float> > m_vecClusterScale;
	// クラスタリングを行う必要があるかどうか
	bool m_bCluster;
	// クラスタ合成方法
	ClusterCombiner::Value m_eClusterCombiner;

	// 検索情報クラス
	SearchInformation& m_cSearchInfo;
	// TFリストのための検索語ノード
	ModVector<OperatorTermNode*> m_vecTfList;
	
	// 粗いKWICのサイズ
	ModVector<ModSize> m_vecKwicSize;
	// KWICで返す長さを入力値の何倍にするか
	ModVector<ModSize> m_vecKwicMarginScaleFactor;
	// 位置情報取得のための検索語ノード
	ModVector<Query::TermNodeMap> m_vecMapTermLeafNode;

	// ビットセットで取得するかどうか
	bool m_bGetByBitSet;

	// 取得数
	ModSize m_uiLimit;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_RESULTSET_H

//
//	Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
