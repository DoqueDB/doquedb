// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SearchResultSet.cpp --
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Inverted";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Common/Object.h"

#include "Inverted/Sign.h"
#include "Inverted/SearchResultSet.h"
#include "Inverted/Parameter.h"

#include "Exception/BadArgument.h"

#include "ModMap.h"
#include "ModHashMap.h"
#include "ModPair.h"
	
_SYDNEY_USING
_SYDNEY_INVERTED_USING

namespace {
	//
	//	CONST
	//	_$$::_Neighbor -- クラスターのマージ対象数のデフォルト
	//
	const int _DefaultNeighbor = 10;
	
	//
	//	VARIABLE local
	//	_$$::_Neighbor -- クラスターのマージ対象数
	//
	ParameterInteger _cNeighbor(
		"Inverted_MergeClusterDistance", _DefaultNeighbor);
	
	//
	//	CONST
	//	_$$::_MaxRoughClusterCount
	//		-- 一度に取得するラフクラスタの最大個数のデフォルト
	//
	const int _DefaultMaxRoughClusterCount = 100;
	
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
	ParameterInteger _MaxRoughClusterCount(
		"Inverted_MaxRoughClusterCount", _DefaultMaxRoughClusterCount);
	
	//
	//	VARIABLE local
	//	_$$::_LocalClusteredLimit -- ラフクラスタ内の詳細クラスタリングの閾値
	//
	ParameterString _LocalClusteredLimit("Inverted_LocalClusteredLimit", "");
}

/////////////////////////////////////
// 検索結果集合クラス
//

//
//	FUNCTION public
//	Inverted::SearchResultSet::SearchResultSet
//		-- コンストラクタ(引数なし)
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
SearchResultSet::SearchResultSet()
{
}

//
//	FUNCTION public
//	Inverted::SearchResultSet::SearchResultSet
//		-- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	Inverted::IndexFileSet* pIndexFileSet
//		転置ファイルセット
//	const ModUInt32 resultType
//		結果集合タイプ
//		上位に返す構造をもつ（内部型ではない）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
SearchResultSet::SearchResultSet(IndexFileSet* pIndexFileSet,
								 const ModUInt32 resultType)
{
	set(pIndexFileSet, resultType);
}

//
//	FUNCTION public
//	Inverted::SearchResultSet::SearchResultSet
//		-- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	Inverted::IndexFileSet* pIndexFileSet
//		転置ファイルセット
//	const ModUInt32 resultType
//		結果集合タイプ
//		上位に返す構造をもつ（内部型ではない）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
SearchResultSet::~SearchResultSet()
{
	for (ModVector<ModPair<ModUInt32, ModInvertedSearchResult*> >::Iterator 
		iter = begin(); iter != end(); ++iter)
		delete (*iter).second;
	Super::clear();
}

//
//	FUNCTION public
//	Inverted::SearchResultSet::set -- 
//
//	NOTES
//
//	ARGUMENTS
//	Inverted::IndexFileSet* pIndexFileSet
//		転置ファイルセット
//	const ModUInt32 resultType
//		結果集合タイプ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SearchResultSet::set(IndexFileSet* pIndexFileSet,
					 const ModUInt32 resultType)
{
	// [NOTE] コンストラクタからも呼ばれる
	
	// [NOTE] このオブジェクトはFullTextには返さないので、内部型の場合もある。
	
	// [NOTE] 転置ファイル１つにつき、１つの検索結果格納領域を用意する
	// ただし、削除転置ファイルの検索結果領域は文書番号を格納する領域になる
	
	ModUInt32 booleanType;
	booleanType = 1 <<_SYDNEY::Inverted::FieldType::Rowid;
	for (IndexFileSet::Iterator iter = pIndexFileSet->begin() ; 
		 iter != pIndexFileSet->end(); ++iter)
	{
		if ((*iter).signature() & Inverted::Sign::DELETE_MASK)
		{
			this->pushBack(ModPair<ModUInt32, ModInvertedSearchResult*>
				((*iter).signature(),
				 ModInvertedSearchResult::factory(booleanType)));
		}
		else
		{
			if ((*iter).getCount() == 0)
				continue;
			this->pushBack(ModPair<ModUInt32, ModInvertedSearchResult*>
					((*iter).signature(),
					 ModInvertedSearchResult::factory(resultType)));

			if (resultType & ( 1 << _SYDNEY::Inverted::FieldType::Tf))
			{
				_add = &SearchResultSet::add_With_Tf;	
			}
			else
			{
				_add = &SearchResultSet::add_Without_Tf;
			}
		}
	}
}

//
//	FUNCTION public
//	Inverted::SearchResultSet::clear -- 検索結果集合のクリア
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
SearchResultSet::clear()
{
	// [NOTE] 各要素はクリアするが、自身はクリアしない
	
	for (ModVector<ModPair<ModUInt32, ModInvertedSearchResult*> >::Iterator
		iter = begin(); iter != end(); ++iter)
	{
		(*iter).second->clear();
	}
}

//
//	FUNCTION public
//	Inverted::SearchResultSet::clear -- 指定検索結果集合のクリア
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
SearchResultSet::clear(ModUInt32 mask)
{
	for(ModVector<ModPair<ModUInt32, ModInvertedSearchResult*> >::Iterator
		iter = begin(); iter != end(); ++iter)
	{
		if ((*iter).first & mask)
			(*iter).second->clear();
	}
}

//
//	FUNCTION private
//	Inverted::SearchResultSet::compose -- まじめな検索結果集合の合成
//
//	NOTES
//
//	ARGUMENTS
//	IndexFileSet* pIndexFileSet,
//	ModInvertedSearchResult*& resultSet
//		合成結果の格納先
//		上位に返す構造を持つ（内部型ではない）
//	
//	RETURN
//
//	EXCEPTIONS
//　
void
SearchResultSet::compose(IndexFileSet* pIndexFileSet,
						 ModInvertedSearchResult*& resultSet)
{
	; _SYDNEY_ASSERT(
		!(resultSet->getType() & (1 << _SYDNEY::Inverted::FieldType::Internal)));
	
	for (Iterator iter = begin(); iter != end(); ++iter)
	{
		// 各索引ファイルの実行結果を順番に処理
		ModInvertedSearchResult *result = (*iter).second;
		
		if (result->getSize() > 0 &&
			((*iter).first & Inverted::Sign::INSERT_MASK))
		{
			// 実行結果が、空ではなく挿入索引の場合
			
			// 検索結果をrowidの昇順にそろえてから検索結果を合成
			
			// 文書IDをrowidに変換
			// 絶対にpFileは見つかるのでNULL検査は必要ない
			IndexFileSet::Iterator pFile = pIndexFileSet->find((*iter).first);
			SortParameter::Value eSort = convertToRowID(pFile,result);
			
			// ソート
			if(eSort != SortParameter::RowIdAsc)
				result->sort(SortParameter::RowIdAsc);

			// resultSetにresultを合成
			compose(resultSet, result);
		}
	}
}

//
//	FUNCTION public
//	Inverted::SearchResultSet::compose
//		-- 打ち切り(limit句の解釈)を伴う検索結果集合の合成
//
//	NOTES
//	offset分だけlimitを後ろにずらすので、offset句は考慮しなくとも良い
//	SearchCapsuleにlimit値を渡すときに、limit <- offset + limitとしている。
//
//	ARGUMENTS
//	IndexFileSet* pIndexFileSet,
//	ModInvertedSearchResult*& resultSet
//		合成結果の格納先
//		上位に返す構造を持っている（内部型ではない）
//	ModSize limit,
//		0以上で呼ばれる必要がある!
//	SortParameter::Value eSort_
//	
//	RETURN
//
//	EXCEPTIONS
//　
void 
SearchResultSet::compose(IndexFileSet* pIndexFileSet,
						 ModInvertedSearchResult*& resultSet,
						 ModSize limit,
						 SortParameter::Value eSort_)
{
	; _SYDNEY_ASSERT(
		!(resultSet->getType() & (1 << _SYDNEY::Inverted::FieldType::Internal)));
	; _SYDNEY_ASSERT(limit >= 0);

	// 各索引ファイルの検索結果を、一つに合成する。

	if (limit == 0)
	{
		// 文書クラスタリングが指定された場合には、
		// Inverted::SearchResultSet::composeでlimitの打ち切り処理をしない
		// （当面の仕様）
		// limitの解釈は、Inverted::SearchResultSet::getClusterで行う。
		// ただし、この仕様は解消可能
		// limitが指定された場合に、resultから検索結果をerasesしないかわりに、
		// rowidに変更する要素をlimitまでにすればよい
		
		compose(pIndexFileSet, resultSet);
	}
	else
	{			
		if (eSort_ == SortParameter::None)
			eSort_ = SortParameter::RowIdAsc;
		
		//
		// 要素数が最大の検索結果集合を調査
		//
		
		Iterator max_iter;
		ModSize max = 0;
		for (Iterator iter = begin(); iter != end(); ++iter)
		{
			ModInvertedSearchResult *result = (*iter).second;
			if(result->getSize() > 0 && 
				((*iter).first & Inverted::Sign::INSERT_MASK))
			{
				// この時点で、削除文書が検索結果集合から削除されているので、
				// 挿入転置ファイルの検索結果集合を見ればよい。
				if(result->getSize() > max)
				{
					max = result->getSize();
					max_iter = iter;
				}
			}
		}

		if(max == 0)
			return;	// 検索結果が０件

		// 最大の検索結果集合を設定
		ModInvertedSearchResult *result = (*max_iter).second;
		IndexFileSet::Iterator pFile = pIndexFileSet->find((*max_iter).first);
		
		if (this->getSize() == 1)
		{
			// 挿入小転置ファイルがない場合
			
			if (eSort_ == SortParameter::ScoreAsc ||
				eSort_ == SortParameter::ScoreDesc)
			{	// 文書スコアでソート
				result->sort(eSort_);				
				result->erase(limit,result->getSize());
				convertToRowID(pFile,result);
			}
			else if (eSort_ == SortParameter::RowIdAsc ||
					 eSort_ == SortParameter::RowIdDesc)		
			{
				// 文書idでソート
				SortParameter::Value eSort = convertToRowID(pFile,result);
				if (eSort_ == SortParameter::RowIdAsc)
				{
					if (eSort == SortParameter::None)
						result->sort(SortParameter::RowIdAsc);
				}
				else
				{
					result->sort(eSort_);
				}
				result->erase(limit, result->getSize());
			}
			else
			{
				convertToRowID(pFile, result);
			}
		}
		else
		{
			// この時点で、resultは、最大要素数の検索結果集合を指す
			// 検索結果集合が２つになるまで検索結果を合成し、resultSetに入れる
			// resultSetの検索結果集合の文書idはrowidに変換しておく
			// 絶対にpFileは見つかるのでNULL検査は必要ない
			for (Iterator iter = begin(); iter != end(); ++iter)
			{
				if ((*iter).first & Inverted::Sign::INSERT_MASK)
				{
					if (iter == max_iter)
						continue;	// 要素数が最大の検索結果集合は、スキップ
					ModInvertedSearchResult* result = (*iter).second;
					if (result->getSize() > 0)
					{
						IndexFileSet::Iterator pFile
							= pIndexFileSet->find((*iter).first);
						// 1. 文書IDをrowidに変換する
						convertToRowID(pFile,result);
						if (eSort_ == SortParameter::None)
						{
							result->sort(SortParameter::RowIdAsc);
						}
						// 2. 検索結果集合の合成
						compose(resultSet,result);
					}
				}
				// resultSetはrowidに変更済み
			}
			
			// ここで、resultSetのサイズのほうが、
			// resultより大きくなっている可能性があるが、
			// すでにresultSetにおいてrowidへの変換処理が終わっているので
			// 無視する。
			// ∵そのような場合を考慮しても処理は高速化されない

			// この時点でresultには、最大検索結果集合、
			// resultSetにはその他の検索結果集合を合成したものが設定されている
			// 以下の処理で、resultとresultSetを合成する
			// sort keyが文書idかスコアかにより処理が分かれる
			// この時点でresultSetはrowidに変更済み

			if (eSort_ == SortParameter::RowIdAsc ||
				eSort_ == SortParameter::RowIdDesc)
			{
				// ソートキーが文書番号
				// 検索結果集合の打ち切りの場所を探す
				// resultのlimit番目以降を削除しても
				// resultとresultSetを合成した結果は、
				// 検索集合の件数が増えるだけなので問題ない
				// 
				// 最大の検索結果集合をlimitで切る
				if (eSort_ == SortParameter::RowIdDesc)
				{
					// 文書番号の降順ソート（まずありえないケース）
					// resultSetは文書番号に関して昇順にソートされているので
					// resultの先頭から　result->getSize() - limit/* + 1*/ 分
					// だけ要素を削除する
					if (result->getSize() > limit/* - 1*/)
						result->erase(0,result->getSize() - limit/* + 1*/);
				}
				else
				{
					// 文書番号の昇順ソート（通常ケース）の場合は、
					// limitで指定されたところから要素の最後までを削除する
					if (result->getSize() > limit)
						result->erase(limit,result->getSize());
				}
			}
			else if (eSort_ == SortParameter::ScoreAsc ||
					 eSort_ == SortParameter::ScoreDesc)
			{
				// ソートキーがスコア
				// 大挿入転置ファイルと小挿入転置ファイルのROWIDは
				// 一致することはないことに注意
				
				result->sort(SortParameter::ScoreDesc);
				if (eSort_ == SortParameter::ScoreDesc)
				{
					if (result->getSize() > limit)
						result->erase(limit, result->getSize());
				}
				else
				{
					// スコア昇順のケース
					// スコアが加算されると順位が下がるので、
					// resultの先頭から　result->getSize() - limit/* + 1*/ 分
					// だけ要素を削除すれば良い
					
					if (result->getSize() > limit/* - 1*/)
						result->erase(0, result->getSize() - limit/* + 1*/);	
				}
			}
			
			// resultは文書IDのままなので、文書IDをrowidに変換する
			// limitでresultのサイズを小さくしているので、
			// convertToRowIDを呼ぶ回数が減り、処理の高速化につながる
			
			convertToRowID(pFile, result);
			result->sort(SortParameter::RowIdAsc);
		}
		//**********************
		// 最後の合成処理
		//**********************
		// resultSetにresultを合成する
		compose(resultSet, result);
	}
	
	if (eSort_ != SortParameter::RowIdAsc)
		resultSet->sort(eSort_);
}

//
//	FUNCTION public
//	Inverted::SearchResultSet::getCluster
//		-- 検索結果のクラスタリングを実施する
//
//	NOTES
//  limitの処理で、cluster_とresultSet_の余計な部分を削除する。
//  クラスタリング以外のlimit処理はcompose()を参照。
//
//	ARGUMENTS
//	OptionDataFile* pOptionDataFile_,
//	ModInvertedSearchResult* resultSet_,
//		クラスタ毎にまとめられた検索結果の格納先
//		上位に返す構造を持っている（内部型ではない）
//	ClusterIDList& cluster_,
//		クラスタIDの格納先
//		initializeClusterIDList() を参照
//	float thresh_,
//		クラスタ閾値
//	ModSize uiLimit_
//		0は制限なしを意味する
//	ModSize& uiClusterCount_
//		取得済みクラスタ数
//	ModSize& uiPos_
//		クラスタリング開始位置(0-base)
//	bool bPhasedClustering_
//		全部をクラスタリングせず、段階的にクラスタリングする
//		【注意】
//		全部のデータが必要なくともuiLimit_=0で呼ばれることがあるので、
//		そういう時でも段階的クラスタリングを実行できるために本フラグは必要。
//
//	RETURN
//	bool
//
//	EXCEPTIONS
//
bool 
SearchResultSet::getCluster(OptionDataFile* pOptionDataFile_,
							ModInvertedSearchResult* resultSet_,
							ClusterIDList& cluster_,
							float thresh_,
							ModSize uiLimit_,
							ModSize& uiClusterCount_,
							ModSize& uiPos_,
							bool bPhasedClustering_)
{
	; _TRMEISTER_ASSERT(
		!(resultSet_->getType() &
		  (1 << _SYDNEY::Inverted::FieldType::Internal)));

	//
	// 前処理
	//

	// リミットの設定
	if (uiLimit_ == 0)
		uiLimit_ = ModSizeMax;

	// cluster_の初期化
	if (cluster_.getSize() == 0)
		initializeClusterIDList(cluster_, resultSet_->getSize());

	//
	// クラスタリング
	//

	if (resultSet_->getSize() - uiPos_ > 1)
	{
		// 未クラスタリング検索結果が2件以上ある場合

		// ラフ・クラスタ格納領域
		ClusterRangeList roughCluster;
		
		if (thresh_ > 0.0 && pOptionDataFile_->isFeatureValue())
		{
			//
			// 詳細クラスタの場合
			//

			float fLocalClusteredLimit = 0.0;
			ModSize requestSize = 0;
			ModSize neighbor = 0;
			setClusterParameter(thresh_, fLocalClusteredLimit,
								requestSize, neighbor);
			
			ModUInt32 resultType = resultSet_->getType();
			
			ModInvertedSearchResult* workSet
				= ModInvertedSearchResult::factory(resultType);
			
			// clusterに併合されている文書かどうかを区別するために、
			// 現在のcluster結果(先頭から昇順に番号が振られているもの)を
			// 保存しておく
			ClusterIDList original = cluster_;
			
			// もともとの検索結果をコピーする
			workSet->reserve(resultSet_->getSize());
			workSet->copy(resultSet_);
			
			ModVector<FeatureSet*> dvmap1;
			
			//
			// 精密クラスタリングの開始
			//
			
			do
			{
				// 精密クラスタがlimitに達するか、
				// 荒いクラスタで最後の文書までクラスタし終わるまで処理を繰り返す。
				// ただし、段階的クラスタリングの場合は、繰り返さない。
				
				// 取得した特徴語セットを保存しておく一時領域
				ModHashMap<ModUInt32, FeatureSetPointer,
					ModHasher<ModSize> > dvmap;

				//
				// 1. ラフ・クラスタリング
				//

				// ラフ・クラスタを最大でクラスタ数分(requestSize)作成
				roughCluster.clear();
				doRoughClustering(resultSet_, uiPos_, requestSize, roughCluster);

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
				ModInt32 clusterno = -1;
				for (ClusterRangeList::Iterator iter = roughCluster.begin();
					 iter != roughCluster.end(); ++iter)
				{
					// 各ラフクラスタを順番に処理

					// 現在参照中のラフクラスタリング内の各データの
					// ドキュメントベクタを作成する。
					setDocumentVectorMap(dvmap1, original,
										 (*iter).first, (*iter).second,
										 pOptionDataFile_, resultSet_, dvmap);

					// INT_MAXは、ラフクラスタリング内の全体を見て
					// 詳細クラスタリングすることを指示している。
					doClustering(dvmap1, (*iter).first, (*iter).second,
								 cluster_, original, fLocalClusteredLimit,
								 INT_MAX);

					setClusterMap(clusterMap, cluster_, original,
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

				ClusterIDList clusterWork;
				clusterWork.reserve(clusterMap.getSize());
				ClusterMap::Iterator cluster_iter = clusterMap.begin();
				for (; cluster_iter != clusterMap.end(); ++cluster_iter)
				{
					; _TRMEISTER_ASSERT(
						(*cluster_iter).first == *((*cluster_iter).second.begin()));
					
					// クラスタIDを格納
					clusterWork.pushBack((*cluster_iter).first);
				}

				//
				// 代表データの詳細クラスタリング(代表クラスタリング)
				//

				// 特徴語ベクトルを取得する
				setDocumentVectorMap(dvmap1, clusterWork,
									 pOptionDataFile_, resultSet_, dvmap);

				// dvmapのインデックスは、精密クラスタリングにより得られる
				// クラスタ番号
				// 速度重視のため、neighbor個先のclusterのみmerge対象となる。
				ClusterIDList workOriginal = clusterWork;
				doClustering(dvmap1, 0, clusterWork.getSize() - 1,
							 clusterWork, workOriginal, thresh_, neighbor);

				ClusterMap workMap;
				setClusterMap(workMap, clusterWork, workOriginal);
				
				// 
				// マージ
				//
				for (cluster_iter = workMap.begin();
					 cluster_iter != workMap.end(); ++cluster_iter)
				{
					// 各代表クラスタを順番に処理

					; _TRMEISTER_ASSERT(
						(*cluster_iter).first == *((*cluster_iter).second.begin()));
					
					ModUInt32 no = *((*cluster_iter).second.begin());
					for (ModVector<ModUInt32>::Iterator iter
							 = (*cluster_iter).second.begin() + 1;
						 iter != (*cluster_iter).second.end(); ++iter)
					{
						// 代表クラスタの各データを順番に処理

						// 代表クラスタの各データとは、
						// 代表データに対応する詳細クラスタのクラスタIDである。
						// 同じ代表クラスタに属する詳細クラスタを一つにマージする。

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

					for (ModVector<ModUInt32>::Iterator index_iter
							 = (*cluster_iter).second.begin();
						 index_iter != (*cluster_iter).second.end();
						 ++index_iter, ++uiPos_)
					{
						// クラスタの各データを順番に処理
						
						int j = *index_iter;
						cluster_[uiPos_] = (*cluster_iter).first;
						resultSet_->setDocID(uiPos_, workSet->getDocID(j));
						resultSet_->setScore(uiPos_, workSet->getScore(j));
						if (resultType & (1 <<_SYDNEY::Inverted::FieldType::Tf))
						{
							resultSet_->setTF(uiPos_, *workSet->getTF(j));
						}
					}
					
					if (++uiClusterCount_ >= uiLimit_)
					{
						// ここで cluster_ の参照しない部分を削除する
						cluster_.erase(cluster_.begin() + uiPos_, cluster_.end());
						resultSet_->erase(uiPos_, resultSet_->getSize());
						break;
					}
				}

				if (roughCluster.getSize() < requestSize)
					
					// 検索結果のクラスタリングが全て終わった場合

					// doRoughClustering()を参照。
					break;
			}
			while (bPhasedClustering_ == false
				   && (uiClusterCount_ < uiLimit_
					   || resultSet_->getSize() - uiPos_ > 1));
			// 段階的クラスタリングではなく、
			// クラスタ数の上限に達していない、または、
			// 未クラスタリングデータ数が2件以上ある
			
			delete workSet;

			//
			// 精密クラスタリングの終了
			//
		}
		else
		{
			//
			// 荒いクラスタリングの場合
			//
			
			// ランキング・スコア値だけをもとにしたクラスタリング
			// (精密クラスタリングは行わない)

			// クラスタリング
			doRoughClustering(resultSet_, uiPos_, uiLimit_, roughCluster);

			// 出力クラスタ数
			uiClusterCount_ = roughCluster.getSize();

			// 整形
			for (ClusterRangeList::Iterator iter = roughCluster.begin();
				 iter != roughCluster.end(); ++iter)
			{
				// 各クラスタを順番に処理
				
				for (uiPos_ = (*iter).first; uiPos_ <= (*iter).second; ++uiPos_)
				{
					// あるクラスタに含まれる文書を順番に処理

					// クラスタIDリストを設定
					cluster_[uiPos_] = iter - roughCluster.begin();
				}
			}
			
			// 出力しない部分を削除
			cluster_.erase(cluster_.begin() + uiPos_, cluster_.end());
			resultSet_->erase(uiPos_, resultSet_->getSize());
		}
	}
	else if (resultSet_->getSize() - uiPos_ == 1)
	{
		// 1件だけの場合
		++uiClusterCount_;
		++uiPos_;
	}

	// [NOTE] クラスタIDリスト数は、検索結果数と必ず等しい。
	; _TRMEISTER_ASSERT(resultSet_->getSize() == cluster_.getSize());

	// [YET] resultSet_->getSize() <= uiPos_ で、
	//  uiPos_のクラスタIDが取得できなくてもtrueを返す。
	return true;
}

//
//	FUNCTION private
//	Inverted::SearchResultSet::doRoughClustering -- 荒いクラスタリング
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
//	ModInvertedSearchResult* resultSet
//		クラスタリング対象の検索結果集合
//	unsigned int from
//		検索結果集合におけるクラスタリングの開始位置(0-base)
//		ちなみに終了位置は最後まで。
//	unsigned int requestedRoughClusterNum
//		生成するクラスタリング数
//	ClusterRangeList& roughCluster
//		クラスタリング結果の格納先
//		一つのClusterRangeElementが一つのクラスタに対応している。
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
SearchResultSet::
doRoughClustering(ModInvertedSearchResult* resultSet,
				  unsigned int from,
				  unsigned int requestedRoughClusterNum,
				  ClusterRangeList& roughCluster)
{
	// クラスタの終了位置
	ModSize i;
	// クラスタの開始位置
	ModSize j = from;
	// クラスタリング対象の総数
	ModSize uiSize = resultSet->getSize();
	; _TRMEISTER_ASSERT(resultSet->getSize() > 1);
	// 平均変化率の調査単位
	ModSize bias  = 1024;
	// 平均変化率の調査開始位置(0-base)
	ModSize uiStart;
	// 平均変化率の調査終了位置(0-base)
	ModSize uiEnd = from;
	// 平均変化率
	ModInvertedDocumentScore D = 0;
	
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
			; _TRMEISTER_ASSERT(
				resultSet->getScore(uiStart) >= resultSet->getScore(uiEnd));
			D = (resultSet->getScore(uiStart) - resultSet->getScore(uiEnd))
				/ (uiEnd - uiStart);
			
			// scoreに変化が現れるまで検索結果のscoreを観察する
		} while (D == 0.0);

		//
		// クラスタリング
		//
		for (i = uiStart; i < uiEnd; ++i)
		{
			if (resultSet->getScore(i) - resultSet->getScore(i + 1) > D)
			{
				// scoreが平均変化率を超える場合

				// [j,i] を新たなクラスタにする
				roughCluster.pushBack(ClusterRangeElement(j, i));
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
			roughCluster.pushBack(ClusterRangeElement(j,i));
			break;
		}
		
		// requestedRoughClusterNumに到達するまでループ
	}
	while(roughCluster.getSize() < requestedRoughClusterNum);
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
//	ModVector<FeatureSet*>& dvmap
//		クラスタリング対象データの特徴語集合のリスト
//	unsigned int from
//		詳細クラスタリングの開始位置
//	unsigned int to
//		詳細クラスタリングの終了位置
//	ClusterIDList& cluster_
//		クラスタリング結果の格納先
//	const ClusterIDList& original_
//		クラスタリング前のクラスタIDの初期値リスト
//	float thresh
//		同一クラスタかどうかを判定する閾値
//		閾値を超える場合、同一クラスタとみなす。
//	unsigned int neighbor
//		内積を取る範囲
//		fromとtoで指定された全てのデータと内積をとるわけではない。
//
//	RETURN
//
//	EXCEPTIONS
// 
void
SearchResultSet::
doClustering(ModVector<FeatureSet*>& dvmap,
			 unsigned int from,
			 unsigned int to,
			 ClusterIDList& cluster_,
			 const ClusterIDList& original_,
			 float thresh,
			 unsigned int neighbor)
{
	// 検査対象のイテレータ
	ClusterIDList::Iterator ci = cluster_.begin() + from;
	// 検査対象の最後のイテレータ
	const ClusterIDList::Iterator cie = cluster_.begin() + to;
	// 検査対象のクラスタIDの初期値を取得するイテレータ
	ClusterIDList::ConstIterator oi = original_.begin() + from;
	// 検索対象の特徴語集合へのイテレータ
	ModVector<FeatureSet*>::Iterator fi = dvmap.begin();
	
	for (; ci <= cie; ++ci, ++oi, ++fi)
	{
		// 各データを順番に処理
		
		if (*ci != *oi)
			// 参照中のデータのクラスタIDが初期値と異なる場合
			
			// すでにどこかのclusterに併合されている文書に関しては
			// 内積をとらない
			continue;

		// 内積対象のイテレータ
		ClusterIDList::Iterator cj = ci;
		// 内積対象の最後のイテレータ
		ClusterIDList::Iterator cje;
		if (static_cast<unsigned int>(cie - ci) > neighbor)
			// 残りのクラスタ数がneigbor個より多い場合
			
			// neighbor個先のclusterしか見ない
			cje = ci + neighbor;
		else
			cje = cie;
		// 内積対象のクラスタIDの初期値を取得するイテレータ
		ClusterIDList::ConstIterator oj = oi;
		// 内積対象の特徴語集合へのイテレータ
		ModVector<FeatureSet*>::Iterator fj = fi;

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
			
			if ((*fi)->innerProduct(*(*fj)) > thresh)
			{
				// 閾値を超える場合
				
				// 内積対象のデータを同じクラスタにする
				*cj = *ci;
			}
		}
	}
}

///////////////////////////////////////////
// Document filtering support functions
///////////////////////////////////////////
// docIdの正規化されたdocument vectorを作成する

void 
SearchResultSet::makeDocumentVector(
	OptionDataFile* pFile_,
	const ModInvertedSearchResult* resultSet_,
	ModUInt32 docId,
	FeatureSet*& dv,
	ModHashMap<ModUInt32, FeatureSetPointer, ModHasher<ModUInt32> >& dvmap)

{
	ModUInt32 rowid = resultSet_->getDocID(docId);
	FeatureSetPointer& p = dvmap[rowid];
	if (p.get() == 0)
	{
		// まだ一度も取得していないので、
		// ファイルから特徴語を取得する
		if (pFile_->getFeatureValue(rowid, p) == false)
		{
			// 特徴語が格納されていない
			_TRMEISTER_THROW0(Exception::BadArgument);
		}
	}
	
	dv = p.get();
}

//
//	FUNCTION private
//	Inverted::SearchResultSet::compose
//		-- composeの下請け関数
//
//	NOTES
//	集合演算関数setUnion()を呼び、検索結果集合同士の和集合を求める
//
//	ARGUMENTS
//	ModInvertedSearchResult*& resultSet
//		合成結果の格納先
//		上位に返す構造を持つ（内部型ではない）
//	ModInvertedSearchResult* result
//	
//	RETURN
//
//	EXCEPTIONS
//　
void
SearchResultSet::compose(ModInvertedSearchResult*& resultSet,
						 ModInvertedSearchResult* result)
{
	; _SYDNEY_ASSERT(
		!(resultSet->getType() & (1 << _SYDNEY::Inverted::FieldType::Internal)));
	
	// resultSetを、resultSetとresultの和集合で置き換える。

	// [YET] addとの使い分け
	
	if (resultSet->getType() & (1 << _SYDNEY::Inverted::FieldType::Internal) ||
		resultSet->getSize() > 0)
	{
		// resultSetが空ではない場合

		// [YET] resultSetは常に非内部型のはず。

		// resultSet + result -> x
		ModInvertedSearchResult *x = result->create();
		resultSet->setUnion(result,x);
		// x -> resultSet
		delete resultSet;
		resultSet = x;
	}
	else
	{
		// resultSetが空の場合

		// resultをresultSetにコピーするだけ
		resultSet->copy(result);
	}
}

// rowidからdocumentIDを求める
void
SearchResultSet::convertDocumentID(InvertedFile* pExpungeFile_,
								   InvertedFile* pFile_, 
								   ModInvertedSearchResult* result_,
								   ModInvertedSearchResult& newResult_)
{
	if (pExpungeFile_ && pFile_ && pExpungeFile_->getCount() > 0)
	{
		for (ModSize i = 0; i < result_->getSize(); ++i)
		{
			ModUInt32 rowid;
			ModUInt32 id = pFile_->convertToDocumentID(
				rowid = result_->getDocID(i));
			if (id != UndefinedDocumentID)
				newResult_.pushBack(id);
		}
	}
}

//
//	FUNCTION public
//	Inverted::SearchResultSet::convertToRowID -- 文書IDをrowidに変換する
//
//	NOTES
//	データ長の取得も同時に実行する
//	変換したrowidが昇順かどうかも検査し、結果を戻り値として返す
//
//	ARGUMENTS
//	IndexFileSet::Iterator pFile
//	ModInvertedSearchResult* result
//
//	RETURN
//	SortParameter::Value
//
//	EXCEPTIONS
//
SortParameter::Value
SearchResultSet::convertToRowID(IndexFileSet::Iterator pFile,
								ModInvertedSearchResult* result)
{
	bool bSorted = true;
	ModUInt32 previd = 0,rowid;
	ModUInt32 uiDocumentLength = 0;
	ModSize i;
	for (i = 0; i < result->getSize(); ++i)
	{
		rowid = pFile->convertToRowID(result->getDocID(i));

		if (rowid != UndefinedRowID)
		{
			result->setDocID(i,rowid);
			bSorted = previd <= rowid;
			previd = rowid;
			if(bSorted == false)
			{
				++i;
				break;
			}
		}
		else
		{
			result->erase(i), --i;
		}
	}
	for (; i < result->getSize(); ++i)
	{
		rowid = pFile->convertToRowID(result->getDocID(i));
		if (rowid != UndefinedRowID)
		{
			result->setDocID(i,rowid);
		}
		else
		{
			result->erase(i), --i;
		}
	}
	return bSorted ? SortParameter::RowIdAsc : SortParameter::None;
}

bool
SearchResultSet::isEmpty()
{
	for (ModVector<ModPair<ModUInt32, ModInvertedSearchResult*> >::Iterator
			iter = begin();	iter != end(); ++iter)
	{
		if ((*iter).first & Inverted::Sign::INSERT_MASK)
		{
			if ((*iter).second->getSize() > 0)
				return false;
		}
	}
	return true;
}

ModInvertedSearchResult	*
SearchResultSet::get(ModUInt32 sig_)
{
	for (ModVector<ModPair<ModUInt32, ModInvertedSearchResult*> >::Iterator
			iter = begin(); iter != end(); ++iter)
	{
		if ((*iter).first== sig_)
			return (*iter).second;
	}
	return NULL;
}

void
SearchResultSet::set(ModUInt32 sig_, ModInvertedSearchResult* result_)
{
	for (ModVector<ModPair<ModUInt32, ModInvertedSearchResult*> >::Iterator
			iter = begin();	iter != end(); ++iter)
	{
		if ((*iter).first== sig_)
		{
			(*iter).second = result_;
			break;
		}
	}
}

//
//	FUNCTION public
//	Inverted::SearchResultSet::intersection
//		-- 共通集合を返す
//
//	NOTES
//	自然文検索で使われる
//
//	ARGUMENTS
//	SearchResultSet& y
//		内部型の場合もある
//	SearchResultSet& z
//		上位に返す構造をもつ（内部型ではない）
//	int wno
//		検索文に含まれる検索語の通し番号
//	bool bFirst_
//		一個目の条件のintersectionかどうか
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SearchResultSet::intersection(SearchResultSet&y, SearchResultSet& z, int wno,
							  bool bFirst_)
{
	// y と z の共通集合を z に設定

	ModBoolean bFirst = (bFirst_ == true) ? ModTrue : ModFalse;
	
	for (ModVector<ModPair<ModUInt32, ModInvertedSearchResult*> >::Iterator
			iter = begin();	iter != end(); ++iter)
	{
		// 各索引ファイルを順番に処理
		
		if ((*iter).first & Inverted::Sign::INSERT_MASK)
		{
			// 挿入転置の場合

			// [YET] 削除転置は無視
			
			ModInvertedSearchResult* y_ = y.get((*iter).first);
			if (y_->getSize() > 0)
			{
				// x <-> z_
				ModInvertedSearchResult* z_ = z.get((*iter).first);
				; _TRMEISTER_ASSERT(
					!(z_->getType() & (1 << _SYDNEY::Inverted::FieldType::Internal)));
				ModInvertedSearchResult* t = z_;
				z_ = (*iter).second;
				z.set((*iter).first, z_);
				(*iter).second = t;
				// x(元z) and y_ -> z_(元x)
				(*iter).second->setIntersection(y_, z_, wno, bFirst);
			}
			else
			{
				// y が空の場合
				
				// y_->getSize() == 0の時もintersectionすべき
				z.get((*iter).first)->clear();
			}
		}
	}
}

//
//	FUNCTION public
//	Inverted::SearchResultSet::merge
//		-- マージ集合を返す
//
//	NOTES
//	自然文検索で使われる
//
//	ARGUMENTS
//	SearchResultSet& y
//		内部型の場合もある
//	SearchResultSet& z
//		上位に返す構造をもつ（内部型ではない）
//	int wno
//		検索文に含まれる検索語の通し番号
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SearchResultSet::merge(SearchResultSet& y, SearchResultSet& z, int wno)
{
	// y を z にマージした結果を z に設定
	// マージは、和集合と異なり、yのみに含まれる要素は、zに含まれない。
	
	for (ModVector<ModPair<ModUInt32, ModInvertedSearchResult*> >::Iterator
			 iter = begin(); iter != end(); ++iter)
	{
		// 各索引ファイルを順番に処理
		
		if ((*iter).first & Inverted::Sign::INSERT_MASK)
		{
			// 挿入転置の場合
			
			// [YET] 削除転置は無視
			
			ModInvertedSearchResult* y_ = y.get((*iter).first);

			// x_ <-> z_
			ModInvertedSearchResult* z_ = z.get((*iter).first);
			; _TRMEISTER_ASSERT(
				!(z_->getType() & (1 << _SYDNEY::Inverted::FieldType::Internal)));
			ModInvertedSearchResult* t = z_;
			z_ = (*iter).second;
			z.set((*iter).first, z_);
			(*iter).second = t;

			// x_(元z_) + y_ -> z_
			(*iter).second->merge(y_, z_, wno);
		}

		// yが空の場合、何もしない。
	}
}

//
//	FUNCTION public
//	Inverted::SearchResultSet::add -- 検索結果を追加する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 sig_
//		転置ファイルの識別子
//	ModInvertedSearchResult* result_
//		追加される検索結果
//	ModInt32 queryTermNo
//		検索文に含まれる検索語の通し番号
//
//	RETURN
//
//	EXCEPTIONS
//
void
SearchResultSet::add(ModUInt32 sig_,
					 ModInvertedSearchResult* result_,
					 ModInt32 queryTermNo)
{
	if(result_ && result_->getSize() > 0)
		// 検索結果にTFがあるかどうかに応じて呼び分ける
		(this->*_add)(sig_,result_,queryTermNo); 
}

//
//	FUNCTION private
//	Inverted::SearchResultSet::add_With_Tf -- 検索結果を追加する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 sig_
//		転置ファイルの識別子
//	ModInvertedSearchResult* result_
//		追加される検索結果(TFあり)
//	ModInt32 queryTermNo
//		検索文に含まれる検索語の通し番号
//
//	RETURN
//
//	EXCEPTIONS
//
void
SearchResultSet::add_With_Tf(ModUInt32 sig_,
							 ModInvertedSearchResult* result_,
							 ModInt32 queryTermNo)
{
	// result(this)を、result(this)とresult_の和集合で置き換える。

	// [YET] composeとの使い分け
	
	ModInvertedSearchResult* result = get(sig_);
	if (result)
	{
		if (queryTermNo < 0)
			result->setUnion(result_);
		else
			result->setUnion(result_, queryTermNo);
	}
	// [YET] なぜcopy不要？
}

//
//	FUNCTION private
//	Inverted::SearchResultSet::add_Without_Tf -- 検索結果を追加する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 sig_
//		転置ファイルの識別子
//	ModInvertedSearchResult* result_
//		追加される検索結果(TFなし)
//	ModInt32 queryTermNo
//		検索文に含まれる検索語の通し番号
//
//	RETURN
//
//	EXCEPTIONS
//
void
SearchResultSet::add_Without_Tf(ModUInt32 sig_,
								ModInvertedSearchResult* result_,
								ModInt32 queryTermNo)
{
	// result(this)を、result(this)とresult_の和集合で置き換える。

	// [YET] composeとの使い分け

	// queryTermNoを無視する
	// 検索結果にTFがないので検索語毎の検索結果もない。
	
	ModInvertedSearchResult *result = get(sig_);
	if (result->getSize())
	{
		// result + result_ -> x
		ModInvertedSearchResult* x = result->create();
		result->setUnion(result_, x);
		// x -> result
		delete result;
		set(sig_, x);
	}
	else
		result->copy(result_);
}

//
//  FUNCTION public
//  Inverted::SearchResultSet::getExpungedDoc -- 
//
//  NOTES
//
//  ARGUMENTS
//
//  RETURN
//
//  EXCEPTINS
//
ModInvertedBooleanResult *
SearchResultSet::getExpungedDoc(IndexFileSet* pIndexFileSet,
								ModUInt32 sig_)
{
	// 大転置・小転置を利用する場合、データの削除は遅延削除になることがある。
	// 例えば、大転置のデータを削除する場合、
	// まず、小転置に削除データを登録（削除小転置へのデータの挿入）し、
	// 適当なタイミングで、その削除データを使って大転置のデータを削除している。
	// 参考 FullText::DelayIndexFile::expunge()
	//
	// 転置は、大転置、更新用小転置、マージ用小転置から構成されており、
	// 以下に、各転置における削除データを示す。
	//
	// * 更新用小転置(Sign::_Insert1,Sign::_Delete1)
	//  削除データはない。
	//  もし更新用挿入転置に、削除したいデータがあれば、直接削除されるため。
	// * マージ用小転置(Sign::_Insert0,Sign::_Delete0)
	//  削除データは、更新用削除小転置に存在する。
	//  ただし、更新用削除小転置には、大転置の削除データも含まれる。
	// * 大転置
	//  削除データは、更新用削除小転置と、マージ用削除小転置に存在する。
	//  ただし、更新用削除小転置には、マージ用小転置の削除データも含まれる。

	// [NOTE] 事前にSearchCapsule::search()の全件検索用で、
	//  更新用削除小転置とマージ用削除小転置を取得しておく。
	
	if (expungedID.getSize() == 0)
	{
		// 削除文書集合の取得が初めての場合
		
		// [NOTE] expungedIDはキャッシュされる。

		ModUInt32 del  = Inverted::Sign::_Delete0;
		ModUInt32 del1 = Inverted::Sign::_Delete1;
		ModInvertedBooleanResult* boolResult;
		ModInvertedBooleanResult* boolResult1;
		IndexFileSet::Iterator IndexFileSet_iter;
		InvertedFile* pExpungeFile = NULL;
		InvertedFile* pExpungeFile1 = NULL;
		InvertedFile* pFile;
		boolResult =  (ModInvertedBooleanResult *)get(del);
		boolResult1 =  (ModInvertedBooleanResult *)get(del1);
		IndexFileSet_iter = pIndexFileSet->find(del);
		if (IndexFileSet_iter != pIndexFileSet->end())
			pExpungeFile = (*IndexFileSet_iter).getInvertedFile();
		IndexFileSet_iter = pIndexFileSet->find(del1);
		if (IndexFileSet_iter != pIndexFileSet->end())
			pExpungeFile1 = (*IndexFileSet_iter).getInvertedFile();

		for(IndexFileSet::Iterator iter = pIndexFileSet->begin();
			iter != pIndexFileSet->end(); ++iter)
		{
			ModInvertedBooleanResult newBoolResult;
			if ((*iter).getCount() &&
				((*iter).signature() & Inverted::Sign::INSERT_MASK))
			{
				if ((*iter).signature() == Inverted::Sign::_Insert1)
					continue;
				pFile = (*iter).getInvertedFile();
				
				if ((*iter).signature() == Inverted::Sign::_FullInvert)
				{
					convertDocumentID(pExpungeFile,
						pFile,
						boolResult,
						newBoolResult);
						convertDocumentID(pExpungeFile1, 
						pFile,
						boolResult1,
						newBoolResult);
				}
				else if ((*iter).signature() == Inverted::Sign::_Insert0)
				{
					convertDocumentID(pExpungeFile1, 
						pFile,
						boolResult1,
						newBoolResult);

				}
				newBoolResult.sort(_SYDNEY::Inverted::SortParameter::RowIdAsc);
				expungedID.pushBack(
					Element((*iter).signature(), newBoolResult));
			}
		}
	}

	for (ExpungedIDSet::Iterator iter = expungedID.begin();
		 iter != expungedID.end(); ++iter)
	{
		if ((*iter).first == sig_)
			return &(*iter).second;
	}
	return &ExpungeZero;
}

//
//	FUNCTION private
//	Inverted::SearchResultSet::setClusterParameter --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
SearchResultSet::setClusterParameter(
	float fGlobalClusteredLimit_,
	float& fLocalClusteredLimit_,
	ModSize& uiMaxRoughClusterCount_,
	ModSize& uiNeighbor_)
{
	// ラフクラスタ内の詳細クラスタリングの閾値
	// [NOTE] キーなし、キーありで値が空文字列、値が0の場合、
	//  1.0とfGlobalClusteredLimit_の真ん中、それ以外は設定されている値を使う。
	ModUnicodeString cstrLocalClusteredLimit = _LocalClusteredLimit;
	fLocalClusteredLimit_ =
		ModUnicodeCharTrait::toFloat(cstrLocalClusteredLimit);
	if (fLocalClusteredLimit_ == 0)
	{
		fLocalClusteredLimit_ = (1 + fGlobalClusteredLimit_) / 2;
	}
	
	// 一度に取得するラフクラスタの最大個数
	// [NOTE] キーなし、キーありで値が空、または、値が0の場合、
	//  デフォルト値を使い、それ以外は設定されている値を使う。
	uiMaxRoughClusterCount_ = _MaxRoughClusterCount;
	uiMaxRoughClusterCount_ = (uiMaxRoughClusterCount_ == 0) ?
		_DefaultMaxRoughClusterCount : uiMaxRoughClusterCount_;
	
	// 詳細クラスタのマージ対象数
	// [NOTE] キーなし、キーありで値が空、または、値が0の場合、
	//  デフォルト値を使い、それ以外は設定されている値を使う。
	uiNeighbor_ = _cNeighbor;
	uiNeighbor_ = (uiNeighbor_ == 0) ? _DefaultNeighbor : uiNeighbor_;
	
	if (uiNeighbor_ > uiMaxRoughClusterCount_ || uiNeighbor_ < 2)
	{
		_SYDNEY_THROW0(Exception::BadArgument);
	}
}

//
//	FUNCTION private
//	Inverted::SearchResultSet::initializeClusterIDList -- クラスタの初期化
//
//	NOTES
//
//	ARGUMENTS
//	ClusterIDList& vecClusterID_
//		クラスタIDを格納するベクタ
//		クラスタの初期値は、全て別々のクラスタ。
//		クラスタIDの初期値は、検索結果の位置に等しい。
//		クラスタID=0のクラスタは、検索結果の先頭データ、
//		クラスタID=1のクラスタは、検索結果の二番目のデータ、
//		…、となる。
//		クラスタIDは、上位には1-baseで返すが、内部的には0-base。
//		FullText::LogicalInterface::getSearchResultTuple()を参照。
//	ModSize uiSize_
//		検索結果数
//
//	RETURN
//
//	EXCEPTIONS
//　
void
SearchResultSet::initializeClusterIDList(ClusterIDList& vecClusterID_,
										 ModSize uiSize_) const
{
	// [YET] 段階的クラスタリングの場合は、もっと効率の良い方法があるかも
	
	vecClusterID_.assign(uiSize_, 0);
	ClusterIDList::Iterator i = vecClusterID_.begin();
	const ClusterIDList::ConstIterator e = vecClusterID_.end();
	for (ModUInt32 n = 0; i < e; ++i, ++n)
		*i = n;
}

//
//	FUNCTION private
//	Inverted::SearchResultSet::setDocumentVectorMap -- 
//
//	NOTES
//
//	ARGUMENTS
//	ModVector<FeatureSet*>& dvmap1
//		特徴語ベクトルの格納先
//	const ClusterIDList& clusterWork
//		詳細クラスタリング結果
//	ModSize uiBegin_
//		詳細クラスタリングの開始位置
//	ModSize uiEnd_
//		詳細クラスタリングの終了位置
//	OptionDataFile* pOptionDataFile_,
//		特徴語ベクトルが格納されているオプションファイル
//	ModInvertedSearchResult* resultSet_
//		検索結果集合
//		特徴語ベクトルをオプションファイルから取得するためのrowidを取得する
//
//	RETURN
//
//	EXCEPTIONS
//　
void
SearchResultSet::setDocumentVectorMap(
	ModVector<FeatureSet*>& dvmap1,
	const ClusterIDList& clusterWork,
	ModSize uiBegin_,
	ModSize uiEnd_,
	OptionDataFile* pOptionDataFile_,
	const ModInvertedSearchResult* resultSet_,
	ModHashMap<ModUInt32, FeatureSetPointer, ModHasher<ModUInt32> >& dvmap)
{
	; _TRMEISTER_ASSERT(uiBegin_ <= uiEnd_);
	; _TRMEISTER_ASSERT(clusterWork.getSize() > uiEnd_);
	
	// 初期化
	dvmap1.assign(uiEnd_ - uiBegin_ + 1, 0);

	ClusterIDList::ConstIterator wi = clusterWork.begin() + uiBegin_;
	const ClusterIDList::ConstIterator we = clusterWork.begin() + uiEnd_;
	ModVector<FeatureSet*>::Iterator dvite = dvmap1.begin();
	for (; wi <= we; ++wi, ++dvite)
	{
		makeDocumentVector(pOptionDataFile_, resultSet_,
						   *wi, *dvite, dvmap);
	}
}

//
//	FUNCTION private
//	Inverted::SearchResultSet::setClusterMap -- クラスタマップの設定
//
//	NOTES
//
//	ARGUMENTS
//	ClusterMap& clusterMap,
//		詳細クラスタリング結果の格納先
//	const ClusterIDList& cluster_
//		詳細クラスタリング結果
//	const ClusterIDList& original_
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
SearchResultSet::setClusterMap(ClusterMap& clusterMap,
							   const ClusterIDList& cluster_,
							   const ClusterIDList& original_,
							   ModSize uiBegin_,
							   ModSize uiEnd_,
							   ModInt32& clusterno) const
{
	; _TRMEISTER_ASSERT(uiBegin_ <= uiEnd_);
	; _TRMEISTER_ASSERT(cluster_.getSize() > uiEnd_);
	; _TRMEISTER_ASSERT(original_.getSize() > uiEnd_);
	
	// 参照中のクラスタへのイテレータ
	ClusterMap::Iterator ci;

	ClusterIDList::ConstIterator i = cluster_.begin() + uiBegin_;
	const ClusterIDList::ConstIterator e = cluster_.begin() + uiEnd_;
	ClusterIDList::ConstIterator j = original_.begin() + uiBegin_;
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
				= clusterMap.insert(*i, ModVector<ModUInt32>());

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
//	Copyright (c) 2007, 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
