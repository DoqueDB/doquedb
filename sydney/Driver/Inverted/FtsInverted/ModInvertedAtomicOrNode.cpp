// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedAtomicOrNode.cpp -- 和集合ノードのアトミック版実装
// 
// Copyright (c) 1999, 2000, 2001, 2002, 2003, 2005, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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

#include "ModInvertedException.h"
#include "ModInvertedAtomicOrNode.h"
#include "ModInvertedQuery.h"
#include "ModInvertedFileCapsule.h"
#include "ModInvertedCompressedLocationListIterator.h"
#include "ModInvertedOrLocationListIterator.h"
#ifdef  SYD_INVERTED // SYDNEY 対応
#include "Inverted/ModInvertedFile.h"
#else
#include "ModInvertedFile.h"
#endif
#define ALL_SIMPLE

//
// FUNCTION public
// ModInvertedAtomicOrNode::ModInvertedAtomicOrNode -- アトミック版和集合ノードの生成
//
// NOTES
// 検索語に対応する検索式表現ノードオブジェクトを新しく生成する。
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
ModInvertedAtomicOrNode::ModInvertedAtomicOrNode(const ModUInt32 resultType_)
	: ModInvertedOperatorOrNode(resultType_)
{
	setType(AtomicNode::atomicOrNode);
}

//
// FUNCTION public
// ModInvertedAtomicOrNode::~ModInvertedAtomicOrNode -- アトミック版和集合ノードの破棄
//
// NOTES
// アトミック版和集合ノードを廃棄する。
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
ModInvertedAtomicOrNode::~ModInvertedAtomicOrNode()
{
	if (scoreCalculator != 0) {
		delete scoreCalculator;
		scoreCalculator = 0;
	}
}

#ifndef DEL_BOOL
// 
// FUNCTION public
// ModInvertedAtomicOrNode::retrieve -- ブーリアン検索の一括実行
// 
// NOTES
// ブーリアン検索の一括実行。内容は OperatorOrNodeのretrieve()を呼び出し
// ているだけ
//
// ARGUMENTS
// ModInvertedBooleanResult& queryResult
//		Boolean検索結果オブジェクト
// ModInvertedQueryNode::Mode processingMode
//		評価モード
// 
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedAtomicOrNode::retrieve(BooleanResult& queryResult,
								  Query::EvaluateMode mode)
{
	OrNode::retrieve(queryResult, mode);
}
#endif
// 
// FUNCTION public
// ModInvertedAtomicOrNode::evaluateScore -- 与えられた文書のスコアを計算する
// 
// NOTES
// 与えられた文書のスコアを計算する
//
// ARGUMENTS
// const DocumentID documentID
//		文書ID
// DocumentScore& score
//		スコア（結果格納用）
// evaluationMode mode
//		評価モード
// 
// RETURN
// ブーリアン検索条件にマッチしていれば True, アンマッチであれば False
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
// 
ModBoolean
ModInvertedAtomicOrNode::evaluateScore(const DocumentID documentID,
									   DocumentScore& score,
									   Query::EvaluateMode mode)
{
	return AtomicNode::evaluateScore(documentID, score, mode);
}

#if (!defined(MOD_DIST)) && (!defined(SYD_INVERTED)) // EVALUATESCORE
// 
// FUNCTION public
// ModInvertedAtomicOrNode::evaluateScore -- 与えられた文書のスコアを計算する（位置も計算する）
// 
// NOTES
// 与えられた文書のスコアを計算する
//
// ARGUMENTS
// const DocumentID documentID
//		文書ID
// DocumentScore& score
//		スコア（結果格納用）
// LocationIterator*& locations,
//		位置情報（結果格納用）
// evaluationMode mode
//		評価モード
// ModInvertedQueryNode* givenEndNode
// 		ここでは未使用。orderedDistanceでのみ使用。
// 
// RETURN
// ブーリアン検索条件にマッチしていれば True, アンマッチであれば False
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModBoolean
ModInvertedAtomicOrNode::evaluateScore(const DocumentID documentID,
									   DocumentScore& score,
									   LocationIterator*& locations,
									   Query::EvaluateMode mode,
									   ModInvertedQueryNode* givenEndNode)
{
	return AtomicNode::evaluateScore(documentID, score, locations, mode);
}
#endif

// 
// FUNCTION public
// ModInvertedAtomicOrNode::evaluate -- 与えられた文書が検索式を満たすかどうかの検査
// 
// NOTES
// 与えられた文書が検索式を満たすかどうか検査する。
// 
// ARGUMENTS
// ModInvertedDocumentID documentID
//		文書ID
// ModInvertedQueryNode::Mode processingMode
//		評価モード
// 
// RETURN
// 与えられた文書が検索式を満たす場合 ModTrue、満たさない場合 ModFalse
// 
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModBoolean
ModInvertedAtomicOrNode::evaluate(DocumentID documentID,
								  Query::EvaluateMode mode)
{
	return OrNode::evaluate(documentID, mode);
}

// 
// FUNCTION public
// ModInvertedAtomicOrNode::lowerBoundScore -- lowerBoundのランキング版（スコアも計算する）
// 
// NOTES
//
// ARGUMENTS
// ModInvertedDocumentID givenID
//		文書ID
// ModInvertedDocumentID& foundID
//		結果格納用の文書IDオブジェクト
// DocumentScore& score
//		スコア（結果格納用）
// evaluationMode mode
//		評価モード
// 
// RETURN
// そのような文書が存在する場合 ModTrue、存在しない場合 ModFalse
// 
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModBoolean
ModInvertedAtomicOrNode::lowerBoundScore(const DocumentID givenID,
										 DocumentID& foundID,
										 DocumentScore& score,
										 Query::EvaluateMode mode)
{
	return AtomicNode::lowerBoundScore(givenID, foundID, score, mode);
}

// 
// FUNCTION public
// ModInvertedAtomicOrNode::lowerBound -- 検索式を満たす文書のうち、文書IDが与えられた値以上で最小の文書の検索
// 
// NOTES
// 文書IDが与えられた値以上で、検索式を満たす文書の内、文書ID最小のものを
// 検索し、そのような文書が存在する場合は、与えられた文書IDオブジェクトに
// 結果を格納する。
// 
// ARGUMENTS
// ModInvertedDocumentID givenID
//		文書ID
// ModInvertedDocumentID& foundID
//		結果格納用の文書IDオブジェクト
// evaluationMode mode
//		評価モード
// 
// RETURN
// そのような文書が存在する場合 ModTrue、存在しない場合 ModFalse
// 
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModBoolean
ModInvertedAtomicOrNode::lowerBound(DocumentID givenID,
									DocumentID& foundID,
									Query::EvaluateMode mode)
{
	return OrNode::lowerBound(givenID, foundID, mode);
}

//
//
// FUNCTION public
// ModInvertedAtomicOrNode::flattenChildren -- 子ノードリストの平坦化
// 
// NOTES
// 子ノードリストの平坦化 (例 #or(#or(X,Y),Z) → #or(X,Y,Z))
// 
// ただしatomicの場合はscoreCalculatorの種類が同じ場合のみ可。
// 
// orFlattenThreshold より子ノード数が多い ORノードは平坦化しない。
//
// またショートワード用に生成された ORノードで、windowノードを親ノード
// にもっている場合は平坦化しない。ショートワード用の ORノードかどうか
// は shortWordLength が 0 以外であることで判断する。windowノードを親
// にもっているかは引数 isChildOfWindowNode で判断する。
//
// 平坦化後不要になったORノードを削除する際に sharedNodeMap を参照し、
// このMapに登録されているノードは削除しない。Queryのデストラクト時に
// sharedNodeMapを参照しながら削除する。
// 
// ARGUMENTS
// const QueryNodePointerMap& sharedNodeMap
//		OR標準形変換時に共有しているノードが登録されているマップ変数
// const ModBoolean isChildOfWindowNode
//		windowノードを親にもっている場合 ModTure となる
// 
// RETURN
// なし
// 
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedAtomicOrNode::flattenChildren(
	const QueryNodePointerMap& sharedNodeMap,
	const ModBoolean isChildOfWindowNode)
{

	ModVector<ModInvertedQueryNode*>::Iterator child = children.begin();

#ifdef ALL_SIMPLE
	// すべての子供が SimpleTokenLeafNode になるかを調べる
	ModBoolean allSimple;
	if (isChildOfWindowNode == ModTrue) {
		// Window の子ノードの場合、すべての子供が Simple でも shorWordLength
		// を書き換えてはいけないので、書き換えが起こらないように false にする
		allSimple = ModFalse;
	} else {
		allSimple = ModTrue;
		while (child != children.end()) {
			switch ((*child)->getType()) {
			case AtomicNode::simpleTokenLeafNode:
				break;
			case AtomicNode::operatorOrNode:
				if ((static_cast<OrNode*>(*child))->getShortWordLength() == 0) {
					allSimple = ModFalse;
					goto fini;
				}
				break;
			default:
				allSimple = ModFalse;
				goto fini;
				break;
			}
			++child;
		}
	fini:
		;
	}
#endif // ALL_SIMPLE

	ModUnicodeString calculatorName;
	ModUnicodeString childCalculatorName;

	// パラメータまで取得する
	this->getCalculatorOrCombinerName(calculatorName, ModTrue);
	child = children.begin();
	while (child != children.end()) {

		// 子ノードを平坦化をする
		(*child)->flattenChildren(sharedNodeMap, isChildOfWindowNode);

        if ((*child)->getType()
			!= ModInvertedQueryBaseNode::atomicOrNode) {
			// AtomicOr ノード以外の場合
			++child;
			continue;
		}

		//
		// AtomicOr ノードの場合
		//
		ModVector<ModInvertedQueryNode*>* grandChildren
			= (*child)->getChildren();

		// パラメータまで取得する
		(*child)->getCalculatorOrCombinerName(childCalculatorName, ModTrue);

		if (calculatorName != childCalculatorName) {
			// calculatorの種類が異なるため平坦化できない
			++child;
			continue;
		}
		
		if (isChildOfWindowNode == ModTrue) {
			// 親ノードにwindowノードがある
			if (this->shortWordLength
				!= (static_cast<OrNode*>(*child))->getShortWordLength()) {
				// 自分のshortWordLengthと子ノードのshortWordLengthが不一致
				// なので平坦化しない
				++child;
				continue;
			}
		}

		if (ModInvertedQuery::orFlattenThreshold < grandChildren->getSize()) {
			//子ノードの数が多いので平坦化しない
			++child;
			continue;
		}

		//
		// 平坦化処理
		//
		ModVector<ModInvertedQueryNode*>::Iterator grandChild;
		grandChild = grandChildren->begin();
		ModInvertedQueryNode* reducedChild;
		reducedChild = *child;

		// OR のあった位置に孫の1番目のノードをセット
		*child++ = *grandChild++;

		// childのindexを計算
		ModSize n = child - children.begin();

		// 孫の2番目以降のノードをセット
		children.insert(child, grandChild, grandChildren->end());

		// child を孫を挿入した位置まで戻す
		child = (children.begin() + n - 1);

		if (sharedNodeMap.find(reducedChild) == sharedNodeMap.end()) {
			// sharedNodeMapには登録されていない
			delete reducedChild;		// OR ノードを削除
		}
	}

#ifdef ALL_SIMPLE
	if (shortWordLength == 0 && allSimple == ModTrue) {
		shortWordLength = ModSizeMax;
	}
#endif
}

// 
// FUNCTION public
// ModInvertedAtomicOrNode::getTermFrequency -- 文書内頻度の取得
// 
// NOTES
// 条件を満たす語句の出現頻度を求める。QueryNode::getTermFrequency() 
// を呼び出すだけ
// 
// ARGUMENTS
// ModInvertedDocumentID documentID
//		文書ID
// 
// Query::EvaluateMode mode
// 		評価モード
// 
// 		calAtomicOrTfByAddChildTfビットがONかつショートワード用Orの場合は、
// 		子ノード()のgetTermFrequency()結果の和をAtomicOrのTfとする
// 		それ以外の場合はQueryNode::getTermFrequency()により
//		OrLocationListIteratorを作成しTfを求める。
// 
// RETURN
// 求めた文書内頻度
// 
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModSize
ModInvertedAtomicOrNode::getTermFrequency(DocumentID documentID,
										  Query::EvaluateMode mode)
{
	if ((mode & Query::calAtomicOrTfByAddChildTf) != 0) {
		// mode の calAtomicOrTfByAddChildTfビットがONの場合は
		// ショートワード用OrのTfは子ノードのTfの和とする。

		ModSize tf = 0;
		for (ModVector<QueryNode*>::Iterator child(this->children.begin()),
				 end(this->children.end()); child != end; ++child) {
			tf += (*child)->getTermFrequency(documentID, mode);
		}

		return tf;
	}

	return ModInvertedQueryBaseNode::getTermFrequency(documentID, mode);
}

//
// FUNCTION public
// ModInvertedAtomicOrNode::prefixString -- 演算子を表わす文字列を返す
//
// NOTES
// OrNodeで定義された内容をオーバライドする。
// 演算子を表わす文字列を返す
//
// ARGUMENTS
// ModString& prefix
//	演算子を表わす文字列を返す(結果格納用)
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void 
ModInvertedAtomicOrNode::prefixString(ModUnicodeString& prefix,
	const ModBoolean withCalOrCombName, 
	const ModBoolean withCalOrCombParam) const
{
	prefix += "#syn";

	if (withCalOrCombName) {
		ModUnicodeString calculatorName;
		getCalculatorOrCombinerName(calculatorName, withCalOrCombParam);
		if (calculatorName.getLength() > 0) {
			prefix += '[';
			prefix += calculatorName;
			prefix += ']';
		}
	}
}

//
// FUNCTION public
// ModInvertedAtomicOrNode::duplicate -- 自分のコピーを作成する
//
// NOTES
// 自分のコピーを作成する。Query::から呼ばれ、検索木を上からたどり、
// 検索木のコピーを作成する際に使用される。
// QueryNode::dupulicateをオーバーライド。TermLeafNodeのコピーを作る。
//
// ARGUMENTS
// const ModInvertedQuery&
//		クエリ。
//
// RETURN
// 生成したコピーのノードのポインタ
//
// EXCEPTIONS
// ModInvertedErrorQueryValidateFail
//
ModInvertedQueryNode*
ModInvertedAtomicOrNode::duplicate(const ModInvertedQuery& rQuery)
{
	ModInvertedAtomicOrNode* node = new ModInvertedAtomicOrNode(firstStepResult->getType());

	if (this->scoreCalculator != 0) {
		node->scoreCalculator = this->scoreCalculator->duplicate();
	}

	ModVector<ModInvertedQueryNode*>::Iterator child = this->children.begin();
	ModVector<ModInvertedQueryNode*>::Iterator end = this->children.end();

	// 子ノードのduplicate
	while (child != end) {

		// 子ノードのduplicateの結果
		ModInvertedQueryNode* newChild;

		newChild = (*child)->duplicate(rQuery);

		// 子ノードを追加
		node->insertChild(newChild);
		++child;
	}

	// totalDocuemntFrequencyのセット
	node->setTotalDocumentFrequency(totalDocumentFrequency);

	return static_cast<OrNode*>(node);
}

//
// FUNCTION public
// ModInvertedAtomicOrNode::getSearchTermList --
//
// NOTES
//
// ARGUMENTS
//
// RETURN
//
// EXCEPTIONS
//
void 
ModInvertedAtomicOrNode::getSearchTermList(
	ModInvertedQuery::SearchTermList& vecSearchTerm_,
	ModSize uiSynonymID_) const
{
	// 同義語集合を表すIDは、格納済み検索語数 + 1 (つまり>0) とする。
	// 上から渡された同義語IDは無視する。
	ModSize uiSynonymID = vecSearchTerm_.getSize() + 1;

	ModVector<ModInvertedQueryNode*>::ConstIterator i = children.begin();
	const ModVector<ModInvertedQueryNode*>::ConstIterator e = children.end();
	for (; i != e; ++i)
	{
		(*i)->getSearchTermList(vecSearchTerm_, uiSynonymID);
	}
}

//
// FUNCTION protected
// ModInvertedAtomicOrNode::validate -- 正規表現ノードの有効化
//
// NOTES
// 初期化としてベクターファイルのオープン，ヒープファイルのオープン，
// 正規表現のコンパイルを行う。
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
ModInvertedAtomicOrNode::validate(
	InvertedFile* invertedFile,
	const Query::ValidateMode mode,
	ModInvertedQuery* rQuery)
{
	// ランキング検索の場合はスコア計算器をセットする
	if ((mode & Query::rankingMode) != 0) {
		if (this->scoreCalculator == 0) {
			// QueryNodeには必ずデフォルトの計算機をセットするように
			// なったので、ここではduplicateだけ
			ScoreCalculator* calculator = rQuery->getDefaultScoreCalculator();
			;ModAssert(calculator != 0);
			setScoreCalculator(calculator->duplicate());
		}
		scoreCalculator->setDocumentLengthFile(
			invertedFile->getDocumentLengthFile());
	}
	
	// totalDocumentFrequencyのセット
	// 以前はduplicate()で行っていたが、validate()で行うのが正しい
	setTotalDocumentFrequency(rQuery->getTotalDocumentFrequency());

	ModUnicodeString termString, tmpTermString;
	getQueryString(termString, ModTrue, ModFalse, ModFalse, ModFalse);
#ifdef V1_6
	ModLanguageSet tmpLangSet;
#endif // V1_6
	ModInvertedTermMatchMode tmpMmode;
	getOriginalString(tmpTermString,
#ifdef V1_6
					  tmpLangSet,
#endif // V1_6
					  tmpMmode);
	setOriginalString(termString,
#ifdef V1_6
					  tmpLangSet,
#endif // V1_6
					  tmpMmode);

	InternalNode::validate(invertedFile,mode,rQuery);
}

// FUNCTION protected
// ModInvertedAtomicOrNode::sortChildren -- 子ノードリストの並べ替え
//
// NOTES
// 子ノードリストを並べ替える。
// 検索処理コスト順に並べ替える。
//
// もともとOperatorOrにあったもの。今はOperatorOrはRankingOrにあったsortChildren
// を使用している
//
// ARGUMENTS
// ModInvertedQuery::ValidateMode mode
//		有効化モード
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedAtomicOrNode::sortChildren(const ModInvertedQuery::ValidateMode mode)
{
	InternalNode::sortChildren(mode);

	// OR とは異なり、ランキングの時でもソートしてかまわない

	ModSort(this->children.begin(), this->children.end(),
			ModInvertedAtomicNode::moreFrequent);
}

// 
// FUNCTION protected
// ModInvertedAtomicOrNode::reevaluate -- 与えられた文書が検索式を満たすかどうかの検査
// 
// NOTES
// 与えられた文書が検索式を満たすかどうか検査する。
// 
// ARGUMENTS
// ModInvertedDocumentID documentID
//		文書ID
// 
// RETURN
// 与えられた文書が検索式を満たす場合 ModTrue、満たさない場合 ModFalse
// 
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModBoolean
ModInvertedAtomicOrNode::reevaluate(ModInvertedDocumentID documentID)
{
	return OrNode::reevaluate(documentID);
}

// 
// FUNCTION protected
// ModInvertedAtomicOrNode::reevaluate -- 与えられた文書が検索式を満たすかどうかの検査
// 
// NOTES
// 与えられた文書が検索式を満たすかどうか検査する。
// 検索式を満たす場合、文書中の出現位置の情報も求める。
// 
// ARGUMENTS
// ModInvertedDocumentID documentID
//		文書ID
// ModInvertedLocationListIterator*& locations
//		出現位置反復子へのポインタ (結果格納用)
// ModSize& uiTF_
//		(位置情報リストを取得できなかった場合) TF
// 
// RETURN
// 与えられた文書が検索式を満たす場合 ModTrue、満たさない場合 ModFalse
// 
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModBoolean
ModInvertedAtomicOrNode::reevaluate(DocumentID documentID,
									LocationIterator*& locations,
									ModSize& uiTF_,
									ModInvertedQueryNode* givenEndNode)
{
	return OrNode::reevaluate(documentID, locations, uiTF_);
}

//
// FUNCTION public
// ModInvertedAtomicOrNode::checkQueryNode -- 子ノードの数をチェックする
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
// ModInvertedQuery* query_
//		クエリ
// const ModBoolean setStringInChildren_
//		子ノードに検索文字列を設定する指示
// const ModBoolean needDF_
//		文書頻度を必要とするかの指示
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void
ModInvertedAtomicOrNode::checkQueryNode(
	ModInvertedQuery* query_,
	const ModBoolean setStringInChildren_,
	const ModBoolean needDF_
	)
{
	if (needDF_ == ModTrue) {
		setNeedDF(needDF_);
	}

	ModUnicodeString termString;
#ifdef V1_6
	ModLanguageSet tmpLangSet;
#endif // V1_6 
	ModInvertedTermMatchMode tmpMmode;
	if (setStringInChildren_ == ModFalse) {
		// 何もしない
	} else if (getOriginalString(termString,
#ifdef V1_6
								 tmpLangSet,
#endif // V1_6
								 tmpMmode) == ModTrue) {
		termString.clear();		// すでに termString にセットされているので、
								// クリアしないといけない
		getQueryString(termString, ModTrue, ModTrue, ModTrue, ModFalse);
		query_->insertTermNode(termString,
							   static_cast<ModInvertedOperatorOrNode*>(const_cast<ModInvertedAtomicOrNode*>(this)));

		ModSize avelen(query_->getAverageDocumentLength());
		if (avelen != 0) {
			ScoreCalculator* calculator = getScoreCalculator();
			; ModAssert(calculator != 0);
			calculator->setAverageDocumentLength(avelen);
		}

	} else {
#ifdef DEBUG
		ModErrorMessage << "termString is not set !" << ModEndl;
#endif // DEBUG
	}

	// 子ノードからオリジナル文字列を集めなくて良い
	ModInvertedQueryInternalNode::checkQueryNode(query_, ModFalse, ModFalse);
}

// 
// FUNCTION public
// ModInvertedOperatorOrNode::lowerBound -- 下限文書の検索(ローカル)
// 
// NOTES
// 文書IDが与えられた値以上で、検索式を満たす文書の内、文書ID最小のものを
// 検索し、そのような文書が存在する場合は、与えられた文書IDオブジェクトに
// 結果を格納する。
//
// ただし、ノードが持つ子ノード群ではなく、引数で渡された子ノード群を使用する。
// 条件を満たさない子ノードは子ノード群から削除する。
// 
// ARGUMENTS
// ModInvertedDocumentID givenDocumentID
//		文書ID
// ModInvertedDocumentID& foundDocumentID
//		結果格納用の文書IDオブジェクト
// Query::EvaluateMode mode
//		評価モード
// ModVector<ModInvertedQueryNode*>& tmpchildren
//		子ノード群
// 
// RETURN
// そのような文書が存在する場合 ModTrue、存在しない場合 ModFalse
// 
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModBoolean
ModInvertedAtomicOrNode::lowerBoundLocal(
	ModInvertedDocumentID givenID,
	ModInvertedDocumentID& foundID,
	Query::EvaluateMode mode,
	ModVector<ModInvertedQueryNode*>& tmpchildren)
{
#if 0
	// Local の場合、lower/upper は使用しない
	if (givenID >= this->lower) {
		if (this->upper == ModSizeMax) {
			return ModFalse;
		}
		if (givenID <= this->upper) {
			foundID = this->upper;
			return ModTrue;
		} 
	}
#endif

	// 子ノードの lowerBound 呼び出し(常に document-at-a-time)
	ModVector<ModInvertedQueryNode*>::Iterator child(tmpchildren.begin());
	ModVector<ModInvertedQueryNode*>::Iterator end(tmpchildren.end());
	ModInvertedDocumentID currentID(givenID);
	ModInvertedDocumentID smallestID(ModInvertedUpperBoundDocumentID);

	for (; child != end;) {
		if ((*child)->lowerBound(givenID, currentID, mode) == ModTrue) {
			if (givenID == currentID) {
				smallestID = currentID;
				break;
			} else if (smallestID > currentID) {
				smallestID = currentID;
			}
			++child;
		} else {
			child = tmpchildren.erase(child);
			end = tmpchildren.end();
		}
	}

	if (smallestID == ModInvertedUpperBoundDocumentID) {
		// 条件を満足する文書はなかった
		this->lower = givenID;
		this->upper = ModSizeMax;
		return ModFalse;
	}

	if ((shortWordLength != 0) || (mode & Query::roughEvaluationFlag) == 0) {
		// ラフモードでないか、ショートワードの時は lower/upper を設定する
		this->upper = smallestID;
		this->lower = givenID;
	}
	foundID = smallestID;
	return ModTrue;
}

//
// FUNCTION public
// ModInvertedAtomicOrNode::getTermFrequencyLocal -- 文書内頻度の取得(ローカル)
//
// NOTES
// 条件を満たす語句の文書内出現頻度を求める。
// ただし、ノードが持つ子ノード群ではなく、引数で渡された子ノード群を使用する。
//
// ARGUMENTS
// ModInvertedDocumentID documentID
//      文書ID
// Query::EvaluateMode mode
//      評価モード
// ModVector<ModInvertedQueryNode*>& tmpchildren
//		子ノード群
//
// RETURN
// 求めた文書内頻度
//
// EXCEPTIONS
// なし
//
ModSize
ModInvertedAtomicOrNode::getTermFrequencyLocal(
	ModInvertedDocumentID documentID,
	Query::EvaluateMode mode,
	ModVector<ModInvertedQueryNode*>& tmpchildren)
{
	OrLocationIterator* tmpLocation
		= static_cast<OrLocationIterator*>(getFreeList());
	if (tmpLocation == 0)
	{
		tmpLocation = new OrLocationIterator(this);
		tmpLocation->reserve(children.getSize());
	}
	LocationIterator::AutoPointer p = tmpLocation;

	//
	// 位置情報リスト(or 最大TF)を取得
	//

 	LocationIterator* childLocation = 0;
	ModSize uiTF = 0;
	ModSize uiMaxTF = 0;
	for (ModVector<QueryNode*>::Iterator child(tmpchildren.begin()),
			 end(tmpchildren.end());
		 child != end; ++child) {
///	高速化のために個別に呼び分ける
///			if ((*child)->evaluate(documentID, tmpLocation, 
///								   defaultEMode ,0) == ModTrue) {
		if ((*child)->evaluate(documentID,
							   mode | Query::roughEvaluationFlag)
			== ModTrue &&
			(*child)->reevaluate(documentID, childLocation, uiTF, 0) == ModTrue) {
			// childノードの示す転置リストにdocumentIDが含まれる

			// もし含まれないとしても、各ノードの条件はORの関係にあるので、
			// 一つでも条件に合うものがあればよい。
			
			if (childLocation != 0)
			{
				// 位置情報リストを取得できた
				
				if (this->shortWordLength > 0 &&
					this->shortWordLength != ModSizeMax) {
					// short word の長さを LocationListIterator にセットする
					// short word の場合、ユーザが与えた長さを使う
					static_cast<ModInvertedCompressedLocationListIterator*>(
						childLocation)->setLength(this->shortWordLength);
				}
				// 位置情報が得られたら tmpLocation へ push
				tmpLocation->pushIterator(childLocation);
				childLocation = 0;
			}
			else
			{
				// 位置情報リストを取得できなかった

				// 各索引語の文書内での位置がわからないので、
				// 正確なTFは計算できない。
				// (包含関係にある検索語を重複計上してしまう)
				// そこで、各ノードから得られるTFの中で最大のTFを返す。
				// ちなみに、各ノードの条件はORの関係にあるので、
				// 最大TFは真のTF(各TFの合計から重複計上を除いた値)を超えない。
				; ModAssert(uiTF > 0);
				if (uiTF > uiMaxTF)
				{
					uiMaxTF = uiTF;
 				}
				uiTF = 0;
			}
		}
	}

	//
	// TFを取得
	//
	
	if (tmpLocation->getSize() > 0)
	{
		// 一つ以上の子ノードから位置情報リストを取得できた
		
		// 有効化
		tmpLocation->initialize();

		if (tmpLocation->isEnd() == ModTrue)
		{
			uiTF = 0;		// ひとつも条件に合うものがなかった
		}
		else
		{
			// 有効化できた
			
			uiTF = tmpLocation->getFrequency();
			if (uiTF == 0) {
				// getFrequency()が0を返す場合はループを回してカウントしなければ
				// ならない

				// TFをカウントするコストが無視できない場合は、
				// maxCountでカウントを打ち切り、推定値を用いる。
				ModSize maxCount = ModInvertedQuery::getTFCountUpperLimit();
				for (; tmpLocation->isEnd() == ModFalse && uiTF < maxCount;
					 tmpLocation->next())
					++uiTF;
				if (tmpLocation->isEnd() == ModFalse) {
					// 上限に達したので、文書長で計算する
					ModSize length, loc;
					if (scoreCalculator->searchDocumentLength(documentID, length)
						== ModTrue &&
						(loc = tmpLocation->getLocation()) != 0)
					{
						float tmpTf = (float)uiTF / loc * length;
						uiTF = (ModSize)tmpTf;
					}
				}
			}
		}
	}
	else
	{
		// 一つも位置情報リストを取得できなかった。

		// 位置を確認することはできないが、条件は満たされたとみなして、
		// 最大TFを返す。
		// もし、全てのノードのreevaluateに失敗したり、子ノードが0だったりした場合は、
		// uiMaxTFは0のままなので、0件を返す。
		uiTF = uiMaxTF;
	}
	return uiTF;
}

#ifdef DEL_BOOL
void
ModInvertedAtomicOrNode::retrieve(ModInvertedBooleanResult *expungedDocumentId,
								const ModInvertedDocumentID maxDocumentId,
								ModInvertedBooleanResult *result)
{
	ModInvertedDocumentID ID(1);
	ModVector<ModInvertedQueryNode*> tmpchildren(children);
	Query::EvaluateMode mode(defaultEMode);

	ModInvertedVector<ModInvertedDocumentID>::Iterator iter = expungedDocumentId->begin();

	while (lowerBoundLocal(ID, ID, mode|ModInvertedQuery::roughEvaluationFlag,
						   tmpchildren) == ModTrue) {
loop:	// 文書ID削除検査
		if(ID >= *iter)
		{
			if( ID == *iter){
				++ID;
				++iter;
				continue;
			}
			if( *iter <= maxDocumentId)
			{
				++iter;
			}
			else
				break;
			goto loop;
		}

		if (evaluate(ID, mode) == ModTrue) 
		{
			result->pushBack(ID);
		}
		++ID;
	}
}
#endif

void 
ModInvertedAtomicOrNode::retrieve(ModInvertedBooleanResult *expungedDocumentId,
								const ModInvertedDocumentID maxDocumentId,
								ModInvertedSearchResult *result)
{
	ModInvertedDocumentID ID(1);
	ModVector<ModInvertedQueryNode*> tmpchildren(children);
	Query::EvaluateMode mode(defaultEMode);

	ModInvertedRankingScoreCalculator* scoreCalculator = getScoreCalculator();

	ModInvertedVector<ModInvertedDocumentID>::Iterator iter = expungedDocumentId->begin();


	while (lowerBoundLocal(ID, ID, mode|ModInvertedQuery::roughEvaluationFlag,
						   tmpchildren) == ModTrue) {
loop:	// 文書ID削除検査
		if(ID >= *iter)
		{
			if( ID == *iter){
				++ID;
				++iter;
				continue;
			}
			if( *iter <= maxDocumentId)
			{
				++iter;
			}
			else
				break;
			goto loop;
		}


		ModUInt32	tf = getTermFrequencyLocal(ID, mode, tmpchildren);
		if( tf != 0)
		{
			ModBoolean exist;
				// tf / ( X + tf ) 部分を計算する
			ModInvertedDocumentScore score = scoreCalculator->firstStep(tf, ID, exist);

			if(exist == ModTrue)
				result->pushBack(ID, score,tf);
		}
		++ID;
	}
}


// 
// FUNCTION public
// ModInvertedAtomicOrNode::doFirstStepInRetrieveScore -- ランキング検索
// 
// NOTES
// ランキング検索の第１ステップの実行。
//
// この関数は基本的には AtomicNode::doFirstStepInRetrieveScore をコピー
// したもの。ただし、子ノード群のコピーをもち、
// lowerBoundLocal のなかで該当文書がないと判断されたノードは配列から
// 除去することで lowerBoundLocal/getTermFrequencyLocal ないのループの
// 回数を減らし、高速化している。
//
// ARGUMENTS
// const ModInvertedDocumentID maxDocumentId
//		最大文書ID
// 
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
// 
void
ModInvertedAtomicOrNode::doFirstStepInRetrieveScore(
	ModInvertedBooleanResult *expungedDocumentId,
	const ModInvertedDocumentID maxDocumentId)
{
	if (getFirstStepStatus() == OrNode::firstDone) {
		// すでに呼ばれているときは何もしない
		return;
	}

	// tf を先に計算する
	ModInvertedSearchResult* result = getRankingResult();

#ifdef MOD_INV_SIMPLE_DF_EVALUATION

	// terminatorとして、(maxDocumentId + 1)をセットする
	expungedDocumentId->pushBack(maxDocumentId + 1);
	//
#ifdef DEL_BOOL
	if(result->getType() == (1 << _SYDNEY::Inverted::FieldType::Rowid))
		retrieve(expungedDocumentId,maxDocumentId,(ModInvertedBooleanResult*)result);
	else
#endif
		retrieve(expungedDocumentId,maxDocumentId,result);

	// terminatorを削除する
	expungedDocumentId->popBack();

#else

	// MOD_INV_SIMPLE_DF_EVALUATIONが無効の場合、検索コードは動作しない
	// 上のコードのようにexpungedDocumentIdを参照しながら検索を行うようなコードにする必要がある

	ModInvertedDocumentID ID(1);
	ModVector<ModInvertedQueryNode*> tmpchildren(children);

	Query::EvaluateMode mode(defaultEMode);

	if (((mode & Query::getTFbyMinEvaluationMode) != 0) &&
		(((mode & Query::getDFbyMinEvaluationMode) == 0) &&
		((mode & Query::getDFbyRoughEvaluationMode) == 0))) {
		// getTFbyMinEvaluationModeがセットされていて、
		// getDFbyMinEvaluationMode/getDFbyRoughEvaluationModeが
		// セットされていない場合は正確な評価が必要
		while (this->lowerBound(ID, ID, mode|Query::roughEvaluationFlag)
			   == ModTrue) {
			if (ID > maxDocumentID) {
				break;
			}

			if (evaluate(ID, mode) == ModTrue) {
				if ((tf = this->getTermFrequency(ID, mode)) != 0) {
					// tf / ( X + tf ) 部分を計算する
					ModBoolean exist;
					DocumentScore firstStep
						= scoreCalculator->firstStep(tf, ID, exist);
					if (exist == ModTrue)
						result->pushBack(ID, firstStep,tf);
				}
			}
			++ID;
		}
	} else {
		while (lowerBoundLocal(ID, ID,
							   mode|ModInvertedQuery::roughEvaluationFlag,
							   tmpchildren) == ModTrue) {
			if (ID > maxDocumentID) {
				break;
			}

			if ((tf = getTermFrequencyLocal(ID, mode, tmpchildren)) != 0) {
				// tf / ( X + tf ) 部分を計算する
				ModBoolean exist;
				ModInvertedDocumentScore firstStep
					= scoreCalculator->firstStep(tf, ID, exist);
				if (exist == ModTrue)
					result->pushBack(ID, firstStep,tf);
			}
			++ID;
		}
	}
#endif
	
	setFirstStepStatus(OrNode::firstDone);
}
//
// Copyright (c) 1999, 2000, 2001, 2002, 2003, 2005, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
