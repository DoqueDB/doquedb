// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedQueryBaseNode.cpp --
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2005, 2008, 2009, 2023 Ricoh Company, Ltd.
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

#ifdef SYD_INVERTED // SYDNEY 対応
#include "SyDefault.h"
#include "SyReinterpretCast.h"
#endif

#include "Common/BitSet.h"

#include "ModInvertedQueryBaseNode.h"
#include "ModInvertedOperatorAndNode.h"
#include "ModInvertedLocationListIterator.h"

//
// FUNCTION public
// ModInvertedQueryBaseNode::ModInvertedQueryBaseNode -- コンストラクタ
//
// NOTES
// コンストラクタ。
//
// ARGUMENTS
// const NodeType nType
//	  ノードタイプ
// const ModSize documentFrequency
//	  全文書数
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
ModInvertedQueryBaseNode::ModInvertedQueryBaseNode(
	const NodeType type_,
	const ModUInt32 resultType_,
	const ModSize documentFrequency_
	):
	type(type_),
	retrieved(documentFrequency_ == UndefinedFrequency ? ModFalse : ModTrue),
	sortFactor(0),
	estimatedDocumentFrequency(documentFrequency_),
	queryNodeForRoughEvaluation(0),
	upper(0),lower(0),
	firstStepStatus(initial),
	needDF(ModFalse),
	totalDocumentFrequency(UndefinedFrequency)
{
	scoreCalculator = 0;
	firstStepResult = ModInvertedSearchResult::factory(resultType_);
}

//
// FUNCTION public
// ModInvertedQueryBaseNode::~ModInvertedQueryBaseNode -- デストラクタ
//
// NOTES
// デストラクタ
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
ModInvertedQueryBaseNode::~ModInvertedQueryBaseNode()
{
	delete firstStepResult;
	firstStepResult = 0;
}

//
// FUNCTION private
// ModInvertedRankingQuery::createRoughNode -- モードに応じたRoughNodeの生成
//
// NOTES
//
// 検索タイプに応じてラフノードを生成する。
// ブーリアン検索の場合はAndNode
// ランキング検索の場合はRankingAndNode
// アトミック検索の場合はAtmicAndNode
// を生成しアドレスを返す。
//
// ARGUMENTS
// const ModInvertedQuery* Query
//      Query
//
// RETURN
// 生成した RoughNode のアドレス
//
// EXCEPTIONS
// なし
//
ModInvertedQueryNode*
ModInvertedQueryBaseNode::createRoughNode(
	const ModInvertedQuery* query,
	const ModInvertedQuery::ValidateMode mode)
{
	ModInvertedOperatorAndNode* roughNode = new ModInvertedOperatorAndNode(firstStepResult->getType());

	if ((mode & Query::rankingMode) != 0) {

        // QueryNodeには必ずデフォルトの計算器をセットするように
		// なったので、ここではduplicateだけ
		ModInvertedRankingScoreCombiner* combiner =
						query->getDefaultAndScoreCombiner();
		;ModAssert(combiner != 0);
		roughNode->setScoreCombiner(combiner->duplicate());
	}

	return roughNode;
}


//
// FUNCTION public
// ModInvertedQueryBaseNode::calcSortFactor -- sortFactor の計算
//
// NOTES
// sortChildren() 関数で使用する sortFactor メンバ変数を計算する。ただ
// し QueryNode では単に estimatedDocumentFrequency を代入しているだけ。
// SimpleTokenLeafNode(末端ノード) はこの関数を継承して使う。中間ノー
// ドはこの関数をオーバライドする。
//
// ARGUMENTS
// なし
//
// RETURN
// 計算した sortFactor 値。
//
// EXCEPTIONS
// なし
//
ModSize
ModInvertedQueryBaseNode::calcSortFactor()
{
	if (this->sortFactor == 0){
		// まだ計算していないのなら、計算する
		this->sortFactor = this->estimateDocumentFrequency();
	}
	return this->sortFactor;
}

//
// FUNCTION public
// ModInvertedQueryNode::getDocumentFrequency -- 文書頻度を得る
//
// NOTES
// 文書頻度を得る。まだretrieve()を実行していないのなら実行し文書頻度を得る
//
// ARGUMENTS
// なし
//
// RETURN
// 文書頻度を得る
//
// EXCEPTIONS
// なし
//
ModSize
ModInvertedQueryBaseNode::getDocumentFrequency(Query::EvaluateMode mode)
{
	if (this->retrieved != ModTrue){
#ifndef DEL_BOOL
		ModInvertedBooleanResult dummy;
		this->retrieve(dummy, mode);
#else

#endif
#ifdef DEBUG
		ModDebugMessage << "getDocumentFrequency: retrive" << ModEndl;
#endif
	}
	return this->estimateDocumentFrequency();
}

//
// FUNCTION public
// ModInvertedQueryBaseNode::getTermFrequency -- 文書内頻度の取得
//
// NOTES
// 条件を満たす語句の文書内出現頻度を求める。
//
// ARGUMENTS
// ModInvertedDocumentID documentID
//      文書ID
// Query::EvaluateMode mode
//      評価モード
//
// RETURN
// 求めた文書内頻度
//
// EXCEPTIONS
// なし
//
ModSize
ModInvertedQueryBaseNode::getTermFrequency(ModInvertedDocumentID documentID,
										   Query::EvaluateMode mode)
{
	ModSize tf = 0;
	
	ModInvertedLocationListIterator* locations = 0;
	if (ModInvertedQueryNode::evaluate(documentID, locations, tf, mode)
		== ModTrue)
	{
		// 条件を満たすことが確認できた
		
		if (locations != 0)
		{
			// 位置情報リストが取得できた

			;ModAssert(tf == 0);
			tf = locations->getFrequency();
			if (tf == 0) {
				// getFrequency()が0を返す場合はループを回してカウントしなければ
				// ならない
				
				// TFをカウントするコストが無視できない場合は、
				// maxCountでカウントを打ち切り、推定値を用いる。
				ModSize maxCount = ModInvertedQuery::getTFCountUpperLimit();
				for (; locations->isEnd() == ModFalse && tf < maxCount;
					 locations->next())
					++tf;
				if (locations->isEnd() == ModFalse) {
					// 上限に達したので、文書長で計算する
					ModSize length, loc;
					if (scoreCalculator->searchDocumentLength(documentID, length)
						== ModTrue &&
						(loc = locations->getLocation()) != 0)
					{
						float tmpTf = (float)tf / loc * length;
						tf = (ModSize)tmpTf;
					}
				}
			}
			locations->release();
		}
		else
		{
			// 位置情報リストが取得できなかった
			
			// TFは取得できたはず
			;ModAssert(tf > 0);
		}
	}
	return tf;
}

#ifdef DEBUG
//
// FUNCTION public
// ModInvertedQueryBaseNode::showSortFactor -- sortFactor を表示
//
// NOTES
// sortFactor を表示(debug用)。サブクラスへ継承される。
//
// ARGUMENTS
// ModUnicodeString&
//      sortFactor 値を含む文字情報
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void
ModInvertedQueryBaseNode::showSortFactor(ModUnicodeString& out)
{
	ModOstrStream tmpStream;
	tmpStream << sortFactor;
	ModUnicodeString sf(tmpStream.getString());
	out += '{';
	out += sf;
	out += '}';

	ModUnicodeString prefix;
	this->prefixString(prefix, ModTrue, ModFalse);
	out += prefix;

	ModUnicodeString content;
	this->contentString(content);
	out += '(';
	out += content;
	out += ')';
}
#endif

//
// FUNCTION protected
// ModInvertedQueryBaseNode::getQueryString -- 検索条件ノードを出力
//
// NOTES
// 検索条件ノードを出力。ラフノードの内容を表示するかどうかを選択できる
//
// ARGUMENTS
// ModOstrStream& out
//      結果格納用オブジェクト
//	const ModBoolean asTermString
//		TermLeafNodeから生成されたorderedDistance/AtomicOrの場合は
//		もとのTermLeafNodeを表示
//	const ModBoolean withCalOrCombName
//		スコア計算機・合成器名も表示
//	const ModBoolean withCalOrCombParam,
//		スコア計算機・合成器のパラメータも表示(但しwithCalOrCombNameがTrueの
//		場合のみ有効)
// ModBoolean withRouh
//      ラフノードを表示するかどうかを示すフラグ（trueで表示）
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void
ModInvertedQueryBaseNode::getQueryString(ModUnicodeString& out,
	const ModBoolean asTermString,
	const ModBoolean withCalOrCombName,
	const ModBoolean withCalOrCombParam,
	const ModBoolean withRouh) const
{
	ModUnicodeString prefix;
	this->prefixString(prefix, withCalOrCombName, withCalOrCombParam);
	out += prefix;

	ModUnicodeString content;
	this->contentString(content);
	out += '(';
	out += content;
	out += ')';

	// rough node 表示
	if (withRouh == ModTrue &&
		queryNodeForRoughEvaluation != 0 &&
		queryNodeForRoughEvaluation !=
		const_cast<ModInvertedQueryBaseNode*>(this)){
		ModUnicodeString rough;

		rough += '<';
		this->queryNodeForRoughEvaluation->getQueryString(rough,
			asTermString, withCalOrCombName, withCalOrCombParam, withRouh);
		rough += '>';
		out+=rough;
	}
}

//
// FUNCTION public
// ModInvertedQueryBaseNode::sharedOriginalString -- OriginalStringの共有化
//
// NOTES
// orderdDistanceNodeが共有化されるときに、
// 共有されるノードにoriginalStringがセットされていない場合がある。
// そのときは削除されるノードのoriginalStringをセットする
//
// ARGUMENTS
//	QueryNode* eraseNode
//		削除されるノード
//	QueryNode* sharedNode
//		共有されるノード
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void
ModInvertedQueryBaseNode::sharedOriginalString(
	QueryNode* eraseNode,
	QueryNode* sharedNode)
{
	ModUnicodeString tmpOriginalString;
#ifdef V1_6
	ModLanguageSet tmpLangSet;
#endif // V1_6
	ModInvertedTermMatchMode tmpMmode;
	sharedNode->getOriginalString(tmpOriginalString,
#ifdef V1_6
								  tmpLangSet,
#endif // V1_6
								  tmpMmode);
	if(tmpOriginalString.getLength() == 0) {
		// 共有されるノードにoriginalStringがセットされていないので、
		// 削除されるノードのoriginalStringをせっとする。
		eraseNode->getOriginalString(tmpOriginalString,
#ifdef V1_6
									 tmpLangSet,
#endif // V1_6
								  	 tmpMmode);
		sharedNode->setOriginalString(tmpOriginalString,
#ifdef V1_6
									  tmpLangSet,
#endif // V1_6
								  	  tmpMmode);
	}
}

#ifdef DEBUG
//
// FUNCTION protected
// ModInvertedQueryBaseNode::showEstimatedValue -- 文書頻度の見積もり値を出力
//
// NOTES
// 文書頻度の見積もり値を出力。サブクラスへ継承される。
//
// ARGUMENTS
// ModUnicodeString& out
//  結果格納用オブジェクト
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void
ModInvertedQueryBaseNode::showEstimatedValue(ModUnicodeString& out)
{
	ModOstrStream tmpStream;
	tmpStream << this->estimateDocumentFrequency();
	ModUnicodeString df(tmpStream.getString());
	out += '{';
	out += df;
	out += '}';

	ModUnicodeString prefix;
	this->prefixString(prefix, ModFalse, ModFalse);
	out += prefix;

	ModUnicodeString content;
	this->contentString(content);
	out += '(';
	out += content;
	out += ')';
}
#endif // DEBUG

//
// Copyright (c) 2000, 2001, 2002, 2003, 2005, 2008, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//

