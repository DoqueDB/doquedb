// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedQuey.h -- 検索式内部表現I/Fオブジェクトインタフェイス
// 
// Copyright (c) 1997, 1999, 2000, 2001, 2002, 2004, 2005, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __ModInvertedQuery_H__
#define __ModInvertedQuery_H__

#include "ModLanguageSet.h"
#include "ModMap.h"
#include "ModTypes.h"
#include "ModUnicodeString.h"
#include "ModInvertedTypes.h"
#include "ModInvertedManager.h"
#include "ModVector.h"
#include "ModInvertedSearchResult.h"
class ModOstrStream;

class ModInvertedFile;
class ModInvertedList;

class ModInvertedQueryNode;
class ModInvertedQueryInternalNode;
class ModInvertedTermLeafNode;
class ModInvertedSimpleTokenLeafNode;
class ModInvertedOperatorOrNode;
class ModInvertedOperatorAndNode;
class ModInvertedOperatorWindowNode;
class ModInvertedLocationListIterator;
class ModInvertedSimpleWindowNode;
class ModInvertedBooleanResultLeafNode;
class ModInvertedRankingScoreCombiner;
class ModInvertedRankingScoreCalculator;
class ModInvertedRankingScoreNegator;

class ModInvertedRankingScoreCombiner;
class ModInvertedRankingScoreCalculator;
class ModInvertedRankingScoreNegator;

//
// CLASS
// ModInvertedQuery -- 検索インタフェイスオブジェクト
//
// NOTES
// 検索式を評価して、検索を実行する。
// 検索式は、合成演算を表すノードで子ノードを結び付けた木構造で内部表現されて
// いる。その木構造の最上位(根本)ノードへのポインタを持ち、実際の処理はその
// 最上位ノードに任せる。
//
class
ModInvertedQuery : public ModInvertedObject {

public:
	typedef ModInvertedFile					InvertedFile;
	typedef ModInvertedQuery				Query;
	typedef ModInvertedQueryNode			QueryNode;
	typedef ModInvertedDocumentID			DocumentID;
	typedef ModInvertedTermLeafNode			TermLeafNode;
	typedef ModInvertedQueryInternalNode	InternalNode;
	typedef	ModInvertedSearchResult		    RankingResultLeafNode;
	typedef ModInvertedOperatorWindowNode	OperatorWindowNode;
	typedef ModInvertedLocationListIterator LocationIterator;
	typedef	ModInvertedBooleanResultLeafNode	BooleanResultLeafNode;
	typedef ModInvertedRankingScoreNegator	ScoreNegator;
	typedef ModInvertedRankingScoreCombiner	ScoreCombiner;
	typedef ModInvertedRankingScoreCalculator	ScoreCalculator;
	typedef ModMap<ModUnicodeString, QueryNode*, ModLess<ModUnicodeString> > QueryNodeMap;
	typedef ModMap<QueryNode*, int, ModLess<QueryNode*> >	QueryNodePointerMap;

	typedef unsigned 	EvaluateMode;	// 検索処理モード
	typedef unsigned	ValidateMode;	// 有効化（最適化）のモード

	// For getSearchTermList()
	struct SearchTerm
	{
		ModUnicodeString m_cstrSearchTerm;
		ModLanguageSet m_cLanguageSet;
		ModInvertedTermMatchMode m_eMatchMode;
		ModSize m_uiSynonymID;
	};
	typedef ModVector<SearchTerm> SearchTermList;

	// ENUM
	// ValidateModeFlag -- 有効化モードフラグ
	//
	// NOTES
	// クエリの有効化モードを指示するためのフラグ。
	//
	enum ValidateModeFlag {
		flattenChildren			= 0x0001,	// 子ノードの平坦化
		sortChildren			= 0x0002,	// 子ノードのソート
		orStanderdStyle			= 0x0004,	// Or標準形へ変換
		makeRough				= 0x0008,	// rough 用ポインタの作成
											// なお、SimpleToken も rough 用
											// ポインタに加える
		sharedNode				= 0x0010,	// 中間ノードの共有化
		toSimpleWindow			= 0x0020,	// OperatorWindowをSimpleWindow
											// へ変換
		// TermLeafNodeのRoughPointerの内容を決定するビット
		tokenMapToRough			= 0x0100,	//トークン分割で得られたtokenのうち
											//前後のtokenと出現頻度を比較して、
											//頻度の低い場合だけrough pointerに
											//セットする。
		bestPathToRough			= 0x0200,	//TermLeafNode::getBestPath()の結果
											//をrough pointerへセットする。ただ
											//しtokenMapToRoughのbitがoffでない
											//とこのモードは実行されない。

											//tokenMapToRough, bestPathToRough 
											//とも off の場合はトークン分割で得
											//られた全てのtokenをrough pointer
											//にセットする。
		tokenizeQuery			= 0x0400,	// トークン分割時queryを使う

		// 異表記正規化関係
		skipNormalizing			= 0x1000,	// 正規化処理をスキップする
		skipExpansion			= 0x2000,	// 異表記展開のみスキップする

		// ランキング検索関係
		rankingMode				= 0x4000,	// ランキング検索を行う

		// 表示関係	名称変更 messageOutput -> printQuery
		printQuery				= 0x10000000,	// メッセージを出力する
		// 名称変更 timeOutput -> printSearchTime
		printSearchTime			= 0x20000000	// 検索時間を出力する
	};

	// ENUM
	// EvaluateModeFlag -- 評価モードフラグ
	//
	// NOTES
	// 検索処理における評価モードを指示するフラグ。
	//
	// 評価の順序の説明。
	// ・term-at-time
	//	 子ノードごとに処理。
	//	 子ノードを固定し、文書を動かして検索条件評価を行ない、その
	//	 子ノードの評価が済んでから、次の子ノードに進む。
	//	 #or およびその派生クラスで採用される方式。
	// ・document-at-a-time
	//	 文書ごとに処理。
	//	 文書を固定し、子ノードを動かして検索条件評価を行ない、その
	//	 文書の評価が済んでから、次の文書に進む。
	//	 #and およびその派生クラスで採用される方式。
	//
	enum EvaluateModeFlag {
		firstStepOnlyInRanking      = 0x10000,
		secondStepOnlyInRanking     = 0x20000,
		andTermAtATimeFlag			= 0x0001,	// #and を term-at-a-time で
		orDocumentAtATimeFlag		= 0x0002,	// #or を document-at-a-time で
		roughEvaluationFlag			= 0x0010,	// 位置条件を無視した粗い評価
		retrieveTF					= 0x0100, 	// retrieveスコア計算でtfを
												// 先に計算
		calAtomicOrTfByAddChildTf	= 0x1000, 	// ショートワード用AtomicOrの
												// Tfを子ノードのTf和とする
		getDFbyMinEvaluationMode	= 0x2000,
		getDFbyRoughEvaluationMode	= 0x4000,
		getTFbyMinEvaluationMode	= 0x8000
	};

	// コンストラクタ
	ModInvertedQuery();
	ModInvertedQuery(const ModInvertedQuery&);

	// デストラクタ
	virtual ~ModInvertedQuery();

	// ルートノード(検索式を表す木構造の最上位ノード)へのポインタのセット
	void setRoot(QueryNode* root_);
	QueryNode* getRoot() const { return this->root; }

#ifndef DEL_BOOL
	virtual void retrieveBoolean( ModInvertedBooleanResult *expungedDocumentId_,
					 const EvaluateMode mode);
	void getBooleanResult(ModInvertedBooleanResult *&booleanResult_ ){booleanResult_ = booleanResult;}
#endif
	// 削除文書を考慮した検索
	virtual void retrieve(
			ModInvertedBooleanResult *expungedDocumentId_,
			const ModInvertedDocumentID maxDocumentId_ = ModInvertedUpperBoundDocumentID
			);


	ModBoolean evaluate(const DocumentID, LocationIterator*&,
						const EvaluateMode);


	//
	// 有効化 (検索可能なようにノードを変更する) と最適化
	void validate(InvertedFile* invertedFile, const ValidateMode mode,
					  const ModSize averageDocumentLength = 0);

	// デフォルトのスコア合成器
	void setDefaultAndScoreCombiner(const ModCharString& combiner);
	void setDefaultAndNotScoreCombiner(const ModCharString& combiner);
	void setDefaultOrScoreCombiner(const ModCharString& combiner);

	ScoreCombiner* getDefaultAndScoreCombiner() const;
	ScoreCombiner* getDefaultAndNotScoreCombiner() const;
	ScoreCombiner* getDefaultOrScoreCombiner() const;

	// デフォルトのスコア否定器
	void setDefaultScoreNegator(const ModCharString& negator);
	ScoreNegator* getDefaultScoreNegator() const;

	// デフォルトのスコア計算器
	void setDefaultScoreCalculator(const ModCharString& calculator);
	ScoreCalculator* getDefaultScoreCalculator() const;

	ModSize getTotalDocumentFrequency() const;

	ModSize getDocumentFrequency(const ModBoolean,
								 const ModInvertedBooleanResult* = 0,
								 const ModInvertedDocumentID
								 = ModInvertedUpperBoundDocumentID);

	// Node表示関数
	void getDescription(ModUnicodeString& out,
						const ModBoolean = ModTrue, const ModBoolean = ModTrue,
						const ModBoolean = ModTrue, const ModBoolean = ModFalse);

	// OrStanderdSharedNode変数に登録する
	void addOrStanderdSharedNode(ModInvertedQueryNode* node);

	static ModSize orStandardThreshold;
	static ModSize orFlattenThreshold;

	void doSecondStepInRetrieveScore(ModInvertedSearchResult* &result);
	void getTermNodes(QueryNodeMap& nodes);
	void insertTermNode(const ModUnicodeString&, ModInvertedQueryNode*);
	ModSize getAverageDocumentLength() const { return averageDocumentLength; }
    void doFirstStepInRetrieveScore(
		ModInvertedBooleanResult *expungedDocumentId,
		const ModInvertedDocumentID maxDocumentId_ = ModInvertedUpperBoundDocumentID
	   );

	// short word が存在している
	void setShortWord() { bShortWord = true; }
	// short word が存在しているか
	bool isShortWord() const { return bShortWord; }

	// 位置情報を走査してTFを求める場合の、その上限
	static ModSize getTFCountUpperLimit();

	// 検索条件を取得(KWIC用)
	void getSearchTermList(SearchTermList& vecSearchTerm_) const;

protected:
	friend class ModInvertedTermLeafNode;
#ifdef V1_4	// 単語単位検索
	friend class ModInvertedOperatorWordNode;
#endif // V1_4 単語単位検索
	friend class ModInvertedQueryInternalNode;

	ModBoolean getSimpleTokenNode(const ModUnicodeString& invertedListKey,
								  const ModSize keyLength,
								  const ModUnicodeString& mapKey,
								  ModInvertedList* invertedList,
								  ModInvertedQueryNode*& node,
								  const ValidateMode mode);
	                              

	// TermLeafNode を削除して OrderedDistance/SimpleTokenLeafにする
	void eraseTermLeafNodeQuery(ModInvertedQueryNode*& nodePlace);

	//  ルートノードへのポインタ
	ModInvertedQueryNode* root;

	// 検索式ノードのマップ
	QueryNodeMap		queryNodeMap;

	// 単純索引語ノードのマップ
	QueryNodeMap		simpleTokenLeafNodeMap;

	QueryNodeMap        termNodeMap;
	ModSize             averageDocumentLength;

	// デフォルトのスコア合成器
	ScoreCombiner* defaultAndScoreCombiner;
	ScoreCombiner* defaultAndNotScoreCombiner;
	ScoreCombiner* defaultOrScoreCombiner;

	// デフォルトスコア否定器
	ScoreNegator* defaultScoreNegator;

	// デフォルトスコア計算器
	ScoreCalculator* defaultScoreCalculator;

	// データーベースに登録されている全文書数
	ModSize totalDocumentFrequency;


private:
	// validate() の補助関数群
	void optimize(const ValidateMode mode);

	void duplicateQueryNode(QueryNode* node,
							QueryNode*& insertPlace) const;



	// Or標準形へ変換必要があるか検査し、変換可能なら変換する
	void convertToOrStanderdStyle(QueryNode*& node, 
							const Query::ValidateMode mode);

	// Or標準形へ変換
	//
	ModInvertedOperatorOrNode* changeOrStanderdStyle(
		ModVector<ModInvertedQueryNode*>& andChildren,
		ModInvertedOperatorOrNode* orNode,
		ModInvertedOperatorAndNode* andNode,
		const Query::ValidateMode mode);

	// 中間ノードの単純化（ノードの共有化の補助関数）
	void changeSimpleTypeNode(ModInvertedQueryNode*& node);

	// SimpleWindowNodeへ変換
	void toSimpleWindowNode(QueryNode*& node);
	virtual ModInvertedSimpleWindowNode* createSimpleWindowNode(
		const ModSize min, const ModSize max, OperatorWindowNode* node) const;

	void validateSimpleTokenLeafNode(ModInvertedQueryNode*& node_,
									 ModInvertedFile* invertedFile_,
									 const ValidateMode mode_);

	// OR標準形への変換時に共有するノードのポインタを登録するマップ
	QueryNodePointerMap		orStanderdSharedNode;

	ModInvertedBooleanResult *booleanResult;
	//
	static const char orStandardThresholdParameterKey[];
	static const char orFlattenThresholdParameterKey[];

	// short word が存在しているかどうか
	bool bShortWord;
};


// 
// FUNCTION public
// ModInvertedQuery::setRoot -- ルートノードへのポインタのセット
// 
// NOTES
// 検索式内部表現の実体であるルートノードへのポインタをセットする。
//
// ARGUMENTS
// ModInvertedQueryNode* root
//	ルートノードへのポインタ
// 
// RETURN
// なし
// 
// EXCEPTIONS
// なし
//
inline void
ModInvertedQuery::setRoot(ModInvertedQueryNode* root_)
{
	this->root = root_;
}

// 
// FUNCTION public
// ModInvertedQuery::getDefaultAndScoreCombiner -- AND用デフォルトスコア合成器の取得
// 
// NOTES
// AND用デフォルトスコア合成器を取得する。
// 
// ARGUMENTS
// なし
// 
// RETURN
// セットされているAnd用スコア合成器
// 
// EXCEPTIONS
// なし
//
inline ModInvertedRankingScoreCombiner*
ModInvertedQuery::getDefaultAndScoreCombiner() const
{
	return defaultAndScoreCombiner;
}

// 
// FUNCTION public
// ModInvertedQuery::getDefaultAndNotScoreCombiner -- ANDNOT用デフォルトスコア合成器の取得
// 
// NOTES
// ANDNOT用デフォルトスコア合成器を取得する。
// 
// ARGUMENTS
// なし
// 
// RETURN
// セットされているAndNot用スコア合成器
// 
// EXCEPTIONS
// なし
//
inline ModInvertedRankingScoreCombiner*
ModInvertedQuery::getDefaultAndNotScoreCombiner() const
{
	return defaultAndNotScoreCombiner;
}

// 
// FUNCTION public
// ModInvertedQuery::getDefaultOrScoreCombiner -- OR用デフォルトスコア合成器の取得
// 
// NOTES
// AND用デフォルトスコア合成器を取得する。
// 
// ARGUMENTS
// なし
// 
// RETURN
// セットされているOr用スコア合成器
// 
// EXCEPTIONS
// なし
//
inline ModInvertedRankingScoreCombiner*
ModInvertedQuery::getDefaultOrScoreCombiner() const
{
	return defaultOrScoreCombiner;
}

// 
// FUNCTION public
// ModInvertedQuery::getDefaultScoreNegator -- デフォルトスコア否定器の取得
// 
// NOTES
// デフォルトスコア否定器を取得する。
// 
// ARGUMENTS
// なし
//	
// RETURN
// スコア否定器へのポインタ
// 
// EXCEPTIONS
// なし
//
inline ModInvertedRankingScoreNegator*
ModInvertedQuery::getDefaultScoreNegator() const
{
	return this->defaultScoreNegator;
}


// 
// FUNCTION public
// ModInvertedQuery::getDefaultScoreCalculator -- デフォルトスコア計算器の取得
// 
// NOTES
// デフォルトスコア計算器を取得する
// 
// ARGUMENTS
// なし
//	
// RETURN
// スコア計算器へのポインタ
// 
// EXCEPTIONS
// なし
//
inline ModInvertedRankingScoreCalculator*
ModInvertedQuery::getDefaultScoreCalculator() const
{
	return this->defaultScoreCalculator;
}

// 
// FUNCTION public
// ModInvertedQuery::getTotalDocumentFrequency -- データーベースに登録されている文書数を得る
// 
// NOTES
// データーベースに登録されている文書数を得る
// 
// ARGUMENTS
// なし
//	
// RETURN
// データーベースに登録されている文書数
// 
// EXCEPTIONS
// なし
//
inline ModSize
ModInvertedQuery::getTotalDocumentFrequency() const
{
	return this->totalDocumentFrequency;
}

#endif //__ModInvertedQuery_H__

//
// Copyright (c) 1997, 1999, 2000, 2001, 2002, 2004, 2005, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//



