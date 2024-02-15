// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SearchResultSet.h --
// 
// Copyright (c) 2007, 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_INVERTED_SEARCHRESULTSET_H
#define __SYDNEY_INVERTED_SEARCHRESULTSET_H

#include "SyDefault.h"
#include "SyInclude.h"
	
#include "LogicalFile/FileID.h"

#include "Inverted/FeatureSet.h"
#include "Inverted/Sign.h"	
#include "Inverted/IntermediateFileID.h"
#include "Inverted/IndexFile.h"
#include "Inverted/IndexFileSet.h"
#include "Inverted/OptionDataFile.h"
#include "Inverted/SortParameter.h"

#include "LogicalFile/OpenOption.h"

#include "Common/WordData.h"
#include "Common/Assert.h"
	
#include "ModVector.h"
#include "ModPair.h"
#include "ModUnicodeString.h"
#include "ModInvertedSearchResult.h"

_SYDNEY_BEGIN
_SYDNEY_INVERTED_BEGIN

//
//  CLASS
//  Inverted::SearchResultSet -- 検索結果集合クラス
//
//  NOTES
//	Vectorをメンバ変数に持つクラスではなく、Vectorを継承したクラス。
//	Vectorの要素は、ModPair。
//	第一要素のModUInt32には、Inverted::Signが設定され
//	第二要素のModInvertedSearchResultには、検索結果が格納される。
//
class SearchResultSet
	: public ModVector<ModPair<ModUInt32, ModInvertedSearchResult*> >
{
	typedef ModMap<ModInt32, ModVector<ModUInt32>, ModLess<ModInt32> > ClusterMap;
	typedef ModVector<ModPair<ModUInt32, ModInvertedBooleanResult> > ExpungedIDSet;
	typedef ModPair<ModUInt32, ModInvertedBooleanResult> Element;
	typedef ModVector<ModPair<ModUInt32, ModInvertedSearchResult*> > Super;

	ExpungedIDSet expungedID;
	ModInvertedBooleanResult ExpungeZero;
	
public:
	typedef Super::Iterator Iterator;

	typedef ModVector<ModInt32> ClusterIDList;
	// クラスタの開始位置と終了位置のペア
	typedef ModPair<ModSize, ModSize> ClusterRangeElement;
	typedef ModVector<ClusterRangeElement> ClusterRangeList;
	
	SearchResultSet();
	SearchResultSet(IndexFileSet* pIndexFileSet, const ModUInt32 resultType);
	virtual ~SearchResultSet();
	void set(IndexFileSet* pIndexFileSet, const ModUInt32 resultType);
	// 検索結果集合のクリア
	void clear();
	// 指定検索結果集合のクリア
	void clear(ModUInt32 mask);
	
	// 検索結果集合の合成
	void compose(IndexFileSet* pIndexFileSet,
				 ModInvertedSearchResult*& resultSet,
				 ModSize limit,
				 SortParameter::Value eSort_);

	// 検索結果のクラスタリング
	bool getCluster(OptionDataFile* pOptionDataFile_,
					ModInvertedSearchResult* resultSet,
					ClusterIDList& cluster_,
					float thresh,
					ModSize uiLimit_,
					ModSize& uiClusterCount_,
					ModSize& uiPos_,
					bool bPhasedClustering_);

	// 文書IDをrowidに変換する
	SortParameter::Value
	convertToRowID(IndexFileSet::Iterator pFile,
					ModInvertedSearchResult *result);
	
	void convertDocumentID(InvertedFile* pExpungeFile_,
						InvertedFile* pFile_, 
						ModInvertedSearchResult* result_,
						ModInvertedSearchResult& newResult_);

	ModInvertedBooleanResult *getExpungedDoc(IndexFileSet *pIndexFileSet,
											ModUInt32 sig_);

	bool isEmpty();

	ModInvertedSearchResult* get(ModUInt32 sig_);

	void set(ModUInt32 sig_,ModInvertedSearchResult* result_);
	// 第一引数:TFを取得する場合には、内部型(TFのスカラー型)
	// 第二引数:TFを取得する場合には、TFのリスト型
	void intersection(SearchResultSet& y,SearchResultSet& z,int wno,
					  bool bFirst_);
	// 第一引数:TFを取得する場合には、内部型(TFのスカラー型)
	// 第二引数:TFを取得する場合には、TFのリスト型
	void merge(SearchResultSet& y,SearchResultSet& z, int wno);

	void add(ModUInt32 sig_,
			ModInvertedSearchResult* result_,
			ModInt32 queryTermNo);
	
private:
	// TFありの検索結果集合
	void add_With_Tf(ModUInt32 sig_,
					ModInvertedSearchResult* result_,
					ModInt32 queryTermNo);
	// TFなしの検索結果集合
	void add_Without_Tf(ModUInt32 sig_,
						ModInvertedSearchResult* result_,
						ModInt32 queryTermNo);

	void compose(IndexFileSet* pIndexFileSet,
				 ModInvertedSearchResult*& resultSet);
	void compose(ModInvertedSearchResult*& resultSet,
				 ModInvertedSearchResult* result);
	
	void doRoughClustering(
		ModInvertedSearchResult* resultSet,
		unsigned int from,
		unsigned int requestedRoughClusterNum,
		ClusterRangeList& roughCluster);

	void doClustering(
		ModVector<FeatureSet*>& dvmap,
		unsigned int from,
		unsigned int to,
		ClusterIDList& cluster_,
		const ClusterIDList& original_,
		float thresh,
		unsigned int neighbor);

	void makeDocumentVector(
		OptionDataFile* pFile_,
		const ModInvertedSearchResult*resultSet_,
		ModUInt32 docId,
		FeatureSet*& dv,
		ModHashMap<ModUInt32, FeatureSetPointer, ModHasher<ModUInt32> >&
		dvmap);
	
	void setClusterParameter(float fGlobalClusteredLimit_,
							 float& fLocalClusteredLimit_,
							 ModSize& uiMaxRoughClusterCount_,
							 ModSize& uiNeighbor_);
	
	// クラスタの初期化
	void initializeClusterIDList(ClusterIDList& vecClusterID_,
								 ModSize uiSize_) const;
	
	// 特徴語ベクトルマップの設定
	void setDocumentVectorMap(
		ModVector<FeatureSet*>& dvmap1,
		const ClusterIDList& clusterWork,
		ModSize uiBegin_,
		ModSize uiEnd_,
		OptionDataFile* pOptionDataFile_,
		const ModInvertedSearchResult* resultSet_,
		ModHashMap<ModUInt32, FeatureSetPointer, ModHasher<ModUInt32> >& dvmap);
	void setDocumentVectorMap(
		ModVector<FeatureSet*>& dvmap1,
		const ClusterIDList& clusterWork,
		OptionDataFile* pOptionDataFile_,
		const ModInvertedSearchResult* resultSet_,
		ModHashMap<ModUInt32, FeatureSetPointer, ModHasher<ModUInt32> >& dvmap)
	{
		setDocumentVectorMap(dvmap1, clusterWork,
							 0, clusterWork.getSize() - 1,
							 pOptionDataFile_, resultSet_, dvmap);
	}


	// クラスタマップの設定
	void setClusterMap(ClusterMap& clusterMap,
					   const ClusterIDList& cluster_,
					   const ClusterIDList& original_,
					   ModSize uiBegin_,
					   ModSize uiEnd_,
					   ModInt32& iPrevClusterID_) const;
	void setClusterMap(ClusterMap& clusterMap,
					   const ClusterIDList& cluster_,
					   const ClusterIDList& original_) const
	{
		ModInt32 iPrevClusterID = -1;
		setClusterMap(clusterMap, cluster_, original_,
					  0, cluster_.getSize() - 1,
					  iPrevClusterID);
	}
	
	
	void (SearchResultSet::*_add)(ModUInt32 sig_,
								ModInvertedSearchResult* result_,
								ModInt32 queryTermNo);
};

_SYDNEY_INVERTED_END
_SYDNEY_END

#endif //__SYDNEY_INVERTED_SEARCHRESULTSET_H

//
//  Copyright (c) 2007, 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
//  All rights reserved.
//
