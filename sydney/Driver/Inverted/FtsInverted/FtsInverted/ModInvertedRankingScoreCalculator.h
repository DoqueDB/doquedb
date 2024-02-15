// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedRankingScoreCalculator.h -- ランキングスコア計算器の宣言
// 
// Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2004, 2005, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef __ModInvertedRankingScoreCalculator_h_
#define __ModInvertedRankingScoreCalculator_h_

#include "ModInvertedTypes.h"
#include "ModString.h"
#include "ModVector.h"

class ModUnicodeString;

class ModInvertedDocumentLengthFile;

class ModInvertedQueryNode;
//
// CLASS
// ModInvertedRankingScoreCalculator -- スコア計算器
//
// NOTES
// ランキングのために、スコアを算出する。
// スコア算出のために、全文書数を保持する。文書長ファイルへのポインタを
// 保持することもでき、その場合は平均文書長や各文書の文書長を文書長ファイル
// から得られる。
// 抽象クラスであり、実際の処理は派生クラスで定義する。
//
class
ModInvertedRankingScoreCalculator {
public:
	// クラス定義内の行を短くするための typedef
	typedef ModInvertedRankingScoreCalculator	ScoreCalculator;
	typedef ModInvertedDocumentLengthFile		LengthFile;
	typedef ModInvertedDocumentScore			DocumentScore;
	typedef ModInvertedDocumentID				DocumentID;

	// スコア計算器の生成
	static ScoreCalculator* create();
	static ScoreCalculator* create(const ModString&);
	static ScoreCalculator* create(const ModUnicodeString&);

	// コンストラクタ
	ModInvertedRankingScoreCalculator();
	ModInvertedRankingScoreCalculator(LengthFile*);
	ModInvertedRankingScoreCalculator(const ScoreCalculator&);

	// デストラクタ
	virtual ~ModInvertedRankingScoreCalculator();

	// 自分自身の複製
	virtual ScoreCalculator* duplicate() const = 0;

	// 重みの計算の準備
	virtual void prepare(const ModSize totalDocumentFrequency,
						 const ModSize documentFrequency
							);
	virtual void prepareEx(	const ModUInt64 totalTermFrequency_,
							const ModUInt64 totalDocumentLength_,
							const ModUInt32 queryTermFrequency_
							){}
	

	DocumentScore getPrepareResult() const;

	// 重みの計算
	//
	//	ModBoolean& existの引数をSydneyのバグに対応するため追加した
	//	Sydneyのマージ処理にバグがあり、本来削除されるべき文書IDが
	//	削除されずにごみとして残ってしまうことがあり、
	//	そのような場合に、exist = ModFalse を設定できるように、
	//	この引数を加えた
	//
	virtual DocumentScore operator()(ModSize inDocumentFrequency,
									 const DocumentID ID,
									 ModBoolean& exist) const;

	virtual DocumentScore firstStep(const ModSize totalDocumentFrequency,
									const ModSize documentFrequency,
									ModBoolean& exist) const = 0;
	virtual DocumentScore secondStep(const ModSize df,
									 const ModSize totalDocument) const = 0;



	//
	// 全文書数を与える
	void setTotalDocumentFrequency(const ModSize);

	// 全文書数を得る
	virtual ModSize getTotalDocumentFrequency() const;

	// 文書長ファイルを与える
	virtual void setDocumentLengthFile(LengthFile* file);

	// 平均文書長ファイルを与える
	virtual void setAverageDocumentLength(const ModSize averageDocumentLength);

	// パラメータセット。各calculatorでオーバーライド
	virtual void setParameter(const ModString&) = 0;

#ifdef DEBUG0
	// デバック用パラメータ表示関数
	virtual void showCalculatorParameters() = 0;
#endif
#ifdef DEBUG
	static int countFirstStep;
	static int countSecondStep;
#endif

	void setQueryNode(ModInvertedQueryNode *queryNode_){ queryNode = queryNode_;}
	ModInvertedQueryNode *getQueryNode() const{ return queryNode ;}

	virtual bool isExtendedFirstStep(){ return false;}
	virtual	DocumentScore firstStepEx(	const ModSize i,				// tf index
										const DocumentID ID) const{return 0.0;}

	// 記述文字列の取得
	virtual void getDescription(ModString&,
								const ModBoolean = ModTrue) const = 0;
	void getDescription(ModUnicodeString&, const ModBoolean = ModTrue) const;

	virtual ModUInt64 getTotalTermFrequency(){ return 0;}

	// 文書長を得る
	virtual ModBoolean searchDocumentLength(ModUInt32 docId, ModSize& length);
	
protected:

	static void parse(ModString& description, 
					  ModString& calculatorName,
					  ModString& parameters);


	ModSize divideParameter(const ModString&, ModVector<ModString>&);

	virtual void precalculate() {}			// TF項の前計算
	ModUInt32 queryTermFrequency;			// 検索語の検索文内のTF(queryTermFrequency)
	ModUInt64 totalTermFrequency;			// 総TF for SRCB
	ModUInt64 totalDocumentLength;			// 全文書長（単語索引以外）または全単語数（単語索引）for SRCB
	ModSize totalDocumentFrequency;			// 全文書数
	LengthFile* documentLengthFile;			// 文書長ファイル
	ModSize averageDocumentLength;			// 平均文書数
	DocumentScore prepareResult;			// prepare() 結果の保存
private:
	ModInvertedQueryNode* queryNode;	
};


// 
// FUNCTION public
// ModInvertedRankingScoreCalculator::~ModInvertedRankingScoreCalculator -- デストラクタ
// 
// NOTES
// スコア計算機を廃棄する。
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
inline
ModInvertedRankingScoreCalculator::~ModInvertedRankingScoreCalculator()
{}

// 
// FUNCTION public
// ModInvertedRankingScoreCalculator::setTotalDocumentFrequency -- 全文書数を与える。
// 
// NOTES
// 与えられた全文書数を記憶する。
// 
// ARGUMENTS
// const ModSize val
//	全文書数。
// 
// RETURN
// なし。
// 
// EXCEPTIONS
// なし
//
inline void
ModInvertedRankingScoreCalculator::setTotalDocumentFrequency(const ModSize val)
{
	totalDocumentFrequency = val;
}

// 
// FUNCTION public
// ModInvertedRankingScoreCalculator::setAverageDocumentLength -- 平均文書長の設定
// 
// NOTES
// 与えられた平均文書長を記憶する。
// 
// ARGUMENTS
// const ModSize averageDocumentLength_
//	平均文書長ファイル
// 
// RETURN
// なし
// 
// EXCEPTIONS
// なし
//
inline void
ModInvertedRankingScoreCalculator::setAverageDocumentLength(
	const ModSize averageDocumentLength_)
{
	if (averageDocumentLength_ != 0) {
		averageDocumentLength = averageDocumentLength_;
		precalculate();
	}
}

// 
// FUNCTION public
// ModInvertedRankingScoreCalculator::getPrepareResult -- 計算済みの結果の取得
// 
// NOTES
// IDF に関わる計算済みの値を取得する。
// 
// ARGUMENTS
// なし
// 
// RETURN
// IDF に関わる計算済みの値
// 
// EXCEPTIONS
// なし
//
inline ModInvertedDocumentScore 
ModInvertedRankingScoreCalculator::getPrepareResult() const
{
	return prepareResult;
}

// 
// FUNCTION public
// ModInvertedRankingScoreCalculator::operator() -- スコアの計算
// 
// NOTES
// 引数から firstStep() を計算し、計算済みの secondStep() の値と乗算する。
//
// ARGUMENTS
// ModSize tf
//		文書内出現頻度
// DocumentID ID 
// 		スコアを計算する文書のID
// 
// RETURN
// スコア
//
// EXCEPTIONS
// なし
//
inline ModInvertedDocumentScore
ModInvertedRankingScoreCalculator::operator()(ModSize tf,
											  const DocumentID docId,
											  ModBoolean& exist) const
{
	return this->firstStep(tf, docId, exist) * this->prepareResult;
}

#endif // __ModInvertedRankingScoreCalculator_h_

//
// Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2004, 2005, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
