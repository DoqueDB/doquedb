// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedQuery.cpp -- 検索式内部表現インタフェイスオブジェクト実装
// 
// Copyright (c) 1997, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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
namespace
{
const char	srcFile[] = __FILE__;
const char	moduleName[] = "Inverted";
}
#include "SyDefault.h"
#include "SyReinterpretCast.h"
#endif

#include "ModAlgorithm.h"
#include "ModInvertedException.h"
#ifdef	SYD_INVERTED // SYDNEY 対応
#include "Inverted/ModInvertedFile.h"
#include "Inverted/ModInvertedList.h"
#else
#include "ModInvertedFile.h"
#include "ModInvertedList.h"
#endif
#include "ModInvertedFileCapsule.h"
#include "ModInvertedQuery.h"

#include "ModInvertedOperatorAndNode.h"
#include "ModInvertedOperatorOrNode.h"
#include "ModInvertedOperatorAndNotNode.h"
#include "ModInvertedSimpleWindowNode.h"
#include "ModInvertedOperatorWindowNode.h"
#include "ModInvertedRegexLeafNode.h"
#include "ModInvertedOperatorLocationNode.h"
#include "ModInvertedOperatorEndNode.h"
#include "ModInvertedTermLeafNode.h"
#include "ModInvertedSimpleTokenLeafNode.h"
#include "ModInvertedBooleanResultLeafNode.h"

#include "ModInvertedRankingScoreCombiner.h"
#include "ModInvertedRankingScoreCalculator.h"
#include "ModInvertedRankingScoreNegator.h"
#include "ModInvertedRankingResultLeafNode.h"
#include "ModInvertedExternalScoreCalculator.h"

#ifdef SYD_INVERTED
#include "Exception/Object.h"
#include "Inverted/Parameter.h"
#endif

//
// CONST
// ModInvertedOperatorOrNode::orStandardThresholdParameterKey
//		-- OR標準化のしきい値取得のパラメータキー
//
// NOTES
// MOD パラメータを用いてOR標準化のしきい値を取得するためのパラメータキー。
//
const char ModInvertedQuery::orStandardThresholdParameterKey[]
	= "InvertedQueryOrStandardThreshold";

//
// VARIABLE
// ModInvertedQuery::orStandardThreshold -- OR標準型変換のしきい値
//
// NOTES
// OR標準型変換時にこの定数値よりORの子ノードの数が大きい場合OR標準型
// への変換をしない。OR標準型変換を行なうとあまりにもノードの数が多く
// なり逆に効率が悪くなる。ANDノード内にある1つ以上のORノードの子ノー
// ド数の積とこの値とを比較する。
// 例）
//		#and(#or(A,B),#or(C,D,E),F,#or(G,H,I,J)) の場合
//				2×3×4＝24
//
ModSize ModInvertedQuery::orStandardThreshold
	= ModInvertedFile::getUInt32FromModParameter(
		ModInvertedQuery::orStandardThresholdParameterKey, 100);

//
// CONST
// ModInvertedQuery::orFlattenThresholdParameterKey
//		-- OR平坦化のしきい値取得のパラメータキー
//
// NOTES
// MOD パラメータを用いてOR平坦化のしきい値を取得するためのパラメータキー。
//
const char ModInvertedQuery::orFlattenThresholdParameterKey[]
	= "InvertedQueryOrFlattenThreshold";

//
// VARIABLE
// ModInvertedQuery::orFlattenThreshold -- OR平坦化のしきい値
//
// NOTES
// OrNode平坦化を行う場合この変数の値より多い子ノードは平坦化しない
//
ModSize ModInvertedQuery::orFlattenThreshold
	= ModInvertedFile::getUInt32FromModParameter(
		ModInvertedQuery::orFlattenThresholdParameterKey, 100);

namespace {
	//
	//	VARIABLE
	//	_$$::_TfCountUpperLimit -- TFを数える上限値
	//
	_SYDNEY::Inverted::ParameterInteger
	_TfCountUpperLimit("Inverted_TfCountUpperLimit", 100);
}

//
// FUNCTION public
// ModInvertedQuery::ModInvertedQuery -- 検索式オブジェクトの生成
//
// NOTES
// 検索式のインタフェイスオブジェクトを作る。
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
ModInvertedQuery::ModInvertedQuery()
	:
	defaultScoreCalculator(0), defaultAndScoreCombiner(0),
	defaultAndNotScoreCombiner(0), defaultOrScoreCombiner(0),
	defaultScoreNegator(0),
	root(0), totalDocumentFrequency(0), bShortWord(false)
{
	ModUInt32 resultType;
	resultType = 1 << _SYDNEY::Inverted::FieldType::Rowid;
	booleanResult = (ModInvertedBooleanResult *)ModInvertedSearchResult::factory(resultType);
}

ModInvertedQuery::ModInvertedQuery(const ModInvertedQuery& original_)
	:
	defaultScoreCalculator(0), defaultAndScoreCombiner(0),
	defaultAndNotScoreCombiner(0), defaultOrScoreCombiner(0),
	defaultScoreNegator(0),
	root(0),
	totalDocumentFrequency(original_.totalDocumentFrequency),
	bShortWord(original_.bShortWord)
{

	// DUPLICAT コピーコンストラクタで検索木のコピーも行うようにした
	ModInvertedQueryNode* tmpNodePointer = 0;
	duplicateQueryNode(original_.root, tmpNodePointer);
	setRoot(tmpNodePointer);

	if (original_.defaultAndScoreCombiner != 0)
		defaultAndScoreCombiner
			= original_.defaultAndScoreCombiner->duplicate();
	if (original_.defaultAndNotScoreCombiner != 0)
		defaultAndNotScoreCombiner
			= original_.defaultAndNotScoreCombiner->duplicate();
	if (original_.defaultOrScoreCombiner != 0)
		defaultOrScoreCombiner
			= original_.defaultOrScoreCombiner->duplicate();
	if (original_.defaultScoreCalculator != 0)
		defaultScoreCalculator
			= original_.defaultScoreCalculator->duplicate();
	if (original_.defaultScoreNegator != 0)
		defaultScoreNegator
			= original_.defaultScoreNegator->duplicate();

	ModUInt32 resultType;
	resultType = 1 << _SYDNEY::Inverted::FieldType::Rowid;
	booleanResult = (ModInvertedBooleanResult *)ModInvertedSearchResult::factory(resultType);

	ModInvertedBooleanResult *tmpBoolean;
	(const_cast<ModInvertedQuery&>(original_)).getBooleanResult(tmpBoolean);
	booleanResult->copy(tmpBoolean);
}

//
// FUNCTION public
// ModInvertedQuery::~ModInvertedQuery -- 検索式オブジェクトの廃棄
//
// NOTES
// 検索式のインタフェイスオブジェクトを廃棄する。
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
ModInvertedQuery::~ModInvertedQuery()
{
	// defaultScoreCalculator/Combiner/Negatorの破棄
	delete this->defaultScoreCalculator;
	delete this->defaultAndScoreCombiner;
	delete this->defaultAndNotScoreCombiner;
	delete this->defaultOrScoreCombiner;
	delete this->defaultScoreNegator;

	ModInvertedQuery::QueryNodeMap::Iterator p = this->queryNodeMap.begin();
	if (p == this->queryNodeMap.end() && this->root != 0) {
		// queryNodeMap を使わずにノードを下にたどりながらノードを破棄する

		// orShanderdSharedNodeに追加登録する
		addOrStanderdSharedNode(this->root);

	} else {
#if defined(DEBUG) || defined(SYD_COVERAGE)
		ModDebugMessage << "delete queryNodeMap" << ModEndl;
#endif
		// queryNodeMap を使ってノードを破棄する
		for (; p != this->queryNodeMap.end(); ++p) {
			if ((*p).second != ModInvertedQueryNode::emptySetNode) {
				// queryNodeMap に登録された内容が空集合以外の場合破棄する
				delete (*p).second;
				if (orStanderdSharedNode.find((*p).second)
					!= orStanderdSharedNode.end()) {
					orStanderdSharedNode.erase((*p).second);
				}
			}
		}
	}

#if defined(DEBUG) || defined(SYD_COVERAGE)
	ModDebugMessage << "delete orStanderdSharedNode" << ModEndl;
#endif
	// orStanderdSharedNode に残ったノードを破棄する
	for (QueryNodePointerMap::Iterator i = orStanderdSharedNode.begin();
			 i != orStanderdSharedNode.end();
		 ++i) {
		delete (*i).first;
	}

#if defined(DEBUG) || defined(SYD_COVERAGE)
	ModDebugMessage << "delete internal node" << ModEndl;
#endif
	for (p = this->simpleTokenLeafNodeMap.begin();
		 p != this->simpleTokenLeafNodeMap.end();
		 ++p) {
		delete (*p).second;
	}

	delete booleanResult;
}


//
// FUNCTION public
// ModInvertedQuery::retrieve -- Boolean検索の実行
//
// NOTES
// Boolean検索を実行し、検索条件を満たす文書の文書IDを結果を格納する。
//
// ARGUMENTS
// ModInvertedResult& result
//		結果格納用オブジェクト
// evaluationMode mode = evaluationDefault
//		検索処理モード
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
#ifndef DEL_BOOL
void
ModInvertedQuery::retrieveBoolean(ModInvertedBooleanResult *expungedDocumentId_,
							 const EvaluateMode mode)
{
	if (root == 0) {
/* purecov:begin deadcode */
		ModErrorMessage << "Invalid root pointer" << ModEndl;
		ModThrowInvertedFileError(ModInvertedErrorInternal);
/* purecov:end */
	}

	root->retrieve(*booleanResult, mode);

	if(booleanResult->getSize()> 0 &&
		expungedDocumentId_ && expungedDocumentId_->getSize() > 0)
	{
	// booleanResultからexpungedDocumentId_を引く
		ModInvertedBooleanResult *tmp = (ModInvertedBooleanResult *)booleanResult->create();
		booleanResult->setDifference(expungedDocumentId_,tmp);
		delete booleanResult;
		booleanResult = tmp;	
	}
}
#endif
void
ModInvertedQuery::retrieve(
		ModInvertedBooleanResult *expungedDocumentId_,
		const ModInvertedDocumentID maxDocumentId_)
{
	root->doFirstStepInRetrieveScore(expungedDocumentId_,maxDocumentId_);
}

ModBoolean
ModInvertedQuery::evaluate(const DocumentID id,
							 LocationIterator*& iterator,
							 const EvaluateMode mode)
{
	if (root == 0) {
/* purecov:begin deadcode */
		ModErrorMessage << "Invalid root pointer" << ModEndl;
		ModThrowInvertedFileError(ModInvertedErrorInternal);
/* purecov:end */
	}

	return root->evaluate(id, iterator, mode);
}


//
// FUNCTION public
// ModInvertedQuery::getDescription -- 文字列記述の取得
//
// NOTES
// Nodeをたどってその文字列記述を取得する
//
// ARGUMENTS
// ModUnicodeString& out
//		取得する文字列
// const ModBoolean asTermString,
//		検索語として出力するかの指示
// const ModBoolean withCalOrCombName,
//		スコア計算器・合成器名を出力するかの指示
// const ModBoolean withCalOrCombParam,
//		スコア計算器・合成器パラメータを出力するかの指示
// const ModBoolean withRough
//		ラフノードを出力するかの指示
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedQuery::getDescription(ModUnicodeString& out,
								 const ModBoolean asTermString,
								 const ModBoolean withCalOrCombName,
								 const ModBoolean withCalOrCombParam,
								 const ModBoolean withRough)
{
	this->root->getQueryString(out,
								 asTermString,
								 withCalOrCombName,
								 withCalOrCombParam,
								 withRough);
}

//
//
// FUNCTION public
// ModInvertedQuery::addOrStanderdSharedNode
//		-- OR標準形変換で共有するノードの登録
//
// NOTES
// OR標準形変換で共有することになったノードをorStanderdSharedNode変数
// へ登録する関数。引数node で指定されたnode をたどってnode 以下全ての
// 中間ノードを登録する。
//
// また rough pointer に中間ノードが登録されているならば、そのノードも
// 登録する。
//
// ARGUMENTS
// ModInvertedQueryNode* node
//		登録するノードへのポインター
//
// RETURN
// 下位からの例外をそのまま返す
//
void
ModInvertedQuery::addOrStanderdSharedNode(ModInvertedQueryNode* node)
{
	QueryNode::NodeType t = ModInvertedAtomicMask(node->getType());

	if (ModInvertedIsInternalNode(t) != ModTrue) {
		// orStanderdSharedNodeに登録が必要なノードを登録する
		// ここでは
		// termLeafNode/rankingResultLeafNode/booleanResultLeafNode/
		// regexLeafNodeの登録を行っている。
		//
		// 登録の際のvalueについては特に意味はない
		// デバックで使用するため現在は
		//		(中間ノード = 1)
		//		termLeafNode = 2
		//		rankingResultLeafNode = 3
		//		booleanResultLeafNode = 4
		//		regexLeafNode = 5
		//
		// として登録している
		//
		if (t == ModInvertedQueryNode::termLeafNode) {
			// orStanderdSharedNode に TermLeafNode を登録
			orStanderdSharedNode[node] = 2;

		} else if (t == ModInvertedQueryNode::rankingResultLeafNode) {
			// orStanderdSharedNode に RankingResultLeafNdoe を登録
			orStanderdSharedNode[node] = 3;

		} else if (t == ModInvertedQueryNode::booleanResultLeafNode) {
			if (node != ModInvertedQueryNode::emptySetNode) {
				// orStanderdSharedNode に BooleanResultLeafNdoe を登録
				orStanderdSharedNode[node] = 4;
			}

		} else if (t == ModInvertedQueryNode::regexLeafNode) {
			// これまでRegexのvaildateで行っていたが、ここで行う事に変更
			orStanderdSharedNode[node] = 5;

		}
		// internalノードでなく上記以外のノードの場合はなにもせずreturn
		// leafNodeの処理はここまで
		return;
	}

	// ここに来るのは中間ノードのケース
	// orStanderdSharedNode に node を登録
	// 登録の際のvalueについては特に意味はない

	ModInvertedQueryNode* roughNode = node->getQueryNodeForRoughEvaluation();

	orStanderdSharedNode[node] = 1;
	if (roughNode != 0) {
		t = roughNode->getType();
		if (ModInvertedAtomicMask(t)
				!= ModInvertedQueryNode::simpleTokenLeafNode) {
			// rough pointer の内容が simpleTokeLeafNode 以外の場
			// 合は削除の対象になる
			orStanderdSharedNode[roughNode] = 1;
		}
	}
	ModVector<ModInvertedQueryNode*>* children = node->getChildren();

	// 中間ノードの子ノードリストを取得できない
	// 異常なケース
	; ModAssert(children != 0);

	ModVector<ModInvertedQueryNode*>::Iterator child;
	for (child = children->begin(); child != children->end(); ++child) {
		// 子ノードに0が含まれる
		// 異常なケース
		; ModAssert(*child != 0);

		addOrStanderdSharedNode(*child); // 再帰呼び出し
	}
}

//
// FUNCTION public
// ModInvertedQuery::setDefaultAndScoreCombiner -- AND用デフォルトスコア合成器の設定
//
// NOTES
// AND用デフォルトスコア合成器を設定する。
//
// ARGUMENTS
// const ModCharString& combiner
//		スコア合成器名
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
// EXCEPTIONS
// なし
//
void
ModInvertedQuery::setDefaultAndScoreCombiner(const ModCharString& combiner)
{
	defaultAndScoreCombiner
		= ModInvertedRankingScoreCombiner::create(combiner);
}

//
// FUNCTION public
// ModInvertedQuery::setDefaultAndNotScoreCombiner -- ANDNOT用デフォルトスコア合成器の設定
//
// NOTES
// ANDNOT用デフォルトスコア合成器を設定する。
//
// ARGUMENTS
// const ModCharString& combiner
//		スコア合成器名
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void
ModInvertedQuery::setDefaultAndNotScoreCombiner(
	const ModCharString& combiner)
{
	defaultAndNotScoreCombiner
		= ModInvertedRankingScoreCombiner::create(combiner);
}

//
// FUNCTION public
// ModInvertedQuery::setDefaultOrScoreCombiner -- OR用デフォルトスコア合成器の設定
//
// NOTES
// OR用デフォルトスコア合成器を設定する。
//
// ARGUMENTS
// const ModCharString& combiner
//		スコア合成器名
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void
ModInvertedQuery::setDefaultOrScoreCombiner(const ModCharString& combiner)
{
	defaultOrScoreCombiner
		= ModInvertedRankingScoreCombiner::create(combiner);
}

//
// FUNCTION public
// ModInvertedQuery::setDefaultScoreCalculator -- デフォルトスコア計算器の設定
//
// NOTES
// デフォルトスコア計算器を設定する。
//
// ARGUMENTS
// const ModCharString& calculator
//		スコア計算器名
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void
ModInvertedQuery::setDefaultScoreCalculator(const ModCharString& calculator)
{
	defaultScoreCalculator
		= ModInvertedRankingScoreCalculator::create(calculator);
}

//
// FUNCTION public
// ModInvertedQuery::setDefaultScoreNegator -- デフォルトスコア否定器の設定
//
// NOTES
// デフォルトスコア否定器を設定する。
//
// ARGUMENTS
// const ModCharString& nagator
//		スコア否定器名
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void
ModInvertedQuery::setDefaultScoreNegator(const ModCharString& nagator)
{
	this->defaultScoreNegator
		= ModInvertedRankingScoreNegator::create(nagator);
}

//
//
// FUNCTION protected
// ModInvertedQuery::validate -- 検索可能なようにノード有効化し最適化する
//
// NOTES
// 検索可能なようにノード有効化し、その後最適化する。
//
// ARGUMENTS
// validateMode mode
//		有効化モード
// ModInvertedFile* invertedFile
//		転置ファイルポインタ
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedQuery::validate(InvertedFile* invertedFile,
							 const Query::ValidateMode mode,
							 const ModSize averageDocumentLength_
							 )
{
	try {

		//
		// 前処理
		//

		if ((mode & rankingMode) != 0) {
			// ランキング検索: デフォルトの計算器、合成器をセット
			if (getDefaultAndScoreCombiner() == 0) {
				defaultAndScoreCombiner
					= ModInvertedRankingScoreCombiner::create();
			}
			if (getDefaultAndNotScoreCombiner() == 0) {
				defaultAndNotScoreCombiner
					= ModInvertedRankingScoreCombiner::create();
			}
			if (getDefaultOrScoreCombiner() == 0) {
				defaultOrScoreCombiner
					= ModInvertedRankingScoreCombiner::create();
			}
			if (getDefaultScoreCalculator() == 0) {
				defaultScoreCalculator
					= ModInvertedRankingScoreCalculator::create();
			}
		} else {
			// ブーリアン検索: 平均文書長をセットしてはいけない
			if (averageDocumentLength_ != 0) {
				ModErrorMessage << "average document length must be 0 "
								<< "for boolean query" << ModEndl;
				ModThrowInvertedFileError(ModInvertedErrorQueryValidateFail);
			}
		}

		// 転置ファイルに登録されている全文書数を取得
		totalDocumentFrequency = invertedFile->getDocumentFrequency();
		averageDocumentLength = averageDocumentLength_;

		//
		// 検索木を作る(コピー作成)
		//
		
		if (root->getType() == QueryNode::simpleTokenLeafNode) {
			// トークンノードだけ特別
			validateSimpleTokenLeafNode(root, invertedFile, mode);

			//
			// ここから未使用
			//
#if 0
			ModInvertedSimpleTokenLeafNode* simpleNode =
				static_cast<ModInvertedSimpleTokenLeafNode*>(root);

			ModUnicodeString tokenString;
			ModSize tokenLength;
			simpleNode->contentString(tokenString);
			simpleNode->getTermLength(tokenLength);

#ifdef SYD_INVERTED
			ModInvertedList* invertedList = invertedFile->getInvertedList();
#else
			ModInvertedList* invertedList = new ModInvertedList();
#endif
			if (invertedFile->getInvertedList(tokenString,
												*invertedList,
												ModInvertedListSearchMode)
				== ModFalse) {
				delete invertedList;
				delete simpleNode;
				root = ModInvertedQueryNode::emptySetNode;
			} else {

				ModSize keyLength(
					invertedFile->getTokenizer()->getTokenLength(tokenString));
				ModInvertedQueryNode* tmpNode = 0;

				// map登録用のキー文字列取得
				ModUnicodeString mapKey;
				getDescription(mapKey);

				if (getSimpleTokenNode(tokenString, keyLength, mapKey,
										 invertedList, tmpNode, mode) == ModTrue) {
					delete invertedList;
				}

				static_cast<ModInvertedSimpleTokenLeafNode*>(tmpNode)->setIterator();
				if ((mode & rankingMode) != 0) {
					tmpNode->setScoreCalculator(defaultScoreCalculator->duplicate());
					tmpNode->getScoreCalculator()->setDocumentLengthFile(
						invertedFile->getDocumentLengthFile());
					tmpNode->getScoreCalculator()->setAverageDocumentLength(
						averageDocumentLength);
					tmpNode->getScoreCalculator()->prepare(
						totalDocumentFrequency,
						invertedList->getDocumentFrequency());
				}
				delete simpleNode;
				root = tmpNode;
			}
#endif
			//
			// ここまで未使用
			//
			
		} else {
			root->validate(invertedFile, mode, this);
		}
#if defined(DEBUG) || defined(SYD_COVERAGE)
	{
		ModUnicodeString tmp;
		this->root->getQueryString(tmp, ModFalse, ModTrue, ModFalse,ModTrue);
		ModDebugMessage << "after validate" << tmp << ModEndl;
	}
#endif	// DEBUG

		//
		// 最適化
		//
		
		// optimizeの引数を変更した
		// 以前はrootを引数で渡していたが、rootはメンバなので引数で渡す必要は
		// ない
		this->optimize(mode);

		// 完成した QueryNode をチェック
		this->root->checkQueryNode(this, ModTrue, ModFalse);

	} catch (ModException& e) {
		ModErrorMessage << e << ModEndl;
		ModThrowInvertedFileError(ModInvertedErrorQueryValidateFail);
#ifndef SYD_INVERTED
	} catch (...) {
		/* purecov:begin deadcode */
		ModUnexpectedThrow(ModModuleInvertedFile);
		/* purecov:end */
#endif
	}

	// RankingAnd,RankingOr用のscoresを確保
	this->root->reserveScores();
}

//
// FUNCTION
// ModInvertedQuery::validateSimpleTokenLeafNode -- SimpleTokenLeafNodeの有効化
//
// NOTES
// SimpleTokenLeafNodeを有効化する。
//
// ARGUMENTS
// ModInvertedQueryNode*& node_
//		SimpleTokenLeafNode
// ModInvertedFile* invertedFile_
//		転置ファイル
// const ValidateMode mode_
//		有効化モード
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedQuery::validateSimpleTokenLeafNode(
	ModInvertedQueryNode*& node_,
	ModInvertedFile* invertedFile_,
	const ValidateMode mode_
	)
{
	// トークンノードだけ特別

	ModInvertedSimpleTokenLeafNode* simpleNode =
#ifdef CC_SUN4_2
		static_cast<ModInvertedSimpleTokenLeafNode*&>(node_);
#else
		static_cast<ModInvertedSimpleTokenLeafNode*>(node_);
#endif

	// 検索語を取得
	ModUnicodeString tokenString;
	ModSize tokenLength;
	simpleNode->contentString(tokenString);
	simpleNode->getTermLength(tokenLength);

	// 転置リストオブジェクトを取得
#ifdef SYD_INVERTED
	ModInvertedList* invertedList = invertedFile_->getInvertedList();
#else
	ModInvertedList* invertedList = new ModInvertedList();
#endif
	// tokenStringに対応した転置リストを割り当てる
	if (invertedFile_->getInvertedList(tokenString,
									   *invertedList,
									   ModInvertedListSearchMode)
		== ModFalse) {
		// 割り当てられなかった(見つからなかった等)場合
		delete invertedList;
		delete simpleNode;
		
		// 空ノードを返す
		node_ = const_cast<ModInvertedQueryNode*>(ModInvertedQueryNode::emptySetNode);
	} else {
		// 割り当てられた場合

		// [NOTE] 索引語の長さは索引タイプにより異なる。
		ModSize keyLength(
			invertedFile_->getTokenizer()->getTokenLength(tokenString));
		ModInvertedQueryNode* tmpNode = 0;

		// map登録用のキー文字列取得
		// [NOTE] 検索語を含んだ検索条件の文字列
		ModUnicodeString mapKey;
		simpleNode->getQueryString(mapKey);

		// SimpleTokenLeafNodeを取得し、tmpNodeに設定
		if (getSimpleTokenNode(tokenString, keyLength, mapKey,
							   invertedList, tmpNode, mode_) == ModTrue) {
			// 登録済みだった場合
			delete invertedList;
		}

		// 転置リストのイテレータを設定
		// [NOTE] 転置リストはある索引語を含む文書IDリスト等が格納されている。
		//  未登録だった転置リストには、イテレータが設定されていないので、
		//  (先頭データを)設定しておく。
		static_cast<ModInvertedSimpleTokenLeafNode*>(tmpNode)->setIterator();
		
		if ((mode_ & rankingMode) != 0) {
			// ランキングモードの場合

			// スコア計算のパラメータ等のデフォルト値を設定
			tmpNode->setScoreCalculator(defaultScoreCalculator->duplicate());
			tmpNode->getScoreCalculator()->setDocumentLengthFile(
				invertedFile_->getDocumentLengthFile());
			tmpNode->getScoreCalculator()->setAverageDocumentLength(
				averageDocumentLength);
			// [YET] invertedList==0では来ない？
			//  SimpleTokeLeafNodeが登録済みだとそうなるが…
			tmpNode->getScoreCalculator()->prepare(
				totalDocumentFrequency,
				invertedList->getDocumentFrequency());
		}
		delete simpleNode;
		
		// SimpleTokenLeafNodeを返す
		node_ = tmpNode;
	}
}

//
// FUNCTION protected ModInvertedQuery::getSimpleTokenNode -- 索引語に
//対応した末端ノードを得る
//
// NOTES
// invertedListKeyに対応した索引語ノードを得る。simpleTokenLeafNodeMap
// に登録されているか確認する。登録されていいた場合は登録済のノードを
// 共有するかたちで使う。登録されていなかった場合は新規に
// SimpleTokenLeafNodeを生成する。
//
// ※ ショートワード用
//		単語単位の時にはショートワードは生成されないので、長さはこの関数ないで
//		計算する
//
// ARGUMENTS
// const ModUnicodeString& invertedListKey
//		キー
// const ModSize keyLength
//		キーの長さ
// const ModUnicodeString& mapKey
//		マップ作成用のキー
// ModInvertedList* invertedList
//		転置ファイル
// ModInvertedQueryNode*& node
//		生成したノード（結果格納用）
// const ValidateMode mode
//		ブーリアン／ランキング検索の判定に用いる
//
// RETURN
// simpleTokenLeafNodeMap に登録済ノードがあれば ModTrue を返す。
// それ外は ModFalseを返す。
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModBoolean
ModInvertedQuery::getSimpleTokenNode(const ModUnicodeString& invertedListKey,
									 const ModSize keyLength,
									 const ModUnicodeString& mapKey,
									 ModInvertedList* invertedList,
									 ModInvertedQueryNode*& node,
									 const ValidateMode mode
									 )
{
#if defined(DEBUG) || defined(SYD_COVERAGE)
	ModDebugMessage << "Query::setSimpleTokenNode mapKay : " << mapKey << ModEndl;
#endif // DEBUG

	QueryNodeMap::Iterator p = simpleTokenLeafNodeMap.find(mapKey);

	if (p != simpleTokenLeafNodeMap.end()) {
		// マップ中にあったのでSimpleTokenLeafNodeは生成しない
		node = (*p).second;
		return ModTrue;

	} else {
		// 出現頻度を得る
		ModSize documentFrequency = invertedList->getDocumentFrequency();

		// マップ中にないので、新たに作成
		ModInvertedSimpleTokenLeafNode* tmpNode
			= new ModInvertedSimpleTokenLeafNode(invertedListKey,
												 keyLength,
												 invertedList,
												root->getRankingResult()->getType(),
												 documentFrequency
												);

		// トークンノードの全てが検索で実際に使われるわけではないので、
		// イテレータのセットは呼び出し先で行なうことに再修正する
		// tmpNode->setIterator();

		// マップに追加登録
		simpleTokenLeafNodeMap[mapKey] = tmpNode;

		node = tmpNode;

		return ModFalse;
	}
}

//
// FUNCTION protected
// ModInvertedQuery::eraseTermLeafNodeQuery -- TermLeafNodeの削除
//
// NOTES
// 引数 nodePlace が TermLeafNode だった場合、TermLeafNode を削除して
// OrderedDistanceNode/SimpleTokenLeafNode にする。
// もし引数 nodePlace が TermLeafNode 以外だった場合 nodePlace に対し
// て eraseTermLeafNode() を呼び出し nodePlace 以下の TermLeafNode を
// 削除する。eraseTermLeafNode() は QueryInternalNode に実装されている。
//
// ARGUMENTS
// ModInvertedQueryNode*& nodePlace
//		このノード以下のTermLeafNodeを削除する
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedQuery::eraseTermLeafNodeQuery(ModInvertedQueryNode*& nodePlace)
{
	ModInvertedQueryNode::NodeType childType;

	childType = ModInvertedAtomicMask(nodePlace->getType());

	if (childType == ModInvertedQueryNode::termLeafNode) {
		// nodePlace が TermLeafNode の場合
		TermLeafNode* termNode =
#ifdef CC_SUN4_2
			static_cast<ModInvertedTermLeafNode*&>(nodePlace);
#else
			static_cast<ModInvertedTermLeafNode*>(nodePlace);
#endif

		//
		// TermLeafNodeは常に削除可能である。
		//

		// 明示的にAtomicを指定された場合(#term(...))はeraseTermLefNodeが
		// 指定されてもTermLeafNodeの削除は行わない。ただしpresiceNodeが
		// EmptySetNodeの場合は削除する
		// nodePlaceをTermLeafNodeのPrecese Pointerで置き換える
		nodePlace = termNode->queryNodeForPreciseEvaluation;
		// nodePlaceのRough PointerをTermLeafNodeのRough Pointerで置き換える
		nodePlace->setQueryNodeForRoughEvaluation(
			termNode->getQueryNodeForRoughEvaluation());
		nodePlace->setOriginalString(termNode->termString,
#ifdef V1_6
									 termNode->getLangSet(),
#endif // V1_6
									 termNode->getMatchMode());

		// pointer を0クリアしてTermLeafNodeを破棄
		termNode->queryNodeForPreciseEvaluation = 0;
		termNode->queryNodeForRoughEvaluation = 0;

		delete termNode;

	} else if (childType == ModInvertedQueryNode::booleanResultLeafNode) {
		// booleanResultLeafNode の場合

#ifdef CC_SUN4_2
		ModInvertedBooleanResultLeafNode* booleanResultNode
			= static_cast<ModInvertedBooleanResultLeafNode*&>(nodePlace);
#else
		ModInvertedBooleanResultLeafNode* booleanResultNode
			= static_cast<ModInvertedBooleanResultLeafNode*>(nodePlace);
#endif

		if (booleanResultNode->isEmptyResultLeafNode() == ModTrue) {
			// 空集合
			this->addOrStanderdSharedNode(nodePlace);
			nodePlace = const_cast<ModInvertedQueryNode*>(ModInvertedQueryNode::emptySetNode);
		}

	} else if (childType == ModInvertedQueryNode::rankingResultLeafNode) {

#ifdef CC_SUN4_2
		ModInvertedRankingResultLeafNode* rankingResultNode
			= static_cast<ModInvertedRankingResultLeafNode*&>(nodePlace);
#else
		ModInvertedRankingResultLeafNode* rankingResultNode
			= static_cast<ModInvertedRankingResultLeafNode*>(nodePlace);
#endif

		// RankingResultLeafNode の場合
		if (rankingResultNode->isEmptyResultLeafNode() == ModTrue) {
			// 空集合
			this->addOrStanderdSharedNode(nodePlace);
			nodePlace = const_cast<ModInvertedQueryNode*>(ModInvertedQueryNode::emptySetNode);
		}
	} else {
		// nodePlace が TermLeafNode 以外の場合
		QueryNode* tmpNode = 0;
		if (nodePlace->eraseTermLeafNode(tmpNode, *this) != ModTrue) {
			// 後処理が必要
			this->addOrStanderdSharedNode(nodePlace);
			if (tmpNode == 0) {
				// 空集合ノードをセット
				nodePlace = const_cast<ModInvertedQueryNode*>(ModInvertedQueryNode::emptySetNode);
			} else {
				// ノードを昇格
				nodePlace = tmpNode;
			}
		}
	}
}

//
// FUNCTION private
// ModInvertedQuery::optimize -- ノードを最適化する
//
// NOTES
// 引数で与えられた nodePlace を最適化する。引数 mode の内容により最適
// 化の内容を選択できる。 defaultMode であった場合全ての最適化を行なう。
//
// nodePlaceに渡されるのはQuery::rootなので引数で渡す必要はない
// 引数を変更しrootは渡さないようにし、これまでnodePlace(=root)に行っていた
// 処理をrootに対して行うように変更した
//
//
// 最適化の内容と順番は以下の通り
//	 1. TermLeafNodeをOrderedDistanceNodeやSimplTokenLeafNodeにする
//	 2. OperatorAndNode と OperatorOrNode の平坦化
//	 3. 論理和標準形へ変換
//	 4. OperatorAndNode と OperatorOrNode の平坦化 (2回目)
//	 5. 中間ノードの共有化
//	 6. OperatorAndNode と OperatorOrNode の子ノードのソート
//	 7. queryNodeForRoughEvaluationを作る
//	 8. RoughPointerの中間ノードを共有化
//
// ARGUMENTS
// const ValidateMode mode
//		最適化のモード。defaultModeの場合全ての最適化を行なう。
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedQuery::optimize(const ValidateMode mode)
{
	// 最適化開始
	if ((mode & ModInvertedQuery::toSimpleWindow) != 0) {
		// OperatorWindowをSimpleWindowへ変換
		toSimpleWindowNode(root);
	}

	// TermLeafNode を削除して
	// OrderedDistanceNode/SimpleTokenLeafNodeにする
	//
	// 以前はvalidaetModeで行うか行わないかを指定していたが、
	// 今は常にeraseTermLeafNodeを行う。
	//
	eraseTermLeafNodeQuery(root);

#if defined(DEBUG) || defined(SYD_COVERAGE)
	{
		ModUnicodeString tmp;
		this->root->getQueryString(tmp, ModFalse, ModTrue, ModFalse,ModTrue);
		ModDebugMessage << "before flattenChildren " << tmp << ModEndl;
	}
#endif	// DEBUG
	if ((mode & ModInvertedQuery::flattenChildren) != 0) {
		//
		// and や or の平坦化
		//
		root->flattenChildren(orStanderdSharedNode, ModFalse);
	}
#if defined(DEBUG) || defined(SYD_COVERAGE)
	{
		ModUnicodeString tmp;
		this->root->getQueryString(tmp, ModFalse, ModTrue, ModFalse,ModTrue);
		ModDebugMessage << "after flattenChildren " << tmp << ModEndl;
	}
#endif	// DEBUG


	if ((mode & ModInvertedQuery::orStanderdStyle) != 0) {
		//
		// OR標準形に変換 (分配律を使って、or を外に出す)
		//
		// calcSortFactor()がModInvertedQueryNode::MaxSortFactorになるのは
		// Regexを含むケース
		// このような場合はOr標準形への変換は行わないようが良い
		if (root->calcSortFactor() != ModInvertedQueryNode::MaxSortFactor) {
			convertToOrStanderdStyle(root, mode);
		}
#if defined(DEBUG) || defined(SYD_COVERAGE)
		else {
			ModDebugMessage << "orStanderdStyle query may include REGEX. "
							<< "shoud not change" << ModEndl;
		}
		ModDebugMessage << "orStanderdStyle" << ModEndl;
#endif
		if ((mode & ModInvertedQuery::flattenChildren) != 0) {
			// OR標準形変換後、もう一回子ノードの平坦化を行なう
			root->flattenChildren(orStanderdSharedNode, ModFalse);
		}
	}
#if defined(DEBUG) || defined(SYD_COVERAGE)
	{
		ModUnicodeString tmp;
		this->root->getQueryString(tmp, ModFalse, ModTrue, ModFalse,ModTrue);
		ModDebugMessage << "after flattenChildren " << tmp << ModEndl;
	}
#endif	// DEBUG

	try {
		if ((mode & ModInvertedQuery::sortChildren) != 0) {
			//
			// and と or のソート
			//
			root->sortChildren(mode);
#ifdef DEBUG
			{ // DEBUG
				// ModOstrStream out;
				ModUnicodeString out;
				this->root->showEstimatedValue(out);
				ModDebugMessage << "sorted children freq: "
								<< out << ModEndl;
				ModUnicodeString out2;
				this->root->showSortFactor(out2);
				ModDebugMessage << "sorted children factor: "
								<< out2 << ModEndl;
			}

#endif
		}

		if ((mode & ModInvertedQuery::sharedNode) != 0) {
#if defined(DEBUG) || defined(SYD_COVERAGE)
			ModDebugMessage << "sharedQueryNode" << ModEndl;
			{
				ModUnicodeString tmp;
				root->getQueryString(
						tmp, ModFalse, ModTrue, ModFalse, ModTrue);
				ModDebugMessage << "before sharedNode: "
							<< tmp << ModEndl;
			}
#endif	// DEBUG
			//
			// ノードの共有化
			//

			if (ModInvertedIsInternalNode(root->getType()) == ModTrue) {
				// sharedQueryNodeの戻り値
				// sharedQueryNodeを行い、その結果子ノードの数がいくつに
				// なったかを返す

				// rootがLeafNodeの場合は-1を返す
				// 中間ノードで
				//	子ノードが1つしかなくなった場合は1を返す
				//	子ノードが空集合ノードの場合は0を返す
				//
				ModSize retGrandChildNum;
				retGrandChildNum = root->
					sharedQueryNode(queryNodeMap, orStanderdSharedNode);
				if (retGrandChildNum != -1) {
					// LeafNode::sharedQueryNode()は何もしないで -1を返す
					// ここに来るのは中間ノードのケース
					if (retGrandChildNum == 1) {
						// sharedQueryNodeの結果子ノードが一つしかなくなった
						// 単純化
						root->changeSimpleTypeNode(&root,
													 orStanderdSharedNode);

					} else if (retGrandChildNum == 0) {
						// 子ノードはemptySetNodeである。
						// 自分にemptySetNodeをセットする
						addOrStanderdSharedNode(root);
						root = const_cast<ModInvertedQueryNode*>(ModInvertedQueryNode::emptySetNode);
					}

					if (root != ModInvertedQueryNode::emptySetNode &&
						ModInvertedIsInternalNode(root->getType())
						== ModTrue) {
						// 最上位のノードがInternalノードの場合
						// queryNodeMap に挿入する
						ModUnicodeString key;
						root->getQueryString(key);
						queryNodeMap[key] = root;
					}
				}
			}
#if defined(DEBUG) || defined(SYD_COVERAGE)
			{
				ModUnicodeString tmp;
				root->getQueryString(
						tmp, ModFalse, ModTrue, ModFalse, ModTrue);
				ModDebugMessage << "after sharedNode: "
								<< tmp << ModEndl;
			}
#endif	// DEBUG
		}
		if ((mode & ModInvertedQuery::makeRough) != 0) {
			//
			// rough用ポインタを作る
			//
			QueryNodePointerMap dummy;
			root->makeRoughPointer(mode, dummy, this);
#if defined(DEBUG) || defined(SYD_COVERAGE)
			{
				ModUnicodeString tmp;
				root->getQueryString(
						tmp, ModFalse, ModTrue, ModFalse, ModTrue);
				ModDebugMessage << "after makeRoughPointer: "
								<< tmp << ModEndl;
			}
#endif	// DEBUG
		}
	} catch (ModException& exception) {
		ModErrorMessage << exception << ModEndl;
		// rough pointer の内容を破棄できるように

		root->addRoughToGlobalNodeMap(
					queryNodeMap, orStanderdSharedNode);

		ModThrowInvertedFileError(ModInvertedErrorQueryValidateFail);
	} catch (...) {
		/* purecov:begin deadcode */

		root->addRoughToGlobalNodeMap(
					queryNodeMap, orStanderdSharedNode);

#ifdef SYD_INVERTED
		_SYDNEY_RETHROW;
#else
		ModUnexpectedThrow(ModModuleInvertedFile);
#endif
		/* purecov:end */
	}

	// sharedRoughNodeは廃止 sharedNodeに統合
	if ((mode & ModInvertedQuery::sharedNode) !=0) {
		// RoughPointerの中間ノードを共有化する
		//
		root->addRoughToGlobalNodeMap(
			queryNodeMap, orStanderdSharedNode);
#if defined(DEBUG) || defined(SYD_COVERAGE)
		ModDebugMessage << "addRoughToGlobalNodeMap" << ModEndl;
#endif
	}
#if defined(DEBUG) || defined(SYD_COVERAGE)
	{
		ModUnicodeString tmp;
		this->root->getQueryString(tmp, ModTrue, ModTrue, ModFalse,ModTrue);
		ModDebugMessage << "finish optimize asTerm" << tmp << ModEndl;
	}
#endif	// DEBUG
}

void
ModInvertedQuery::doFirstStepInRetrieveScore(
		ModInvertedBooleanResult *expungedDocumentId_,
		const ModInvertedDocumentID maxDocumentId_
		 )
{
	root->doFirstStepInRetrieveScore(expungedDocumentId_,maxDocumentId_);
}

//
//	FUNCTION public static
//	ModInvertedQuery::getTFCountUpperLimit -- TFを数える上限
//
ModSize
ModInvertedQuery::getTFCountUpperLimit()
{
	return static_cast<ModSize>(_TfCountUpperLimit.get());
}

//
// FUNCTION public
// ModInvertedQuery::getSearchTermList --
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
ModInvertedQuery::getSearchTermList(
	ModInvertedQuery::SearchTermList& vecSearchTerm_) const
{
	// uiSynonymIDは同義語集合のIDを示し、0は同義語でないことを示す。
	// 参照：ModInvertedAtomicOrNode::getSearchTermList()
	ModSize uiSynonymID = 0;
	root->getSearchTermList(vecSearchTerm_, uiSynonymID);
}

//
// FUNCTION private
// ModInvertedRankingQuery::duplicateQueryNode -- 検索木のコピーを作る
//
// NOTES
// 検索木のコピーを作る。以前のcreateRankingQueryNode()の機能。
//
// 検索木をrootからたどりながら検索木のコピーを作る。
// (scoreCalculator/Combinerの準備？)
//
// ARGUMENTS
// const QueryNode*
//		コピー元（参照用）
// QueryNode*& insertPlace
//		コピー先
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedQuery::duplicateQueryNode(QueryNode* node,
									 QueryNode*& insertPlace) const
{
	ModInvertedQueryNode::NodeType nodeType =
		ModInvertedAtomicMask(node->getType());

	// QueryNode::duplicateを修正した
	// 中間ノードのduplicateでは子ノードのコピーも行うように修正
	// rootにduplicateを行うだけですべてのノードのコピーが作られる
	insertPlace = node->duplicate(*this);
}

//
// FUNCTION private
// ModInvertedQuery::convertToOrStanderdStyle -- Or標準形へ変換必要があるか検査し、変換可能である場合はOr標準形への変換を行う。
//
// NOTES
// Or 標準形へ変換する必要があるかを検査し、変換可能ならOr標準形に変換する
//
// 引数 node で与えられたものが AndNode で、その AndNode の子ノードに
// OrNode が含まれている場合 Or標準形へ変換できると判断する。ただし、
// AndNodeに含まれるOrNodeの子ノードの合計数が orStandardThreshold 値
// 以下の場合のみOr標準形へ変換する。
//
// 上記のような条件を満した場合changeOrStanderdStyle() を呼び出し Or標
// 準形に変換する。また引数 node の内容がInternalNodeであり
// OrderdDistanceNodeでない場合convertToOrStanderdStyle() を呼び出すことに
// より node 以下全ノードを検査する。
//
// Or標準形変換を行なったことによりノードの共有が発生する。共有されて
// いるノードはorStanderdSharedNodeというMap型の変数へ登録し、ノードを
// 破棄する際の情報として使う。
//
// ARGUMENTS
// ModInvertedQueryNode* node
//		Or 標準形へ変換できるか検査するノード
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedQuery::convertToOrStanderdStyle(QueryNode*& node,
	const ValidateMode mode)
{
	QueryNode::NodeType type = ModInvertedAtomicMask(node->getType());
	ModVector<ModInvertedQueryNode*>::Iterator p;
	ModVector<ModInvertedQueryNode*>* children;

	children = node->getChildren();

	if (type != ModInvertedQueryNode::operatorAndNode) {
		// AndNode以外の場合
		if (children != 0) {
			// 中間ノードの場合
			// ただし OrderedDistance operatorWordの子ノードに
			// Andが来ることはない
			if (type != ModInvertedQueryNode::orderedDistanceNode
#ifdef V1_4
				&& type != ModInvertedQueryNode::operatorWordNode
#endif
				) {

				for (p = children->begin(); p != children->end(); ++p) {
					// 子ノードに対して再帰呼出し
					convertToOrStanderdStyle(*p, mode);
				}
			}
		}
		return;
	}

#ifdef CC_SUN4_2
	ModInvertedOperatorAndNode* andNode
		= static_cast<ModInvertedOperatorAndNode*&>(node);
#else
	ModInvertedOperatorAndNode* andNode
		= static_cast<ModInvertedOperatorAndNode*>(node);
#endif

	//
	// AndNode の場合
	//
	// AndNodeの子ノードでOrNode 以外のものを入れる

	if ((mode & rankingMode) != 0) {
		// ランキング検索の場合はここでAndのCombinerのAssociative/Communicativ
		// をチェックする
		ModInvertedRankingScoreCombiner* sc;
		sc = andNode->getScoreCombiner();

		if (sc != 0) {
			// Combinerがセットされているのはランキング検索の場合
			//
			//
			if (sc->isAssociative() != ModTrue
					|| sc->isCommutative() != ModTrue) {
				// 変換不可
				p = children->begin();
				for (p = children->begin(); p != children->end(); ++p) {
					// 子ノードに対して再帰呼出し
					convertToOrStanderdStyle(*p,mode);
				}
				return;
			}
		}
	}

	QueryNode::NodeType childType;
	ModVector<ModInvertedQueryNode*> tmpChild;
	for (p=children->begin(); p != children->end(); ++p) {

		// 子ノードに対してconvertToOrStanderdStyle()
		convertToOrStanderdStyle(*p, mode);

		// AndNode の子ノードを検査
		childType = (*p)->getType();

		if (childType != ModInvertedQueryNode::operatorOrNode) {
			// Or以外のノードなので変換不可
			// AtomicOrも変換できないのでここに来る
			tmpChild.pushBack(*p);
			continue;

		} else {
			// もしshortWord用のOrが常にAtomicOrならここも必要ない
			if ((*p)->isShortWordOrNode() == ModTrue) {
				// shortWord用 orNode

				tmpChild.pushBack(*p);
				continue;
			}

			if ((mode & rankingMode) != 0) {
				// ランキング検索のときは常にor標準形にしない
				tmpChild.pushBack(*p);
				continue;
			}

			// AndNode の子ノードに OrNode を発見
			ModInvertedOperatorOrNode* orNode;
			ModInvertedOperatorOrNode* newOrNode;

			// 代入して次にすすめる
			orNode = static_cast<ModInvertedOperatorOrNode*>(*p);
			++p;

			// orNode の子ノードの数をもとめる
			ModSize orNodeTotalChildren = orNode->getChildrenSize();

			ModSize size;
			while (p != children->end()) {
				tmpChild.pushBack(*p);	// 残りの子ノードを全て追加

				childType = (*p)->getType();

				if (childType == QueryNode::operatorOrNode) {
					// ORの場合
					// マスクをかけていないのでoperatorOrの場合のみ
					// ここに来る (AtomicOrは変換不可)

					// 他に変換可能なOrノードがある場合はorNodeTotalChildren
					// の計算を行う

					// shortWord用以外のorNode
					if ((*p)->isShortWordOrNode() == ModFalse) {

						if ((mode & rankingMode) != 0) {
							// ランキング検索の場合
							ModInvertedRankingScoreCombiner* sc
								= (*p)->getScoreCombiner();
							if (sc->isAssociative() == ModTrue &&
								sc->isCommutative() == ModTrue) {
								// 変換可
								size = (*p)->getChildrenSize();
								orNodeTotalChildren *= size;
							}
						} else {
							// ブーリアンの場合
							// 常に変換可
							size = (*p)->getChildrenSize();
							orNodeTotalChildren *= size;
						}
					}
				} else {
					;
				}

				if (orStandardThreshold < orNodeTotalChildren) {
					break;				// or標準形への変換中止
				}
				++p;
			}
#if defined(DEBUG) || defined(SYD_COVERAGE)
			ModDebugMessage << "orNodeTotalChildren = "
							<< orNodeTotalChildren
							<< ", orStandardThreshold = "
							<< orStandardThreshold
							<< ModEndl;
#endif

			if (orStandardThreshold < orNodeTotalChildren) {
#if defined(DEBUG) || defined(SYD_COVERAGE)
				ModDebugMessage << "Stop converting to OR normal forms" << ModEndl;
#endif
				// or標準形への変換中止
				break;
			}

			newOrNode = changeOrStanderdStyle(
				tmpChild, orNode,
#ifdef CC_SUN4_2
				static_cast<ModInvertedOperatorAndNode*&>(node),
#else
				static_cast<ModInvertedOperatorAndNode*>(node),
#endif
				mode);

			//古い AndNode と古い OrNode は消去する
			if (orStanderdSharedNode.find(orNode)
				== orStanderdSharedNode.end()) {
				delete orNode;
			}
			if (orStanderdSharedNode.find(node)
				!= orStanderdSharedNode.end()) {
				// 置き換えるノードは共有されていた
				// -> 新しいノードも共有される
				addOrStanderdSharedNode(newOrNode);
			} else {
				// 共有されていないので削除する
				delete node;
			}

			//node を OR標準形に変換したノードにする
			node = newOrNode;

			// OR標準形に変換したことによって共有されるノードを
			// orStanderdSharedNode に挿入しておく
			// Query のデストラクト時に使う
			ModVector<ModInvertedQueryNode*>::Iterator p;
			for (p = tmpChild.begin(); p != tmpChild.end(); ++p) {
				addOrStanderdSharedNode(*p);
			}
			convertToOrStanderdStyle(node,mode);

			// OR標準形に変換終了
			break;
		}
	}
}

//
// FUNCTION private
// ModInvertedQuery::changeOrStanderdStyle -- Or 標準形へ変換
//
// NOTES
// Or 標準形へ変換する。
// (例)
//	 #and(A,#or(B,C)) → #or(#and(A,B),#and(A,C))
//
// ARGUMENTS
// ModVector<ModInvertedQueryNode*>& andChildren
//		変換すべき AndNode の子ノード(OrNodeを取り除いたもの)
// ModInvertedOperatorOrNode* orNode
//		Or 標準形へ変換する OrNode
//
// RETURN
// ORノード
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModInvertedOperatorOrNode*
ModInvertedQuery::changeOrStanderdStyle(
	ModVector<ModInvertedQueryNode*>& andChildren,
	ModInvertedOperatorOrNode* orNode,
	ModInvertedOperatorAndNode* andNode,
	const Query::ValidateMode mode)
{
	; ModAssert(ModInvertedAtomicMask(orNode->getType())
						== ModInvertedQueryNode::operatorOrNode);

	// 新らしい OrNode を作成
	ModInvertedOperatorOrNode* newOrNode = 0;

	try {
		newOrNode = new ModInvertedOperatorOrNode(root->getRankingResult()->getType());

		if ((mode & rankingMode) != 0) {
			newOrNode->setScoreCombiner(
						(orNode->getScoreCombiner())->duplicate());
		}

		ModVector<ModInvertedQueryNode*>* orChildren;
		orChildren = orNode->getChildren();

		ModVector<ModInvertedQueryNode*>::Iterator
							orChild(orChildren->begin());
		for (; orChild != orChildren->end(); ++orChild) {

			// 新らしい AndNode を生成
			ModInvertedOperatorAndNode* newAndNode = new ModInvertedOperatorAndNode(root->getRankingResult()->getType());

			if ((mode & rankingMode) != 0) {
				newAndNode->setScoreCombiner(
						(andNode->getScoreCombiner())->duplicate());
			}

			// 新らしい AndNode を新らしい OrNode の子ノードにする
			newOrNode->insertChild(newAndNode);

			// 新らしい AndNode へ子ノードを insert
			ModVector<ModInvertedQueryNode*>::Iterator andChild;
			andChild = andChildren.begin();
			for (;andChild != andChildren.end(); ++andChild) {
				newAndNode->insertChild(*andChild);
			}

			// 元の OrNode の子ノードを新らしい AndNode へ追加
			newAndNode->insertChild(*orChild);
		}

#ifdef SYD_INVERTED
	} catch (...) {
#else
	} catch (ModException& exception) {
#endif
		ModVector<ModInvertedQueryNode*>::Iterator
									p((newOrNode->getChildren())->begin());
		// ここで生成したandを削除
		for (; p != (newOrNode->getChildren())->end(); ++p) {
			delete *p;
		}

		// ここで生成したorを削除
		delete newOrNode;

#ifdef SYD_INVERTED
		_SYDNEY_RETHROW;
#else
		ModRethrow(exception);

	} catch (...) {
		/* purecov:begin deadcode */
		ModUnexpectedThrow(ModModuleInvertedFile);
		/* purecov:end */
#endif
	}

	return newOrNode;
}

//
// FUNCTION private
// ModInvertedQuery::toSimpleWindowNode -- SimpleWindowNodeへ変換
//
// NOTES
// OperatorWindowNodeをSimpleWindowNodeへ変換する。変換の条件は、
// orderedとunorderedの場合では違う。
//
// orderedのOperatorWindowNodeではchildrenが全てTermLeafNodeであった場
// 合SimpleWindowNodeへ変換する。先頭のTermLeafNodeのgetTermLength()を
// 実行し検索語の長さを求めSimpleWindowNode用のmin, maxを計算する。
//
// unorderedのOperatorWindowNodeの場合はchildrenが全てTermLeafNodeであ
// り、それぞれの検索語の長さが同一であった場合SimpleWindowNodeへ変換
// する。min, maxを計算はorderedと同じ。
//
// またOperatorWindowNodeのchildren の数が一つであった場合は、その
// childrenを外に出してOperatorWindowNodeを破棄する。（単純化する）
//
// toSimpleWindowNode() は再帰的に呼び出すので、全ノードに対して処理さ
// れる
//
// ARGUMENTS
// QueryNode*& node
//		OperatorWindowNode から SimpleWindowNode へ変換するノードのルート
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedQuery::toSimpleWindowNode(QueryNode*& node)
{
	QueryNode::NodeType type = ModInvertedAtomicMask(node->getType());

	if (type == ModInvertedQueryNode::unorderedOperatorWindowNode ||
		type == ModInvertedQueryNode::orderedOperatorWindowNode) {

#ifdef CC_SUN4_2
		ModInvertedOperatorWindowNode* wNode =
			static_cast<ModInvertedOperatorWindowNode*&>(
				static_cast<ModInvertedQueryInternalNode*&>(node));
#else
		ModInvertedOperatorWindowNode* wNode =
			static_cast<ModInvertedOperatorWindowNode*>(
				static_cast<ModInvertedQueryInternalNode*>(node));
#endif
		//
		// OperatorWindowNode の場合
		//
		if (wNode->getChildrenSize() == 1) {
			// 子ノードが1つしかないので単純化する
			// 子ノードを昇格させる
			// node = wNode->children[0];
			node = *((wNode->getChildren())->begin());
			// OperatorWindowNode破棄
			delete wNode;
			// 再帰コール
			toSimpleWindowNode(node);

			return;
		}

		ModBoolean allTermNode = ModTrue; // Trueに初期化
		ModSize termLength = 0;		 // 最初に0クリア

		ModVector<ModInvertedQueryNode*>* children = wNode->getChildren();

		ModVector<ModInvertedQueryNode*>::Iterator p = children->begin();
		for (;p != children->end(); ++p) {
			QueryNode::NodeType childtype(
					ModInvertedAtomicMask((*p)->getType()));

			if (childtype != ModInvertedQueryNode::termLeafNode) {
				// 子ノードにTermLeafNodeでないものがあれば
				// SimpleWindowには変換できない
				allTermNode = ModFalse; // FlagをFalseに変更
				toSimpleWindowNode(*p); // 再帰コール
			} else {
				// 子ノードが TermLeafNode である。
				// 語長を調べる
				ModSize tmpLength;

				if(allTermNode == ModTrue) {
					// allTermNodeがすでにFalseになっている場合は
					// チェックする必要はない
					(*p)->getTermLength(tmpLength);

					if (tmpLength == 0
						|| tmpLength == (ModSize)-1L) {
						// shortWord または検索語の長さが異なるので、
						// SimpleWindowには変換できない。
						allTermNode = ModFalse;

					} else if (termLength != 0
								&& tmpLength != termLength && type ==
									ModInvertedQueryNode::unorderedOperatorWindowNode) {
						// unorderedの場合は全ての子ノードの検索語の長さが
						// 一定でなくてはいけない。検索語の長さが一定では
						// なかったのでSimpleWindowには変換できない。Flag
						// をFalseに変更
						allTermNode = ModFalse;
					}

					if (termLength == 0) {
						// 一つ目の子ノードの場合
						termLength = tmpLength;
					}
				}
			}
		}
		if (allTermNode == ModTrue) {

			// オリジナルのノードの子ノードリストを取得
			ModVector<ModInvertedQueryNode*>* windowChildren
							= wNode->getChildren();
			ModVector<ModInvertedQueryNode*>::Iterator p
						= windowChildren->begin();

			// 全ての子ノードがTermLeafNodeであったので
			// OperatorWindowNode から SimpleWindowNodeへ変換できる
			ModInvertedQueryNode* termNode
				= static_cast<TermLeafNode*>(*p);

#if defined(DEBUG) || defined(SYD_COVERAGE)
			if (termNode == 0) {
				ModAssert(0);
			}
#endif // DEBUG
			termNode->getTermLength(termLength);
			ModSize min = wNode->getMinimalDistance() + termLength - 1;
			ModSize max = wNode->getMaximalDistance() + termLength - 1;
			if(max > ModInt32Max) {
				// maxの値がINT_MAXを超えてしまったので、変換中止
				return;
			}

			// simpleWindowNodeを生成
			ModInvertedSimpleWindowNode* simpleNode
				= this->createSimpleWindowNode(min, max, wNode);

			// childrenをセットする
			for (p = windowChildren->begin(); p != windowChildren->end();
					p++) {
				simpleNode->insertChild(*p);
			}

			// OperatorWindowNodeを破棄する
			delete node;
			// node を書き換える
			node = static_cast<ModInvertedQueryInternalNode*>(simpleNode);

			// simpleWindowNodeへの変換終了
		}
	} else {
		ModVector<ModInvertedQueryNode*>* children = node->getChildren();
		ModVector<ModInvertedQueryNode*>::Iterator p;

		if (children != 0) {
			// OperatorWindowNode以外の internalNode の場合
			// 再帰的に呼び出す
			for (p = children->begin(); p != children->end(); ++p) {
				toSimpleWindowNode(*p);
			}
		}
	}
}

//
// FUNCTION private
// ModInvertedQuery::createSimpleWindowNode -- SimpleWindowNodeの生成
//
// NOTES
// ModInvertedSimpleWindowNode を生成し、そのアドレスを返す。
// toSimpleWindowNode 関数の下請け。
//
// ARGUMENTS
// const ModSize min
//		SimpleWindowNodeの最小間隔
// const ModSize max
//		SimpleWindowNodeの最大間隔
//
// RETURN
// 生成した SimpleWindowNode のアドレス
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModInvertedSimpleWindowNode*
ModInvertedQuery::createSimpleWindowNode(const ModSize min,
										 const ModSize max,
										 OperatorWindowNode* node) const
{
	ModInvertedSimpleWindowNode* newNode
		= new ModInvertedSimpleWindowNode(min, max, totalDocumentFrequency,
				root->getRankingResult()->getType());

	if (node->getScoreCalculator() != 0) {
		// この関数が呼ばれるときには有効化が終わっているので、
		// スコア計算器が必要なランキング検索では、必ずスコア計算器が
		// 用意されている。それを Simple に移す

		newNode->setScoreCalculator(
				(node->getScoreCalculator())->duplicate());

	}

	if (node->isOrderedType() == ModTrue) {
		newNode->setOrderedType();
	} else {
		newNode->setUnorderedType();
	}

	return newNode;
}


//
// FUNCTION
// ModInvertedQuery::doSecondStepInRetrieveScore -- ランキング検索の第２ステップ
//
// NOTES
// ランキング検索で、スコア計算の第２ステップのみを実施する。
// 第１ステップの結果を用い、最終的な検索結果を生成する。
// 第１ステップを実施していない場合の結果は不定。
//
// ARGUMENTS
// ModInvertedRankingResult* result_
//			検索結果
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedQuery::doSecondStepInRetrieveScore(ModInvertedSearchResult*& result_)
{
	root->doSecondStepInRetrieveScore(result_);
}

//
// FUNCTION
// ModInvertedQuery::getTermNodes -- 検索語ノードの取得
//
// NOTES
// ランキング検索の第１ステップを実施した後で、検索語（検索文字列）に対応する
// クエリノードを取得する。
// 各クエリノードに対応する検索文字列とのマップとして結果を得る。
// ※まだ単一検索文字列からなるクエリにしか対応していない。
//
// ARGUMENTS
// QueryNodeMap& nodes
//			検索文字列とクエリノードのマップ
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedQuery::getTermNodes(QueryNodeMap& nodes)
{
	nodes = termNodeMap;
}

//
// FUNCTION
// ModInvertedQuery::insertTermNode -- 検索語ノードの登録
//
// NOTES
// 有効化において、検索文字列に対応するクエリノードをマップに登録する。
//
// ARGUMENTS
// const ModUnicodeString& termString
//			検索文字列
// ModInvertedQueryNode* node
//			クエリノード
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedQuery::insertTermNode(const ModUnicodeString& termString,
ModInvertedQueryNode* node)
{
	// [YET] simpleTokenLeafNodeとtermNodeの違いは？
	//  名前から simpleTokenLeafNodeMap ⊆ termNodeMap とは思うが。
	
	QueryNodeMap::Iterator p(termNodeMap.find(termString));
	if (p == termNodeMap.end()) {
		// マップにないので、追加する
#if defined(DEBUG) || defined(SYD_COVERAGE)
		ModDebugMessage << "new term inserted: " << termString << ModEndl;
#endif
		termNodeMap[termString] = node;
	} else {
		// すでにマップにあったので何もしない
		ModDebugMessage << "already exist: " << termString << ModEndl;
	}
}

//
// ModInvertedQuery::getDocumentFrequency -- 文書頻度の取得
//
// NOTES
// 文書頻度を得る。
// ただし、削除済み文書IDを除き、最大文書IDを超える文書は除くものとする。
//
// ARGUMENTS
// const ModBoolean estimate_
//		近似値指定
// const ModInvertedBooleanResult* expungedIds_
//		削除済み文書ID
// const ModInvertedDocumentID maxid_
//		最大文書ID
//
// RETURN
// 登録文書数
//
// EXCEPTIONS
// なし
//
ModSize
ModInvertedQuery::getDocumentFrequency(
	const ModBoolean estimate_,
	const ModInvertedBooleanResult* expungedIds_,
	const ModInvertedDocumentID maxid_
	)
{
	if (root == ModInvertedQueryNode::emptySetNode) {
		// 空のノードであれば、文書頻度は常に０
		return 0;
	}

	if (estimate_ == ModTrue) {
		//
		// 近似値指定の場合
		//

		// 削除文書などに関わりなく近似値を返す
		return ModMin(root->estimateDocumentFrequency(),
						totalDocumentFrequency);
	}

	if (expungedIds_ == 0) {
		//
		// 削除文書がない場合
		//
		
		if (maxid_ == ModInvertedUpperBoundDocumentID) {
			// 索引ファイルが小転置の場合
			return root->getDocumentFrequency(
				ModInvertedFileCapsuleBooleanSearch::defaultEvaluateMode);
		}
		
		// 索引ファイルが大転置の場合
		if (root->getType() == ModInvertedQueryNode::simpleTokenLeafNode) {
			// トークンノードの場合
			
			// 転置リストから情報を取る
			ModInvertedList* list =
				static_cast<ModInvertedSimpleTokenLeafNode*>(root)->getInvertedList();
			if (list->getLastDocumentID() <= maxid_) {
				// 最終文書IDが上限以下の場合
				
				// 転置リストの文書頻度をそのまま使える
				return list->getDocumentFrequency();
			}
		}
	}

	//
	// DFを取得
	//
	
	// 削除文書がある場合か、
	// 索引ファイルが大転置で、転置リストの文書頻度をそのまま使えない場合

	// さぼれない場合
	ModInvertedBooleanResult tmpResult;
#if 1
	root->retrieve(tmpResult,
					 ModInvertedFileCapsuleBooleanSearch::defaultEvaluateMode);
#else
//	root->retrieve(expungedIds_,maxid_,&tmpResult);
#endif
	if (maxid_ != ModInvertedUpperBoundDocumentID) {
		
		// 大転置の場合

		// 余分な実行結果をDF取得対象から外す
		
		ModInvertedBooleanResult::Iterator
			newEnd(ModLowerBound(tmpResult.begin(), tmpResult.end(), maxid_));
		if (newEnd != tmpResult.end()) {
			
			// 余分な実行結果が存在する場合

			// maxid_ 以降を対象から外す
			if (maxid_ == *newEnd) {
				++newEnd;
			}
			tmpResult.erase(newEnd, tmpResult.end());
		}
	}

	//
	// 削除文書の処理
	//
	
#if 1	// すでにdoFirstStepInRetreiveScore()で削除文書は、削除済み
	if (expungedIds_ != 0) {
#if 0
		tmpResult -= *expungedIds_;
#endif
		ModInvertedSearchResult *tmp = (ModInvertedSearchResult *)&tmpResult;
		tmp->setDifference((ModInvertedSearchResult*)expungedIds_);
	}
#endif

	return tmpResult.getSize();
}

//
// Copyright (c) 1997, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
