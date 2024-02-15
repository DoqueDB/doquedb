// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedQueryInternalNode.cpp -- 中間ノードクラス
// 
// Copyright (c) 1997, 1999, 2000, 2001, 2002, 2003, 2005, 2023 Ricoh Company, Ltd.
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
#include "Inverted/ModInvertedFile.h"
#endif

#include "ModOstrStream.h"
#include "ModAssert.h"

#include "ModInvertedQueryInternalNode.h"	// インタフェイスファイル
#include "ModInvertedOperatorAndNode.h"
#include "ModInvertedOperatorOrNode.h"
#include "ModInvertedOrderedDistanceNode.h"
#include "ModInvertedTermLeafNode.h"
#include "ModInvertedSimpleTokenLeafNode.h"
#include "ModInvertedBooleanResultLeafNode.h"
#include "ModInvertedRankingResultLeafNode.h"
#include "ModInvertedRankingScoreCombiner.h"
#include "ModInvertedQueryParser.h"
#include "ModInvertedLocationListIterator.h"
#ifdef  SYD_INVERTED // SYDNEY 対応
#include "Inverted/ModInvertedList.h"
#else
#include "ModInvertedList.h"
#endif

//
// CONST
//
// makeRoughMapでbooleanResultLeafNode/rankingResultLeafNodeをラフノードに追加
// する際のサイズの制限。booleanResultLeafNode/rankingResultLeafNodeの件数が
// sizeLimitOfResultLeafNodeToAddToRoughMapを超える場合はラフノードには追加し
// ない(速度低下の恐れがある)
//
// とりあえずModSizeMaxとし、すべて登録する。
/*static*/ ModSize
ModInvertedQueryInternalNode::sizeLimitOfResultLeafNodeToAddToRoughMap = ModSizeMax;


//
// FUNCTION public
// ModInvertedQueryInternalNode::ModInvertedQueryInternalNode -- コンストラクタ
//
// NOTES
// コンストラクタ。
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
ModInvertedQueryInternalNode::ModInvertedQueryInternalNode(const NodeType type_,const ModUInt32 resultType_)
	: ModInvertedQueryBaseNode(type_,resultType_), freeList(0)
{}

//
// FUNCTION public
// ModInvertedQueryInternalNode::~ModInvertedQueryInternalNode -- デストラクタ
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
ModInvertedQueryInternalNode::~ModInvertedQueryInternalNode()
{
	while (freeList)
	{
		ModInvertedLocationListIterator* p = freeList;
		freeList = p->nextInstance;
		delete p;
	}
	if (scoreCombiner != 0) {
		// スコア合成器がセットされている場合は削除
		// ブーリアン検索の場合、Atomicなノードの場合はセットされていない
		delete scoreCombiner;
		scoreCombiner = 0;
	}
}

//
// FUNCTION public
// ModInvertedQueryInternalNode::sharedQueryNode -- 中間ノードの共有化
// 
// NOTES
// 中間ノードの共有化
//
// 子ノードのgetQueryString() の結果（検索式）とそのノードへのポインタ
// を 引数 globalNodeMap に格納していく。検索式が key でノードへのポイ
// ンタが value。
//
// globalNodeMap への登録時に同じ key が登録されていたら共有化できると
// 考え、子ノー ドの実体を delete し children に設定されているポインタ
// を張り変える。
//
// また、ブーリアン検索の場合は、カレントの中間ノードの子ノードに同じ key
// のノードがないかをlocalNodeMap 変数を使って調査する。もし同じ key のノード
// が発見できたらそのノードを delete し、children からも削除し、その子ノード
// を完全に消去する。このようにchildren が削除され child の数が 1 になった
// 場合さらにそのノードを単純化できるので、 changeSimpleNodeType() 関
// 数を呼び出す。
// ランキング検索の場合はlocalNodeMapを用いchildを削除するとスコアが変わってし
// まうため、カレントの中間ノードの検査は行わない。
//
// sharedQueryNode() は再帰的に呼び出す形式をなっていて、globalNodeMap 
// には下位のノードから順番にデーターを挿入していくことになる。
//
// 上記の処理が終了後子ノードを順番に見ていき、子ノードの中に空集合ノードが
// 見つかった場合(自分自身も空集合になるので)は0を返し呼び出し側で
// 自分自身にemptySetNodeをセットしてもらう
//
// 空集合ノードが見つからなかった場合は子ノードの数を返す。子ノードの数が1
// の場合は呼び出し側でchangeSimpleTypeNodeが呼ばれ単純化される。
//
// 子ノードを一つしかない、location/end/scale/wordの場合は子ノードの数が
// 1であるため、changeSimpleTypeNodeが呼ばれるが、この関数ではこれらの
// ノードの場合はなにも処理をしないのでlocation(A)でAを昇格させるような
// ことはない
//
// この関数は以下のクラスでオーバーライドされる
//	・ModInvertedOperatorOrNode
//		Orは子ノードに空集合が含まれても自分が空集合になるとは限らない
//		emptySetNodeがあったときの処理が異なる
//	・ModInvertedOperatorAndNotNode
//		子ノードの中に同じノードがあったときの処理が特殊
//	・ModInvertedSimpleWindowNode
//		このノードはlocalNodeMapを使って共有を行ってはいけない
//	・ModInvertedOperatorWindowNode
//		このノードはlocalNodeMapを使って共有を行ってはいけない
//	・ModInvertedOrderedDistanceNode
//		このノードはlocalNodeMapを使って共有を行ってはいけない
//
// ARGUMENTS
// QueryNodeMap& globalNodeMap
//		全ノードの QueryNodeMap
// QueryNodePointerMap& nodePointerMap
//		OR標準形変換により共有されているノードが登録されているMap
//
// RETURN
// children の数
//		ただし、子ノードにEmptySetNodeを含む場合は自分がEmptySetNodeになるため
//		0を返す。
//
// 0または1を返した場合には呼び出し側で後処理が必要。
// 	0の場合
//		このノードは空集合ノードになる。空集合ノードに置き換える
// 	1の場合
//		  子ノードが一つしか無くなったので単純化する
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
// 
ModSize
ModInvertedQueryInternalNode::sharedQueryNode(
	QueryNodeMap& globalNodeMap,
	QueryNodePointerMap& nodePointerMap)
{
	QueryNodeMap::Iterator p;
	ModVector<ModInvertedQueryNode*>::Iterator child;
	ModInvertedQueryNode* tmpEndNode = 0;
	
	ModSize retGrandChildNum;		// 子ノードのsharedQueryNodeの戻値

	QueryNodeMap localNodeMap;		// この internal node 内のマップ


	child = children.begin();
	for (; child != children.end(); ++child) {
		if (ModInvertedIsInternalNode((*child)->getType()) != ModTrue) {
			// リーフノードの場合なにもしない
			continue;					// 次の child へ
		}
		// internal node の場合

		// 子ノードに対して sharedQueryNode() を呼ぶ
		retGrandChildNum = static_cast<InternalNode*>(*child)->
			sharedQueryNode(globalNodeMap, nodePointerMap);

		if (retGrandChildNum == 1) {
			// ノードの共有化をした結果 children が 1つしかなくなった
			// 場合ノードを単純化する
			changeSimpleTypeNode(child, nodePointerMap);

		} else if (retGrandChildNum == 0) {
			// 子ノードのsharedQueryNodeの戻り値が0
			// この子ノードはEmptySetNode
			addQueryNodePointerMap(*child, nodePointerMap);
			*child = const_cast<ModInvertedQueryNode*>(emptySetNode);
			continue;
		}


		// childがまだInternalNodeであるか検査する
		if (ModInvertedIsInternalNode((*child)->getType()) != ModTrue) {
			// リーフノードの場合はなにもせず次の子ノードに行く
			continue;
		}

		// 子ノードの QueryString を取得する
		ModUnicodeString key;
		(*child)->getQueryString(key);

		QueryNode* value = *child;      // QueryNode ポインタをセット

		if (getScoreCombiner() == 0) {

			// ブーリアンの場合はlocalNodeMapの調査も行う。
			// ランキング検索の場合はlocalNodeMapを用いて子ノードの削除を行うと
			// スコアが変わってしまうため行わない。

			// localNodeMap に同じkeyがあるか調査する
			p = localNodeMap.find(key);

			if (p != localNodeMap.end()) {
#ifdef DEBUG
				ModDebugMessage << "delete node local: " << key << ModEndl;
#endif
				// 既に同じQueryStringのノードがあるこの子ノードを削除する
				addQueryNodePointerMap(*child, nodePointerMap);

				child = children.erase(child);      // childrenからの削除する
				--child;

				continue;                   // 次の child へ
			} else {
				// 同じkeyはないのでlocalNodeMap へ挿入する
				localNodeMap[key] = value;
			}
		}

		// globalNodeMapに同じQueryStringのノードがあるか調査する
		p = globalNodeMap.find(key);
		if (p != globalNodeMap.end()) {
#ifdef DEBUG
			ModDebugMessage << "delete node global: " << key << ModEndl;
#endif
			if (*child == (*p).second) {
				// すでに共有しているので特に処理しない
#ifdef DEBUG
				ModDebugMessage << "already shared" << ModEndl;
#endif
			} else {
				// 既に同じQueryStringのノードがある childのnodeを破棄する

				// OrderedDistanceの共有の場合はendNodeを考慮する必要がある
				if (ModInvertedAtomicMask((*child)->getType())
					== ModInvertedQueryNode::orderedDistanceNode) {
					// tmpEndNodeが0以外なら、
					// 削除される側のOrderedDistanceがendNodeを持っている
					tmpEndNode = (*child)->getEndNode();

					// originalStringも共有する
					sharedOriginalString(*child, (*p).second);

#ifdef DEBUG
					ModDebugMessage << "share OrderedDistanceNode {"
									<< key << "}" << ModEndl;
#endif // DEBUG
				}

				addQueryNodePointerMap(*child, nodePointerMap);

				// childは先にglobalNodeMapにあったnodeへのポインタをセットする
				*child = (*p).second;

				// OrderedDistance かつ endNodeを持っていた
				if (tmpEndNode != 0) {
					(*child)->setEndNode(tmpEndNode);
					tmpEndNode = 0;
				}
			}
		} else {
			// globalNodeMap へ挿入する
			globalNodeMap[key] = value;
		}
	}

	// 子ノードのチェック
	for (child = children.begin(); child != children.end(); ++child) {
		if (*child == emptySetNode) {
#ifdef DEBUG
			ModDebugMessage << "include EmptySetNode" << ModEndl;
#endif // DEBUG
			// 子ノードにEmptySetNodeを含む
			child = children.begin();
			while (child != children.end()) {
				// childrenにEmptySetNodeを含むため、自分自身もEmptySetNodeに
				// なる。このためchildrenを削除する。
				addQueryNodePointerMap(*child, nodePointerMap);
				child = children.erase(child);
			}
			return 0;
		}
	}
	return children.getSize();
}

//
// FUNCTION public
// ModInvertedQueryInternalNode::addRoughToGlobalNodeMap -- RoughPointerの内容を共有化する
// 
// NOTES
// 粗い評価用に作られたノードの内容(queryNodeForRoughEvaluation)を
// GlobalNodeMapへ登録し、中間ノードとの共有化が行なえるようにする。
//
// ARGUMENTS
// QueryNodeMap& globalNodeMap
//		中間ノードの共有化を行なううえで使用するMap変数
// QueryNodePointerMap& nodePointerMap
//		共有化の結果必要なくなったノードを登録するMap変数
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedQueryInternalNode::addRoughToGlobalNodeMap(
	QueryNodeMap& globalNodeMap,
	QueryNodePointerMap& nodePointerMap)
{
	ModVector<ModInvertedQueryNode*>::Iterator child = children.begin();
	for (; child != children.end(); ++child) {
		if (ModInvertedIsInternalNode((*child)->getType()) == ModTrue) {
			// 子ノードが internalNode であれば再帰的に呼び出す
			(static_cast<InternalNode*>(*child))
				->addRoughToGlobalNodeMap(globalNodeMap, nodePointerMap);
		}
	}
	if (queryNodeForRoughEvaluation == 0 ||
		ModInvertedIsInternalNode(
			queryNodeForRoughEvaluation->getType()) != ModTrue) {
		// queryNodeForRoughEvaluation が設定されてないかinternalNode以外
		// の場合はなにもしない
		return;
	}

	// RoughPointerのノードの QueryString を取得する
	ModUnicodeString key;
	queryNodeForRoughEvaluation->getQueryString(key);
#ifdef DEBUG
	ModDebugMessage << "addRoughToGlobalNodeMap : " << key << ModEndl;
#endif

	// globalNodeMapに同じQueryStringのノードがあるか調査する
	QueryNodeMap::Iterator p = globalNodeMap.find(key);
	if (p != globalNodeMap.end()) {
#ifdef DEBUG
		ModDebugMessage << "delete Rough node global: " << key << ModEndl;
#endif
		if (queryNodeForRoughEvaluation == (*p).second) {
			// すでに共有しているので特に処理しない
#ifdef DEBUG
			ModDebugMessage << "already shared" << ModEndl;
#endif
		} else {
			// 既に同じQueryStringのノードがある
			// RoughEvaluationのnodeを破棄する
			// ノードは直接deleteするのではなくnodePointerMapに登録する
			// value として代入している 16 には特に意味はない
			nodePointerMap[queryNodeForRoughEvaluation] = 16;

			// queryNodeForRoughEvaluationへは先にglobalNodeMapにあっ
			// たnodeへのポインタをセットする
			queryNodeForRoughEvaluation = (*p).second;
			; ModAssert(queryNodeForRoughEvaluation->
				getQueryNodeForRoughEvaluation() ==
				queryNodeForRoughEvaluation);
		}
	} else {
		// globalNodeMap へ挿入する
		globalNodeMap[key] = queryNodeForRoughEvaluation;
		; ModAssert(queryNodeForRoughEvaluation->
			getQueryNodeForRoughEvaluation() ==
			queryNodeForRoughEvaluation);
	}
}

//
// FUNCTION public
// ModInvertedQueryInternalNode::eraseTermLeafNode -- TermLeafNodeの消去
// 
// NOTES
// TermLeafNode を削除してOrderedDistanceNode/SimpleTokenLeafNodeにす
// る。QueryNode の eraseTermLeafNode をオーバライドしている
//
// ここでは中間ノードのデフォルトの処理を記述。
// Or/AtomicOr/AndNot以外の中間ノードの時に呼ばれる。
//
// Or/AtomicOr/AndNot以外の中間ノードのでは子ノードの中に空集合ノードが
// 一つでもあった場合自分自身が空集合ノードになる。
// このため子ノードにひとつでも EmptySetNode であった場合 
// noEmpty を ModFalse にして ModFalse で戻る。そうすることで、呼び出
// し側で このノード自信を削除し EmptySetNodeに置き換える。
//
// この関数は特殊な処理が必要なOr/AtomicOr(実際にはOrのものを使う)/Andnot
// でオーバーライドされる。
// 
// Or系のノード、Andnotでは子ノードに空集合ノードが含まれても自分自身が
// 空集合ノードになるとか限らないのでnoEmptyの条件が異なる。
// 
// ARGUMENTS
// QueryNode*& node
//		子ノードが1つしかなくなって昇格させる場合のノードポインタ (結果格納用)
// 		andnotノードの時にのみ使用する。
// Query& query
//		Query
// 
// RETURN
// この関数の呼び出し側で後処理が必要な場合 ModFalse を返す、特に必要
// ない場合 ModTrue を返す
// 
// 上のノードで必要な処理は
//  return_value = ModFalse && node !=0
// 		nodeに昇格させる子ノード(呼び出し側から見た場合には孫ノード)の
//		ポインタがセットされているので、自分自信(呼び出し側から見た場合には
//		子ノード)削除し、nodeで置き換える
// 		※ただしnodeを使うのはAndNot::eraseTermLeafNode()のみ
// 
// 
//  return_value = ModFalse && node ==0
// 		自分自身(呼び出し側から見た場合は子ノード)が空集合になるケース。
// 		emptySetNodeに置き換える。
// 
// EXCEPTIONS
// 下位からの例外をそのまま返す
// 
ModBoolean
ModInvertedQueryInternalNode::eraseTermLeafNode(
	QueryNode*& node, 
	Query& query)
{
	ModBoolean noEmpty = ModTrue;		// 空集合なしフラグ
	node = 0;							// 常に 0

	ModVector<ModInvertedQueryNode*>::Iterator child(children.begin());
	ModInvertedQueryNode::NodeType childType;
	
	for (;child != children.end(); ++child) {
		// 子ノードを順番にチェックしていく

		childType = ModInvertedAtomicMask((*child)->getType());

		if (childType == ModInvertedQueryNode::termLeafNode) {
			// child が TermLeafNode の場合
			// ポインタを付け換える
			TermLeafNode* termNode = static_cast<TermLeafNode*>(*child);

			QueryNode* preciseNode
				= termNode->getQueryNodeForPreciseEvaluation();

			if (preciseNode == emptySetNode) {
				// 空集合ノード発見
				noEmpty = ModFalse;		// 空集合ありにセット
			}

			// 明示的にAtomicを指定された場合(#term(...))はeraseTermLefNodeが
			// 指定されてもTermLeafNodeの削除は行わない。ただしpresiceNodeが
			// EmptySetNodeの場合は削除する
			
			*child = termNode->getQueryNodeForPreciseEvaluation();
			(*child)->setQueryNodeForRoughEvaluation(
				termNode->getQueryNodeForRoughEvaluation());
			(*child)->setOriginalString(termNode->termString,
#ifdef V1_6
										termNode->getLangSet(),
#endif // V1_6			
										termNode->getMatchMode());
			termNode->setQueryNodeForPreciseEvaluation(0);
			termNode->setQueryNodeForRoughEvaluation(0);

			delete termNode;

		} else if (childType == ModInvertedQueryNode::booleanResultLeafNode) {
			// booleanResultLeafNode の場合
			query.addOrStanderdSharedNode(*child);
			if (static_cast<ModInvertedBooleanResultLeafNode*>(*child)
				->isEmptyResultLeafNode() == ModTrue) {
				// 空集合
				noEmpty = ModFalse;
			}
		} else if (childType == ModInvertedQueryNode::rankingResultLeafNode) {
			// RankingResultLeafNode の場合
			query.addOrStanderdSharedNode(*child);
			if (static_cast<ModInvertedRankingResultLeafNode*>(*child)
				->isEmptyResultLeafNode() == ModTrue) {
				// 空集合
				noEmpty = ModFalse;
			}

		} else {
			// child が TermLeafNode 以外の場合再帰呼び出し
			QueryNode* tmpNode = 0;
			if ((*child)->eraseTermLeafNode(tmpNode, query) != ModTrue) {
				// 後処理が必要
				query.addOrStanderdSharedNode(*child);
				if (tmpNode == 0) {
					// 子ノードに空集合が含まれていた
					noEmpty = ModFalse;
				} else {
					// 子ノードを昇格
					// childがandnotの場合にここに来るケースがある
					*child = tmpNode;
				}
			}
		}
	}
	return noEmpty;
}

// FUNCTION public
// ModInvertedOperatorWindowNode::sortChildren -- 子ノードリストの並べ替え
//
// NOTES
// unordered の時だけ子ノードリストを並べ替える。子ノードに対しても 
// sortChildren() を呼び出す。AndNodeのsortChildren()をオーバライドし
// ている。
//
// ARGUMENTS
// const ModInvertedQuery::ValidateMode mode
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedQueryInternalNode::sortChildren(
	const ModInvertedQuery::ValidateMode mode)
{
	// 子ノードにsortが必要であればソートする
	ModVector<ModInvertedQueryNode*>::Iterator child = children.begin();
	for (;child != children.end(); ++child) {
		(*child)->sortChildren(mode);
	}
}

// 
// FUNCTION public
// ModInvertedQueryInternalNode::calcSortFactor -- sortFactor の計算
// 
// NOTES
// sortChildren() 関数で使用する sortFactor メンバ変数を計算する。
// QueryNode::calcSortFactor() をオーバライドする。中間ノードの 
// sortFactor は、各子ノードの sortFactor の和となる。
// 
// ARGUMENTS
// なし
// 
// RETURN
// 計算した sortFactor 値。
// 
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModSize
ModInvertedQueryInternalNode::calcSortFactor()
{
	if (this->sortFactor == 0) {
		// まだ計算していないのなら、計算する
		ModVector<ModInvertedQueryNode*>::Iterator child(children.begin());
		ModSize childSortFactor;

		for (;child != children.end(); ++child) {
			childSortFactor = (*child)->calcSortFactor();
			if (childSortFactor == ModInvertedQueryNode::MaxSortFactor) {
				// MaxSortFactorが返されるのは自分以下のノードにRegex
				// が含まれるケース
				// 既に上限に達しているのでそのまま返す
				this->sortFactor = childSortFactor;
				return this->sortFactor;
			}
			this->sortFactor += childSortFactor;
		}
	}
	return this->sortFactor;
}

#ifdef DEBUG
//
// FUNCTION public
// ModInvertedQueryNode::showSortFactor -- sortFactor を表示
//
// NOTES
// sortFactor を表示(debug用)。中間ノード用。QueryNodeで定義されている
// 内容をオーバライドしている。
//
// ARGUMENTS
// ModOstrStream& out
//		sortFactor 値を含む文字情報
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedQueryInternalNode::showSortFactor(ModUnicodeString& out)
{
	ModOstrStream tmpStream;
	tmpStream << calcSortFactor();
	ModUnicodeString sf(tmpStream.getString());

	out += '{';
	out += sf;
	out += '}';

	ModUnicodeString prefix;
	this->prefixString(prefix, ModTrue, ModFalse);

	out += prefix;
	out += '(';

	//
	// 子ノードの内容を表示
	//
	ModVector<ModInvertedQueryNode*>::Iterator p(children.begin());
	ModUnicodeString childString;
	(*p)->showSortFactor(childString);
	out+=childString;
	for (++p; p != this->children.end(); ++p) {
		childString.clear();
		out += ',';
		(*p)->showSortFactor(childString);
		out+=childString;
	}
	out += ')';
}
#endif

//
// FUNCTION public
// ModInvertedQueryInternalNode::checkQueryNode -- 子ノードの数をチェックする
//
// NOTES
// 有効化の最後に呼び出されて、子ノードの数をチェックする。もし異常で
// あれば例外を投げる。
//
// ここでは InternalNode 用の定義がされている。子ノードの数が 0 の場合
// 異常と判断する。OrderedDistanceNode や AndNotNode は 子ノードが 2 
// 以外ありえないという特殊なものなので、それぞれのノードでこの関数を
// オーバライドする。
//
// また、子ノードに対しても再帰的に本関数を呼び出す。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedQueryInternalNode::checkQueryNode(
	ModInvertedQuery* query_,
	const ModBoolean setStringInChildren_,
	const ModBoolean needDF_
	)
{
	ModSize s = this->children.getSize();
	if (s == 0) {
		// 子ノードがないのは異常
		ModErrorMessage << "children is empty." << ModEndl;
		ModThrowInvertedFileError(ModInvertedErrorQueryValidateFail);
	}
	for (ModSize i(0); i < s ; ++i) {
		this->children[i]->checkQueryNode(query_, setStringInChildren_, needDF_);
	}
}

//
// FUNCTION public
// ModInvertedQueryInternalNode::validate -- ノードの有効化
//
// NOTES
// ノードの有効化。InternalNodeは何もしないで子ノードに対してvalidate()
// を行う。
// 有効化を行うのはRegex/Term/Resultでありこれらでオーバーライドしている。
//
//
// ARGUMENTS
// ModInvertedFile* invertedFile
//		転置ファイルへのポインタ
// const Query::ValidateMode mode
//		有効化モード
// ModInvertedQuery* rQuery
//		クエリ
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void 
ModInvertedQueryInternalNode::validate(InvertedFile* invertedFile,
									   const Query::ValidateMode mode,
									   ModInvertedQuery* rQuery)
{
	ModVector<ModInvertedQueryNode*>::Iterator child = children.begin();
	for (; child != children.end(); ++child) {
		if ((*child)->getType() == simpleTokenLeafNode) {
			// トークンノードだけ特別
			rQuery->validateSimpleTokenLeafNode(*child, invertedFile, mode);
		} else {
			(*child)->validate(invertedFile, mode, rQuery);
		}
	}
}

#ifdef DEBUG
//
// FUNCTION protected
// ModInvertedQueryInternalNode::showEstimatedValue -- 文書頻度見積もり値付き出力
// NOTES
// 文書頻度見積もり値を表示(debug用)。中間ノード用。QueryNodeで定義されている
// 内容をオーバライドしている。
//
// ARGUMENTS
// ModUnicodeString& out
//		sortFactor 値を含む文字情報
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedQueryInternalNode::showEstimatedValue(ModUnicodeString& out)
{
	ModUnicodeString prefix;
	this->prefixString(prefix, ModFalse, ModFalse);

	ModOstrStream tmpStream;

	tmpStream << this->estimateDocumentFrequency();
	ModUnicodeString df = ModUnicodeString(tmpStream.getString());

	out += '{';
	out += df;
	out += '}';
	out += prefix;
	out += '(';

	//
	// 子ノードの内容を表示
	//
	ModVector<ModInvertedQueryNode*>::Iterator p(children.begin());
	(*p)->showEstimatedValue(out);
	for (++p; p != this->children.end(); ++p) {
		out += ',';
		(*p)->showEstimatedValue(out);
	}
	out += ')';
}
#endif // DEBUG

// 
// FUNCTION protected
// ModInvertedQueryInternalNode::getQueryString -- 検索条件ノードを出力
// 
// NOTES
// 検索条件ノードを出力。ラフノードの内容を表示するかどうかを選択できる
//
// ARGUMENTS
// ModOstrStream& out
//		結果格納用オブジェクト
// ModBoolean withRouh
//		ラフノードを表示するかどうかを示すフラグ（trueで表示）
// 
// RETURN
// なし
// 
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedQueryInternalNode::getQueryString(ModUnicodeString& out,
	const ModBoolean asTermString,
	const ModBoolean withCalOrCombName,
	const ModBoolean withCalOrCombParam,
	const ModBoolean withRouh) const
{
	//
	// Thisノードを出力する
	//

	// [YET] ThisはPrecise Nodeとは限らないと思われる。

	ModUnicodeString termString;
#ifdef V1_6
	ModLanguageSet tmpLangSet;
#endif // V1_6
	ModInvertedTermMatchMode tmpMmode;
	if (asTermString == ModTrue && getOriginalString(termString,
#ifdef V1_6
													 tmpLangSet,
#endif // V1_6
													 tmpMmode) != ModFalse) {

		//
		// 検索条件の検索語の部分を取得できた場合
		//

		//
		// まず検索条件の検索語以外の部分を出力
		//
		
		out += "#term[";
#ifdef V1_6
		if(tmpMmode == ModInvertedTermExactWordMode) {
			out += "e,";
		} else if(tmpMmode == ModInvertedTermWordHead) {
			out += "h,";
		} else if(tmpMmode == ModInvertedTermWordTail) {
			out += "t,";
		} else if(tmpMmode == ModInvertedTermMultiLanguageMode) {
			out += "m,";
		} else {
			out += "n,";
		}
#else
		// 子ノードの内容を取得
		ModBoolean isChildTerm(ModFalse);
		ModUnicodeString childString,childPrefixString;
		ModVector<ModInvertedQueryNode*>::ConstIterator p;
		p = children.begin();
		for (; p != this->children.end(); ++p) {
			(*p)->getQueryString(childString,
					asTermString, withCalOrCombName, withCalOrCombParam, withRouh);

			//子ノードをチェックし、term[h/t]だった場合は出力文字列に追加
			if (childString.compare("#token(\"EMPTY\")") == 0) {
				this->prefixString(childPrefixString, ModFalse, ModFalse);
				if (childPrefixString.compare("#distance[0]") == 0) {
					out += 'h';
				} else {
					out += 't';
				}
				isChildTerm = ModTrue;
				break;
			}
		}

		if(isChildTerm == ModFalse) {
			out += 'n';
		}
#endif // V1_6

		if (withCalOrCombName == ModTrue) {
			ModUnicodeString calculatorName;
			getCalculatorOrCombinerName(calculatorName, withCalOrCombParam);

			if (calculatorName.getLength() > 0) {
#ifndef V1_6
				out += ',';
#endif // NOT V1_6
				out += calculatorName;
			}
		}
#ifdef V1_6
		out += ',';
		out += tmpLangSet.getName();
#endif // V1_6
		out += "](";

		//
		// 検索条件の文字列部分を出力
		//
		
		ModUnicodeString tmp;
		ModInvertedQueryParser::convertTermString(termString , tmp);
		out += tmp;
		
	} else {
		
		//
		// 検索条件の文字列を取得できない場合
		//
		
		// calculator/combinerの表示ON/OFFが出来るようにする
		ModUnicodeString prefix;
		this->prefixString(prefix, withCalOrCombName, withCalOrCombParam);

		out += prefix;
		out += '(';

		if (children.isEmpty() == ModFalse) {
			//
			// 子ノードの内容を表示
			//

			// 一番目の子ノードを表示
			ModVector<ModInvertedQueryNode*>::ConstIterator p;
			p = children.begin();
			(*p)->getQueryString(termString,
								 asTermString, withCalOrCombName,
								 withCalOrCombParam, withRouh);
			out += termString;

			// 二番目以降の子ノードを表示
			for (++p; p != this->children.end(); ++p) {
				out += ',';
				termString.clear();
				(*p)->getQueryString(termString,
									 asTermString, withCalOrCombName,
									 withCalOrCombParam, withRouh);
				out += termString;
			}
		}
	}
	out += ')';

	//
	// rough node 表示
	//
	
	ModUnicodeString roughString;
	if (withRouh == ModTrue
		&& queryNodeForRoughEvaluation != 0
		&& queryNodeForRoughEvaluation != 
			const_cast<ModInvertedQueryInternalNode*>(this) ) {
		
		//
		// Thisと異なるNodeがRough Nodeに設定されている場合
		//
		
		out += '<';
		this->queryNodeForRoughEvaluation->getQueryString(roughString,
				asTermString, withCalOrCombName, withCalOrCombParam, withRouh);
		out += roughString;
		out += '>';
	}
}

//
// FUNCTION protected
// ModInvertedQueryInternalNode::changeSimpleTypeNode -- 中間ノードの単純化
// 
// NOTES
// 中間ノードの単純化。
//
// #and(条件) → 条件
// #or(条件) → 条件
// #and-not(条件,()) → 条件
// #and-not((),条件) → ()
//
// というような children が1つしかない中間ノード単純化する関数。中間ノー
// ドの共有化関数の補助関数。
// ランキング検索の場合はcheckSimpleTypeNode()を用いて親ノードと子ノードが
// 変換可能な組合せかチェックする必要がある。
//
//
// ARGUMENTS
// ModVector<ModInvertedQueryNode*>::Iterator child
//		単純化する中間ノードの反復子
// QueryNodePointerMap& nodePointerMap
//		OR標準形変換により共有されているノードが登録されているMap
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedQueryInternalNode::changeSimpleTypeNode(
	ModVector<ModInvertedQueryNode*>::Iterator child,
	QueryNodePointerMap& nodePointerMap)
{

	ModVector<ModInvertedQueryNode*>* grandChildren;
	grandChildren = ((InternalNode*)*child)->getChildren();
	ModVector<ModInvertedQueryNode*>::Iterator grandChild;
	grandChild = grandChildren->begin();

	NodeType cType = ModInvertedAtomicMask((*child)->getType());

	if (cType == ModInvertedQueryNode::operatorLocationNode ||
		cType == ModInvertedQueryNode::operatorEndNode ||
#ifdef V1_4     // 単語単位検索
		cType == ModInvertedQueryNode::operatorWordNode ||
#endif // V1_4
 		cType == ModInvertedQueryNode::operatorScaleNode) {
		return;
	}

	if (cType == ModInvertedQueryNode::operatorAndNotNode) {
		if (*grandChild == QueryNode::emptySetNode) {
			// #and-not((),条件) → ()
			// 1番目のgrandChildが0であった場合
			// 2番目の And-Not の子ノードを破棄
			addQueryNodePointerMap(*(grandChild + 1), nodePointerMap);
			// nodePointerMap に And-Not 自身を登録
			addQueryNodePointerMap(*child, nodePointerMap);

			// 空集合ノードをセット
			*child = const_cast<ModInvertedQueryNode*>(QueryNode::emptySetNode);

		} else if (*(grandChild + 1) == QueryNode::emptySetNode) {

			if (checkSimpleTypeNode(*child, *grandChild) != ModTrue) {
				return;
			}
			// #and-not(条件,()) → 条件
			// 2番目のgrandChildが0であった場合
			// nodePointerMap に And-Not 自身を登録
			nodePointerMap[*child] = 1;
			// grandChild(第1パラメータ)を And-not の位置に昇格
			*child = *grandChild;
		} else {
			; ModAssert(0);
		}

	} else if (cType == ModInvertedQueryNode::operatorAndNode) {

		if (checkSimpleTypeNode(*child,*grandChild) != ModTrue) {
			// 変換不可
			return;
		}

		// Andを破棄
		addQueryNodePointerMap(*child, nodePointerMap);
		*child = *grandChild;

	} else if (cType == ModInvertedQueryNode::operatorOrNode) {
		if (checkSimpleTypeNode(*child,*grandChild) != ModTrue) {
			// 変換不可
			return;
		}

		// Or を破棄
		// ただしshortWordのOrの場合には子ノードが一つでも
		// Orの破棄は行わない。(BUG_5052対応)
		// shortWord用Orを破棄するとshortWordであるという情報を失うため
		// shortWord用のOrは破棄しない。

		if ((static_cast<OrNode*>(*child))->getShortWordLength() == 0) {
			// shortWord用のOrでないのでOrを破棄
			addQueryNodePointerMap(*child, nodePointerMap);
			*child = *grandChild;
		}
	} else {
		; ModAssert(0);
	}
}

//
// FUNCTION protected
// ModInvertedQueryInternalNode::checkSimpleTypeNode -- 親ノードと子ノードの関係が単純化可能か検査する
// 
// NOTES
// 中間ノードの単純化(changeSimpleTypeNode():子ノードがひとつしかない中間ノード
// の単純化)から呼出され、親ノード、子ノードの関係が単純化可能なものかチェック
// する。
// ブーリアン検索の場合は常に単純化可能、
// ランキング検索の場合は下表のようになる 
//		親ノード	子ノード
//		AtomicNode	AtomicNode		calculatorの種類が同じ場合は変換可
//		AtomicNode	RankingNode		変換不可
//		RankingNode AtomicNode		変換可
//		RankingNode RankingNode		変換可
// 
// ARGUMENTS
// ModVector<ModInvertedQueryNode*>::Iterator child
//		単純化する中間ノードの反復子
// QueryNodePointerMap& nodePointerMap
//		OR標準形変換により共有されているノードが登録されているMap
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModBoolean
ModInvertedQueryInternalNode::checkSimpleTypeNode(
	ModInvertedQueryNode* upperNode, ModInvertedQueryNode* lowerNode)
{
	ModBoolean ret = ModTrue;

	QueryNode::NodeType upperType = upperNode->getType();

	if (ModInvertedIsAtomicQueryNode(upperType) == ModFalse) {
		// 親ノードがアトミックノードでなければ変換可 
		return ret;

	} else {
		// アトミックノードの場合
		QueryNode::NodeType lowerType = lowerNode->getType();

		if (ModInvertedIsAtomicQueryNode(lowerType) != ModTrue) {
			// 親ノードがアトミックで子ノードがランキングの場合は変換不可
			// 但し例外あり

			//  例外のケースか確認
			if ((ModInvertedAtomicMask(lowerType) 
						== ModInvertedQueryNode::termLeafNode) || 
				(ModInvertedAtomicMask(lowerType) 
						== ModInvertedQueryNode::simpleTokenLeafNode)) {


				// TermLeafNodeの場合はAtomicノードの判定条件を使う
				// lowerTypeにAtomicNodeをセット
				lowerType = QueryNode::atomicNode;
			} else {
				// 変換不可
				ret = ModFalse;
			}
		}

		if (ModInvertedIsAtomicQueryNode(lowerType) == ModTrue) {
			// 下のノードもアトミック
			// (rankingTermLeafNodeの場合)
			ModUnicodeString upperCalculatorName;
			ModUnicodeString lowerCalculatorName;
			// パラメータまで比較する
			upperNode->getCalculatorOrCombinerName(
				upperCalculatorName, ModTrue);
			lowerNode->getCalculatorOrCombinerName(
				lowerCalculatorName, ModTrue);

			if (upperCalculatorName != lowerCalculatorName) {
				// アトミック・アトミックの組合せではcalculatorの種類が
				// 同じ場合のみ変換可
				ret = ModFalse;
			}
		}
	}
	return ret;
}

//
// FUNCTION protected
// ModInvertedQueryInternalNode::addQueryNodePointerMap -- 破棄すべき中間ノードを登録する関数
// 
// NOTES
// 破棄すべき中間ノードを登録する関数
//
// OR標準形変換後中間ノードの共有化を行なう。ノードの共有化で共有する
// ことになったノードは破棄することとなるが、そのノードがOR標準形変換
// にて共有しているノードである場合破棄することはできない。この判断を
// するためにOR標準形変換時に登録した nodePointerMap へ破棄すべきノー
// ドは登録し、Queryのデストラクト時に delete を行なうようにする。本関
// 数は nodePointerMap へ引数 nodeIterator で示されたノードを子ノード
// も含めて全て登録するものである。
//
// さらに queryNodeForRoughEvaluation にノードがセットされていた場合は
// これも nodePointerMap へ登録する。
//
// ARGUMENTS
// ModInvertedQueryNode* node
//		追加登録するノードの反復子
// QueryNodePointerMap& nodePointerMap
//		OR標準形変換により共有されているノードが登録されているMap
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedQueryInternalNode::addQueryNodePointerMap(
	ModInvertedQueryNode* node,
	QueryNodePointerMap& nodePointerMap)
{
	if (ModInvertedIsInternalNode(node->getType()) != ModTrue ||
		node == emptySetNode) {
		// internalノードでない、空集合ノードの場合は何もしない
		return;
	}

	// nodePointerMap に node を登録
	nodePointerMap[node] = 1;

	// queryNodeForRoughEvaluation をチェック
	if (node->getQueryNodeForRoughEvaluation() != 0) {
		// queryNodeForRoughEvaluation にセットされているノードをセット

		if (ModInvertedAtomicMask(
			node->getQueryNodeForRoughEvaluation()->getType())
			!= simpleTokenLeafNode) {
			// simpleTokenLeafNode以外の場合は登録する
			nodePointerMap[node->getQueryNodeForRoughEvaluation()] = 1;
		}

	}

	ModVector<ModInvertedQueryNode*>* grandChildren =
		static_cast<InternalNode*>(node)->getChildren();

	ModVector<ModInvertedQueryNode*>::Iterator grandChild;
	for (grandChild = grandChildren->begin();
		 grandChild != grandChildren->end();
		 ++grandChild) {
		addQueryNodePointerMap(*grandChild, nodePointerMap);
	}
}

//
// FUNCTION protected
// ModInvertedQueryInternalNode::makeRoughMap -- 子ノードのラフノードを集めたマップを生成する。makeRoughPointer() の下請け関数
// 
// NOTES
// 引数で渡された子ノードが中間ノードでラフポインタがセットされていた
// らそのラフポインタの内容を map に加える。ラフポインタがセットされて
// いなかった場合は makeRoughPointer() をそのノードに対して呼び出す。
//
// 中間ノードでなく索引語ノード(simpleTokeLeafNode) であり、モードが索
// 引語ノード(simpleTokeLeafNode)もラフポインタに加えるモードであった
// 場合はその索引語ノードを map に加える。
//
// もし渡されたノードが空集合ノードであった場合はラフポインタの内容を
// 全て削除し ModFalseでリターンする。
//
// ラフノードをあつめるのに ModMap を使っている理由は同じノードを複数
// 入れないためである。その為 KEY にノードのアドレスを指定してだぶるこ
// とのないようにしている。VALUE には 1 とか 2 を代入しているが、特に
// 意味はない。
//
//
// ARGUMENTS
// const Query::ValidateMode mode
//		ValidateMode
// QueryNode* node
//		このノードのラフノードを集める
// QueryNodePointerMap& nodePointerMap
//		結果格納要のマップ
// const ModInvertedQuery* Query
//		Query
//
// RETURN
// マップ生成に成功した場合はModTrue、失敗した場合はModFalse
// (失敗=子ノードのラフノードに空集合ノードがあった場合→マップをクリアしている)
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
//
//
ModBoolean
ModInvertedQueryInternalNode::makeRoughMap(const Query::ValidateMode mode,
										   QueryNode* node,
										   QueryNodePointerMap& map,
										   const ModInvertedQuery* Query)
{
	QueryNode::NodeType cType = ModInvertedAtomicMask(node->getType());
	if (ModInvertedIsInternalNode(cType) == ModTrue) {
		// 中間ノードの場合 makeRoughPointer を呼び出す
		QueryNodePointerMap tmpMap;
		node->makeRoughPointer(mode, tmpMap, Query);

		// 子ノードで生成したmapの内容を自分のマップに入れる
		map.insert(tmpMap.begin(), tmpMap.end());

	} else {
		// LeafNode である
		if (cType == ModInvertedQueryNode::simpleTokenLeafNode) {

			// makeRoughOfSimpleNode → makeRoughに従う
			// ここに来るのはmakeRoughのケースなのでモードのチェックは不要
			map[node] = 2;		// 2を代入していることには特に意味はない

		} else if (cType == ModInvertedQueryNode::booleanResultLeafNode) {
			if (node == QueryNode::emptySetNode) {
				// booleanResultLeafNodeでかつ空集合ノード
				map.erase(map.begin(), map.end());
				return ModFalse;
			} 
			else {
				// mapにBooleanResultLeafNodeを登録
				if ((static_cast<ModInvertedBooleanResultLeafNode*>(node))
					->getSize() <= sizeLimitOfResultLeafNodeToAddToRoughMap) {
					// booleanResultLeafNode/rankingResultLeafNodeは件数が
					// sizeLimitOfResultLeafNodeToAddToRoughMapより小さい場合
					// のみラフノードに追加する

					map[node] = 1;
				}
			}
		} else if (cType == ModInvertedQueryNode::rankingResultLeafNode) {
			if (node == QueryNode::emptySetNode) {
				// RankingResultLeafNodeでかつ空集合ノード
				map.erase(map.begin(), map.end());
				return ModFalse;
			} 
			else {
				// mapにBooleanResultLeafNodeを登録
				if (static_cast<ModInvertedRankingResultLeafNode*>(node)
					->getSize() <= sizeLimitOfResultLeafNodeToAddToRoughMap) {
					// booleanResultLeafNode/rankingResultLeafNodeは件数が
					// sizeLimitOfResultLeafNodeToAddToRoughMapより小さい場合
					// のみラフノードに追加する

					map[node] = 1;
				}
			}
		}
	}
	return ModTrue;
}

//
// FUNCTION protected
// ModInvertedQueryInternalNode::makeRoughNode -- ラフノードを作る
// 
// NOTES
// 引数mapに指定された子ノードをもつラフノードを作成する。
//
// ラフノードは And ノードでこのAndノードのラフポインタはAndノード自身
// を指すようにセットする。
//
// ただしラフノードの子ノードになるノードがひとつしかない場合はAndノー
// ドは作らず、そのノードが直接ラフノードとなる。
//
// Andノードのラフノードを作成する場合、子ノードが全て
// simpleTokenLeafNodeであった場合は、ラフノードを新たに生成することは
// せず、AndノードのラフポインタをそのAndを指すようにセットするだけに
// する。
//
// ARGUMENTS
// const QueryNodePointerMap& map
//		ラフノードにセットすべきノードを表わす
// const ModInvertedQuery* Query
//		クエリ
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedQueryInternalNode::makeRoughNode(const QueryNodePointerMap& map,
									const ModInvertedQuery* Query,
									const ModInvertedQuery::ValidateMode mode)
{
	// BUG_5393対応 000807 - And/AndNotから移動
	//	↓
	// すでに共有されて、ラフノード作成済みのノードであっても、
	// Mapの作成は行う必要がある。 (親ノードのラフノード作成のため)
	//
	// 以前はAnd/AndNotのmakeRoughPointerでチェックするようにしていたが、
	// それではMapを作成できないため、親ノードのラフノードが作成されなかった
	//
	if (queryNodeForRoughEvaluation != 0) {
		// 共有されているノードで既にラフノードは作成済み
		// ラフノードの作成は行わない
		return;
	}

	ModSize size = map.getSize();
	if (size == 0) {
		// rough pointer は作成できなかった
		; ModAssert(queryNodeForRoughEvaluation == 0);
		return;

	} else if (size == 1) {
		// rough pointer の内容は simpleTokenNode 一つ
		QueryNodePointerMap::ConstIterator i = map.begin();
		this->queryNodeForRoughEvaluation = (*i).first;

		queryNodeForRoughEvaluation->
			setQueryNodeForRoughEvaluation(queryNodeForRoughEvaluation);

		return;
	}

	// ノード数が2以上
	if (this->getType() == ModInvertedQueryNode::operatorAndNode) {
		// and ノード
		ModVector<ModInvertedQueryNode*>::ConstIterator child
			= this->children.begin();
		for (; child != this->children.end(); ++child) {
			if (ModInvertedAtomicMask((*child)->getType())
				!= ModInvertedQueryNode::simpleTokenLeafNode) {
				break;
			}
		}
		if (child == this->children.end()) {
			// 全てが simpleTokenLeafNode であった
			// ラフポインタの内容を自分自身とし、ラフノードは生成しない
			this->queryNodeForRoughEvaluation = this;
			return;
		}
	}
	// ノード数が2以上なのでラフノードとしてのANDノードを生成
	this->queryNodeForRoughEvaluation = createRoughNode(Query, mode);

		// 生成したANDノードに子ノードを挿入する
	ModVector<ModInvertedQueryNode*>* roughChildren
		= (static_cast<AndNode*>(queryNodeForRoughEvaluation))->getChildren();

	roughChildren->reserve(size);
		
	QueryNodePointerMap::ConstIterator p = map.begin();
	QueryNodePointerMap::ConstIterator end = map.end();
	for (;p != end; ++p) {
		roughChildren->pushBack((*p).first);
	}

	// ラフノードの子ノードを sortFactor でソート
	ModSort(roughChildren->begin(), roughChildren->end(),
			ModInvertedQueryNode::lessSortFactor);

	// ラフノードのラフポインタは自分を指すようにする
	queryNodeForRoughEvaluation
		->setQueryNodeForRoughEvaluation(queryNodeForRoughEvaluation);

}

//
// FUNCTION public
// ModInvertedQueryInternalNode::reserveScores -- scoresをリザーブ
//
// NOTES
// 子ノードを辿りrankingOr,rankingAndの場合はメンバ変数であるscoresを
// リザーブする。本関数がコールされるのはrankingOr,rankingAnd以外の
// 中間ノードの場合であるため、自分の子ノードをたどる。
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
void
ModInvertedQueryInternalNode::reserveScores()
{
	ModVector<ModInvertedQueryNode*>::Iterator childNodes;
	for (childNodes = this->children.begin();
		 childNodes != this->children.end();
		 ++childNodes) {
		// 再帰呼び出し
		static_cast<BaseNode*>(*childNodes)->reserveScores();
	}

	// ラフノードを使ったスコア計算でscoresがリザーブされていないとエラー
	if (this->queryNodeForRoughEvaluation != 0 &&
		this->queryNodeForRoughEvaluation != this) { 
		static_cast<BaseNode*>(this->queryNodeForRoughEvaluation)->reserveScores();
	}
}

//
// FUNCTION public
// ModInvertedQueryInternalNode::flattenChildren -- 子ノードリストの平坦化
//
// NOTES
// 子ノードリストの平坦化。ただしOperatorWindowNodeは平坦化しないので、
// 自分の子供に再帰的にflattenChildren() をコールする。その際に
// isChildOfWindowNode を ModTrue としてコールする。
//
// ARGUMENTS
// const QueryNodePointerMap& sharedNodeMap
//              OR標準形変換時に共有しているノードが登録されているマップ変数
// const ModBoolean isChildOfWindowNode
//              windowノードを親にもっている場合 ModTure となる
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedQueryInternalNode::flattenChildren(
		const QueryNodePointerMap& sharedNodeMap,
		const ModBoolean isChildOfWindowNode)
{
	ModVector<ModInvertedQueryNode*>::Iterator p(children.begin());
	ModVector<ModInvertedQueryNode*>::Iterator end(children.end());
	for (; p != end; ++p) {
		(*p)->flattenChildren(sharedNodeMap, ModTrue);
	}
}

//
// FUNCTION public
// ModInvertedQueryInternalNode::doFirstStepInRetrieveScore()
//
// NOTES
//
// ARGUMENTS
// const ModInvertedDocumentID maxDocumentId
//			ドキュメントID
//
// RETURN
//
// EXCEPTIONS
//
void
ModInvertedQueryInternalNode::doFirstStepInRetrieveScore(
		ModInvertedBooleanResult *expungedDocumentId,
		const ModInvertedDocumentID maxDocumentId)
{
	ModVector<ModInvertedQueryNode*>::Iterator child(children.begin());

	for (;child != children.end(); ++child) {
		// 各子ノードに対してdoFirstStepInRetrieveScore()を呼び出す
		(*child)->doFirstStepInRetrieveScore(expungedDocumentId,maxDocumentId);
	}
}

//
// FUNCTION
// ModInvertedQueryInternalNode::doSecondStepInRetrieveScore -- ランキング検索の第２ステップ
//
// NOTES
// ランキング検索で、スコア計算の第２ステップのみを実施する。
// 第１ステップの結果を用い、最終的な検索結果を生成する。
// 第１ステップを実施していない場合の結果は不定。
//
// ARGUMENTS
// ModInvertedRankingResult* result_
//      検索結果
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedQueryInternalNode::doSecondStepInRetrieveScore(
	ModInvertedSearchResult* &result_)
{
	doSecondStepInRetrieveScore();
	ModInvertedDocumentID id(1);
	ModInvertedDocumentScore score;
	ModInvertedSearchResult *result = ModInvertedSearchResult::factory(getRankingResult()->getType());
	while (lowerBoundScoreForSecondStep(id, id, score) == ModTrue) {
		result->pushBack(id, score);
		++id;
	}
	result_ = result;
	delete getRankingResult();
	setRankingResult(result);
}

//
// FUNCTION
// ModInvertedQueryInternalNode::doSecondStepInRetrieveScore -- ランキング検索の第２ステップ
//
// NOTES
// ランキング検索で、スコア計算の第２ステップのみを実施する。
// ここでは子ノードに対してdoSecondStepInRetrieveScore()を呼び出すだけ。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedQueryInternalNode::doSecondStepInRetrieveScore()
{
	ModVector<ModInvertedQueryNode*>::Iterator child(children.begin());

	for (;child != children.end(); ++child) {
		// 各子ノードに対してdoFirstStepInRetrieveScore()を呼び出す
		(*child)->doSecondStepInRetrieveScore();
	}
}

//
//	FUNCTION public
//	ModInvertedQueryInternalNode::pushFreeList -- 不要な位置情報反復子をpushする
//
void
ModInvertedQueryInternalNode::pushFreeList(
	ModInvertedLocationListIterator* location)
{
	location->nextInstance = freeList;
	freeList = location;
}

//
//	FUNCTION public
//	ModInvertedQueryInternalNode::getFreeList -- 位置情報反復子を得る
//
ModInvertedLocationListIterator*
ModInvertedQueryInternalNode::getFreeList()
{
	ModInvertedLocationListIterator* p = freeList;
	if (p)
	{
		freeList = p->nextInstance;
	}
	return p;
}

//
// Copyright (c) 1997, 1999, 2000, 2001, 2002, 2003, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
